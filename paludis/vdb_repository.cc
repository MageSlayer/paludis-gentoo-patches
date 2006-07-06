/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/dep_atom.hh>
#include <paludis/dep_atom_flattener.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/ebuild.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/config_file.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/vdb_repository.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/system.hh>
#include <paludis/util/log.hh>
#include <paludis/util/pstream.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/tokeniser.hh>

#include <fstream>
#include <functional>
#include <algorithm>
#include <vector>

/** \file
 * Implementation for VDBRepository.
 *
 * \ingroup grpvdbrepository
 */

using namespace paludis;

namespace
{
    /**
     * Holds an entry in a VDB.
     */
    struct VDBEntry
    {
        /// Our package name.
        QualifiedPackageName name;

        /// Our package version.
        VersionSpec version;

        /// Our metadata, may be zero.
        VersionMetadata::Pointer metadata;

        /// Our built USE flags.
        std::set<UseFlagName> use;

        /// Constructor
        VDBEntry(const QualifiedPackageName & n, const VersionSpec & v) :
            name(n),
            version(v),
            metadata(0)
        {
        }

        /// Comparison operator
        bool operator< (const VDBEntry & other) const
        {
            if (name < other.name)
                return true;
            if (name > other.name)
                return false;
            if (version < other.version)
                return true;
            return false;
        }

        /**
         * Compare a VDBEntry by category only.
         *
         * \ingroup grpvdbrepository
         */
        struct CompareCategory
        {
            bool operator() (const CategoryNamePart & c, const VDBEntry & e) const
            {
                return c < e.name.get<qpn_category>();
            }

            bool operator() (const VDBEntry & e, const CategoryNamePart & c) const
            {
                return e.name.get<qpn_category>() < c;
            }
        };

        /**
         * Compare a VDBEntry by name only.
         *
         * \ingroup grpvdbrepository
         */
        struct ComparePackage
        {
            bool operator() (const QualifiedPackageName & c, const VDBEntry & e) const
            {
                return c < e.name;
            }

            bool operator() (const VDBEntry & e, const QualifiedPackageName & c) const
            {
                return e.name < c;
            }
        };

        /**
         * Compare a VDBEntry by name and version.
         *
         * \ingroup grpvdbrepository
         */
        struct CompareVersion
        {
            bool operator() (const std::pair<QualifiedPackageName, VersionSpec> & c,
                    const VDBEntry & e) const
            {
                if (c.first < e.name)
                    return true;
                else if (c.first > e.name)
                    return false;
                else if (c.second < e.version)
                    return true;
                else
                    return false;
            }

            bool operator() (const VDBEntry & e,
                    const std::pair<QualifiedPackageName, VersionSpec> & c) const
            {
                if (e.name < c.first)
                    return true;
                else if (e.name > c.first)
                    return false;
                else if (e.version < c.second)
                    return true;
                else
                    return false;
            }
        };
    };

    /**
     * Figure out the format of environment.bz2. If VDB_FORMAT is "paludis-1",
     * or if there's no VDB_FORMAT and there're no lines with () and no =, it's
     * an env dump. Otherwise it's a source file.
     *
     * \ingroup grpvdbrepository
     */
    bool is_full_env(const FSEntry & vdb_dir)
    {
        bool result(false);

        if ((vdb_dir / "VDB_FORMAT").is_regular_file())
        {
            std::ifstream f(stringify(vdb_dir / "VDB_FORMAT").c_str());
            if (! f)
                throw EnvironmentVariableActionError("Could not read '" +
                        stringify(vdb_dir / "VDB_FORMAT") + "'");
            result = ("paludis-1" != strip_trailing_string(std::string(
                            (std::istreambuf_iterator<char>(f)),
                            std::istreambuf_iterator<char>()), "\n"));
        }
        else if (0 == run_command("bunzip2 < " + stringify(vdb_dir / "environment.bz2") +
                    " | grep -q '^[^=]\\+()'"))
            result = true;

        return result;
    }
}

namespace paludis
{
    /**
     * Implementation data for VDBRepository.
     *
     * \ingroup grpvdbrepository
     */
    template <>
    struct Implementation<VDBRepository> :
        InternalCounted<Implementation<VDBRepository> >
    {
        /// Our owning db.
        const PackageDatabase * const db;

        /// Our owning env.
        const Environment * const env;

        /// Our base location.
        FSEntry location;

        /// Root location
        FSEntry root;

        /// Build root
        FSEntry buildroot;

        /// World file
        FSEntry world_file;

        /// Do we have entries loaded?
        mutable bool entries_valid;

        /// Our entries, keep this sorted!
        mutable std::vector<VDBEntry> entries;

        /// Load entries.
        void load_entries() const;

        /// Load metadata for one entry.
        void load_entry(std::vector<VDBEntry>::iterator) const;

        /// Do we have provide map loaded?
        mutable bool has_provide_map;

        /// Provide map.
        mutable std::map<QualifiedPackageName, QualifiedPackageName> provide_map;

        /// Constructor.
        Implementation(const VDBRepositoryParams &);

        /// Destructor.
        ~Implementation();

        /// Invalidate.
        void invalidate() const;
    };
}

Implementation<VDBRepository>::Implementation(const VDBRepositoryParams & p) :
    db(p.get<vdbrpk_package_database>()),
    env(p.get<vdbrpk_environment>()),
    location(p.get<vdbrpk_location>()),
    root(p.get<vdbrpk_root>()),
    buildroot(p.get<vdbrpk_buildroot>()),
    world_file(p.get<vdbrpk_world>()),
    entries_valid(false),
    has_provide_map(false)
{
}

Implementation<VDBRepository>::~Implementation()
{
}

void
Implementation<VDBRepository>::load_entries() const
{
    Context context("When loading VDBRepository entries from '" +
            stringify(location) + "':");

    entries.clear();
    entries_valid = true;
    try
    {
        for (DirIterator cat_i(location), cat_iend ; cat_i != cat_iend ; ++cat_i)
        {
            if (! cat_i->is_directory())
                continue;

            for (DirIterator pkg_i(*cat_i), pkg_iend ; pkg_i != pkg_iend ; ++pkg_i)
            {
                PackageDepAtom atom("=" + cat_i->basename() + "/" + pkg_i->basename());
                entries.push_back(VDBEntry(atom.package(), *atom.version_spec_ptr()));
            }
        }

        std::sort(entries.begin(), entries.end());
    }
    catch (...)
    {
        entries_valid = false;
        throw;
    }
}

void
Implementation<VDBRepository>::invalidate() const
{
    entries_valid = false;
    entries.clear();

    has_provide_map = false;
    provide_map.clear();
}

namespace
{
    /**
     * Fetch the contents of a VDB file.
     *
     * \ingroup grpvdbrepository
     */
    std::string
    file_contents(const FSEntry & location, const QualifiedPackageName & name,
            const VersionSpec & v, const std::string & key)
    {
        Context context("When loading VDBRepository entry for '" + stringify(name)
                + "-" + stringify(v) + "' key '" + key + "' from '" + stringify(location) + "':");

        FSEntry f(location / stringify(name.get<qpn_category>()) /
                (stringify(name.get<qpn_package>()) + "-" + stringify(v)));
        if (! (f / key).is_regular_file())
            return "";

        std::ifstream ff(stringify(f / key).c_str());
        if (! ff)
            throw VDBRepositoryKeyReadError("Could not read '" + stringify(f / key) + "'");
        return strip_leading(strip_trailing(std::string((std::istreambuf_iterator<char>(ff)),
                        std::istreambuf_iterator<char>()), " \t\n"), " \t\n");
    }

    /**
     * Filter if a USE flag is a -flag.
     *
     * \ingroup grpvdbrepository
     */
    struct IsPositiveFlag
    {
        bool operator() (const std::string & f) const
        {
            return 0 != f.compare(0, 1, "-");
        }
    };
}

void
Implementation<VDBRepository>::load_entry(std::vector<VDBEntry>::iterator p) const
{
    Context context("When loading VDBRepository entry for '" + stringify(p->name)
            + "-" + stringify(p->version) + "' from '" + stringify(location) + "':");

    p->metadata = VersionMetadata::Pointer(new VersionMetadata::Ebuild(PortageDepParser::parse_depend));
    p->metadata->get<vm_deps>().set<vmd_build_depend_string>(
            file_contents(location, p->name, p->version, "DEPEND"));
    p->metadata->get<vm_deps>().set<vmd_run_depend_string>(
            file_contents(location, p->name, p->version, "RDEPEND"));
    p->metadata->set<vm_license>(file_contents(location, p->name, p->version, "LICENSE"));
    p->metadata->get_ebuild_interface()->set<evm_keywords>("*");
    p->metadata->get_ebuild_interface()->set<evm_inherited>(
            file_contents(location, p->name, p->version, "INHERITED"));
    p->metadata->get_ebuild_interface()->set<evm_iuse>(
            file_contents(location, p->name, p->version, "IUSE"));
    p->metadata->get<vm_deps>().set<vmd_post_depend_string>(
            file_contents(location, p->name, p->version, "PDEPEND"));
    p->metadata->get_ebuild_interface()->set<evm_provide>(
            file_contents(location, p->name, p->version, "PROVIDE"));
    p->metadata->set<vm_eapi>(file_contents(location, p->name, p->version, "EAPI"));
    p->metadata->set<vm_homepage>(file_contents(location, p->name, p->version, "HOMEPAGE"));
    p->metadata->set<vm_description>(file_contents(location, p->name, p->version, "DESCRIPTION"));

    std::string slot(file_contents(location, p->name, p->version, "SLOT"));
    if (slot.empty())
    {
        Log::get_instance()->message(ll_warning, lc_no_context, "VDBRepository entry '" +
                stringify(p->name) + "-" + stringify(p->version) + "' in '" +
                stringify(location) + "' has empty SLOT, setting to \"0\"");
        slot = "0";
    }
    p->metadata->set<vm_slot>(SlotName(slot));

    std::string raw_use(file_contents(location, p->name, p->version, "USE"));
    p->use.clear();
    WhitespaceTokeniser::get_instance()->tokenise(raw_use,
            filter_inserter(create_inserter<UseFlagName>(
                    std::inserter(p->use, p->use.begin())), IsPositiveFlag()));
}

VDBRepository::VDBRepository(const VDBRepositoryParams & p) :
    Repository(RepositoryName("installed"),
            RepositoryCapabilities::create((
                    param<repo_installable>(static_cast<InstallableInterface *>(0)),
                    param<repo_installed>(this),
                    param<repo_mask>(static_cast<MaskInterface *>(0)),
                    param<repo_news>(static_cast<NewsInterface *>(0)),
                    param<repo_sets>(this),
                    param<repo_syncable>(static_cast<SyncableInterface *>(0)),
                    param<repo_uninstallable>(this),
                    param<repo_use>(this),
                    param<repo_world>(this),
                    param<repo_environment_variable>(this)
                    ))),
    PrivateImplementationPattern<VDBRepository>(new Implementation<VDBRepository>(p))
{
    RepositoryInfoSection config_info("Configuration information");

    config_info.add_kv("location", stringify(_imp->location));
    config_info.add_kv("root", stringify(_imp->root));
    config_info.add_kv("format", "vdb");
    config_info.add_kv("world", stringify(_imp->world_file));
    config_info.add_kv("buildroot", stringify(_imp->buildroot));

    _info->add_section(config_info);
}

VDBRepository::~VDBRepository()
{
}

bool
VDBRepository::do_has_category_named(const CategoryNamePart & c) const
{
    Context context("When checking for category '" + stringify(c) +
            "' in " + stringify(name()) + ":");

    if (! _imp->entries_valid)
        _imp->load_entries();

    std::pair<std::vector<VDBEntry>::const_iterator, std::vector<VDBEntry>::const_iterator>
        r(std::equal_range(_imp->entries.begin(), _imp->entries.end(), c,
                    VDBEntry::CompareCategory()));
    return r.first != r.second;
}

bool
VDBRepository::do_has_package_named(const QualifiedPackageName & q) const
{
    Context context("When checking for package '" + stringify(q) +
            "' in " + stringify(name()) + ":");

    if (! _imp->entries_valid)
        _imp->load_entries();

    std::pair<std::vector<VDBEntry>::const_iterator, std::vector<VDBEntry>::const_iterator>
        r(std::equal_range(_imp->entries.begin(), _imp->entries.end(), q,
                    VDBEntry::ComparePackage()));
    return r.first != r.second;
}

CategoryNamePartCollection::ConstPointer
VDBRepository::do_category_names() const
{
    Context context("When fetching category names in " + stringify(name()) + ":");

    if (! _imp->entries_valid)
        _imp->load_entries();

    CategoryNamePartCollection::Pointer result(new CategoryNamePartCollection);

    for (std::vector<VDBEntry>::const_iterator c(_imp->entries.begin()), c_end(_imp->entries.end()) ;
            c != c_end ; ++c)
        result->insert(c->name.get<qpn_category>());

    return result;
}

QualifiedPackageNameCollection::ConstPointer
VDBRepository::do_package_names(const CategoryNamePart & c) const
{
    /* this isn't particularly fast because it isn't called very often. avoid
     * changing the data structures used to make this faster at the expense of
     * slowing down single item queries. */

    Context context("When fetching package names in category '" + stringify(c)
            + "' in " + stringify(name()) + ":");

    QualifiedPackageNameCollection::Pointer result(new QualifiedPackageNameCollection);

    std::pair<std::vector<VDBEntry>::const_iterator, std::vector<VDBEntry>::const_iterator>
        r(std::equal_range(_imp->entries.begin(), _imp->entries.end(), c,
                    VDBEntry::CompareCategory()));

    for ( ; r.first != r.second ; ++(r.first))
        result->insert(r.first->name);

    return result;
}

VersionSpecCollection::ConstPointer
VDBRepository::do_version_specs(const QualifiedPackageName & n) const
{
    Context context("When fetching versions of '" + stringify(n) + "' in "
            + stringify(name()) + ":");

    VersionSpecCollection::Pointer result(new VersionSpecCollection);

    std::pair<std::vector<VDBEntry>::const_iterator, std::vector<VDBEntry>::const_iterator>
        r(std::equal_range(_imp->entries.begin(), _imp->entries.end(), n,
                    VDBEntry::ComparePackage()));

    for ( ; r.first != r.second ; ++(r.first))
        result->insert(r.first->version);

    return result;
}

bool
VDBRepository::do_has_version(const QualifiedPackageName & q,
        const VersionSpec & v) const
{
    Context context("When checking for version '" + stringify(v) + "' in '"
            + stringify(q) + "' in " + stringify(name()) + ":");

    VersionSpecCollection::ConstPointer versions(do_version_specs(q));
    return versions->end() != versions->find(v);
}

VersionMetadata::ConstPointer
VDBRepository::do_version_metadata(
        const QualifiedPackageName & q, const VersionSpec & v) const
{
    Context context("When fetching metadata for '" + stringify(q) +
            "-" + stringify(v) + "':");

    if (! _imp->entries_valid)
        _imp->load_entries();

    std::pair<std::vector<VDBEntry>::iterator, std::vector<VDBEntry>::iterator>
        r(std::equal_range(_imp->entries.begin(), _imp->entries.end(), std::make_pair(
                        q, v), VDBEntry::CompareVersion()));

    if (r.first == r.second)
    {
        Log::get_instance()->message(ll_warning, lc_context, "version lookup failed for request for '" +
                stringify(q) + "-" + stringify(v) + "' in repository '" +
                stringify(name()) + "'");
        return VersionMetadata::ConstPointer(new VersionMetadata(&PortageDepParser::parse_depend));
    }
    else
    {
        if (! r.first->metadata)
            _imp->load_entry(r.first);
        return r.first->metadata;
    }
}

Contents::ConstPointer
VDBRepository::do_contents(
        const QualifiedPackageName & q, const VersionSpec & v) const
{
    Context context("When fetching contents for '" + stringify(q) +
            "-" + stringify(v) + "':");

    if (! _imp->entries_valid)
        _imp->load_entries();

    std::pair<std::vector<VDBEntry>::iterator, std::vector<VDBEntry>::iterator>
        r(std::equal_range(_imp->entries.begin(), _imp->entries.end(), std::make_pair(
                        q, v), VDBEntry::CompareVersion()));

    if (r.first == r.second)
    {
        Log::get_instance()->message(ll_warning, lc_context,
                "version lookup failed for request for '" +
                stringify(q) + "-" + stringify(v) + "' in repository '" +
                stringify(name()) + "'");
        return Contents::ConstPointer(new Contents);
    }

    Contents::Pointer result(new Contents);

    FSEntry f(_imp->location / stringify(q.get<qpn_category>()) /
            (stringify(q.get<qpn_package>()) + "-" + stringify(v)));
    if (! (f / "CONTENTS").is_regular_file())
    {
        Log::get_instance()->message(ll_warning, lc_context,
                "CONTENTS lookup failed for request for '" +
                stringify(q) + "-" + stringify(v) + "' in vdb '" +
                stringify(_imp->location) + "'");
        return result;
    }

    std::ifstream ff(stringify(f / "CONTENTS").c_str());
    if (! ff)
        throw VDBRepositoryKeyReadError("Could not read '" + stringify(f / "CONTENTS") + "'");

    std::string line;
    unsigned line_number(0);
    while (std::getline(ff, line))
    {
        ++line_number;

        std::vector<std::string> tokens;
        WhitespaceTokeniser::get_instance()->tokenise(line, std::back_inserter(tokens));
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
            result->add(ContentsEntry::Pointer(new ContentsFileEntry(tokens.at(1))));
        else if ("dir" == tokens.at(0))
            result->add(ContentsEntry::Pointer(new ContentsDirEntry(tokens.at(1))));
        else if ("misc" == tokens.at(0))
            result->add(ContentsEntry::Pointer(new ContentsMiscEntry(tokens.at(1))));
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

            result->add(ContentsEntry::Pointer(new ContentsSymEntry(
                            tokens.at(1), tokens.at(3))));
        }
    }

    return result;
}

UseFlagState
VDBRepository::do_query_use(const UseFlagName & f,
        const PackageDatabaseEntry * const e) const
{
    if (! _imp->entries_valid)
        _imp->load_entries();

    if (e->get<pde_repository>() == name())
    {

        std::pair<std::vector<VDBEntry>::iterator, std::vector<VDBEntry>::iterator>
            r(std::equal_range(_imp->entries.begin(), _imp->entries.end(), std::make_pair(
                            e->get<pde_name>(), e->get<pde_version>()), VDBEntry::CompareVersion()));

        if (r.first == r.second)
            return use_unspecified;

        if (!r.first->metadata)
            _imp->load_entry(r.first);

        if (r.first->use.end() != r.first->use.find(f))
            return use_enabled;
        else
            return use_disabled;
    }
    else
        return use_unspecified;
}

bool
VDBRepository::do_query_use_mask(const UseFlagName &, const PackageDatabaseEntry *) const
{
    return false;
}

bool
VDBRepository::do_query_use_force(const UseFlagName &, const PackageDatabaseEntry *) const
{
    return false;
}

CountedPtr<Repository>
VDBRepository::make_vdb_repository(
        const Environment * const env,
        const PackageDatabase * const db,
        const std::map<std::string, std::string> & m)
{
    Context context("When making VDB repository from repo_file '" +
            (m.end() == m.find("repo_file") ? std::string("?") : m.find("repo_file")->second) + "':");

    std::string location;
    if (m.end() == m.find("location") || ((location = m.find("location")->second)).empty())
        throw VDBRepositoryConfigurationError("Key 'location' not specified or empty");

    std::string root;
    if (m.end() == m.find("root") || ((root = m.find("root")->second)).empty())
        root = "/";

    std::string world;
    if (m.end() == m.find("world") || ((world = m.find("world")->second)).empty())
        world = location + "/world";

    std::string buildroot;
    if (m.end() == m.find("buildroot") || ((buildroot = m.find("buildroot")->second)).empty())
        buildroot = "/var/tmp/paludis";

    return CountedPtr<Repository>(new VDBRepository(VDBRepositoryParams::create((
                        param<vdbrpk_environment>(env),
                        param<vdbrpk_package_database>(db),
                        param<vdbrpk_location>(location),
                        param<vdbrpk_root>(root),
                        param<vdbrpk_world>(world),
                        param<vdbrpk_buildroot>(buildroot)))));
}

VDBRepositoryConfigurationError::VDBRepositoryConfigurationError(
        const std::string & msg) throw () :
    ConfigurationError("VDB repository configuration error: " + msg)
{
}

VDBRepositoryKeyReadError::VDBRepositoryKeyReadError(
        const std::string & msg) throw () :
    ConfigurationError("VDB repository key read error: " + msg)
{
}

bool
VDBRepository::do_is_arch_flag(const UseFlagName &) const
{
    return false;
}

bool
VDBRepository::do_is_expand_flag(const UseFlagName &) const
{
    return false;
}

bool
VDBRepository::do_is_licence(const std::string &) const
{
    return false;
}

bool
VDBRepository::do_is_mirror(const std::string &) const
{
    return false;
}

void
VDBRepository::do_uninstall(const QualifiedPackageName & q, const VersionSpec & v, const InstallOptions & o) const
{
    Context context("When uninstalling '" + stringify(q) + "-" + stringify(v) +
            "' from '" + stringify(name()) + "':");

    if (! _imp->root.is_directory())
        throw PackageInstallActionError("Couldn't uninstall '" + stringify(q) + "-" +
                stringify(v) + "' because root ('" + stringify(_imp->root) + "') is not a directory");

    VersionMetadata::ConstPointer metadata(0);
    if (! has_version(q, v))
        throw PackageInstallActionError("Couldn't uninstall '" + stringify(q) + "-" +
                stringify(v) + "' because has_version failed");
    else
        metadata = version_metadata(q, v);

    PackageDatabaseEntry e(q, v, name());

    FSEntryCollection::Pointer eclassdirs(new FSEntryCollection);
    eclassdirs->append(FSEntry(_imp->location / stringify(q.get<qpn_category>()) /
                (stringify(q.get<qpn_package>()) + "-" + stringify(v))));

    FSEntry pkg_dir(_imp->location / stringify(q.get<qpn_category>()) /
            (stringify(q.get<qpn_package>()) + "-" + stringify(v)));

    CountedPtr<FSEntry, count_policy::ExternalCountTag> load_env(0);
    if (is_full_env(pkg_dir))
        load_env.assign(new FSEntry(pkg_dir / "environment.bz2"));

    EbuildUninstallCommand uninstall_cmd(EbuildCommandParams::create((
                    param<ecpk_environment>(_imp->env),
                    param<ecpk_db_entry>(&e),
                    param<ecpk_ebuild_dir>(pkg_dir),
                    param<ecpk_files_dir>(pkg_dir),
                    param<ecpk_eclassdirs>(eclassdirs),
                    param<ecpk_portdir>(_imp->location),
                    param<ecpk_distdir>(pkg_dir),
                    param<ecpk_buildroot>(_imp->buildroot)
                    )),
            EbuildUninstallCommandParams::create((
                    param<ecupk_root>(stringify(_imp->root) + "/"),
                    param<ecupk_disable_cfgpro>(o.get<io_noconfigprotect>()),
                    param<ecupk_unmerge_only>(! metadata->get_ebuild_interface()->get<evm_virtual>().empty()),
                    param<ecupk_load_environment>(load_env.raw_pointer())
                    )));

    uninstall_cmd();
}

DepAtom::Pointer
VDBRepository::do_package_set(const std::string & s, const PackageSetOptions &) const
{
    Context context("When fetching package set '" + s + "' from '" +
            stringify(name()) + "':");

    if ("everything" == s)
    {
        AllDepAtom::Pointer result(new AllDepAtom);
        GeneralSetDepTag::Pointer tag(new GeneralSetDepTag("everything"));

        if (! _imp->entries_valid)
            _imp->load_entries();

        for (std::vector<VDBEntry>::const_iterator p(_imp->entries.begin()),
                p_end(_imp->entries.end()) ; p != p_end ; ++p)
        {
            PackageDepAtom::Pointer atom(new PackageDepAtom(p->name));
            atom->set_tag(tag);
            result->add_child(atom);
        }

        return result;
    }
    else if ("world" == s)
    {
        AllDepAtom::Pointer result(new AllDepAtom);
        GeneralSetDepTag::Pointer tag(new GeneralSetDepTag("world"));

        if (_imp->world_file.exists())
        {
            LineConfigFile world(_imp->world_file);

            for (LineConfigFile::Iterator line(world.begin()), line_end(world.end()) ;
                    line != line_end ; ++line)
            {
                PackageDepAtom::Pointer atom(new PackageDepAtom(QualifiedPackageName(*line)));
                atom->set_tag(tag);
                result->add_child(atom);
            }
        }
        else
            Log::get_instance()->message(ll_warning, lc_no_context,
                    "World file '" + stringify(_imp->world_file) +
                    "' doesn't exist");

        return result;
    }
    else
        return DepAtom::Pointer(0);
}

void
VDBRepository::invalidate() const
{
    _imp->invalidate();
}

Repository::ProvideMapIterator
VDBRepository::begin_provide_map() const
{
    if (! _imp->has_provide_map)
    {
        Context context("When loading VDB PROVIDEs map:");

        Log::get_instance()->message(ll_debug, lc_no_context, "Starting VDB PROVIDEs map creation");

        if (! _imp->entries_valid)
            _imp->load_entries();

        for (std::vector<VDBEntry>::iterator e(_imp->entries.begin()),
                e_end(_imp->entries.end()) ; e != e_end ; ++e)
        {
            Context loop_context("When loading VDB PROVIDEs entry for '"
                    + stringify(e->name) + "-" + stringify(e->version) + "':");

            try
            {
                if (! e->metadata)
                    _imp->load_entry(e);
                const std::string provide_str(e->metadata->get_ebuild_interface()->get<evm_provide>());
                if (provide_str.empty())
                    continue;

                DepAtom::ConstPointer provide(PortageDepParser::parse(provide_str,
                            PortageDepParserPolicy<PackageDepAtom, false>::get_instance()));
                PackageDatabaseEntry dbe(e->name, e->version, name());
                DepAtomFlattener f(_imp->env, &dbe, provide);

                for (DepAtomFlattener::Iterator p(f.begin()), p_end(f.end()) ; p != p_end ; ++p)
                {
                    QualifiedPackageName pp((*p)->text());

                    if (pp.get<qpn_category>() != CategoryNamePart("virtual"))
                        Log::get_instance()->message(ll_warning, lc_no_context, "PROVIDE of non-virtual '"
                                + stringify(pp) + "' from '" + stringify(e->name) + "-"
                                + stringify(e->version) + "' in '" + stringify(name())
                                + "' will not work as expected");

                    _imp->provide_map.insert(std::make_pair(pp, e->name));
                }
            }
            catch (const InternalError &)
            {
                throw;
            }
            catch (const Exception & ee)
            {
                Log::get_instance()->message(ll_warning, lc_no_context, "Skipping VDB PROVIDE entry for '"
                        + stringify(e->name) + "-" + stringify(e->version) + "' due to exception '"
                        + stringify(ee.message()) + "' (" + stringify(ee.what()) + ")");
            }
        }

        Log::get_instance()->message(ll_debug, lc_no_context, "Done VDB PROVIDEs map creation");

        _imp->has_provide_map = true;
    }

    return _imp->provide_map.begin();
}

Repository::ProvideMapIterator
VDBRepository::end_provide_map() const
{
    return _imp->provide_map.end();
}

void
VDBRepository::add_to_world(const QualifiedPackageName & n) const
{
    bool found(false);

    if (_imp->world_file.exists())
    {
        LineConfigFile world(_imp->world_file);

        for (LineConfigFile::Iterator line(world.begin()), line_end(world.end()) ;
                line != line_end ; ++line)
            if (QualifiedPackageName(*line) == n)
            {
                found = true;
                break;
            }
    }

    if (! found)
    {
        std::ofstream world(stringify(_imp->world_file).c_str(), std::ios::out | std::ios::app);
        if (! world)
            Log::get_instance()->message(ll_warning, lc_no_context, "Cannot append to world file '"
                    + stringify(_imp->world_file) + "', skipping world update");
        else
            world << n << std::endl;
    }
}

void
VDBRepository::remove_from_world(const QualifiedPackageName & n) const
{
    std::list<std::string> world_lines;

    if (_imp->world_file.exists())
    {
        std::ifstream world_file(stringify(_imp->world_file).c_str());

        if (! world_file)
        {
            Log::get_instance()->message(ll_warning, lc_no_context, "Cannot read world file '"
                    + stringify(_imp->world_file) + "', skipping world update");
            return;
        }

        std::string line;
        while (std::getline(world_file, line))
        {
            if (strip_leading(strip_trailing(line, " \t"), "\t") != stringify(n))
                world_lines.push_back(line);
            else
                Log::get_instance()->message(ll_debug, lc_no_context, "Removing line '"
                            + line + "' from world file '" + stringify(_imp->world_file));
        }
    }

    std::ofstream world_file(stringify(_imp->world_file).c_str());

    if (! world_file)
    {
        Log::get_instance()->message(ll_warning, lc_no_context, "Cannot write world file '"
                + stringify(_imp->world_file) + "', skipping world update");
        return;
    }

    std::copy(world_lines.begin(), world_lines.end(),
            std::ostream_iterator<std::string>(world_file, "\n"));
}

std::string
VDBRepository::get_environment_variable(
        const PackageDatabaseEntry & for_package,
        const std::string & var) const
{
    Context context("When fetching environment variable '" + var + "' for '" +
            stringify(for_package) + "':");

    FSEntry vdb_dir(_imp->location / stringify(for_package.get<pde_name>().get<qpn_category>())
            / (stringify(for_package.get<pde_name>().get<qpn_package>()) + "-" +
                stringify(for_package.get<pde_version>())));

    if (! vdb_dir.is_directory())
        throw EnvironmentVariableActionError("Could not find VDB entry for '"
                + stringify(for_package) + "'");

    if ((vdb_dir / var).is_regular_file())
    {
        std::ifstream f(stringify(vdb_dir / var).c_str());
        if (! f)
            throw EnvironmentVariableActionError("Could not read '" +
                    stringify(vdb_dir / var) + "'");
        return strip_trailing_string(
                std::string((std::istreambuf_iterator<char>(f)),
                    std::istreambuf_iterator<char>()), "\n");
    }
    else if ((vdb_dir / "environment.bz2").is_regular_file())
    {
        if (is_full_env(vdb_dir))
        {
            PStream p("bash -c '( bunzip2 < " + stringify(vdb_dir / "environment.bz2" ) +
                    " ; echo echo \\$" + var + " ) | bash 2>/dev/null'");
            std::string result(strip_trailing_string(std::string(
                            (std::istreambuf_iterator<char>(p)),
                            std::istreambuf_iterator<char>()), "\n"));
            if (0 != p.exit_status())
                throw EnvironmentVariableActionError("Could not load environment.bz2");
            return result;
        }
        else
        {
            PStream p("bunzip2 < " + stringify(vdb_dir / "environment.bz2" ));
            KeyValueConfigFile k(&p);

            if (0 != p.exit_status())
                throw EnvironmentVariableActionError("Could not get variable '" + var +
                        "' from environment.bz2 for '" + stringify(for_package) + "'");

            return k.get(var);
        }
    }
    else
        throw EnvironmentVariableActionError("Could not get variable '" + var + "' for '"
                + stringify(for_package) + "'");
}

