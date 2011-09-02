/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/util/visitor_cast.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/set.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/system.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/process.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/join.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/make_null_shared_ptr.hh>

#include <paludis/action.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/environment.hh>
#include <paludis/set_file.hh>
#include <paludis/hook.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/common_sets.hh>
#include <paludis/output_manager.hh>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Imp<EInstalledRepository>
    {
        EInstalledRepositoryParams params;
        Mutex world_mutex;

        Imp(const EInstalledRepositoryParams & p) :
            params(p)
        {
        }
    };
}

EInstalledRepository::EInstalledRepository(const EInstalledRepositoryParams & p,
        const RepositoryName & n, const RepositoryCapabilities & c) :
    Repository(p.environment(), n, c),
    _imp(p)
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
EInstalledRepository::some_ids_might_not_be_masked() const
{
    return true;
}

bool
EInstalledRepository::is_suitable_destination_for(const std::shared_ptr<const PackageID> & id) const
{
    auto repo(_imp->params.environment()->fetch_repository(id->repository_name()));
    std::string f(repo->format_key() ? repo->format_key()->parse_value() : "");
    return f == "e" || f == "ebuild" || f == "exheres" || f == "portage";
}

bool
EInstalledRepository::want_pre_post_phases() const
{
    return true;
}

HookResult
EInstalledRepository::perform_hook(const Hook & hook, const std::shared_ptr<OutputManager> &)
{
    Context context("When performing hook '" + stringify(hook.name()) + "' for repository '"
            + stringify(name()) + "':");

    if (hook.name() == "sync_all_post")
        perform_updates();

    return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
}

const bool
EInstalledRepository::is_unimportant() const
{
    return false;
}

std::shared_ptr<const CategoryNamePartSet>
EInstalledRepository::unimportant_category_names(const RepositoryContentMayExcludes &) const
{
    std::shared_ptr<CategoryNamePartSet> result(std::make_shared<CategoryNamePartSet>());
    result->insert(CategoryNamePart("virtual"));
    return result;
}

std::string
EInstalledRepository::get_environment_variable(
        const std::shared_ptr<const PackageID> & id,
        const std::string & var) const
{
    Context context("When fetching environment variable '" + var + "' for '" +
            stringify(*id) + "':");

    FSPath ver_dir(id->fs_location_key()->parse_value());

    if (! ver_dir.stat().is_directory_or_symlink_to_directory())
        throw ActionFailedError("Could not find Exndbam entry for '" + stringify(*id) + "'");

    if ((ver_dir / var).stat().is_regular_file_or_symlink_to_regular_file())
    {
        SafeIFStream f(ver_dir / var);
        return strip_trailing_string(
                std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>()), "\n");
    }
    else if ((ver_dir / "environment.bz2").stat().is_regular_file_or_symlink_to_regular_file())
    {
        return snoop_variable_from_environment_file(ver_dir / "environment.bz2", var);
    }
    else
        throw ActionFailedError("Could not get variable '" + var + "' for '" + stringify(*id) + "'");
}

std::string
EInstalledRepository::snoop_variable_from_environment_file(
        const FSPath & f,
        const std::string & var) const
{
    std::string x("cat");
    if (is_file_with_extension(f, ".bz2", { }))
        x = "bunzip2";

    std::stringstream p;
    Process env_process(ProcessCommand({"bash", "-c", "( " + x + " < " + stringify(f) +
                " ; echo echo \\$" + var + " ) | bash -O extglob 2>/dev/null"}));
    env_process.capture_stdout(p);
    int exit_status(env_process.run().wait());
    std::string result(strip_trailing_string(std::string(
                    (std::istreambuf_iterator<char>(p)),
                    std::istreambuf_iterator<char>()), "\n"));
    if (0 != exit_status)
        throw ActionFailedError("Could not load environment.bz2");
    return result;
}

void
EInstalledRepository::perform_config(
        const std::shared_ptr<const ERepositoryID> & id,
        const ConfigAction & a) const
{
    Context context("When configuring '" + stringify(*id) + "':");

    if (! _imp->params.root().stat().is_directory())
        throw ActionFailedError("Couldn't configure '" + stringify(*id) +
                "' because root ('" + stringify(_imp->params.root()) + "') is not a directory");

    std::shared_ptr<OutputManager> output_manager(a.options.make_output_manager()(a));

    FSPath ver_dir(id->fs_location_key()->parse_value());

    std::shared_ptr<FSPathSequence> eclassdirs(std::make_shared<FSPathSequence>());
    eclassdirs->push_back(ver_dir);

    FSPath load_env(ver_dir / "environment.bz2");
    EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_config());

    for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
        EbuildConfigCommand config_cmd(make_named_values<EbuildCommandParams>(
                    n::builddir() = _imp->params.builddir(),
                    n::clearenv() = phase->option("clearenv"),
                    n::commands() = join(phase->begin_commands(), phase->end_commands(), " "),
                    n::distdir() = ver_dir,
                    n::ebuild_dir() = ver_dir,
                    n::ebuild_file() = ver_dir / (stringify(id->name().package()) + "-" + stringify(id->version()) + ".ebuild"),
                    n::eclassdirs() = eclassdirs,
                    n::environment() = _imp->params.environment(),
                    n::exlibsdirs() = std::make_shared<FSPathSequence>(),
                    n::files_dir() = ver_dir,
                    n::maybe_output_manager() = output_manager,
                    n::package_builddir() = _imp->params.builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-config"),
                    n::package_id() = id,
                    n::permitted_directories() = make_null_shared_ptr(),
                    n::portdir() = ver_dir,
                    n::root() = stringify(_imp->params.root()),
                    n::sandbox() = phase->option("sandbox"),
                    n::sydbox() = phase->option("sydbox"),
                    n::userpriv() = phase->option("userpriv")
                ),

                make_named_values<EbuildConfigCommandParams>(
                    n::load_environment() = &load_env
                ));

        config_cmd();
    }

    output_manager->succeeded();
}

void
EInstalledRepository::perform_info(
        const std::shared_ptr<const ERepositoryID> & id,
        const InfoAction & a) const
{
    Context context("When infoing '" + stringify(*id) + "':");

    if (! _imp->params.root().stat().is_directory())
        throw ActionFailedError("Couldn't info '" + stringify(*id) +
                "' because root ('" + stringify(_imp->params.root()) + "') is not a directory");

    std::shared_ptr<OutputManager> output_manager(a.options.make_output_manager()(a));

    FSPath ver_dir(id->fs_location_key()->parse_value());

    auto eclassdirs(std::make_shared<FSPathSequence>());
    eclassdirs->push_back(ver_dir);

    FSPath load_env(ver_dir / "environment.bz2");

    EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_info());

    for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
        if (phase->option("installed=false"))
            continue;

        /* try to find an info_vars file from the original repo */
        std::shared_ptr<const Set<std::string> > i;
        if (id->from_repositories_key())
        {
            auto fr(id->from_repositories_key()->parse_value());
            for (Set<std::string>::ConstIterator o(fr->begin()), o_end(fr->end()) ;
                    o != o_end ; ++o)
            {
                RepositoryName rn(*o);
                if (_imp->params.environment()->has_repository_named(rn))
                {
                    const std::shared_ptr<const Repository> r(
                            _imp->params.environment()->fetch_repository(rn));
                    Repository::MetadataConstIterator m(r->find_metadata("info_vars"));
                    if (r->end_metadata() != m)
                    {
                        const MetadataCollectionKey<Set<std::string> > * const mm(
                                visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**m));
                        if (mm)
                        {
                            i = mm->parse_value();
                            break;
                        }
                    }
                }
            }
        }

        /* try to find an info_vars file from any repo */
        if (! i)
        {
            for (auto r(_imp->params.environment()->begin_repositories()), r_end(_imp->params.environment()->end_repositories()) ;
                    r != r_end ; ++r)
            {
                Repository::MetadataConstIterator m((*r)->find_metadata("info_vars"));
                if ((*r)->end_metadata() != m)
                {
                    const MetadataCollectionKey<Set<std::string> > * const mm(
                            visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**m));
                    if (mm)
                    {
                        i = mm->parse_value();
                        break;
                    }
                }
            }
        }

        EbuildInfoCommand info_cmd(make_named_values<EbuildCommandParams>(
                    n::builddir() = _imp->params.builddir(),
                    n::clearenv() = phase->option("clearenv"),
                    n::commands() = join(phase->begin_commands(), phase->end_commands(), " "),
                    n::distdir() = ver_dir,
                    n::ebuild_dir() = ver_dir,
                    n::ebuild_file() = ver_dir / (stringify(id->name().package()) + "-" + stringify(id->version()) + ".ebuild"),
                    n::eclassdirs() = eclassdirs,
                    n::environment() = _imp->params.environment(),
                    n::exlibsdirs() = std::make_shared<FSPathSequence>(),
                    n::files_dir() = ver_dir,
                    n::maybe_output_manager() = output_manager,
                    n::package_builddir() = _imp->params.builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-info"),
                    n::package_id() = id,
                    n::permitted_directories() = make_null_shared_ptr(),
                    n::portdir() = ver_dir,
                    n::root() = stringify(_imp->params.root()),
                    n::sandbox() = phase->option("sandbox"),
                    n::sydbox() = phase->option("sydbox"),
                    n::userpriv() = phase->option("userpriv")
                ),

                make_named_values<EbuildInfoCommandParams>(
                    n::expand_vars() = std::make_shared<Map<std::string, std::string> >(),
                    n::info_vars() = i ? i : std::make_shared<const Set<std::string> >(),
                    n::load_environment() = &load_env,
                    n::profiles() = std::make_shared<FSPathSequence>(),
                    n::profiles_with_parents() = std::make_shared<FSPathSequence>(),
                    n::use() = "",
                    n::use_ebuild_file() = false,
                    n::use_expand() = "",
                    n::use_expand_hidden() = ""
                    ));

        info_cmd();
    }

    output_manager->succeeded();
}

void
EInstalledRepository::populate_sets() const
{
    add_common_sets_for_installed_repo(_imp->params.environment(), *this);
}

bool
EInstalledRepository::sync(const std::string &, const std::string &, const std::shared_ptr<OutputManager> &) const
{
    return false;
}

