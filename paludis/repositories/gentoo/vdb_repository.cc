/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/repositories/gentoo/vdb_repository.hh>
#include <paludis/repositories/gentoo/vdb_version_metadata.hh>
#include <paludis/repositories/gentoo/vdb_merger.hh>
#include <paludis/repositories/gentoo/vdb_unmerger.hh>

#include <paludis/dep_spec.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/repositories/gentoo/ebuild.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/config_file.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/repository_name_cache.hh>

#include <paludis/util/collection_concrete.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fast_unique_copy.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/log.hh>
#include <paludis/util/pstream.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>

#include <fstream>
#include <functional>
#include <algorithm>
#include <vector>
#include <list>

/** \file
 * Implementation for VDBRepository.
 *
 * \ingroup grpvdbrepository
 */

using namespace paludis;

#include <paludis/repositories/gentoo/vdb_repository-sr.cc>

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
        std::tr1::shared_ptr<VDBVersionMetadata> metadata;

        /// Our built USE flags.
        std::set<UseFlagName> use;

        /// Our installed date.
        time_t installed_time;

        /// Constructor
        VDBEntry(const QualifiedPackageName & n, const VersionSpec & v) :
            name(n),
            version(v),
            installed_time(0)
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
                return c < e.name.category;
            }

            bool operator() (const VDBEntry & e, const CategoryNamePart & c) const
            {
                return e.name.category < c;
            }

            bool operator() (const VDBEntry & e, const VDBEntry & c) const
            {
                return e.name.category < c.name.category;
            }
        };

        /**
         * Extract category from a VDBEntry.
         *
         * \ingroup grpvdbrepository
         */
        struct ExtractCategory
        {
            CategoryNamePart operator() (const VDBEntry & e) const
            {
                return e.name.category;
            }
        };

        /**
         * Extract package from a VDBEntry.
         *
         * \ingroup grpvdbrepository
         */
        struct ExtractPackage
        {
            QualifiedPackageName operator() (const VDBEntry & e) const
            {
                return e.name;
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

            bool operator() (const VDBEntry & e, const VDBEntry & c) const
            {
                return e.name < c.name;
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

        FSEntry f(location / stringify(name.category) /
                (stringify(name.package) + "-" + stringify(v)));
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


namespace paludis
{
    /**
     * Implementation data for VDBRepository.
     *
     * \ingroup grpvdbrepository
     */
    template <>
    struct Implementation<VDBRepository>
    {
        VDBRepositoryParams params;

        /// Our owning env.
        Environment * const env;

        /// Our base location.
        FSEntry location;

        /// Root location
        FSEntry root;

        /// Build root
        FSEntry buildroot;

        /// World file
        FSEntry world_file;

        /// Provides cache
        FSEntry provides_cache;

        /// Do we have entries loaded?
        mutable bool entries_valid;

        /// Do we have category entries loaded?
        mutable MakeHashedSet<CategoryNamePart>::Type category_entries_valid;

        /// Our entries, keep this sorted!
        mutable std::vector<VDBEntry> entries;

        /// Load entries.
        void load_entries() const;
        void load_entries_for(const CategoryNamePart &) const;

        /// Load metadata for one entry.
        void load_entry(std::vector<VDBEntry>::iterator) const;

        /// Provieds data
        mutable std::tr1::shared_ptr<RepositoryProvidesInterface::ProvidesCollection> provides;

        const FSEntry names_cache_dir;

        std::tr1::shared_ptr<RepositoryNameCache> names_cache;

        /// Constructor.
        Implementation(const VDBRepository * const, const VDBRepositoryParams &);

        /// Destructor.
        ~Implementation();

        /// Invalidate.
        void invalidate() const;
    };

    Implementation<VDBRepository>::Implementation(const VDBRepository * const r,
            const VDBRepositoryParams & p) :
        params(p),
        env(p.environment),
        location(p.location),
        root(p.root),
        buildroot(p.buildroot),
        world_file(p.world),
        provides_cache(p.provides_cache),
        entries_valid(false),
        names_cache_dir(p.names_cache),
        names_cache(new RepositoryNameCache(names_cache_dir, r))
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

        Log::get_instance()->message(ll_debug, lc_context, "VDB load entries started");

        entries.clear();
        category_entries_valid.clear();
        entries_valid = true;
        try
        {
            for (DirIterator cat_i(location), cat_iend ; cat_i != cat_iend ; ++cat_i)
                load_entries_for(CategoryNamePart(cat_i->basename()));

            std::sort(entries.begin(), entries.end());
        }
        catch (...)
        {
            entries_valid = false;
            throw;
        }

        Log::get_instance()->message(ll_debug, lc_context, "VDB load entries done");
    }

    void
    Implementation<VDBRepository>::load_entries_for(const CategoryNamePart & cat) const
    {
        MakeHashedSet<CategoryNamePart>::Type::const_iterator i(category_entries_valid.find(cat));
        if (i != category_entries_valid.end())
            return;

        Context context("When loading VDBRepository entries for '" + stringify(cat) + "' from '" +
                stringify(location) + "':");

        Log::get_instance()->message(ll_debug, lc_context, "VDB load entries for '" +
                stringify(cat) + "' started");

        try
        {
            category_entries_valid.insert(cat);

            FSEntry dir(location / stringify(cat));
            if (! dir.is_directory())
                return;

            for (DirIterator pkg_i(dir), pkg_iend ; pkg_i != pkg_iend ; ++pkg_i)
            {
                if (! pkg_i->is_directory())
                    continue;

                if ('-' == pkg_i->basename().at(0))
                    continue;

                try
                {
                    PackageDepSpec spec("=" + stringify(cat) + "/" + pkg_i->basename(), pds_pm_permissive);
                    entries.push_back(VDBEntry(*spec.package_ptr(),
                                spec.version_requirements_ptr()->begin()->version_spec));
                }
                catch (const Exception & e)
                {
                    Log::get_instance()->message(ll_warning, lc_context, "Ignoring VDB entry '"
                            + stringify(*pkg_i) + "' due to exception '" + stringify(e.message()) + "' ("
                            + e.what() + ")");
                }
            }

            std::sort(entries.begin(), entries.end());
        }
        catch (...)
        {
            category_entries_valid.erase(cat);
            throw;
        }
    }

    void
    Implementation<VDBRepository>::invalidate() const
    {
        entries_valid = false;
        entries.clear();
        category_entries_valid.clear();
    }

    void
    Implementation<VDBRepository>::load_entry(std::vector<VDBEntry>::iterator p) const
    {
        Context context("When loading VDBRepository entry for '" + stringify(p->name)
                + "-" + stringify(p->version) + "' from '" + stringify(location) + "':");


        p->metadata = std::tr1::shared_ptr<VDBVersionMetadata>(new VDBVersionMetadata);

        {
            Context local_context("When loading key 'DEPEND':");
            p->metadata->build_depend_string = file_contents(location, p->name, p->version, "DEPEND");
        }
        {
            Context local_context("When loading key 'RDEPEND':");
            p->metadata->run_depend_string = file_contents(location, p->name, p->version, "RDEPEND");
        }
        {
            Context local_context("When loading key 'LICENSE':");
            p->metadata->license_string = file_contents(location, p->name, p->version, "LICENSE");
        }
        p->metadata->keywords = "*";
        {
            Context local_context("When loading key 'INHERITED':");
            p->metadata->inherited = file_contents(location, p->name, p->version, "INHERITED");
        }
        {
            Context local_context("When loading key 'IUSE':");
            p->metadata->iuse = file_contents(location, p->name, p->version, "IUSE");
        }
        {
            Context local_context("When loading key 'PDEPEND':");
            p->metadata->post_depend_string = file_contents(location, p->name, p->version, "PDEPEND");
        }
        {
            Context local_context("When loading key 'PROVIDE':");
            p->metadata->provide_string = file_contents(location, p->name, p->version, "PROVIDE");
        }
        {
            Context local_context("When loading key 'SRC_URI':");
            p->metadata->src_uri_string = file_contents(location, p->name, p->version, "SRC_URI");
        }
        {
            Context local_context("When loading key 'EAPI':");
            p->metadata->eapi = file_contents(location, p->name, p->version, "EAPI");
        }
        {
            Context local_context("When loading key 'HOMEPAGE':");
            p->metadata->homepage = file_contents(location, p->name, p->version, "HOMEPAGE");
        }
        {
            Context local_context("When loading key 'DESCRIPTION':");
            p->metadata->description = file_contents(location, p->name, p->version, "DESCRIPTION");
        }

        {
            Context local_context("When loading key 'SLOT':");
            std::string slot(file_contents(location, p->name, p->version, "SLOT"));

            if (slot.empty())
            {
                Log::get_instance()->message(ll_warning, lc_no_context, "VDBRepository entry '" +
                        stringify(p->name) + "-" + stringify(p->version) + "' in '" +
                        stringify(location) + "' has empty SLOT, setting to \"0\"");
                slot = "0";
            }
            p->metadata->slot = SlotName(slot);
        }

        {
            Context local_context("When loading key 'REPOSITORY':");
            std::string repo(file_contents(location, p->name, p->version, "REPOSITORY"));
            if (! repo.empty())
                p->metadata->source.reset(new PackageDatabaseEntry(p->name, p->version,
                            RepositoryName(repo)));
        }

        {
            Context local_context("When loading key 'USE':");
            std::string raw_use(file_contents(location, p->name, p->version, "USE"));
            p->use.clear();
            WhitespaceTokeniser::get_instance()->tokenise(raw_use,
                    filter_inserter(create_inserter<UseFlagName>(
                            std::inserter(p->use, p->use.begin())), IsPositiveFlag()));
        }
    }
}

VDBRepository::VDBRepository(const VDBRepositoryParams & p) :
    Repository(RepositoryName("installed"),
            RepositoryCapabilities::create()
            .installable_interface(0)
            .installed_interface(this)
            .mask_interface(0)
            .news_interface(0)
            .sets_interface(this)
            .syncable_interface(0)
            .uninstallable_interface(this)
            .use_interface(this)
            .world_interface(this)
            .environment_variable_interface(this)
            .mirrors_interface(0)
            .provides_interface(this)
            .virtuals_interface(0)
            .destination_interface(this)
            .config_interface(this)
            .contents_interface(this)
            .licenses_interface(0)
            .portage_interface(0),
            "vdb"),
    PrivateImplementationPattern<VDBRepository>(new Implementation<VDBRepository>(this, p))
{
    std::tr1::shared_ptr<RepositoryInfoSection> config_info(new RepositoryInfoSection("Configuration information"));

    config_info->add_kv("location", stringify(_imp->location));
    config_info->add_kv("root", stringify(_imp->root));
    config_info->add_kv("format", "vdb");
    config_info->add_kv("world", stringify(_imp->world_file));
    config_info->add_kv("provides_cache", stringify(_imp->provides_cache));
    config_info->add_kv("names_cache", stringify(_imp->names_cache_dir));
    config_info->add_kv("buildroot", stringify(_imp->buildroot));

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
        _imp->load_entries_for(q.category);

    std::pair<std::vector<VDBEntry>::const_iterator, std::vector<VDBEntry>::const_iterator>
        r(std::equal_range(_imp->entries.begin(), _imp->entries.end(), q,
                    VDBEntry::ComparePackage()));
    return r.first != r.second;
}

std::tr1::shared_ptr<const CategoryNamePartCollection>
VDBRepository::do_category_names() const
{
    Context context("When fetching category names in " + stringify(name()) + ":");

    if (! _imp->entries_valid)
        _imp->load_entries();

    std::tr1::shared_ptr<CategoryNamePartCollection> result(new CategoryNamePartCollection::Concrete);

#if 0
    for (std::vector<VDBEntry>::const_iterator c(_imp->entries.begin()), c_end(_imp->entries.end()) ;
            c != c_end ; ++c)
        result->insert(c->name.category);
#else
    fast_unique_copy(_imp->entries.begin(), _imp->entries.end(),
            transform_inserter(result->inserter(), VDBEntry::ExtractCategory()),
            VDBEntry::CompareCategory());
#endif

    return result;
}

std::tr1::shared_ptr<const QualifiedPackageNameCollection>
VDBRepository::do_package_names(const CategoryNamePart & c) const
{
    /* this isn't particularly fast because it isn't called very often. avoid
     * changing the data structures used to make this faster at the expense of
     * slowing down single item queries. */

    Context context("When fetching package names in category '" + stringify(c)
            + "' in " + stringify(name()) + ":");

    std::tr1::shared_ptr<QualifiedPackageNameCollection> result(new QualifiedPackageNameCollection::Concrete);

    std::pair<std::vector<VDBEntry>::const_iterator, std::vector<VDBEntry>::const_iterator>
        r(std::equal_range(_imp->entries.begin(), _imp->entries.end(), c,
                    VDBEntry::CompareCategory()));
#if 0
    for ( ; r.first != r.second ; ++(r.first))
        result->insert(r.first->name);
#endif
    fast_unique_copy(r.first, r.second,
            transform_inserter(result->inserter(), VDBEntry::ExtractPackage()),
            VDBEntry::ComparePackage());

    return result;
}

std::tr1::shared_ptr<const VersionSpecCollection>
VDBRepository::do_version_specs(const QualifiedPackageName & n) const
{
    Context context("When fetching versions of '" + stringify(n) + "' in "
            + stringify(name()) + ":");

    if (! _imp->entries_valid)
        _imp->load_entries_for(n.category);

    std::tr1::shared_ptr<VersionSpecCollection> result(new VersionSpecCollection::Concrete);

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

    std::tr1::shared_ptr<const VersionSpecCollection> versions(do_version_specs(q));
    return versions->end() != versions->find(v);
}

std::tr1::shared_ptr<const VersionMetadata>
VDBRepository::do_version_metadata(
        const QualifiedPackageName & q, const VersionSpec & v) const
{
    Context context("When fetching metadata for '" + stringify(q) +
            "-" + stringify(v) + "':");

    if (! _imp->entries_valid)
        _imp->load_entries_for(q.category);

    std::pair<std::vector<VDBEntry>::iterator, std::vector<VDBEntry>::iterator>
        r(std::equal_range(_imp->entries.begin(), _imp->entries.end(), std::make_pair(
                        q, v), VDBEntry::CompareVersion()));

    if (r.first == r.second)
        throw NoSuchPackageError(stringify(PackageDatabaseEntry(q, v, name())));
    else
    {
        if (! r.first->metadata)
            _imp->load_entry(r.first);
        return r.first->metadata;
    }
}

std::tr1::shared_ptr<const Contents>
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
        return std::tr1::shared_ptr<const Contents>(new Contents);
    }

    std::tr1::shared_ptr<Contents> result(new Contents);

    FSEntry f(_imp->location / stringify(q.category) /
            (stringify(q.package) + "-" + stringify(v)));
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
            result->add(std::tr1::shared_ptr<ContentsEntry>(new ContentsFileEntry(tokens.at(1))));
        else if ("dir" == tokens.at(0))
            result->add(std::tr1::shared_ptr<ContentsEntry>(new ContentsDirEntry(tokens.at(1))));
        else if ("misc" == tokens.at(0))
            result->add(std::tr1::shared_ptr<ContentsEntry>(new ContentsMiscEntry(tokens.at(1))));
        else if ("fif" == tokens.at(0))
            result->add(std::tr1::shared_ptr<ContentsEntry>(new ContentsFifoEntry(tokens.at(1))));
        else if ("dev" == tokens.at(0))
            result->add(std::tr1::shared_ptr<ContentsEntry>(new ContentsDevEntry(tokens.at(1))));
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

            result->add(std::tr1::shared_ptr<ContentsEntry>(new ContentsSymEntry(
                            tokens.at(1), tokens.at(3))));
        }
    }

    return result;
}

time_t
VDBRepository::do_installed_time(const QualifiedPackageName & q,
        const VersionSpec & v) const
{
    Context context("When finding installed time for '" + stringify(q) +
            "-" + stringify(v) + "':");

    if (! _imp->entries_valid)
        _imp->load_entries_for(q.category);

    std::pair<std::vector<VDBEntry>::iterator, std::vector<VDBEntry>::iterator>
        r(std::equal_range(_imp->entries.begin(), _imp->entries.end(), std::make_pair(
                        q, v), VDBEntry::CompareVersion()));

    if (r.first == r.second)
        throw NoSuchPackageError(stringify(PackageDatabaseEntry(q, v, name())));
    else
    {
        if (0 == r.first->installed_time)
        {
            FSEntry f(_imp->location / stringify(q.category) / (stringify(q.package) + "-"
                        + stringify(v)) / "CONTENTS");
            try
            {
                r.first->installed_time = f.ctime();
            }
            catch (const FSError & e)
            {
                Log::get_instance()->message(ll_warning, lc_no_context, "Can't get ctime of '"
                        + stringify(f) + "' due to exception '" + e.message() + "' (" + e.what()
                        + ")");
                r.first->installed_time = 1;
            }
        }
        return r.first->installed_time;
    }
}

UseFlagState
VDBRepository::do_query_use(const UseFlagName & f,
        const PackageDatabaseEntry * const e) const
{
    if (e && e->repository == name())
    {
        if (! _imp->entries_valid)
            _imp->load_entries_for(e->name.category);

        std::pair<std::vector<VDBEntry>::iterator, std::vector<VDBEntry>::iterator>
            r(std::equal_range(_imp->entries.begin(), _imp->entries.end(), std::make_pair(
                            e->name, e->version), VDBEntry::CompareVersion()));

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
VDBRepository::do_query_use_mask(const UseFlagName & u, const PackageDatabaseEntry * e) const
{
    return use_disabled == do_query_use(u, e);
}

bool
VDBRepository::do_query_use_force(const UseFlagName & u, const PackageDatabaseEntry * e) const
{
    return use_enabled == do_query_use(u, e);
}

std::tr1::shared_ptr<Repository>
VDBRepository::make_vdb_repository(
        Environment * const env,
        std::tr1::shared_ptr<const AssociativeCollection<std::string, std::string> > m)
{
    std::string repo_file(m->end() == m->find("repo_file") ? std::string("?") : m->find("repo_file")->second);
    Context context("When making VDB repository from repo_file '" + repo_file + "':");

    std::string location;
    if (m->end() == m->find("location") || ((location = m->find("location")->second)).empty())
        throw VDBRepositoryConfigurationError("Key 'location' not specified or empty");

    std::string root;
    if (m->end() == m->find("root") || ((root = m->find("root")->second)).empty())
        root = "/";

    std::string world;
    if (m->end() == m->find("world") || ((world = m->find("world")->second)).empty())
        world = location + "/world";

    std::string provides_cache;
    if (m->end() == m->find("provides_cache") || ((provides_cache = m->find("provides_cache")->second)).empty())
    {
        Log::get_instance()->message(ll_warning, lc_no_context, "The provides_cache key is not set in '"
                + repo_file + "'. You should read http://paludis.pioto.org/cachefiles.html and select an "
                "appropriate value.");
        provides_cache = "/var/empty";
    }

    std::string names_cache;
    if (m->end() == m->find("names_cache") || ((names_cache = m->find("names_cache")->second)).empty())
    {
        Log::get_instance()->message(ll_warning, lc_no_context, "The names_cache key is not set in '"
                + repo_file + "'. You should read http://paludis.pioto.org/cachefiles.html and select an "
                "appropriate value.");
        names_cache = "/var/empty";
    }

    std::string buildroot;
    if (m->end() == m->find("buildroot") || ((buildroot = m->find("buildroot")->second)).empty())
        buildroot = "/var/tmp/paludis";

    return std::tr1::shared_ptr<Repository>(new VDBRepository(VDBRepositoryParams::create()
                .environment(env)
                .location(location)
                .root(root)
                .world(world)
                .buildroot(buildroot)
                .provides_cache(provides_cache)
                .names_cache(names_cache)));
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

void
VDBRepository::do_uninstall(const QualifiedPackageName & q, const VersionSpec & v, const UninstallOptions & o) const
{
    _uninstall(q, v, o, false);
}

void
VDBRepository::_uninstall(const QualifiedPackageName & q, const VersionSpec & v, const UninstallOptions & o,
        bool reinstalling) const
{
    Context context("When uninstalling '" + stringify(q) + "-" + stringify(v) +
            "' from '" + stringify(name()) + (reinstalling ? "' for a reinstall:" : "':"));

    if (! _imp->root.is_directory())
        throw PackageInstallActionError("Couldn't uninstall '" + stringify(q) + "-" +
                stringify(v) + "' because root ('" + stringify(_imp->root) + "') is not a directory");

    if ((! reinstalling) && (! has_version(q, v)))
        throw PackageInstallActionError("Couldn't uninstall '" + stringify(q) + "-" +
                stringify(v) + "' because has_version failed");

    std::string reinstalling_str(reinstalling ? "-reinstalling-" : "");

    PackageDatabaseEntry e(q, v, name());

    std::tr1::shared_ptr<FSEntryCollection> eclassdirs(new FSEntryCollection::Concrete);
    eclassdirs->append(FSEntry(_imp->location / stringify(q.category) /
                (reinstalling_str + stringify(q.package) + "-" + stringify(v))));

    FSEntry pkg_dir(_imp->location / stringify(q.category) / (reinstalling_str + stringify(q.package) + "-" + stringify(v)));

    std::tr1::shared_ptr<FSEntry> load_env(new FSEntry(pkg_dir / "environment.bz2"));

    EbuildCommandParams params(EbuildCommandParams::create()
            .environment(_imp->env)
            .db_entry(&e)
            .ebuild_dir(pkg_dir)
            .files_dir(pkg_dir)
            .eclassdirs(eclassdirs)
            .portdir(_imp->location)
            .distdir(pkg_dir)
            .buildroot(_imp->buildroot));

    EbuildUninstallCommandParams uninstall_params(EbuildUninstallCommandParams::create()
            .phase(up_preremove)
            .root(stringify(_imp->root) + "/")
            .disable_cfgpro(o.no_config_protect)
            .unmerge_only(false)
            .loadsaveenv_dir(pkg_dir)
            .load_environment(load_env.get()));

    EbuildUninstallCommand uninstall_cmd_pre(params, uninstall_params);
    uninstall_cmd_pre();

    /* load CONFIG_PROTECT, CONFIG_PROTECT_MASK from vdb, supplement with env */
    std::string config_protect, config_protect_mask;
    {
        std::ifstream c(stringify(pkg_dir / "CONFIG_PROTECT").c_str());
        config_protect = std::string((std::istreambuf_iterator<char>(c)), std::istreambuf_iterator<char>()) +
            " " + getenv_with_default("CONFIG_PROTECT", "");

        std::ifstream c_m(stringify(pkg_dir / "CONFIG_PROTECT_MASK").c_str());
        config_protect_mask = std::string((std::istreambuf_iterator<char>(c_m)), std::istreambuf_iterator<char>()) +
            " " + getenv_with_default("CONFIG_PROTECT_MASK", "");
    }

    /* unmerge */
    VDBUnmerger unmerger(
            VDBUnmergerOptions::create()
            .environment(_imp->params.environment)
            .root(root())
            .contents_file(pkg_dir / "CONTENTS")
            .config_protect(config_protect)
            .config_protect_mask(config_protect_mask)
            .package_name(q)
            .version(v)
            .repository(this));

    unmerger.unmerge();

    uninstall_params.phase = up_postremove;
    EbuildUninstallCommand uninstall_cmd_post(params, uninstall_params);
    uninstall_cmd_post();

    /* remove vdb entry */
    for (DirIterator d(pkg_dir, false), d_end ; d != d_end ; ++d)
        FSEntry(*d).unlink();
    pkg_dir.rmdir();
}

void
VDBRepository::do_config(const QualifiedPackageName & q, const VersionSpec & v) const
{
    Context context("When configuring '" + stringify(q) + "-" + stringify(v) +
            "' from '" + stringify(name()) + "':");

    if (! _imp->root.is_directory())
        throw PackageInstallActionError("Couldn't configure '" + stringify(q) + "-" +
                stringify(v) + "' because root ('" + stringify(_imp->root) + "') is not a directory");

    std::tr1::shared_ptr<const VersionMetadata> metadata;
    if (! has_version(q, v))
        throw PackageInstallActionError("Couldn't configure '" + stringify(q) + "-" +
                stringify(v) + "' because has_version failed");
    else
        metadata = version_metadata(q, v);

    PackageDatabaseEntry e(q, v, name());

    std::tr1::shared_ptr<FSEntryCollection> eclassdirs(new FSEntryCollection::Concrete);
    eclassdirs->append(FSEntry(_imp->location / stringify(q.category) /
                (stringify(q.package) + "-" + stringify(v))));

    FSEntry pkg_dir(_imp->location / stringify(q.category) /
            (stringify(q.package) + "-" + stringify(v)));

    std::tr1::shared_ptr<FSEntry> load_env(new FSEntry(pkg_dir / "environment.bz2"));

    EbuildConfigCommand config_cmd(EbuildCommandParams::create()
            .environment(_imp->env)
            .db_entry(&e)
            .ebuild_dir(pkg_dir)
            .files_dir(pkg_dir)
            .eclassdirs(eclassdirs)
            .portdir(_imp->location)
            .distdir(pkg_dir)
            .buildroot(_imp->buildroot),

            EbuildConfigCommandParams::create()
            .root(stringify(_imp->root) + "/")
            .load_environment(load_env.get()));

    config_cmd();
}

std::tr1::shared_ptr<DepSpec>
VDBRepository::do_package_set(const SetName & s) const
{
    Context context("When fetching package set '" + stringify(s) + "' from '" +
            stringify(name()) + "':");

    if ("everything" == s.data())
    {
        std::tr1::shared_ptr<AllDepSpec> result(new AllDepSpec);
        std::tr1::shared_ptr<GeneralSetDepTag> tag(new GeneralSetDepTag(SetName("everything"), stringify(name())));

        if (! _imp->entries_valid)
            _imp->load_entries();

        for (std::vector<VDBEntry>::const_iterator p(_imp->entries.begin()),
                p_end(_imp->entries.end()) ; p != p_end ; ++p)
        {
            std::tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(
                        std::tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(p->name))));
            spec->set_tag(tag);
            result->add_child(spec);
        }

        return result;
    }
    else if ("world" == s.data())
    {
        std::tr1::shared_ptr<AllDepSpec> result(new AllDepSpec);
        std::tr1::shared_ptr<GeneralSetDepTag> tag(new GeneralSetDepTag(SetName("world"), stringify(name())));

        if (_imp->world_file.exists())
        {
            LineConfigFile world(_imp->world_file);

            for (LineConfigFile::Iterator line(world.begin()), line_end(world.end()) ;
                    line != line_end ; ++line)
            {
                try
                {
                    if (std::string::npos == line->find('/'))
                    {
                        std::tr1::shared_ptr<DepSpec> spec(_imp->env->package_set(SetName(*line)));
                        if (spec)
                            result->add_child(spec);
                        else
                            Log::get_instance()->message(ll_warning, lc_no_context, "World file '"
                                    + stringify(_imp->world_file) + "' entry '" + *line +
                                    " is not a known package set");
                    }
                    else
                    {
                        std::tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(
                                    std::tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(*line))));
                        spec->set_tag(tag);
                        result->add_child(spec);
                    }
                }
                catch (const NameError & n)
                {
                    Log::get_instance()->message(ll_warning, lc_no_context, "World file '"
                            + stringify(_imp->world_file) + "' entry '" + *line + " is broken: '"
                            + n.message() + "' (" + n.what() + ")");
                }
            }
        }
        else
            Log::get_instance()->message(ll_warning, lc_no_context,
                    "World file '" + stringify(_imp->world_file) +
                    "' doesn't exist");

        return result;
    }
    else
        return std::tr1::shared_ptr<DepSpec>();
}

std::tr1::shared_ptr<const SetsCollection>
VDBRepository::sets_list() const
{
    Context context("While generating the list of sets:");

    std::tr1::shared_ptr<SetsCollection> result(new SetsCollection::Concrete);
    result->insert(SetName("everything"));
    result->insert(SetName("world"));
    return result;
}

void
VDBRepository::invalidate()
{
    _imp.reset(new Implementation<VDBRepository>(this, _imp->params));
}

void
VDBRepository::add_string_to_world(const std::string & n) const
{
    Context context("When adding '" + n + "' to world file '" +
            stringify(_imp->world_file) + "':");

    bool found(false);

    if (_imp->world_file.exists())
    {
        LineConfigFile world(_imp->world_file);

        for (LineConfigFile::Iterator line(world.begin()), line_end(world.end()) ;
                line != line_end ; ++line)
            if (*line == n)
            {
                found = true;
                break;
            }
    }

    if (! found)
    {
        /* portage is retarded, and doesn't ensure that the last entry in world has
         * a newline character after it. */
        bool world_file_needs_newline(false);
        {
            std::ifstream world(stringify(_imp->world_file).c_str(), std::ios::in);
            if (world)
            {
                world.seekg(0, std::ios::end);
                if (0 != world.tellg())
                {
                    world.seekg(-1, std::ios::end);
                    if ('\n' != world.get())
                        world_file_needs_newline = true;
                }
            }
        }

        if (world_file_needs_newline)
            Log::get_instance()->message(ll_warning, lc_no_context, "World file '"
                    + stringify(_imp->world_file) + "' lacks final newline");

        std::ofstream world(stringify(_imp->world_file).c_str(), std::ios::out | std::ios::app);
        if (! world)
            Log::get_instance()->message(ll_warning, lc_no_context, "Cannot append to world file '"
                    + stringify(_imp->world_file) + "', skipping world update");
        else
        {
            if (world_file_needs_newline)
                world << std::endl;
            world << n << std::endl;
        }
    }
}

void
VDBRepository::remove_string_from_world(const std::string & n) const
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

void
VDBRepository::add_to_world(const QualifiedPackageName & n) const
{
    add_string_to_world(stringify(n));
}

void
VDBRepository::add_to_world(const SetName & n) const
{
    add_string_to_world(stringify(n));
}

void
VDBRepository::remove_from_world(const QualifiedPackageName & n) const
{
    remove_string_from_world(stringify(n));
}

void
VDBRepository::remove_from_world(const SetName & n) const
{
    remove_string_from_world(stringify(n));
}

std::string
VDBRepository::get_environment_variable(
        const PackageDatabaseEntry & for_package,
        const std::string & var) const
{
    Context context("When fetching environment variable '" + var + "' for '" +
            stringify(for_package) + "':");

    FSEntry vdb_dir(_imp->location / stringify(for_package.name.category)
            / (stringify(for_package.name.package) + "-" +
                stringify(for_package.version)));

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
        throw EnvironmentVariableActionError("Could not get variable '" + var + "' for '"
                + stringify(for_package) + "'");
}

std::tr1::shared_ptr<const RepositoryProvidesInterface::ProvidesCollection>
VDBRepository::provided_packages() const
{
    if (_imp->provides)
        return _imp->provides;

    if (! load_provided_using_cache())
        load_provided_the_slow_way();

    return _imp->provides;
}

std::tr1::shared_ptr<const VersionMetadata>
VDBRepository::provided_package_version_metadata(const RepositoryProvidesEntry & p) const
{
    std::tr1::shared_ptr<const VersionMetadata> m(version_metadata(p.provided_by_name, p.version));
    std::tr1::shared_ptr<VDBVirtualVersionMetadata> result(new VDBVirtualVersionMetadata(
                m->slot, PackageDatabaseEntry(p.provided_by_name, p.version, name())));

    result->eapi = m->eapi;
    result->build_depend_string = stringify(p.provided_by_name);
    result->run_depend_string = stringify(p.provided_by_name);

    return result;
}

std::tr1::shared_ptr<const UseFlagNameCollection>
VDBRepository::do_arch_flags() const
{
    return std::tr1::shared_ptr<const UseFlagNameCollection>(new UseFlagNameCollection::Concrete);
}

std::tr1::shared_ptr<const UseFlagNameCollection>
VDBRepository::do_use_expand_flags() const
{
    return std::tr1::shared_ptr<const UseFlagNameCollection>(new UseFlagNameCollection::Concrete);
}

std::tr1::shared_ptr<const UseFlagNameCollection>
VDBRepository::do_use_expand_prefixes() const
{
    return std::tr1::shared_ptr<const UseFlagNameCollection>(new UseFlagNameCollection::Concrete);
}

std::tr1::shared_ptr<const UseFlagNameCollection>
VDBRepository::do_use_expand_hidden_prefixes() const
{
    return std::tr1::shared_ptr<const UseFlagNameCollection>(new UseFlagNameCollection::Concrete);
}

UseFlagName
VDBRepository::do_use_expand_name(const UseFlagName & u) const
{
    return u;
}

UseFlagName
VDBRepository::do_use_expand_value(const UseFlagName & u) const
{
    return u;
}

bool
VDBRepository::load_provided_using_cache() const
{
    if (_imp->provides_cache == FSEntry("/var/empty"))
        return false;

    Context context("When loading VDB PROVIDEs map using '" + stringify(_imp->provides_cache) + "':");

    std::tr1::shared_ptr<ProvidesCollection> result(new ProvidesCollection::Concrete);

    if (! _imp->provides_cache.is_regular_file())
    {
        Log::get_instance()->message(ll_warning, lc_no_context, "Provides cache at '"
                + stringify(_imp->provides_cache) + "' is not a regular file.");
        return false;
    }

    std::ifstream provides_cache(stringify(_imp->provides_cache).c_str());

    std::string version;
    std::getline(provides_cache, version);

    if (version != "paludis-2")
    {
        Log::get_instance()->message(ll_warning, lc_no_context, "Can't use provides cache at '"
                + stringify(_imp->provides_cache) + "' because format '" + version + "' is not 'paludis-2'");
        return false;
    }

    std::string for_name;
    std::getline(provides_cache, for_name);
    if (for_name != stringify(name()))
    {
        Log::get_instance()->message(ll_warning, lc_no_context, "Can't use provides cache at '"
                + stringify(_imp->provides_cache) + "' because it was generated for repository '"
                + for_name + "'. You must not have multiple name caches at the same location.");
        return false;
    }

    std::string line;
    while (std::getline(provides_cache, line))
    {
        std::vector<std::string> tokens;
        WhitespaceTokeniser::get_instance()->tokenise(line, std::back_inserter(tokens));
        if (tokens.size() < 3)
            continue;

        PackageDatabaseEntry dbe(QualifiedPackageName(tokens.at(0)), VersionSpec(tokens.at(1)), name());
        DepSpecFlattener f(_imp->env, &dbe, PortageDepParser::parse(
                    join(next(next(tokens.begin())), tokens.end(), " "),
                    PortageDepParser::Policy::text_is_package_dep_spec(false, pds_pm_permissive)));

        for (DepSpecFlattener::Iterator p(f.begin()), p_end(f.end()) ; p != p_end ; ++p)
            result->insert(RepositoryProvidesEntry::create()
                    .virtual_name((*p)->text())
                    .version(dbe.version)
                    .provided_by_name(dbe.name));
    }

    _imp->provides = result;
    return true;
}

void
VDBRepository::load_provided_the_slow_way() const
{
    Context context("When loading VDB PROVIDEs map the slow way:");

    Log::get_instance()->message(ll_debug, lc_no_context, "Starting VDB PROVIDEs map creation");

    std::tr1::shared_ptr<ProvidesCollection> result(new ProvidesCollection::Concrete);

    if (! _imp->entries_valid)
        _imp->load_entries();

    for (std::vector<VDBEntry>::iterator e(_imp->entries.begin()),
            e_end(_imp->entries.end()) ; e != e_end ; ++e)
    {
        Context loop_context("When loading VDB PROVIDEs entry for '"
                + stringify(e->name) + "-" + stringify(e->version) + "':");

        try
        {
            std::string provide_str;
            if (e->metadata)
                provide_str = e->metadata->ebuild_interface->provide_string;
            else
            {
                // _imp->load_entry(e); slow
                provide_str = file_contents(_imp->location, e->name, e->version, "PROVIDE");
            }
            if (provide_str.empty())
                continue;

            std::tr1::shared_ptr<const DepSpec> provide(PortageDepParser::parse(provide_str,
                        PortageDepParser::Policy::text_is_package_dep_spec(false, pds_pm_permissive)));
            PackageDatabaseEntry dbe(e->name, e->version, name());
            DepSpecFlattener f(_imp->env, &dbe, provide);

            for (DepSpecFlattener::Iterator p(f.begin()), p_end(f.end()) ; p != p_end ; ++p)
            {
                QualifiedPackageName pp((*p)->text());

                if (pp.category != CategoryNamePart("virtual"))
                    Log::get_instance()->message(ll_warning, lc_no_context, "PROVIDE of non-virtual '"
                            + stringify(pp) + "' from '" + stringify(e->name) + "-"
                            + stringify(e->version) + "' in '" + stringify(name())
                            + "' will not work as expected");

                result->insert(RepositoryProvidesEntry::create()
                        .virtual_name(pp)
                        .version(e->version)
                        .provided_by_name(e->name));
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

    _imp->provides = result;
}

void
VDBRepository::regenerate_cache() const
{
    regenerate_provides_cache();
    _imp->names_cache->regenerate_cache();
}

void
VDBRepository::regenerate_provides_cache() const
{
    if (_imp->provides_cache == FSEntry("/var/empty"))
        return;

    Context context("When generating VDB repository provides cache at '"
            + stringify(_imp->provides_cache) + "':");

    FSEntry(_imp->provides_cache).unlink();
    _imp->provides_cache.dirname().mkdir();

    if (! _imp->entries_valid)
        _imp->load_entries();

    std::ofstream f(stringify(_imp->provides_cache).c_str());
    if (! f)
    {
        Log::get_instance()->message(ll_warning, lc_context, "Cannot write to '"
                + stringify(_imp->provides_cache) + "'");
        return;
    }

    f << "paludis-2" << std::endl;
    f << name() << std::endl;

    for (std::vector<VDBEntry>::const_iterator c(_imp->entries.begin()), c_end(_imp->entries.end()) ;
            c != c_end ; ++c)
    {
        std::string provide_str;
        if (c->metadata)
            provide_str = c->metadata->ebuild_interface->provide_string;
        else
            provide_str = file_contents(_imp->location, c->name, c->version, "PROVIDE");

        provide_str = strip_leading(strip_trailing(provide_str, " \t\r\n"), " \t\r\n");
        if (provide_str.empty())
            continue;

        f << c->name << " " << c->version << " " << provide_str << std::endl;
    }
}

std::tr1::shared_ptr<const CategoryNamePartCollection>
VDBRepository::do_category_names_containing_package(const PackageNamePart & p) const
{
    if (! _imp->names_cache->usable())
        return Repository::do_category_names_containing_package(p);

    std::tr1::shared_ptr<const CategoryNamePartCollection> result(
            _imp->names_cache->category_names_containing_package(p));

    return result ? result : Repository::do_category_names_containing_package(p);
}

bool
VDBRepository::is_suitable_destination_for(const PackageDatabaseEntry & e) const
{
    std::string f(_imp->env->package_database()->fetch_repository(e.repository)->format());
    return f == "ebuild" || f == "ebin";
}

bool
VDBRepository::is_default_destination() const
{
    return _imp->env->root() == root();
}

std::string
VDBRepository::do_describe_use_flag(const UseFlagName &,
        const PackageDatabaseEntry * const) const
{
    return "";
}

FSEntry
VDBRepository::root() const
{
    return _imp->root;
}

bool
VDBRepository::want_pre_post_phases() const
{
    return true;
}

void
VDBRepository::merge(const MergeOptions & m)
{
    Context context("When merging '" + stringify(m.package) + "' at '" + stringify(m.image_dir)
            + "' to VDB repository '" + stringify(name()) + "':");

    if (! is_suitable_destination_for(m.package))
        throw PackageInstallActionError("Not a suitable destination for '" + stringify(m.package) + "'");

    bool is_replace(has_version(m.package.name, m.package.version));

    FSEntry tmp_vdb_dir(_imp->params.location);
    tmp_vdb_dir.mkdir();
    tmp_vdb_dir /= stringify(m.package.name.category);
    tmp_vdb_dir.mkdir();
    tmp_vdb_dir /= ("-checking-" + stringify(m.package.name.package) + "-" + stringify(m.package.version));
    tmp_vdb_dir.mkdir();

    WriteVDBEntryCommand write_vdb_entry_command(
            WriteVDBEntryParams::create()
            .environment(_imp->params.environment)
            .db_entry(m.package)
            .output_directory(tmp_vdb_dir)
            .environment_file(m.environment_file));

    write_vdb_entry_command();

    /* load CONFIG_PROTECT, CONFIG_PROTECT_MASK from vdb */
    std::string config_protect, config_protect_mask;
    {
        std::ifstream c(stringify(tmp_vdb_dir / "CONFIG_PROTECT").c_str());
        config_protect = std::string((std::istreambuf_iterator<char>(c)), std::istreambuf_iterator<char>());

        std::ifstream c_m(stringify(tmp_vdb_dir / "CONFIG_PROTECT_MASK").c_str());
        config_protect_mask = std::string((std::istreambuf_iterator<char>(c_m)), std::istreambuf_iterator<char>());
    }

    FSEntry vdb_dir(_imp->params.location);
    vdb_dir /= stringify(m.package.name.category);
    vdb_dir /= (stringify(m.package.name.package) + "-" + stringify(m.package.version));

    VDBMerger merger(
            VDBMergerOptions::create()
            .environment(_imp->params.environment)
            .image(m.image_dir)
            .root(root())
            .contents_file(vdb_dir / "CONTENTS")
            .config_protect(config_protect)
            .config_protect_mask(config_protect_mask)
            .package(&m.package));

    if (! merger.check())
    {
        for (DirIterator d(tmp_vdb_dir, false), d_end ; d != d_end ; ++d)
            FSEntry(*d).unlink();
        tmp_vdb_dir.rmdir();
        throw PackageInstallActionError("Not proceeding with install due to merge sanity check failing");
    }

    if (is_replace)
        vdb_dir.rename(vdb_dir.dirname() / ("-reinstalling-" + vdb_dir.basename()));
    tmp_vdb_dir.rename(vdb_dir);

    merger.merge();

    if (is_replace)
    {
        UninstallOptions uninstall_options(false);
        _uninstall(m.package.name, m.package.version, uninstall_options, true);
    }

    VDBPostMergeCommand post_merge_command(
            VDBPostMergeCommandParams::create()
            .root(root()));

    post_merge_command();
}

