/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
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

#include <python/paludis_python.hh>
#include <python/exception.hh>

#include <paludis/query.hh>
#include <paludis/dep_spec.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/environment.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

class PythonQueryDelegate;

class PythonQuery :
    public Query
{
    public:
        PythonQuery();

        virtual tr1::shared_ptr<RepositoryNameSequence> repositories(const Environment &) const
        {
            return tr1::shared_ptr<RepositoryNameSequence>();
        }

        virtual tr1::shared_ptr<CategoryNamePartSet> categories(const Environment &,
                tr1::shared_ptr<const RepositoryNameSequence>) const
        {
            return tr1::shared_ptr<CategoryNamePartSet>();
        }

        virtual tr1::shared_ptr<QualifiedPackageNameSet> packages(const Environment &,
                tr1::shared_ptr<const RepositoryNameSequence>,
                tr1::shared_ptr<const CategoryNamePartSet>) const
        {
            return tr1::shared_ptr<QualifiedPackageNameSet>();
        }

        virtual tr1::shared_ptr<PackageIDSequence> ids(const Environment &,
                tr1::shared_ptr<const RepositoryNameSequence>,
                tr1::shared_ptr<const QualifiedPackageNameSet>) const
        {
            return tr1::shared_ptr<PackageIDSequence>();
        }

        virtual std::string as_human_readable_string() const = 0;
};

class PythonQueryDelegate :
    public QueryDelegate
{
    private:
        PythonQuery * _q;

    public:
        PythonQueryDelegate(PythonQuery * pq) :
            _q(pq)
        {
        }

        tr1::shared_ptr<RepositoryNameSequence> repositories(const Environment & e) const
        {
            return _q->repositories(e);
        }

        tr1::shared_ptr<CategoryNamePartSet> categories(const Environment & e,
                tr1::shared_ptr<const RepositoryNameSequence> r) const
        {
            return _q->categories(e, r);
        }

        tr1::shared_ptr<QualifiedPackageNameSet> packages(const Environment & e,
                tr1::shared_ptr<const RepositoryNameSequence> r,
                tr1::shared_ptr<const CategoryNamePartSet> c) const
        {
            return _q->packages(e, r, c);
        }

        tr1::shared_ptr<PackageIDSequence> ids(const Environment & e,
                tr1::shared_ptr<const RepositoryNameSequence> r,
                tr1::shared_ptr<const QualifiedPackageNameSet> q) const
        {
            return _q->ids(e, r, q);
        }

        std::string as_human_readable_string() const
        {
            return _q->as_human_readable_string();
        }
};

PythonQuery::PythonQuery() :
    Query(tr1::shared_ptr<const PythonQueryDelegate>(new PythonQueryDelegate(this)))
{
}

struct PythonQueryWrapper :
    PythonQuery,
    bp::wrapper<PythonQuery>
{
    tr1::shared_ptr<RepositoryNameSequence> repositories(const Environment & e) const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("repositories"))
            return f(boost::cref(e));
        return PythonQuery::repositories(e);
    }

    tr1::shared_ptr<RepositoryNameSequence> default_repositories(const Environment & e) const
    {
        return PythonQuery::repositories(e);
    }

    tr1::shared_ptr<CategoryNamePartSet> categories(const Environment & e,
            tr1::shared_ptr<const RepositoryNameSequence> r) const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("categories"))
            return f(boost::cref(e), r);
        return PythonQuery::categories(e, r);
    }

    tr1::shared_ptr<CategoryNamePartSet> default_categories(const Environment & e,
            tr1::shared_ptr<const RepositoryNameSequence> r) const
    {
        return PythonQuery::categories(e, r);
    }

    tr1::shared_ptr<QualifiedPackageNameSet> packages(const Environment & e,
            tr1::shared_ptr<const RepositoryNameSequence> r,
            tr1::shared_ptr<const CategoryNamePartSet> c) const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("packages"))
            return f(boost::cref(e), r, c);
        return PythonQuery::packages(e, r, c);
    }

    tr1::shared_ptr<QualifiedPackageNameSet> default_packages(const Environment & e,
            tr1::shared_ptr<const RepositoryNameSequence> r,
            tr1::shared_ptr<const CategoryNamePartSet> c) const
    {
        return PythonQuery::packages(e, r, c);
    }

    tr1::shared_ptr<PackageIDSequence> ids(const Environment & e,
            tr1::shared_ptr<const RepositoryNameSequence> r,
            tr1::shared_ptr<const QualifiedPackageNameSet> q) const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("ids"))
            return f(boost::cref(e), r, q);
        return PythonQuery::ids(e, r, q);
    }

    tr1::shared_ptr<PackageIDSequence> default_ids(const Environment & e,
            tr1::shared_ptr<const RepositoryNameSequence> r,
            tr1::shared_ptr<const QualifiedPackageNameSet> q) const
    {
        return PythonQuery::ids(e, r, q);
    }

    std::string as_human_readable_string() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("as_human_readable_string"))
            return f();
        else
            throw PythonMethodNotImplemented("Query", "as_human_readable_string");
    }
};

template <typename A_>
class class_supports_action :
    public bp::class_<query::SupportsAction<A_>, bp::bases<Query> >
{
    public:
        class_supports_action(const std::string & action) :
            bp::class_<query::SupportsAction<A_>, bp::bases<Query> >(
                    ("Supports" + action + "Action").c_str(),
                    ("Fetch packages that support " + action + "Action.").c_str(),
                    bp::init<>("__init__()")
                    )
        {
        }
};


void expose_query() PALUDIS_VISIBLE
{
    /**
     * Query
     */
    bp::class_<Query>
        q(
         "Query",
         "Parameter for a PackageDatabase query.",
         bp::no_init
        );
        q.def("__and__", operator&);

    bp::class_<PythonQueryWrapper, bp::bases<Query>, boost::noncopyable>
        (
         "QueryBase",
         "Parameter for a PackageDatabase query.\n"
         "This class can be subclassed in Python",
         bp::init<>()
        )
        .def("repositories", &PythonQuery::repositories, &PythonQueryWrapper::default_repositories,
                "repositories(Environment) -> list of RepositoryName\n"
                "Fetch the names of repositories potentially containing matches.\n"
                "All returned repositories must exist.\n\n"
                "Default behaviour: return all repositories."
            )

        .def("categories", &PythonQuery::categories, &PythonQueryWrapper::default_categories,
                "categories(Environment, RepositoryNameIterable) -> list of CategoryNamePart\n"
                "Fetch the names of categories potentially containing matches.\n\n"
                "Default behaviour: return all categories in the provided\n"
                "repository collection."
            )

        .def("packages", &PythonQuery::packages, &PythonQueryWrapper::default_packages,
                "packages(Environment, RepositoryNameIterable, CategoryNamePartIterable) "
                    "-> list of QualifiedPackageName\n"
                "Fetch the names of packages potentially containing matches.\n\n"
                "Default behaviour: return all packages in the provided repository\n"
                "in the provided categories.\n\n"
                "Note that some entries in the categories collection (but not in\n"
                "the repositories collection) may not exist."
            )

        .def("ids", &PythonQuery::ids, &PythonQueryWrapper::default_ids,
                "ids(Environment, RepositoryNameIterable, QualifiedPackageNameIterable)\n"
                "Fetch the IDs of matching packages.\n\n"
                "Default behaviour: return all IDs in the provided packages.\n\n"
                "Note that some entries in the qualified package name collection\n"
                "(but not in the repositories collection) may not exist."
            )
        ;

    /* I need to think about it yet... */
    bp::scope query = q;

    /**
     * Matches
     */
    bp::class_<query::Matches, bp::bases<Query> >
        (
         "Matches",
         "Fetch packages matching a given PackageDepSpec.",
         bp::init<const PackageDepSpec &>("__init__(PackageDepSpec)")
        );

    /**
     * Package
     */
    bp::class_<query::Package, bp::bases<Query> >
        (
         "Package",
         "Fetch packages with a given package name.",
         bp::init<const QualifiedPackageName &>("__init__(QualifiedPackageName)")
        );

    /**
     * Repository
     */
    bp::class_<query::Repository, bp::bases<Query> >
        (
         "Repository",
         "Fetch packages in a given repository.",
         bp::init<const RepositoryName &>("__init__(RepositoryName)")
        );

    /**
     * Category
     */
    bp::class_<query::Category, bp::bases<Query> >
        (
         "Category",
         "Fetch packages in a given Category.",
         bp::init<const CategoryNamePart &>("__init__(CategoryNamePart)")
        );

    /**
     * NotMasked
     */
    bp::class_<query::NotMasked, bp::bases<Query> >
        (
         "NotMasked",
         "Fetch packages that are not masked.",
         bp::init<>("__init__()")
        );

    /**
     * SupportsAction
     */
    class_supports_action<InstallAction>("Install");
    class_supports_action<UninstallAction>("Uninstall");
    class_supports_action<InstalledAction>("Installed");
    class_supports_action<PretendAction>("Pretend");
    class_supports_action<ConfigAction>("Config");

    /**
     * InstalledAtRoot
     */
    bp::class_<query::InstalledAtRoot, bp::bases<Query> >
        (
         "InstalledAtRoot",
         "Fetch packages that are installed at a particular root.",
         bp::init<const FSEntry &>("__init__(path)")
        );

    /**
     * All
     */
    bp::class_<query::All, bp::bases<Query> >
        (
         "All",
         "Fetch all packages.",
         bp::init<>("__init__()")
        );
}
