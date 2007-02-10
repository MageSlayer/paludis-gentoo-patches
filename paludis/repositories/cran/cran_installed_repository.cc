/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006,2007 Danny van Dyk <kugelfang@gentoo.org>
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
#include <paludis/config_file.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/repositories/cran/cran_description.hh>
#include <paludis/repositories/cran/cran_dep_parser.hh>
#include <paludis/repositories/cran/cran_installed_repository.hh>
#include <paludis/repositories/cran/cran_version_metadata.hh>
#include <paludis/util/collection_concrete.hh>
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

#include <paludis/repositories/cran/cran_installed_repository-sr.cc>

namespace paludis
{
    /// Map for metadata.
    typedef MakeHashedMap<std::pair<QualifiedPackageName, VersionSpec>, std::tr1::shared_ptr<VersionMetadata> >::Type MetadataMap;

    template <>
    struct Implementation<CRANInstalledRepository>
    {
        CRANInstalledRepositoryParams params;

        /// Our owning env.
        const Environment * const env;

        /// Our base location.
        FSEntry location;

        /// Root location 
        FSEntry root;

        /// World file.
        FSEntry world_file;

        // Do we have entries loaded?
        mutable bool entries_valid;

        mutable std::vector<CRANDescription> entries;

        /// Load entries.
        void load_entries() const;

        /// Metadata cache
        mutable MetadataMap metadata;

        /// Provide map.
        mutable std::map<QualifiedPackageName, QualifiedPackageName> provide_map;

        /// Constructor.
        Implementation(const CRANInstalledRepositoryParams &);

        /// Destructor.
        ~Implementation();
    };
}

Implementation<CRANInstalledRepository>::Implementation(const CRANInstalledRepositoryParams & p) :
    params(p),
    env(p.environment),
    location(p.location),
    root(p.root),
    world_file(p.world),
    entries_valid(false)
{
}

Implementation<CRANInstalledRepository>::~Implementation()
{
}

void
Implementation<CRANInstalledRepository>::load_entries() const
{
    Context context("When loading CRANInstalledRepository entries from '" +
            stringify(location) + "':");

    entries.clear();
    entries_valid = true;
    try
    {
        for (DirIterator d(location), d_end ; d != d_end ; ++d)
        {
            Context local_context("When parsing directoryy '" + stringify(*d) + "'.");
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

CRANInstalledRepository::CRANInstalledRepository(const CRANInstalledRepositoryParams & p) :
    Repository(RepositoryName("cran_installed"),
            RepositoryCapabilities::create()
            .mask_interface(0)
            .installable_interface(0)
            .installed_interface(this)
            .contents_interface(this)
            .news_interface(0)
            .sets_interface(this)
            .syncable_interface(0)
            .uninstallable_interface(this)
            .use_interface(0)
            .world_interface(this)
            .environment_variable_interface(0)
            .mirrors_interface(0)
            .virtuals_interface(0)
            .provides_interface(0)
            .config_interface(0)
            .destination_interface(this),
            "cran_installed"),
    PrivateImplementationPattern<CRANInstalledRepository>(new Implementation<CRANInstalledRepository>(p))
{
    std::tr1::shared_ptr<RepositoryInfoSection> config_info(new RepositoryInfoSection("Configuration information"));

    config_info->add_kv("location", stringify(_imp->location));
    config_info->add_kv("root", stringify(_imp->root));
    config_info->add_kv("format", std::string("cran-installed"));

    _info->add_section(config_info);
}

CRANInstalledRepository::~CRANInstalledRepository()
{
}

bool
CRANInstalledRepository::do_has_category_named(const CategoryNamePart & c) const
{
    return (CategoryNamePart("cran") == c);
}

bool
CRANInstalledRepository::do_has_package_named(const QualifiedPackageName & q) const
{
    Context context("When checking for package '" + stringify(q) +
            "' in " + stringify(name()) + ":");

    if (! do_has_category_named(q.category))
        return false;

    if (! _imp->entries_valid)
        _imp->load_entries();

    std::pair<std::vector<CRANDescription>::const_iterator, std::vector<CRANDescription>::const_iterator>
        r(std::equal_range(_imp->entries.begin(), _imp->entries.end(), q,
                    CRANDescription::ComparePackage()));

    return r.first != r.second;
}

std::tr1::shared_ptr<const CategoryNamePartCollection>
CRANInstalledRepository::do_category_names() const
{
    if (! _imp->entries_valid)
        _imp->load_entries();

    std::tr1::shared_ptr<CategoryNamePartCollection> result(new CategoryNamePartCollection::Concrete);
    result->insert(CategoryNamePart("cran"));
    return result;
}

std::tr1::shared_ptr<const QualifiedPackageNameCollection>
CRANInstalledRepository::do_package_names(const CategoryNamePart & c) const
{
    Context context("When fetching package names in category '" + stringify(c)
            + "' in " + stringify(name()) + ":");

    if (! _imp->entries_valid)
        _imp->load_entries();

    std::tr1::shared_ptr<QualifiedPackageNameCollection> result(new QualifiedPackageNameCollection::Concrete);
    if (! do_has_category_named(c))
        return result;

    std::vector<CRANDescription>::const_iterator e(_imp->entries.begin()), e_end(_imp->entries.end());
    for ( ; e != e_end ; ++e)
        result->insert(e->name);

    return result;
}

std::tr1::shared_ptr<const VersionSpecCollection>
CRANInstalledRepository::do_version_specs(const QualifiedPackageName & n) const
{
    Context context("When fetching versions of '" + stringify(n) + "' in "
            + stringify(name()) + ":");

    std::tr1::shared_ptr<VersionSpecCollection> result(new VersionSpecCollection::Concrete);

    if (! _imp->entries_valid)
        _imp->load_entries();

    for (std::vector<CRANDescription>::const_iterator e(_imp->entries.begin()), e_end(_imp->entries.end()) ;
            e != e_end ; ++e)
        if (n == e->name)
            result->insert(e->version);

    return result;
}

bool
CRANInstalledRepository::do_has_version(const QualifiedPackageName & q,
        const VersionSpec & v) const
{
    Context context("When checking for version '" + stringify(v) + "' in '"
            + stringify(q) + "' in " + stringify(name()) + ":");

    std::tr1::shared_ptr<const VersionSpecCollection> versions(do_version_specs(q));
    return versions->end() != versions->find(v);
}

namespace
{
    /**
     * Fetch the contents of a VDB file.
     *
     * \ingroup grpcranrepository
     */
    std::string
    file_contents(const FSEntry & location, const QualifiedPackageName & name,
            const std::string & key)
    {
        Context context("When loading metadata for '" + stringify(name)
                + "' key '" + key + "' from '" + stringify(location) + "':");

        FSEntry f(location / stringify(name.package));
        if (! (f / key).is_regular_file())
            return "";

        std::ifstream ff(stringify(f / key).c_str());
        if (! ff)
            throw InternalError("CRANInstalledRepository", "Could not read '" + stringify(f / key) + "'");
        return strip_leading(strip_trailing(std::string((std::istreambuf_iterator<char>(ff)),
                        std::istreambuf_iterator<char>()), " \t\n"), " \t\n");
    }
}

std::tr1::shared_ptr<const VersionMetadata>
CRANInstalledRepository::do_version_metadata(
        const QualifiedPackageName & q, const VersionSpec & v) const
{
    if (_imp->metadata.end() != _imp->metadata.find(
                std::make_pair(q, v)))
            return _imp->metadata.find(std::make_pair(q, v))->second;

    Context context("When fetching metadata for " + stringify(q) +
            "-" + stringify(v));

    if (! has_version(q, v))
        throw NoSuchPackageError(stringify(PackageDatabaseEntry(q, v, name())));

    std::tr1::shared_ptr<VersionMetadata> result;

    FSEntry d(_imp->location);
    d /= stringify(q.package);
    d /= "DESCRIPTION";

    if (d.is_regular_file())
    {
        CRANDescription description(stringify(q.package), d, true);
        // Don't put this into CRANDescription, as it's only relevant to CRANInstalledRepository
        std::string repo(file_contents(_imp->location, q, "REPOSITORY"));
        if (! repo.empty())
            description.metadata->origins_interface->source.reset(new PackageDatabaseEntry(stringify(q.package), v,
                    RepositoryName(repo)));
        result = description.metadata;
    }
    else
    {
        Log::get_instance()->message(ll_warning, lc_no_context, "has_version failed for request for '" +
                stringify(q) + "-" + stringify(v) + "' in repository '" +
                stringify(name()) + "': No DESCRIPTION file present.");
        result.reset(new CRANVersionMetadata(true));
        result->eapi = "UNKNOWN";
        return result;
    }

    _imp->metadata.insert(std::make_pair(std::make_pair(q, v), result));
    return result;
}

std::tr1::shared_ptr<const Contents>
CRANInstalledRepository::do_contents(
        const QualifiedPackageName & q, const VersionSpec & v) const
{
    Context context("When fetching contents for " + stringify(q) +
            "-" + stringify(v));

    std::tr1::shared_ptr<Contents> result(new Contents);

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
        throw InternalError(PALUDIS_HERE, "TODO reading " + stringify(_imp->location) + " name " +
                stringify(q) + " version " + stringify(v) + " CONTENTS"); /// \todo

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

std::tr1::shared_ptr<Repository>
CRANInstalledRepository::make_cran_installed_repository(
        Environment * const env,
        std::tr1::shared_ptr<const AssociativeCollection<std::string, std::string> > m)
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

    return std::tr1::shared_ptr<Repository>(new CRANInstalledRepository(CRANInstalledRepositoryParams::create()
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

bool
CRANInstalledRepository::do_is_licence(const std::string &) const
{
    return false;
}

void
CRANInstalledRepository::do_uninstall(const QualifiedPackageName & q, const VersionSpec & v,
        const UninstallOptions &) const
{
    Context context("When uninstalling '" + stringify(q) + "-" + stringify(v) +
            "' from '" + stringify(name()) + "':");

    if (! _imp->location.is_directory())
        throw PackageInstallActionError("Couldn't uninstall '" + stringify(q) + "-" +
                stringify(v) + "' because its location ('" + stringify(_imp->location) + "') is not a directory");

    std::tr1::shared_ptr<const VersionMetadata> vm(do_version_metadata(q, v));

    MakeEnvCommand cmd(LIBEXECDIR "/paludis/cran.bash unmerge", "");
    cmd = cmd("PN", vm->cran_interface->package);
    cmd = cmd("PV", stringify(v));
    cmd = cmd("PALUDIS_CRAN_LIBRARY", stringify(_imp->location));
    cmd = cmd("PALUDIS_EBUILD_DIR", std::string(LIBEXECDIR "/paludis/"));
    cmd = cmd("PALUDIS_BASHRC_FILES", _imp->env->bashrc_files());

    if (0 != run_command(cmd))
        throw PackageUninstallActionError("Couldn't unmerge '" + stringify(q) + "-" + stringify(v) + "'");
}

std::tr1::shared_ptr<DepAtom>
CRANInstalledRepository::do_package_set(const SetName & s) const
{
    Context context("When fetching package set '" + stringify(s) + "' from '" +
            stringify(name()) + "':");

    if ("everything" == s.data())
    {
        std::tr1::shared_ptr<AllDepAtom> result(new AllDepAtom);
        if (! _imp->entries_valid)
            _imp->load_entries();

        for (std::vector<CRANDescription>::const_iterator p(_imp->entries.begin()),
                p_end(_imp->entries.end()) ; p != p_end ; ++p)
        {
            std::tr1::shared_ptr<PackageDepAtom> atom(new PackageDepAtom(p->name));
            result->add_child(atom);
        }

        return result;
    }
    else if ("world" == s.data())
    {
        std::tr1::shared_ptr<AllDepAtom> result(new AllDepAtom);

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
                        std::tr1::shared_ptr<DepAtom> atom(_imp->env->package_set(SetName(*line)));
                        if (atom)
                            result->add_child(atom);
                        else
                            Log::get_instance()->message(ll_warning, lc_no_context,
                                    "Entry '" + stringify(*line) + "' in world file '" + stringify(_imp->world_file)
                                    + "' is not a known set");
                    }
                    else
                    {
                        std::tr1::shared_ptr<PackageDepAtom> atom(new PackageDepAtom(QualifiedPackageName(*line)));
                        result->add_child(atom);
                    }
                }
                catch (const NameError & e)
                {
                    Log::get_instance()->message(ll_warning, lc_no_context,
                            "Entry '" + stringify(*line) + "' in world file '" + stringify(_imp->world_file)
                            + "' gave error '" + e.message() + "' (" + e.what() + ")");
                }
            }
        }
        else
            Log::get_instance()->message(ll_warning, lc_no_context, "World file '" + stringify(_imp->world_file)
                    + "' doesn't exist");

        return result;
    }
    else
        return std::tr1::shared_ptr<DepAtom>();
}

std::tr1::shared_ptr<const SetsCollection>
CRANInstalledRepository::sets_list() const
{
    Context context("While generating the list of sets:");

    std::tr1::shared_ptr<SetsCollection> result(new SetsCollection::Concrete);
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
CRANInstalledRepository::add_string_to_world(const std::string & n) const
{
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
        std::ofstream world(stringify(_imp->world_file).c_str(), std::ios::out | std::ios::app);
        if (! world)
            Log::get_instance()->message(ll_warning, lc_no_context, "Cannot append to world file '"
                    + stringify(_imp->world_file) + "', skipping world update");
        else
            world << n << std::endl;
    }
}

void
CRANInstalledRepository::remove_string_from_world(const std::string & n) const
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
            if (strip_leading(strip_trailing(line, " \t"), "\t") != n)
                world_lines.push_back(line);
            else
                Log::get_instance()->message(ll_debug, lc_context, "Removing line '"
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

bool
CRANInstalledRepository::is_suitable_destination_for(const PackageDatabaseEntry & e) const
{
    return _imp->env->package_database()->fetch_repository(e.repository)->format() == "cran";
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
    return _imp->root;
}

bool
CRANInstalledRepository::is_default_destination() const
{
    return _imp->env->root() == root();
}

