/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Danny van Dyk
 * Copyright (c) 2007 Ciaran McCreesh
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

#include <paludis/hashed_containers.hh>
#include <paludis/environment.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/repository_info.hh>
#include <paludis/dep_spec.hh>
#include <paludis/dep_tag.hh>
#include <paludis/util/config_file.hh>
#include <paludis/set_file.hh>
#include <paludis/action.hh>
#include <paludis/repositories/cran/cran_package_id.hh>
#include <paludis/repositories/cran/cran_dep_parser.hh>
#include <paludis/repositories/cran/cran_installed_repository.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/system.hh>
#include <paludis/util/log.hh>
#include <paludis/util/map.hh>
#include <paludis/util/set.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/visitor-impl.hh>

#include <functional>
#include <algorithm>
#include <fstream>

using namespace paludis;

#include <paludis/repositories/cran/cran_installed_repository-sr.cc>

typedef MakeHashedMap<QualifiedPackageName, tr1::shared_ptr<const cranrepository::CRANPackageID> >::Type IDMap;

namespace paludis
{
    template <>
    struct Implementation<CRANInstalledRepository>
    {
        CRANInstalledRepositoryParams params;

        mutable bool has_ids;
        mutable IDMap ids;

        /// Constructor.
        Implementation(const CRANInstalledRepositoryParams &);

        /// Destructor.
        ~Implementation();
    };
}

Implementation<CRANInstalledRepository>::Implementation(const CRANInstalledRepositoryParams & p) :
    params(p),
    has_ids(false)
{
}

Implementation<CRANInstalledRepository>::~Implementation()
{
}

#if 0
void
Implementation<CRANInstalledRepository>::need_ids() const
{
    Context context("When loading CRANInstalledRepository IDs from '" + stringify(location) + "':");

    entries_valid = true;
    try
    {
        for (DirIterator d(location), d_end ; d != d_end ; ++d)
        {
            Context local_context("When parsing directoryy '" + stringify(*d) + "':");
            if (! d->is_directory())
                continue;

            FSEntry f(FSEntry(stringify(*d)) / "DESCRIPTION");
            if (! f.is_regular_file())
                continue;

            std::string n(d->basename());
            CRANDescription::normalise_name(n);

            CRANDescription desc(n, f, true);
            entries.push_back(desc);

            QualifiedPackageName q("cran/" + n);
            metadata.insert(std::make_pair(std::make_pair(q, VersionSpec(desc.version)), desc.metadata));
        }

        if (! FSEntry(location / "paludis" / "bundles").is_directory())
            return;

        // Load Bundle Metadata
        for (DirIterator f(location / "paludis" / "bundles"), f_end ; f != f_end ; ++f)
        {
            Context local_context("When parsing file '" + stringify(*f) + "'.");

            if (! f->is_regular_file())
            {
                continue;
            }

            std::string n(f->basename());
            std::string::size_type pos(n.find_last_of("."));
            if (std::string::npos != pos)
                n.erase(pos);
            CRANDescription::normalise_name(n);

            CRANDescription desc(n, *f, true);
            entries.push_back(desc);

            QualifiedPackageName q("cran/" + n);
            metadata.insert(std::make_pair(std::make_pair(q, VersionSpec(desc.version)), desc.metadata));
        }

    }
    catch (...)
    {
        entries_valid = false;
        throw;
    }
}
#endif

CRANInstalledRepository::CRANInstalledRepository(const CRANInstalledRepositoryParams & p) :
    Repository(RepositoryName("installed-cran"),
            RepositoryCapabilities::create()
            .installed_interface(this)
            .sets_interface(this)
            .syncable_interface(0)
            .use_interface(0)
            .world_interface(this)
            .environment_variable_interface(0)
            .mirrors_interface(0)
            .virtuals_interface(0)
            .provides_interface(0)
            .destination_interface(this)
            .e_interface(0)
            .qa_interface(0)
            .make_virtuals_interface(0)
            .hook_interface(0)
            .manifest_interface(0),
            "installed_cran"),
    PrivateImplementationPattern<CRANInstalledRepository>(new Implementation<CRANInstalledRepository>(p))
{
    tr1::shared_ptr<RepositoryInfoSection> config_info(new RepositoryInfoSection("Configuration information"));

    config_info->add_kv("location", stringify(_imp->params.location));
    config_info->add_kv("root", stringify(_imp->params.root));
    config_info->add_kv("format", "installed_cran");

    _info->add_section(config_info);
}

CRANInstalledRepository::~CRANInstalledRepository()
{
}

bool
CRANInstalledRepository::has_category_named(const CategoryNamePart & c) const
{
    return (CategoryNamePart("cran") == c);
}

bool
CRANInstalledRepository::has_package_named(const QualifiedPackageName & q) const
{
    Context context("When checking for package '" + stringify(q) + "' in " + stringify(name()) + ":");

    if (! has_category_named(q.category))
        return false;

    need_ids();

    return _imp->ids.end() != _imp->ids.find(q);
}

tr1::shared_ptr<const CategoryNamePartSet>
CRANInstalledRepository::category_names() const
{
    tr1::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);
    result->insert(CategoryNamePart("cran"));
    return result;
}

tr1::shared_ptr<const QualifiedPackageNameSet>
CRANInstalledRepository::package_names(const CategoryNamePart & c) const
{
    Context context("When fetching package names in category '" + stringify(c)
            + "' in " + stringify(name()) + ":");

    tr1::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);
    if (! has_category_named(c))
        return result;

    need_ids();

    std::transform(_imp->ids.begin(), _imp->ids.end(), result->inserter(),
            tr1::mem_fn(&std::pair<const QualifiedPackageName, tr1::shared_ptr<const cranrepository::CRANPackageID> >::first));

    return result;
}

tr1::shared_ptr<const PackageIDSequence>
CRANInstalledRepository::package_ids(const QualifiedPackageName & n) const
{
    Context context("When fetching versions of '" + stringify(n) + "' in "
            + stringify(name()) + ":");

    tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence);
    if (! has_package_named(n))
        return result;

    need_ids();

    IDMap::const_iterator i(_imp->ids.find(n));
    if (i != _imp->ids.end())
        result->push_back(i->second);
    return result;
}

#if 0
tr1::shared_ptr<const Contents>
CRANInstalledRepository::do_contents(const Package ID & id) const
{
    Context context("When fetching contents for " + stringify(id) + ":");

    tr1::shared_ptr<Contents> result(new Contents);

    if (! _imp->entries_valid)
        _imp->load_entries();

    if (! has_version(q, v))
        return result;

    std::string pn = stringify(q.package);
    CRANDescription::normalise_name(pn);
    FSEntry f(_imp->location / "paludis" / pn / "CONTENTS");
    if (! f.is_regular_file())
    {
        Log::get_instance()->message(ll_warning, lc_no_context, "CONTENTS lookup failed for request for' " +
                stringify(q) + "-" + stringify(v) + "' in CRANInstalled '" +
                stringify(_imp->location) + "'");
        return result;
    }

    std::ifstream ff(stringify(f).c_str());
    if (! ff)
        throw ConfigurationError("Could not read '" + stringify(f) + "'");

    std::string line;
    unsigned line_number(0);
    while (std::getline(ff, line))
    {
        ++line_number;

        std::vector<std::string> tokens;
        WhitespaceTokeniser::tokenise(line, std::back_inserter(tokens));
        if (tokens.empty())
            continue;

        if (tokens.size() < 2)
        {
            Log::get_instance()->message(ll_warning, lc_no_context, "CONTENTS for '" +
                    stringify(q) + "-" + stringify(v) + "' in vdb '" +
                    stringify(_imp->location) + "' has broken line " +
                    stringify(line_number) + ", skipping");
            continue;
        }

        if ("obj" == tokens.at(0))
            result->add(tr1::shared_ptr<ContentsEntry>(new ContentsFileEntry(tokens.at(1))));
        else if ("dir" == tokens.at(0))
            result->add(tr1::shared_ptr<ContentsEntry>(new ContentsDirEntry(tokens.at(1))));
        else if ("misc" == tokens.at(0))
            result->add(tr1::shared_ptr<ContentsEntry>(new ContentsMiscEntry(tokens.at(1))));
        else if ("sym" == tokens.at(0))
        {
            if (tokens.size() < 4)
            {
                Log::get_instance()->message(ll_warning, lc_no_context, "CONTENTS for '" +
                        stringify(q) + "-" + stringify(v) + "' in vdb '" +
                        stringify(_imp->location) + "' has broken sym line " +
                        stringify(line_number) + ", skipping");
                continue;
            }

            result->add(tr1::shared_ptr<ContentsEntry>(new ContentsSymEntry(
                            tokens.at(1), tokens.at(3))));
        }
    }

    return result;
}

time_t
CRANInstalledRepository::do_installed_time(const QualifiedPackageName & q,
        const VersionSpec & v) const
{
    Context context("When finding installed time for '" + stringify(q) +
            "-" + stringify(v) + "':");

    if (! _imp->entries_valid)
        _imp->load_entries();

    std::vector<CRANDescription>::iterator r(_imp->entries.begin()), r_end(_imp->entries.end());
    for ( ; r != r_end ; ++r)
    {
        if (q == r->name)
            break;
    }

    if (r == r_end)
        throw NoSuchPackageError(stringify(PackageDatabaseEntry(q, v, name())));
    else
    {
        if (0 == r->installed_time)
        {
            std::string pn(stringify(q.package));
            CRANDescription::normalise_name(pn);
            FSEntry f(_imp->location / "paludis" / pn / "CONTENTS");
            try
            {
                r->installed_time = f.ctime();
            }
            catch (const FSError & e)
            {
                Log::get_instance()->message(ll_warning, lc_no_context, "Can't get ctime of '"
                        + stringify(f) + "' due to exception '" + e.message() + "' (" + e.what()
                        + ")");
                r->installed_time = 1;
            }
        }
        return r->installed_time;
    }
}
#endif

tr1::shared_ptr<Repository>
CRANInstalledRepository::make_cran_installed_repository(
        Environment * const env,
        tr1::shared_ptr<const Map<std::string, std::string> > m)
{
    Context context("When making CRAN installed repository from repo_file '" + 
            (m->end() == m->find("repo_file") ? std::string("?") : m->find("repo_file")->second) + "':");

    std::string location;
    if (m->end() == m->find("location") || ((location = m->find("location")->second)).empty())
        throw CRANInstalledRepositoryConfigurationError("Key 'location' not specified or empty");

    std::string root;
    if (m->end() == m->find("root") || ((root = m->find("root")->second)).empty())
        root = stringify(env->root());

    std::string world;
    if (m->end() == m->find("world") || ((world = m->find("world")->second)).empty())
        world = location + "/world";

    return tr1::shared_ptr<Repository>(new CRANInstalledRepository(CRANInstalledRepositoryParams::create()
                .environment(env)
                .location(location)
                .root(root)
                .world(world)));
}

CRANInstalledRepositoryConfigurationError::CRANInstalledRepositoryConfigurationError(
        const std::string & msg) throw () :
    ConfigurationError("CRAN installed repository configuration error: " + msg)
{
}

#if 0
void
CRANInstalledRepository::do_uninstall(const QualifiedPackageName & q, const VersionSpec & v,
        const UninstallOptions &) const
{
    Context context("When uninstalling '" + stringify(q) + "-" + stringify(v) +
            "' from '" + stringify(name()) + "':");

    if (! _imp->location.is_directory())
        throw PackageInstallActionError("Couldn't uninstall '" + stringify(q) + "-" +
                stringify(v) + "' because its location ('" + stringify(_imp->location) + "') is not a directory");

    tr1::shared_ptr<const VersionMetadata> vm(do_version_metadata(q, v));
    tr1::shared_ptr<const FSEntryCollection> bashrc_files(_imp->env->bashrc_files());

    Command cmd(Command(LIBEXECDIR "/paludis/cran.bash unmerge")
            .with_setenv("PN", vm->cran_interface->package)
            .with_setenv("PV", stringify(v))
            .with_setenv("PALUDIS_CRAN_LIBRARY", stringify(_imp->location))
            .with_setenv("PALUDIS_EBUILD_DIR", std::string(LIBEXECDIR "/paludis/"))
            .with_setenv("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " ")));

    if (0 != run_command(cmd))
        throw PackageUninstallActionError("Couldn't unmerge '" + stringify(q) + "-" + stringify(v) + "'");
}
#endif

tr1::shared_ptr<SetSpecTree::ConstItem>
CRANInstalledRepository::package_set(const SetName & s) const
{
    Context context("When fetching package set '" + stringify(s) + "' from '" +
            stringify(name()) + "':");

    if ("everything" == s.data())
    {
        tr1::shared_ptr<ConstTreeSequence<SetSpecTree, AllDepSpec> > result(new ConstTreeSequence<SetSpecTree, AllDepSpec>(
                    tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));

        need_ids();

        for (IDMap::const_iterator p(_imp->ids.begin()), p_end(_imp->ids.end()) ;
                p != p_end ; ++p)
        {
            tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> > spec(
                    new TreeLeaf<SetSpecTree, PackageDepSpec>(make_shared_ptr(
                            new PackageDepSpec(make_shared_ptr(new QualifiedPackageName(p->first))))));
            result->add(spec);
        }

        return result;
    }
    else if ("world" == s.data())
    {
        tr1::shared_ptr<ConstTreeSequence<SetSpecTree, AllDepSpec> > result(new ConstTreeSequence<SetSpecTree, AllDepSpec>(
                    tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
        tr1::shared_ptr<GeneralSetDepTag> tag(new GeneralSetDepTag(SetName("world"), stringify(name())));

        if (_imp->params.world.exists())
        {
            SetFile world(SetFileParams::create()
                    .file_name(_imp->params.world)
                    .type(sft_simple)
                    .parse_mode(pds_pm_unspecific)
                    .tag(tag)
                    .environment(_imp->params.environment));

            return world.contents();
        }
        else
        {
            Log::get_instance()->message(ll_warning, lc_no_context) << "World file '" << _imp->params.world << "' doesn't exist";
            return result;
        }
    }
    else
        return tr1::shared_ptr<SetSpecTree::ConstItem>();
}

tr1::shared_ptr<const SetNameSet>
CRANInstalledRepository::sets_list() const
{
    Context context("While generating the list of sets:");

    tr1::shared_ptr<SetNameSet> result(new SetNameSet);
    result->insert(SetName("everything"));
    result->insert(SetName("world"));
    return result;
}

void
CRANInstalledRepository::invalidate()
{
    _imp.reset(new Implementation<CRANInstalledRepository>(_imp->params));
}

void
CRANInstalledRepository::invalidate_masks()
{
}

void
CRANInstalledRepository::add_string_to_world(const std::string & n) const
{
    Context context("When adding '" + n + "' to world file '" + stringify(_imp->params.world) + "':");

    if (! _imp->params.world.exists())
    {
        std::ofstream f(stringify(_imp->params.world).c_str());
        if (! f)
        {
            Log::get_instance()->message(ll_warning, lc_no_context, "Cannot create world file '"
                    + stringify(_imp->params.world) + "'");
            return;
        }
    }

    SetFile world(SetFileParams::create()
            .file_name(_imp->params.world)
            .type(sft_simple)
            .parse_mode(pds_pm_unspecific)
            .tag(tr1::shared_ptr<DepTag>())
            .environment(_imp->params.environment));
    world.add(n);
    world.rewrite();
}

void
CRANInstalledRepository::remove_string_from_world(const std::string & n) const
{
    Context context("When removing '" + n + "' from world file '" + stringify(_imp->params.world) + "':");

    if (_imp->params.world.exists())
    {
        SetFile world(SetFileParams::create()
                .file_name(_imp->params.world)
                .type(sft_simple)
                .parse_mode(pds_pm_unspecific)
                .tag(tr1::shared_ptr<DepTag>())
                .environment(_imp->params.environment));

        world.remove(n);
        world.rewrite();
    }
}

bool
CRANInstalledRepository::is_suitable_destination_for(const PackageID & e) const
{
    std::string f(e.repository()->format());
    return f == "cran";
}

void
CRANInstalledRepository::add_to_world(const QualifiedPackageName & n) const
{
    add_string_to_world(stringify(n));
}

void
CRANInstalledRepository::remove_from_world(const QualifiedPackageName & n) const
{
    remove_string_from_world(stringify(n));
}

void
CRANInstalledRepository::add_to_world(const SetName & n) const
{
    add_string_to_world(stringify(n));
}

void
CRANInstalledRepository::remove_from_world(const SetName & n) const
{
    remove_string_from_world(stringify(n));
}

FSEntry
CRANInstalledRepository::root() const
{
    return _imp->params.root;
}

bool
CRANInstalledRepository::is_default_destination() const
{
    return _imp->params.environment->root() == root();
}

bool
CRANInstalledRepository::want_pre_post_phases() const
{
    return true;
}

void
CRANInstalledRepository::merge(const MergeOptions & m)
{
    Context context("When merging '" + stringify(*m.package_id) + "' at '" + stringify(m.image_dir)
            + "' to repository '" + stringify(name()) + "':");

    if (! is_suitable_destination_for(*m.package_id))
        throw InstallActionError("Not a suitable destination for '" + stringify(*m.package_id) + "'");

}

namespace
{
    struct SupportsActionQuery :
        ConstVisitor<SupportsActionTestVisitorTypes>
    {
        bool result;

        SupportsActionQuery() :
            result(false)
        {
        }

        void visit(const SupportsActionTest<InstalledAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<InstallAction> &)
        {
        }

        void visit(const SupportsActionTest<ConfigAction> &)
        {
        }

        void visit(const SupportsActionTest<PretendAction> &)
        {
        }

        void visit(const SupportsActionTest<FetchAction> &)
        {
        }

        void visit(const SupportsActionTest<UninstallAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<InfoAction> &)
        {
        }
    };
}

bool
CRANInstalledRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    a.accept(q);
    return q.result;
}

void
CRANInstalledRepository::need_ids() const
{
}

