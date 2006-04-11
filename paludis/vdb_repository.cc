/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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
#include <paludis/dep_parser.hh>
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

using namespace paludis;

namespace
{
    struct VDBEntry
    {
        QualifiedPackageName name;
        VersionSpec version;
        VersionMetadata::Pointer metadata;

        VDBEntry(const QualifiedPackageName & n, const VersionSpec & v) :
            name(n),
            version(v),
            metadata(0)
        {
        }

        bool operator< (const VDBEntry & other) const
        {
            if (name < other.name)
                return true;
            if (name > other.name)
                return false;
            if (version < other.version)
                return true;
            if (version > other.version)
                return false;
        }

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
}

namespace paludis
{
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

        /// Do we have entries loaded?
        mutable bool entries_valid;

        /// Our entries.
        mutable std::vector<VDBEntry> entries;

        /// Load entries.
        void load_entries() const;

        /// Load metadata for one entry.
        void load_entry(std::vector<VDBEntry>::iterator) const;

        /// Constructor.
        Implementation(const Environment * const,
                const PackageDatabase * const d, const FSEntry & l,
                const FSEntry & r);

        /// Destructor.
        ~Implementation();
    };
}

Implementation<VDBRepository>::Implementation(const Environment * const env,
        const PackageDatabase * const d,
        const FSEntry & l, const FSEntry & r) :
    db(d),
    env(env),
    location(l),
    root(r),
    entries_valid(false)
{
}

Implementation<VDBRepository>::~Implementation()
{
}

void
Implementation<VDBRepository>::load_entries() const
{
    entries.clear();
    entries_valid = true;
    try
    {
        for (DirIterator cat_i(location), cat_iend ; cat_i != cat_iend ; ++cat_i)
        {
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

namespace
{
    std::string
    file_contents(const FSEntry & location, const QualifiedPackageName & name,
            const VersionSpec & v, const std::string & key)
    {
        FSEntry f(location / stringify(name.get<qpn_category>()) /
                (stringify(name.get<qpn_package>()) + "-" + stringify(v)));
        if (! (f / key).is_regular_file())
        {
            Log::get_instance()->message(ll_warning, "metadata key lookup failed for request for '" +
                    stringify(name) + "-" + stringify(v) + "' in vdb '" +
                    stringify(location) + "' for key '" + key + "'");
            return "";
        }

        std::ifstream ff(stringify(f / key).c_str());
        if (! ff)
            throw InternalError(PALUDIS_HERE, "TODO reading " + stringify(location) + " name " +
                    stringify(name) + " version " + stringify(v) + " key " + key); /// \todo
        return strip_leading(strip_trailing(std::string((std::istreambuf_iterator<char>(ff)),
                        std::istreambuf_iterator<char>()), " \t\n"), " \t\n");
    }
}

void
Implementation<VDBRepository>::load_entry(std::vector<VDBEntry>::iterator p) const
{
    p->metadata = VersionMetadata::Pointer(new VersionMetadata);
    p->metadata->set(vmk_depend,    file_contents(location, p->name, p->version, "DEPEND"));
    p->metadata->set(vmk_rdepend,   file_contents(location, p->name, p->version, "RDEPEND"));
    p->metadata->set(vmk_license,   file_contents(location, p->name, p->version, "LICENSE"));
    p->metadata->set(vmk_keywords,  "*");
    p->metadata->set(vmk_inherited, file_contents(location, p->name, p->version, "INHERITED"));
    p->metadata->set(vmk_iuse,      file_contents(location, p->name, p->version, "IUSE"));
    p->metadata->set(vmk_pdepend,   file_contents(location, p->name, p->version, "PDEPEND"));
    p->metadata->set(vmk_provide,   file_contents(location, p->name, p->version, "PROVIDE"));
    p->metadata->set(vmk_eapi,      file_contents(location, p->name, p->version, "EAPI"));

    std::string slot(file_contents(location, p->name, p->version, "SLOT"));
    if (slot.empty())
    {
        Log::get_instance()->message(ll_warning, "VDBRepository entry '" +
                stringify(p->name) + "-" + stringify(p->version) + "' in '" +
                stringify(location) + "' has empty SLOT, setting to \"0\"");
        slot = "0";
    }
    p->metadata->set(vmk_slot,      slot);
}

VDBRepository::VDBRepository(
        const Environment * const e, const PackageDatabase * const d,
        const FSEntry & location, const FSEntry & root) :
    Repository(RepositoryName("installed")),
    PrivateImplementationPattern<VDBRepository>(new Implementation<VDBRepository>(e,
                d, location, root))
{
    _info.insert(std::make_pair(std::string("location"), location));
    _info.insert(std::make_pair(std::string("root"), root));
    _info.insert(std::make_pair(std::string("format"), std::string("vdb")));
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
VDBRepository::do_has_package_named(const CategoryNamePart & c,
        const PackageNamePart & p) const
{
    Context context("When checking for package '" + stringify(c) + "/"
            + stringify(p) + "' in " + stringify(name()) + ":");

    if (! _imp->entries_valid)
        _imp->load_entries();

    std::pair<std::vector<VDBEntry>::const_iterator, std::vector<VDBEntry>::const_iterator>
        r(std::equal_range(_imp->entries.begin(), _imp->entries.end(), QualifiedPackageName(c, p),
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
VDBRepository::do_has_version(const CategoryNamePart & c,
        const PackageNamePart & p, const VersionSpec & v) const
{
    Context context("When checking for version '" + stringify(v) + "' in '"
            + stringify(c) + "/" + stringify(p) + "' in " + stringify(name()) + ":");

    VersionSpecCollection::ConstPointer versions(do_version_specs(QualifiedPackageName(c, p)));
    return versions->end() != versions->find(v);
}

VersionMetadata::ConstPointer
VDBRepository::do_version_metadata(
        const CategoryNamePart & c, const PackageNamePart & p, const VersionSpec & v) const
{
    Context context("When fetching metadata for " + stringify(c) + "/" + stringify(p) +
            "-" + stringify(v));

    std::pair<std::vector<VDBEntry>::iterator, std::vector<VDBEntry>::iterator>
        r(std::equal_range(_imp->entries.begin(), _imp->entries.end(), std::make_pair(
                        QualifiedPackageName(c, p), v), VDBEntry::CompareVersion()));

    if (r.first == r.second)
    {
        Log::get_instance()->message(ll_warning, "version lookup failed for request for '" +
                stringify(c) + "/" + stringify(p) + "-" + stringify(v) + "' in repository '" +
                stringify(name()) + "'");
        return VersionMetadata::ConstPointer(new VersionMetadata);
    }
    else
    {
        if (! r.first->metadata)
            _imp->load_entry(r.first);
        return r.first->metadata;
    }
}

bool
VDBRepository::do_query_repository_masks(const CategoryNamePart &,
        const PackageNamePart &, const VersionSpec &) const
{
    return false;
}

bool
VDBRepository::do_query_profile_masks(const CategoryNamePart &,
        const PackageNamePart &, const VersionSpec &) const
{
    return false;
}

UseFlagState
VDBRepository::do_query_use(const UseFlagName &) const
{
    return use_unspecified;
}

bool
VDBRepository::do_query_use_mask(const UseFlagName &) const
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

    return CountedPtr<Repository>(new VDBRepository(env, db, location, root));
}

VDBRepositoryConfigurationError::VDBRepositoryConfigurationError(
        const std::string & msg) throw () :
    ConfigurationError("VDB repository configuration error: " + msg)
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
VDBRepository::do_install(const QualifiedPackageName &, const VersionSpec &) const
{
    throw PackageInstallActionError("PortageRepository doesn't support do_install");
}

void
VDBRepository::do_uninstall(const QualifiedPackageName & q, const VersionSpec & v) const
{
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
    std::string actions;
    if (metadata->get(vmk_virtual).empty())
        actions = "prerm unmerge postrm";
    else
        actions = "unmerge";

    std::string cmd(make_env_command(
                getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis") +
                "/ebuild.bash '" +
                stringify(_imp->location) + "/" +
                stringify(q.get<qpn_category>()) + "/" +
                stringify(q.get<qpn_package>()) + "-" + stringify(v) + "/" +
                stringify(q.get<qpn_package>()) + "-" + stringify(v) + ".ebuild' " + actions)
            ("P", stringify(q.get<qpn_package>()) + "-" + stringify(v.remove_revision()))
            ("PV", stringify(v.remove_revision()))
            ("PR", v.revision_only())
            ("PN", stringify(q.get<qpn_package>()))
            ("PVR", stringify(v.remove_revision()) + "-" + v.revision_only())
            ("PF", stringify(q.get<qpn_package>()) + "-" + stringify(v))
            ("CATEGORY", stringify(q.get<qpn_category>()))
            ("ECLASSDIR", stringify(_imp->location) + "/" +
                stringify(q.get<qpn_category>()) + "/" +
                stringify(q.get<qpn_package>()) + "-" + stringify(v) + "/")
            ("ROOT", stringify(_imp->root) + "/")
            ("PALUDIS_TMPDIR", BIGTEMPDIR "/paludis/")
            ("PALUDIS_CONFIG_DIR", SYSCONFDIR "/paludis/")
            ("PALUDIS_BASHRC_FILES", _imp->env->bashrc_files())
            ("PALUDIS_COMMAND", _imp->env->paludis_command())
            ("KV", kernel_version())
            ("PALUDIS_EBUILD_LOG_LEVEL", Log::get_instance()->log_level_string())
            ("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis")));

    if (0 != run_command(cmd))
        throw PackageInstallActionError("Couldn't uninstall '" + stringify(q) + "-" +
                stringify(v) + "'");
}

DepAtom::Pointer
VDBRepository::do_package_set(const std::string & s) const
{
    AllDepAtom::Pointer result(new AllDepAtom);

    if ("everything" == s)
    {
        if (! _imp->entries_valid)
            _imp->load_entries();

        for (std::vector<VDBEntry>::const_iterator p(_imp->entries.begin()),
                p_end(_imp->entries.end()) ; p != p_end ; ++p)
            result->add_child(PackageDepAtom::Pointer(new PackageDepAtom(p->name)));
    }

    return result;
}

bool
VDBRepository::do_sync() const
{
    return false;
}

