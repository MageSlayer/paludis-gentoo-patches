/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/compare.hh>

#include <list>
#include <map>
#include <set>

/** \file
 * Implementation of PackageDatabase.
 */

using namespace paludis;

#include "package_database_entry-sr.cc"

std::ostream &
paludis::operator<< (std::ostream & s, const PackageDatabaseEntry & v)
{
    s << v.name << "-" << v.version << "::" << v.repository;
    return s;
}

PackageDatabaseError::PackageDatabaseError(const std::string & message) throw () :
    Exception(message)
{
}

PackageDatabaseLookupError::PackageDatabaseLookupError(const std::string & message) throw () :
    PackageDatabaseError(message)
{
}

DuplicateRepositoryError::DuplicateRepositoryError(const std::string & name) throw () :
    PackageDatabaseError("A repository named '" + name + "' already exists")
{
}

NoSuchPackageError::NoSuchPackageError(const std::string & name) throw () :
    PackageDatabaseLookupError("Could not find '" + name + "'"),
    _name(name)
{
}

NoSuchRepositoryError::NoSuchRepositoryError(const std::string & name) throw () :
    PackageDatabaseLookupError("Could not find repository '" + name + "'")
{
}

/**
 * Name data for an AmbiguousPackageNameError.
 *
 * \ingroup grpexceptions
 */
struct AmbiguousPackageNameError::NameData
{
    /// Our query name
    std::string name;

    /// Our match names
    std::list<std::string> names;
};

template <typename I_>
AmbiguousPackageNameError::AmbiguousPackageNameError(const std::string & name,
        I_ begin, const I_ end) throw () :
    PackageDatabaseLookupError("Ambiguous package name '" + name + "' (candidates are " +
            join(begin, end, ", ") + ")"),
    _name_data(new NameData)
{
    _name_data->name = name;
    std::transform(begin, end, std::back_inserter(_name_data->names),
            &stringify<typename std::iterator_traits<I_>::value_type>);
}

AmbiguousPackageNameError::AmbiguousPackageNameError(const AmbiguousPackageNameError & other) :
    PackageDatabaseLookupError(other),
    _name_data(new NameData)
{
    _name_data->name = other._name_data->name;
    _name_data->names = other._name_data->names;
}

AmbiguousPackageNameError::~AmbiguousPackageNameError() throw ()
{
    delete _name_data;
}

AmbiguousPackageNameError::OptionsIterator
AmbiguousPackageNameError::begin_options() const
{
    return OptionsIterator(_name_data->names.begin());
}

AmbiguousPackageNameError::OptionsIterator
AmbiguousPackageNameError::end_options() const
{
    return OptionsIterator(_name_data->names.end());
}

const std::string &
AmbiguousPackageNameError::name() const
{
    return _name_data->name;
}

namespace paludis
{
    /**
     * Implementation data for a PackageDatabase.
     *
     * \ingroup grppackagedatabase
     */
    template<>
    struct Implementation<PackageDatabase> :
        InternalCounted<Implementation<PackageDatabase> >
    {
        /**
         * Our Repository instances.
         */
        std::list<Repository::ConstPointer> repositories;

        /// Our environment.
        const Environment * environment;
    };
}

PackageDatabase::PackageDatabase(const Environment * const e) :
    PrivateImplementationPattern<PackageDatabase>(new Implementation<PackageDatabase>)
{
    _imp->environment = e;
}

PackageDatabase::~PackageDatabase()
{
}

void
PackageDatabase::add_repository(const Repository::ConstPointer r)
{
    Context c("When adding a repository named '" + stringify(r->name()) + "':");

    IndirectIterator<std::list<Repository::ConstPointer>::const_iterator, const Repository>
        r_c(_imp->repositories.begin()),
        r_end(_imp->repositories.end());
    for ( ; r_c != r_end ; ++r_c)
        if (r_c->name() == r->name())
            throw DuplicateRepositoryError(stringify(r->name()));

    _imp->repositories.push_back(r);
}

QualifiedPackageName
PackageDatabase::fetch_unique_qualified_package_name(
        const PackageNamePart & p) const
{
    QualifiedPackageNameCollection::Pointer result(new QualifiedPackageNameCollection::Concrete);

    IndirectIterator<std::list<Repository::ConstPointer>::const_iterator, const Repository>
        r(_imp->repositories.begin()),
        r_end(_imp->repositories.end());
    for ( ; r != r_end ; ++r)
    {
        CategoryNamePartCollection::ConstPointer cats(r->category_names());
        CategoryNamePartCollection::Iterator c(cats->begin()), c_end(cats->end());
        for ( ; c != c_end ; ++c)
            if (r->has_package_named(*c + p))
                    result->insert(*c + p);
    }

    if (result->empty())
        throw NoSuchPackageError(stringify(p));
    if (result->size() > 1)
        throw AmbiguousPackageNameError(stringify(p), result->begin(), result->end());

    return *(result->begin());
}

PackageDatabaseEntryCollection::Pointer
PackageDatabase::_do_query(const PackageDepAtom & a, const InstallState installed_state) const
{
    PackageDatabaseEntryCollection::Pointer result(new PackageDatabaseEntryCollection::Concrete);

    IndirectIterator<std::list<Repository::ConstPointer>::const_iterator, const Repository>
        r(_imp->repositories.begin()),
        r_end(_imp->repositories.end());
    for ( ; r != r_end ; ++r)
    {
        if ((installed_state == is_installed_only) && ! r->installed_interface)
            continue;

        if ((installed_state == is_uninstalled_only) && r->installed_interface)
            continue;

        if (! r->has_category_named(a.package().category))
            continue;

        if (! r->has_package_named(a.package()))
            continue;

        VersionSpecCollection::ConstPointer versions(r->version_specs(a.package()));
        VersionSpecCollection::Iterator v(versions->begin()), v_end(versions->end());
        for ( ; v != v_end ; ++v)
        {
            PackageDatabaseEntry e(a.package(), *v, r->name());
            if (! match_package(_imp->environment, a, e))
                continue;

            result->insert(e);
        }
    }

    return result;
}

PackageDatabaseEntryCollection::Pointer
PackageDatabase::query(const PackageDepAtom & a, const InstallState s) const
{
    return _do_query(a, s);
}

Repository::ConstPointer
PackageDatabase::fetch_repository(const RepositoryName & n) const
{
    std::list<Repository::ConstPointer>::const_iterator
        r(_imp->repositories.begin()),
        r_end(_imp->repositories.end());
    for ( ; r != r_end ; ++r)
        if ((*r)->name() == n)
            return *r;

    throw NoSuchRepositoryError(stringify(n));
}

const RepositoryName &
PackageDatabase::better_repository(const RepositoryName & r1,
        const RepositoryName & r2) const
{
    IndirectIterator<std::list<Repository::ConstPointer>::const_iterator, const Repository>
        r(_imp->repositories.begin()),
        r_end(_imp->repositories.end());
    for ( ; r != r_end ; ++r)
    {
        if (r->name() == r1)
            return r2;
        else if (r->name() == r2)
            return r1;
    }

    throw InternalError(PALUDIS_HERE, "better_repository called on non-owned repositories");
}

RepositoryName
PackageDatabase::favourite_repository() const
{
    for (RepositoryIterator r(_imp->repositories.begin()), r_end(_imp->repositories.end()) ;
            r != r_end ; ++r)
        if ((*r)->can_be_favourite_repository())
            return (*r)->name();

    return RepositoryName("unnamed");
}

PackageDatabaseEntryCollection::Pointer
PackageDatabase::query(PackageDepAtom::ConstPointer a, const InstallState s) const
{
    return _do_query(*a.raw_pointer(), s);
}

PackageDatabase::RepositoryIterator
PackageDatabase::begin_repositories() const
{
    return RepositoryIterator(_imp->repositories.begin());
}

PackageDatabase::RepositoryIterator
PackageDatabase::end_repositories() const
{
    return RepositoryIterator(_imp->repositories.end());
}

