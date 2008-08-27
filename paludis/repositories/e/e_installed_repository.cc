/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/visitor_cast.hh>
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
#include <paludis/action.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/environment.hh>
#include <paludis/set_file.hh>
#include <paludis/hook.hh>
#include <paludis/dep_tag.hh>
#include <paludis/user_dep_spec.hh>
#include <fstream>

using namespace paludis;
using namespace paludis::erepository;

#include <paludis/repositories/e/e_installed_repository-sr.cc>

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
    Repository(p.environment, n, c),
    PrivateImplementationPattern<EInstalledRepository>(new Implementation<EInstalledRepository>(p)),
    _imp(PrivateImplementationPattern<EInstalledRepository>::_imp)
{
}

EInstalledRepository::~EInstalledRepository()
{
}

namespace
{
    struct SomeIDsMightSupportVisitor :
        ConstVisitor<SupportsActionTestVisitorTypes>
    {
        bool result;

        void visit(const SupportsActionTest<UninstallAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<InstalledAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<ConfigAction> &)
        {
           result = true;
        }

        void visit(const SupportsActionTest<InfoAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<PretendAction> &)
        {
            result = false;
        }

        void visit(const SupportsActionTest<FetchAction> &)
        {
            result = false;
        }

        void visit(const SupportsActionTest<PretendFetchAction> &)
        {
            result = false;
        }

        void visit(const SupportsActionTest<InstallAction> &)
        {
            result = false;
        }
    };
}

bool
EInstalledRepository::some_ids_might_support_action(const SupportsActionTestBase & test) const
{
    SomeIDsMightSupportVisitor v;
    test.accept(v);
    return v.result;
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
    return _imp->params.environment->root() == installed_root_key()->value();
}

bool
EInstalledRepository::want_pre_post_phases() const
{
    return true;
}

UseFlagState
EInstalledRepository::query_use(const UseFlagName & f, const PackageID & e) const
{
    if (this != e.repository().get())
        return use_unspecified;

    if (! static_cast<const ERepositoryID *>(&e)->use_key())
        return use_unspecified;

    if (static_cast<const ERepositoryID *>(&e)->use_key()->value()->end() != static_cast<const ERepositoryID *>(&e)->use_key()->value()->find(f))
        return use_enabled;
    else
        return use_disabled;
}

bool
EInstalledRepository::query_use_mask(const UseFlagName & u, const PackageID & e) const
{
    return use_disabled == query_use(u, e);
}

bool
EInstalledRepository::query_use_force(const UseFlagName & u, const PackageID & e) const
{
    return use_enabled == query_use(u, e);
}

HookResult
EInstalledRepository::perform_hook(const Hook & hook) const
{
    Context context("When performing hook '" + stringify(hook.name()) + "' for repository '"
            + stringify(name()) + "':");

    return HookResult(0, "");
}

std::string
EInstalledRepository::describe_use_flag(const UseFlagName &, const PackageID &) const
{
    return "";
}

std::tr1::shared_ptr<const UseFlagNameSet>
EInstalledRepository::use_expand_flags() const
{
    return std::tr1::shared_ptr<const UseFlagNameSet>(new UseFlagNameSet);
}

std::tr1::shared_ptr<const UseFlagNameSet>
EInstalledRepository::use_expand_prefixes() const
{
    return std::tr1::shared_ptr<const UseFlagNameSet>(new UseFlagNameSet);
}

std::tr1::shared_ptr<const UseFlagNameSet>
EInstalledRepository::use_expand_hidden_prefixes() const
{
    return std::tr1::shared_ptr<const UseFlagNameSet>(new UseFlagNameSet);
}

std::tr1::shared_ptr<SetSpecTree::ConstItem>
EInstalledRepository::package_set(const SetName & s) const
{
    using namespace std::tr1::placeholders;

    Context context("When fetching package set '" + stringify(s) + "' from '" +
            stringify(name()) + "':");

    if ("everything" == s.data())
    {
        std::tr1::shared_ptr<ConstTreeSequence<SetSpecTree, AllDepSpec> > result(new ConstTreeSequence<SetSpecTree, AllDepSpec>(
                    std::tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
        std::tr1::shared_ptr<GeneralSetDepTag> tag(new GeneralSetDepTag(SetName("everything"), stringify(name())));

        std::tr1::shared_ptr<const CategoryNamePartSet> cats(category_names());
        for (CategoryNamePartSet::ConstIterator i(cats->begin()), i_end(cats->end()) ;
                i != i_end ; ++i)
        {
            std::tr1::shared_ptr<const QualifiedPackageNameSet> pkgs(package_names(*i));
            for (QualifiedPackageNameSet::ConstIterator e(pkgs->begin()), e_end(pkgs->end()) ;
                    e != e_end ; ++e)
            {
                std::tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(make_package_dep_spec().package(*e)));
                spec->set_tag(tag);
                result->add(std::tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> >(new TreeLeaf<SetSpecTree, PackageDepSpec>(spec)));
            }
        }

        return result;
    }
    else if ("world" == s.data())
    {
        if (_imp->params.deprecated_world.exists())
        {
            std::tr1::shared_ptr<GeneralSetDepTag> tag(new GeneralSetDepTag(SetName("world"), stringify(name())));

            SetFile world(SetFileParams::create()
                    .file_name(_imp->params.deprecated_world)
                    .type(sft_simple)
                    .parser(std::tr1::bind(&parse_user_package_dep_spec, _1,
                            _imp->params.environment, UserPackageDepSpecOptions() + updso_no_disambiguation + updso_throw_if_set, filter::All()))
                    .tag(tag)
                    .set_operator_mode(sfsmo_natural)
                    .environment(_imp->params.environment));
            return world.contents();
        }

        return std::tr1::shared_ptr<SetSpecTree::ConstItem>(new ConstTreeSequence<SetSpecTree, AllDepSpec>(
                    std::tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
    }
    else
        return std::tr1::shared_ptr<SetSpecTree::ConstItem>();
}

std::tr1::shared_ptr<const SetNameSet>
EInstalledRepository::sets_list() const
{
    Context context("While generating the list of sets:");

    std::tr1::shared_ptr<SetNameSet> result(new SetNameSet);
    result->insert(SetName("everything"));
    if (_imp->params.deprecated_world.exists())
        result->insert(SetName("world"));
    return result;
}

std::tr1::shared_ptr<const CategoryNamePartSet>
EInstalledRepository::unimportant_category_names() const
{
    std::tr1::shared_ptr<CategoryNamePartSet> result(make_shared_ptr(new CategoryNamePartSet));
    result->insert(CategoryNamePart("virtual"));
    return result;
}

std::tr1::shared_ptr<const UseFlagNameSet>
EInstalledRepository::arch_flags() const
{
    return std::tr1::shared_ptr<const UseFlagNameSet>(new UseFlagNameSet);
}

char
EInstalledRepository::use_expand_separator(const PackageID & id) const
{
    if (this != id.repository().get())
        return '\0';
    const std::tr1::shared_ptr<const EAPI> & eapi(static_cast<const ERepositoryID &>(id).eapi());
    return eapi->supported() ? eapi->supported()->ebuild_options()->use_expand_separator() : '\0';
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
        throw ActionError("Could not find Exndbam entry for '" + stringify(*id) + "'");

    if ((ver_dir / var).is_regular_file_or_symlink_to_regular_file())
    {
        std::ifstream f(stringify(ver_dir / var).c_str());
        if (! f)
            throw ActionError("Could not read '" + stringify(ver_dir / var) + "'");
        return strip_trailing_string(
                std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>()), "\n");
    }
    else if ((ver_dir / "environment.bz2").is_regular_file_or_symlink_to_regular_file())
    {
        std::stringstream p;
        Command cmd(Command("bash -c '( bunzip2 < " + stringify(ver_dir / "environment.bz2" ) +
                    " ; echo echo \\$" + var + " ) | bash 2>/dev/null'").with_captured_stdout_stream(&p));
        int exit_status(run_command(cmd));
        std::string result(strip_trailing_string(std::string(
                        (std::istreambuf_iterator<char>(p)),
                        std::istreambuf_iterator<char>()), "\n"));
        if (0 != exit_status)
            throw ActionError("Could not load environment.bz2");
        return result;
    }
    else
        throw ActionError("Could not get variable '" + var + "' for '" + stringify(*id) + "'");
}

void
EInstalledRepository::perform_config(const std::tr1::shared_ptr<const ERepositoryID> & id) const
{
    Context context("When configuring '" + stringify(*id) + "':");

    if (! _imp->params.root.is_directory())
        throw InstallActionError("Couldn't configure '" + stringify(*id) +
                "' because root ('" + stringify(_imp->params.root) + "') is not a directory");

    FSEntry ver_dir(id->fs_location_key()->value());

    std::tr1::shared_ptr<FSEntrySequence> eclassdirs(new FSEntrySequence);
    eclassdirs->push_back(ver_dir);

    std::tr1::shared_ptr<FSEntry> load_env(new FSEntry(ver_dir / "environment.bz2"));
    EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_config());

    for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
        EbuildConfigCommand config_cmd(make_named_values<EbuildCommandParams>(
                    value_for<n::builddir>(_imp->params.builddir),
                    value_for<n::commands>(join(phase->begin_commands(), phase->end_commands(), " ")),
                    value_for<n::distdir>(ver_dir),
                    value_for<n::ebuild_dir>(ver_dir),
                    value_for<n::ebuild_file>(ver_dir / (stringify(id->name().package) + "-" + stringify(id->version()) + ".ebuild")),
                    value_for<n::eclassdirs>(eclassdirs),
                    value_for<n::environment>(_imp->params.environment),
                    value_for<n::exlibsdirs>(make_shared_ptr(new FSEntrySequence)),
                    value_for<n::files_dir>(ver_dir),
                    value_for<n::package_id>(id),
                    value_for<n::portdir>(ver_dir),
                    value_for<n::sandbox>(phase->option("sandbox")),
                    value_for<n::userpriv>(phase->option("userpriv"))
                ),

                make_named_values<EbuildConfigCommandParams>(
                    value_for<n::load_environment>(load_env.get()),
                    value_for<n::root>(stringify(_imp->params.root))
                ));

        config_cmd();
    }
}

void
EInstalledRepository::perform_info(const std::tr1::shared_ptr<const ERepositoryID> & id) const
{
    Context context("When infoing '" + stringify(*id) + "':");

    if (! _imp->params.root.is_directory())
        throw InstallActionError("Couldn't info '" + stringify(*id) +
                "' because root ('" + stringify(_imp->params.root) + "') is not a directory");

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
                if (_imp->params.environment->package_database()->has_repository_named(rn))
                {
                    const std::tr1::shared_ptr<const Repository> r(
                            _imp->params.environment->package_database()->fetch_repository(rn));
                    Repository::MetadataConstIterator m(r->find_metadata("info_vars"));
                    if (r->end_metadata() != m)
                    {
                        const MetadataCollectionKey<Set<std::string> > * const mm(
                                visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**m));
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
                    r(_imp->params.environment->package_database()->begin_repositories()),
                    r_end(_imp->params.environment->package_database()->end_repositories()) ;
                    r != r_end ; ++r)
            {
                Repository::MetadataConstIterator m((*r)->find_metadata("info_vars"));
                if ((*r)->end_metadata() != m)
                {
                    const MetadataCollectionKey<Set<std::string> > * const mm(
                            visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**m));
                    if (mm)
                    {
                        i = mm->value();
                        break;
                    }
                }
            }
        }

        EbuildInfoCommand info_cmd(make_named_values<EbuildCommandParams>(
                    value_for<n::builddir>(_imp->params.builddir),
                    value_for<n::commands>(join(phase->begin_commands(), phase->end_commands(), " ")),
                    value_for<n::distdir>(ver_dir),
                    value_for<n::ebuild_dir>(ver_dir),
                    value_for<n::ebuild_file>(ver_dir / (stringify(id->name().package) + "-" + stringify(id->version()) + ".ebuild")),
                    value_for<n::eclassdirs>(eclassdirs),
                    value_for<n::environment>(_imp->params.environment),
                    value_for<n::exlibsdirs>(make_shared_ptr(new FSEntrySequence)),
                    value_for<n::files_dir>(ver_dir),
                    value_for<n::package_id>(id),
                    value_for<n::portdir>(ver_dir),
                    value_for<n::sandbox>(phase->option("sandbox")),
                    value_for<n::userpriv>(phase->option("userpriv"))
                ),

                make_named_values<EbuildInfoCommandParams>(
                    value_for<n::expand_vars>(make_shared_ptr(new Map<std::string, std::string>)),
                    value_for<n::info_vars>(i ? i : make_shared_ptr(new const Set<std::string>)),
                    value_for<n::load_environment>(load_env.get()),
                    value_for<n::profiles>(make_shared_ptr(new FSEntrySequence)),
                    value_for<n::root>(stringify(_imp->params.root)),
                    value_for<n::use>(""),
                    value_for<n::use_ebuild_file>(false),
                    value_for<n::use_expand>("")
                    ));

        info_cmd();
    }
}

