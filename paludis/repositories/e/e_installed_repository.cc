/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <paludis/repositories/e/e_installed_repository.hh>
#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/repositories/e/e_repository_params.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/eapi_phase.hh>
#include <paludis/repositories/e/ebuild.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/system.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/action.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/environment.hh>
#include <paludis/set_file.hh>
#include <paludis/hook.hh>
#include <paludis/dep_tag.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Implementation<EInstalledRepository>
    {
        EInstalledRepositoryParams params;
        Mutex world_mutex;

        Implementation(const EInstalledRepositoryParams & p) :
            params(p)
        {
        }
    };
}

EInstalledRepository::EInstalledRepository(const EInstalledRepositoryParams & p,
        const RepositoryName & n, const RepositoryCapabilities & c) :
    Repository(p.environment(), n, c),
    PrivateImplementationPattern<EInstalledRepository>(new Implementation<EInstalledRepository>(p)),
    _imp(PrivateImplementationPattern<EInstalledRepository>::_imp)
{
}

EInstalledRepository::~EInstalledRepository()
{
}

namespace
{
    struct SomeIDsMightSupportVisitor
    {
        bool visit(const SupportsActionTest<UninstallAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<ConfigAction> &) const
        {
           return true;
        }

        bool visit(const SupportsActionTest<InfoAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<PretendAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<FetchAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<PretendFetchAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<InstallAction> &) const
        {
            return false;
        }
    };
}

bool
EInstalledRepository::some_ids_might_support_action(const SupportsActionTestBase & test) const
{
    SomeIDsMightSupportVisitor v;
    return test.accept_returning<bool>(v);
}

bool
EInstalledRepository::is_suitable_destination_for(const PackageID & e) const
{
    std::string f(e.repository()->format_key() ? e.repository()->format_key()->value() : "");
    return f == "ebuild" || f == "exheres" || f == "portage";
}

bool
EInstalledRepository::is_default_destination() const
{
    return _imp->params.environment()->root() == installed_root_key()->value();
}

bool
EInstalledRepository::want_pre_post_phases() const
{
    return true;
}

HookResult
EInstalledRepository::perform_hook(const Hook & hook)
{
    Context context("When performing hook '" + stringify(hook.name()) + "' for repository '"
            + stringify(name()) + "':");

    if (hook.name() == "sync_all_post")
        perform_updates();

    return make_named_values<HookResult>(value_for<n::max_exit_status>(0), value_for<n::output>(""));
}

std::tr1::shared_ptr<const CategoryNamePartSet>
EInstalledRepository::unimportant_category_names() const
{
    std::tr1::shared_ptr<CategoryNamePartSet> result(make_shared_ptr(new CategoryNamePartSet));
    result->insert(CategoryNamePart("virtual"));
    return result;
}

std::string
EInstalledRepository::get_environment_variable(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::string & var) const
{
    Context context("When fetching environment variable '" + var + "' for '" +
            stringify(*id) + "':");

    FSEntry ver_dir(id->fs_location_key()->value());

    if (! ver_dir.is_directory_or_symlink_to_directory())
        throw ActionFailedError("Could not find Exndbam entry for '" + stringify(*id) + "'");

    if ((ver_dir / var).is_regular_file_or_symlink_to_regular_file())
    {
        SafeIFStream f(ver_dir / var);
        return strip_trailing_string(
                std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>()), "\n");
    }
    else if ((ver_dir / "environment.bz2").is_regular_file_or_symlink_to_regular_file())
    {
        std::stringstream p;
        Command cmd(Command("bash -c '( bunzip2 < " + stringify(ver_dir / "environment.bz2" ) +
                    " ; echo echo \\$" + var + " ) | bash -O extglob 2>/dev/null'").with_captured_stdout_stream(&p));
        int exit_status(run_command(cmd));
        std::string result(strip_trailing_string(std::string(
                        (std::istreambuf_iterator<char>(p)),
                        std::istreambuf_iterator<char>()), "\n"));
        if (0 != exit_status)
            throw ActionFailedError("Could not load environment.bz2");
        return result;
    }
    else
        throw ActionFailedError("Could not get variable '" + var + "' for '" + stringify(*id) + "'");
}

void
EInstalledRepository::perform_config(
        const std::tr1::shared_ptr<const ERepositoryID> & id,
        const ConfigAction & a) const
{
    Context context("When configuring '" + stringify(*id) + "':");

    if (! _imp->params.root().is_directory())
        throw ActionFailedError("Couldn't configure '" + stringify(*id) +
                "' because root ('" + stringify(_imp->params.root()) + "') is not a directory");

    std::tr1::shared_ptr<OutputManager> output_manager(a.options.make_output_manager()(a));

    FSEntry ver_dir(id->fs_location_key()->value());

    std::tr1::shared_ptr<FSEntrySequence> eclassdirs(new FSEntrySequence);
    eclassdirs->push_back(ver_dir);

    std::tr1::shared_ptr<FSEntry> load_env(new FSEntry(ver_dir / "environment.bz2"));
    EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_config());

    for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
        EbuildConfigCommand config_cmd(make_named_values<EbuildCommandParams>(
                    value_for<n::builddir>(_imp->params.builddir()),
                    value_for<n::clearenv>(phase->option("clearenv")),
                    value_for<n::commands>(join(phase->begin_commands(), phase->end_commands(), " ")),
                    value_for<n::distdir>(ver_dir),
                    value_for<n::ebuild_dir>(ver_dir),
                    value_for<n::ebuild_file>(ver_dir / (stringify(id->name().package()) + "-" + stringify(id->version()) + ".ebuild")),
                    value_for<n::eclassdirs>(eclassdirs),
                    value_for<n::environment>(_imp->params.environment()),
                    value_for<n::exlibsdirs>(make_shared_ptr(new FSEntrySequence)),
                    value_for<n::files_dir>(ver_dir),
                    value_for<n::maybe_output_manager>(output_manager),
                    value_for<n::package_builddir>(_imp->params.builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-config")),
                    value_for<n::package_id>(id),
                    value_for<n::portdir>(ver_dir),
                    value_for<n::sandbox>(phase->option("sandbox")),
                    value_for<n::sydbox>(phase->option("sydbox")),
                    value_for<n::userpriv>(phase->option("userpriv"))
                ),

                make_named_values<EbuildConfigCommandParams>(
                    value_for<n::load_environment>(load_env.get()),
                    value_for<n::root>(stringify(_imp->params.root()))
                ));

        config_cmd();
    }
}

void
EInstalledRepository::perform_info(
        const std::tr1::shared_ptr<const ERepositoryID> & id,
        const InfoAction & a) const
{
    Context context("When infoing '" + stringify(*id) + "':");

    if (! _imp->params.root().is_directory())
        throw ActionFailedError("Couldn't info '" + stringify(*id) +
                "' because root ('" + stringify(_imp->params.root()) + "') is not a directory");

    std::tr1::shared_ptr<OutputManager> output_manager(a.options.make_output_manager()(a));

    FSEntry ver_dir(id->fs_location_key()->value());

    std::tr1::shared_ptr<FSEntrySequence> eclassdirs(new FSEntrySequence);
    eclassdirs->push_back(ver_dir);

    std::tr1::shared_ptr<FSEntry> load_env(new FSEntry(ver_dir / "environment.bz2"));

    EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_info());

    for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
        if (phase->option("installed=false"))
            continue;

        /* try to find an info_vars file from the original repo */
        std::tr1::shared_ptr<const Set<std::string> > i;
        if (id->from_repositories_key())
        {
            for (Set<std::string>::ConstIterator o(id->from_repositories_key()->value()->begin()),
                    o_end(id->from_repositories_key()->value()->end()) ;
                    o != o_end ; ++o)
            {
                RepositoryName rn(*o);
                if (_imp->params.environment()->package_database()->has_repository_named(rn))
                {
                    const std::tr1::shared_ptr<const Repository> r(
                            _imp->params.environment()->package_database()->fetch_repository(rn));
                    Repository::MetadataConstIterator m(r->find_metadata("info_vars"));
                    if (r->end_metadata() != m)
                    {
                        const MetadataCollectionKey<Set<std::string> > * const mm(
                                simple_visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**m));
                        if (mm)
                        {
                            i = mm->value();
                            break;
                        }
                    }
                }
            }
        }

        /* try to find an info_vars file from any repo */
        if (! i)
        {
            for (PackageDatabase::RepositoryConstIterator
                    r(_imp->params.environment()->package_database()->begin_repositories()),
                    r_end(_imp->params.environment()->package_database()->end_repositories()) ;
                    r != r_end ; ++r)
            {
                Repository::MetadataConstIterator m((*r)->find_metadata("info_vars"));
                if ((*r)->end_metadata() != m)
                {
                    const MetadataCollectionKey<Set<std::string> > * const mm(
                            simple_visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**m));
                    if (mm)
                    {
                        i = mm->value();
                        break;
                    }
                }
            }
        }

        EbuildInfoCommand info_cmd(make_named_values<EbuildCommandParams>(
                    value_for<n::builddir>(_imp->params.builddir()),
                    value_for<n::clearenv>(phase->option("clearenv")),
                    value_for<n::commands>(join(phase->begin_commands(), phase->end_commands(), " ")),
                    value_for<n::distdir>(ver_dir),
                    value_for<n::ebuild_dir>(ver_dir),
                    value_for<n::ebuild_file>(ver_dir / (stringify(id->name().package()) + "-" + stringify(id->version()) + ".ebuild")),
                    value_for<n::eclassdirs>(eclassdirs),
                    value_for<n::environment>(_imp->params.environment()),
                    value_for<n::exlibsdirs>(make_shared_ptr(new FSEntrySequence)),
                    value_for<n::files_dir>(ver_dir),
                    value_for<n::maybe_output_manager>(output_manager),
                    value_for<n::package_builddir>(_imp->params.builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-info")),
                    value_for<n::package_id>(id),
                    value_for<n::portdir>(ver_dir),
                    value_for<n::sandbox>(phase->option("sandbox")),
                    value_for<n::sydbox>(phase->option("sydbox")),
                    value_for<n::userpriv>(phase->option("userpriv"))
                ),

                make_named_values<EbuildInfoCommandParams>(
                    value_for<n::expand_vars>(make_shared_ptr(new Map<std::string, std::string>)),
                    value_for<n::info_vars>(i ? i : make_shared_ptr(new const Set<std::string>)),
                    value_for<n::load_environment>(load_env.get()),
                    value_for<n::profiles>(make_shared_ptr(new FSEntrySequence)),
                    value_for<n::root>(stringify(_imp->params.root())),
                    value_for<n::use>(""),
                    value_for<n::use_ebuild_file>(false),
                    value_for<n::use_expand>(""),
                    value_for<n::use_expand_hidden>("")
                    ));

        info_cmd();
    }
}

namespace
{
    std::tr1::shared_ptr<SetSpecTree> get_everything_set(
            const Environment * const env,
            const Repository * const repo)
    {
        Context context("When making 'everything' set from '" + stringify(repo->name()) + "':");

        std::tr1::shared_ptr<SetSpecTree> result(new SetSpecTree(make_shared_ptr(new AllDepSpec)));

        std::tr1::shared_ptr<const PackageIDSequence> ids((*env)[selection::BestVersionOnly(
                    generator::InRepository(repo->name()))]);
        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
            result->root()->append(make_shared_ptr(new PackageDepSpec(
                            make_package_dep_spec(PartiallyMadePackageDepSpecOptions())
                            .package((*i)->name())
                            )));

        return result;
    }
}

void
EInstalledRepository::populate_sets() const
{
    _imp->params.environment()->add_set(
            SetName("everything"),
            SetName("everything::" + stringify(name())),
            std::tr1::bind(get_everything_set, _imp->params.environment(), this),
            true);
}

