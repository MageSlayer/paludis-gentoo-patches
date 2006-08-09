/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
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


namespace paludis
{
    /// Map for metadata.
    typedef MakeHashedMap<std::pair<QualifiedPackageName, VersionSpec>, VersionMetadata::Pointer>::Type MetadataMap;

    template <>
    struct Implementation<CRANInstalledRepository> :
        InternalCounted<Implementation<CRANInstalledRepository> >
    {
        /// Our owning db..
        const PackageDatabase * const db;

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

        /// Load metadata for one entry.
        void load_entry(std::vector<CRANDescription>::iterator) const;

        /// Metadata cache
        mutable MetadataMap metadata;

        /// Provide map.
        mutable std::map<QualifiedPackageName, QualifiedPackageName> provide_map;

        /// Constructor.
        Implementation(const CRANInstalledRepositoryParams &);

        /// Destructor.
        ~Implementation();

        /// Invalidate.
        void invalidate() const;
    };
}

Implementation<CRANInstalledRepository>::Implementation(const CRANInstalledRepositoryParams & p) :
    db(p.get<craninstrpk_package_database>()),
    env(p.get<craninstrpk_environment>()),
    location(p.get<craninstrpk_location>()),
    root(p.get<craninstrpk_root>()),
    world_file(p.get<craninstrpk_world>()),
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
            Context context("When parsing directoryy '" + stringify(*d) + "'.");
            if (! d->is_directory())
                continue;

            FSEntry f(FSEntry(stringify(*d)) / "DESCRIPTION");
            if (! f.is_regular_file())
                continue;

            std::string n(d->basename());
            CRANDescription::normalise_name(n);

            CRANDescription desc(n, f);
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

void
Implementation<CRANInstalledRepository>::invalidate() const
{
    entries_valid = false;
    entries.clear();
}

CRANInstalledRepository::CRANInstalledRepository(const CRANInstalledRepositoryParams & p) :
    Repository(RepositoryName("cran_installed"),
            RepositoryCapabilities::create((
                    param<repo_mask>(static_cast<MaskInterface *>(0)),
                    param<repo_installable>(static_cast<InstallableInterface *>(0)),
                    param<repo_installed>(this),
                    param<repo_news>(static_cast<NewsInterface *>(0)),
                    param<repo_sets>(this),
                    param<repo_syncable>(static_cast<SyncableInterface *>(0)),
                    param<repo_uninstallable>(this),
                    param<repo_use>(static_cast<UseInterface *>(0)),
                    param<repo_world>(this),
                    param<repo_environment_variable>(static_cast<EnvironmentVariableInterface *>(0)),
                    param<repo_mirrors>(static_cast<MirrorInterface *>(0))
                    ))),
    PrivateImplementationPattern<CRANInstalledRepository>(new Implementation<CRANInstalledRepository>(p))
{
    RepositoryInfoSection::Pointer config_info(new RepositoryInfoSection("Configuration information"));

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

    if (! do_has_category_named(q.get<qpn_category>()))
        return false;

    if (! _imp->entries_valid)
        _imp->load_entries();

    std::pair<std::vector<CRANDescription>::const_iterator, std::vector<CRANDescription>::const_iterator>
        r(std::equal_range(_imp->entries.begin(), _imp->entries.end(), q,
                    CRANDescription::ComparePackage()));

    return r.first != r.second;
}

CategoryNamePartCollection::ConstPointer
CRANInstalledRepository::do_category_names() const
{
    if (! _imp->entries_valid)
        _imp->load_entries();

    CategoryNamePartCollection::Pointer result(new CategoryNamePartCollection::Concrete);
    result->insert(CategoryNamePart("cran"));
    return result;
}

QualifiedPackageNameCollection::ConstPointer
CRANInstalledRepository::do_package_names(const CategoryNamePart & c) const
{
    Context context("When fetching package names in category '" + stringify(c)
            + "' in " + stringify(name()) + ":");

    QualifiedPackageNameCollection::Pointer result(new QualifiedPackageNameCollection::Concrete);
    if (! do_has_category_named(c))
        return result;

    std::vector<CRANDescription>::const_iterator e(_imp->entries.begin()), e_end(_imp->entries.end());
    for ( ; e != e_end ; ++e)
        result->insert(e->name);

    return result;
}

VersionSpecCollection::ConstPointer
CRANInstalledRepository::do_version_specs(const QualifiedPackageName & n) const
{
    Context context("When fetching versions of '" + stringify(n) + "' in "
            + stringify(name()) + ":");

    VersionSpecCollection::Pointer result(new VersionSpecCollection::Concrete);

    std::pair<std::vector<CRANDescription>::const_iterator, std::vector<CRANDescription>::const_iterator>
        r(std::equal_range(_imp->entries.begin(), _imp->entries.end(), n,
                    CRANDescription::ComparePackage()));

    for ( ; r.first != r.second ; ++(r.first))
        result->insert(r.first->version);

    return result;
}

bool
CRANInstalledRepository::do_has_version(const QualifiedPackageName & q,
        const VersionSpec & v) const
{
    Context context("When checking for version '" + stringify(v) + "' in '"
            + stringify(q) + "' in " + stringify(name()) + ":");

    VersionSpecCollection::ConstPointer versions(do_version_specs(q));
    return versions->end() != versions->find(v);
}

VersionMetadata::ConstPointer
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

    VersionMetadata::Pointer result(0);

    FSEntry d(_imp->location);
    d /= stringify(q.get<qpn_package>());
    d /= "DESCRIPTION";

    if (d.is_regular_file())
    {
        CRANDescription description(stringify(q.get<qpn_package>()), d);
        result = description.metadata;
    }
    else
    {
        Log::get_instance()->message(ll_warning, lc_no_context, "has_version failed for request for '" +
                stringify(q) + "-" + stringify(v) + "' in repository '" +
                stringify(name()) + "': No DESCRIPTION file present.");
        result.assign(new VersionMetadata(CRANDepParser::parse));
        result->set<vm_eapi>("UNKNOWN");
        return result;
    }

    _imp->metadata.insert(std::make_pair(std::make_pair(q, v), result));
    return result;
}

Contents::ConstPointer
CRANInstalledRepository::do_contents(
        const QualifiedPackageName & q, const VersionSpec & v) const
{
    Context context("When fetching contents for " + stringify(q) +
            "-" + stringify(v));

    Contents::Pointer result(new Contents);

    if (! _imp->entries_valid)
        _imp->load_entries();

    if (! has_version(q, v)) 
        return result;

    std::string pn = stringify(q.get<qpn_package>());
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

CountedPtr<Repository>
CRANInstalledRepository::make_cran_installed_repository(
        const Environment * const env,
        const PackageDatabase * const db,
        AssociativeCollection<std::string, std::string>::ConstPointer m)
{
    Context context("When making CRAN installed repository from repo_file '" + 
            (m->end() == m->find("repo_file") ? std::string("?") : m->find("repo_file")->second) + "':");

    std::string location;
    if (m->end() == m->find("location") || ((location = m->find("location")->second)).empty())
        throw CRANInstalledRepositoryConfigurationError("Key 'location' not specified or empty");

    std::string root;
    if (m->end() == m->find("root") || ((root = m->find("root")->second)).empty())
        root = "/";

    std::string world;
    if (m->end() == m->find("world") || ((world = m->find("world")->second)).empty())
        world = location + "/world";

    return CountedPtr<Repository>(new CRANInstalledRepository(CRANInstalledRepositoryParams::create((
                        param<craninstrpk_environment>(env),
                        param<craninstrpk_package_database>(db),
                        param<craninstrpk_location>(location),
                        param<craninstrpk_root>(root),
                        param<craninstrpk_world>(world)))));
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
        const InstallOptions &) const
{
    Context context("When uninstalling '" + stringify(q) + "-" + stringify(v) +
            "' from '" + stringify(name()) + "':");

    if (! _imp->location.is_directory())
        throw PackageInstallActionError("Couldn't uninstall '" + stringify(q) + "-" +
                stringify(v) + "' because its location ('" + stringify(_imp->location) + "') is not a directory");

    VersionMetadata::ConstPointer vm(do_version_metadata(q, v));

    MakeEnvCommand cmd(LIBEXECDIR "/paludis/cran.bash unmerge", "");
    cmd = cmd("PN", vm->get_cran_interface()->get<cranvm_package>());
    cmd = cmd("PV", stringify(v));
    cmd = cmd("PALUDIS_CRAN_LIBRARY", stringify(_imp->location));
    cmd = cmd("PALUDIS_EBUILD_DIR", std::string(LIBEXECDIR "/paludis/"));
    cmd = cmd("PALUDIS_BASHRC_FILES", _imp->env->bashrc_files());



    if (0 != run_command(cmd))
        throw PackageUninstallActionError("Couldn't unmerge '" + stringify(q) + "-" + stringify(v) + "'");
}

DepAtom::Pointer
CRANInstalledRepository::do_package_set(const std::string & s, const PackageSetOptions &) const
{
    Context context("When fetching package set '" + s + "' from '" +
            stringify(name()) + "':");

    if ("everything" == s)
    {
        AllDepAtom::Pointer result(new AllDepAtom);
        if (! _imp->entries_valid)
            _imp->load_entries();

        for (std::vector<CRANDescription>::const_iterator p(_imp->entries.begin()),
                p_end(_imp->entries.end()) ; p != p_end ; ++p)
        {
            PackageDepAtom::Pointer atom(new PackageDepAtom(p->name));
            result->add_child(atom);
        }

        return result;
    }
    else if ("world" == s)
    {
        AllDepAtom::Pointer result(new AllDepAtom);

        if (_imp->world_file.exists())
        {
            LineConfigFile world(_imp->world_file);

            for (LineConfigFile::Iterator line(world.begin()), line_end(world.end()) ;
                    line != line_end ; ++line)
            {
                PackageDepAtom::Pointer atom(new PackageDepAtom(QualifiedPackageName(*line)));
                result->add_child(atom);
            }
        }
        else
            Log::get_instance()->message(ll_warning, lc_no_context, "World file '" + stringify(_imp->world_file)
                    + "' doesn't exist");

        return result;
    }
    else
        return DepAtom::Pointer(0);
}

bool
CRANInstalledRepository::do_sync() const
{
    return false;
}

void
CRANInstalledRepository::invalidate() const
{
    _imp->invalidate();
}

Repository::ProvideMapIterator
CRANInstalledRepository::begin_provide_map() const
{
    return _imp->provide_map.end();
}

Repository::ProvideMapIterator
CRANInstalledRepository::end_provide_map() const
{
    return _imp->provide_map.end();
}

void
CRANInstalledRepository::add_to_world(const QualifiedPackageName & n) const
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
CRANInstalledRepository::remove_from_world(const QualifiedPackageName & n) const
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
