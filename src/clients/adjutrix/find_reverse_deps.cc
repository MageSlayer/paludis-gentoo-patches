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

#include "find_reverse_deps.hh"
#include "command_line.hh"
#include <output/colour.hh>

#include <paludis/util/compare.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/save.hh>
#include <paludis/util/log.hh>
#include <paludis/query.hh>

#include <set>
#include <map>
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

namespace
{
    class ReverseDepChecker :
        public DepSpecVisitorTypes::ConstVisitor,
        public DepSpecVisitorTypes::ConstVisitor::VisitChildren<ReverseDepChecker, AllDepSpec>
    {
        private:
            std::tr1::shared_ptr<const PackageDatabase> _db;
            const PackageDatabaseEntryCollection & _entries;
            std::string _depname;
            std::string _p;

            bool _in_any;
            bool _in_use;
            std::string _flags;

            bool _found_matches;

        public:
            using DepSpecVisitorTypes::ConstVisitor::VisitChildren<ReverseDepChecker, AllDepSpec>::visit;

            ReverseDepChecker(std::tr1::shared_ptr<const PackageDatabase> db,
                    const PackageDatabaseEntryCollection & entries,
                    const std::string & p) :
                _db(db),
                _entries(entries),
                _depname(""),
                _p(p),
                _in_any(false),
                _in_use(false),
                _found_matches(false)
            {
            }

            void check(std::tr1::shared_ptr<const DepSpec> spec, const std::string & depname)
            {
                _depname = depname;
                spec->accept(this);
            }

            bool found_matches()
            {
                return _found_matches;
            }

            void visit(const AnyDepSpec * const);

            void visit(const UseDepSpec * const);

            void visit(const PackageDepSpec * const);

            void visit(const PlainTextDepSpec * const); 

            void visit(const BlockDepSpec * const);
    };

    void
    ReverseDepChecker::visit(const AnyDepSpec * const a)
    {
        Save<bool> in_any_save(&_in_any, true);

        std::for_each(a->begin(), a->end(), accept_visitor(this));
    }

    void
    ReverseDepChecker::visit(const UseDepSpec * const a)
    {
        Save<bool> in_use_save(&_in_use, true);
        Save<std::string> flag_save(&_flags);

        if (! _flags.empty())
            _flags += " ";
        _flags += (a->inverse() ? "!" : "") + stringify(a->flag());

        std::for_each(a->begin(), a->end(), accept_visitor(this));
    }

    void
    ReverseDepChecker::visit(const PackageDepSpec * const a)
    {
        std::tr1::shared_ptr<const PackageDatabaseEntryCollection> dep_entries(_db->query(
                    query::Matches(*a), qo_order_by_version));
        std::tr1::shared_ptr<PackageDatabaseEntryCollection> matches(new PackageDatabaseEntryCollection::Concrete);

        bool header_written = false;

        for (PackageDatabaseEntryCollection::Iterator e(dep_entries->begin()), e_end(dep_entries->end()) ;
                e != e_end ; ++e)
        {
            if (_entries.find(*e) != _entries.end())
            {
                _found_matches |= true;

                if (! header_written)
                {
                    std::cout << "  " << _p << " " + _depname + " on one of:" << std::endl;
                    header_written = true;
                }
                std::cout << "    " << stringify(*e);

                if (_in_use || _in_any)
                {
                    std::cout << " (";

                    if (_in_any)
                        std::cout << "any-of";

                    if (_in_use && _in_any)
                        std::cout << ", ";

                    if (_in_use)
                        std::cout << "condition USE='" << _flags << "'";

                    std::cout << ")";
                }
                std::cout << std::endl;
            }
        }
    }

    void
    ReverseDepChecker::visit(const PlainTextDepSpec * const)
    {
    }

    void
    ReverseDepChecker::visit(const BlockDepSpec * const)
    {
    }

    void write_repository_header(std::string spec, const std::string &)
    {
        cout << "Reverse dependencies for '" << spec << "':" << std::endl;
    }

    int check_one_package(const Environment & env, const Repository & r,
            const PackageDatabaseEntryCollection & entries, const QualifiedPackageName & p)
    {
        Context context("When checking package '" + stringify(p) + "':");

        std::tr1::shared_ptr<PackageDatabaseEntryCollection> p_entries(env.package_database()->query(
                query::Package(p), qo_order_by_version));

        bool found_matches(false);

        for (PackageDatabaseEntryCollection::Iterator e(p_entries->begin()), e_end(p_entries->end()) ;
                e != e_end ; ++e)
        {
            try
            {
                std::tr1::shared_ptr<const VersionMetadata> metadata(r.version_metadata(e->name, e->version));
                ReverseDepChecker checker(env.package_database(), entries,
                        stringify(p) + "-" + stringify(e->version));

                if (metadata->deps_interface)
                {
                    checker.check(metadata->deps_interface->build_depend(), std::string("DEPEND"));
                    checker.check(metadata->deps_interface->run_depend(), std::string("RDEPEND"));
                    checker.check(metadata->deps_interface->post_depend(), std::string("PDEPEND"));
                    checker.check(metadata->deps_interface->suggested_depend(), std::string("SDEPEND"));
                }

                found_matches |= checker.found_matches();
            }
            catch (Exception & exception)
            {
                cerr << "Caught exception:" << endl;
                cerr << "  * " << exception.backtrace("\n  * ") << endl;
                cerr << "  * " << exception.message() << " (" << exception.what() << ")" << endl;
                return (found_matches ? 0 : 1) | 2;
            }
        }

        return found_matches ? 0 : 1;
    }
}

int do_find_reverse_deps(NoConfigEnvironment & env)
{
    Context context("When performing find-reverse-deps action:");

    std::tr1::shared_ptr<PackageDepSpec> spec;
    try
    {
        if (std::string::npos == CommandLine::get_instance()->begin_parameters()->find('/'))
        {
            spec.reset(new PackageDepSpec(
                        std::tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(
                                env.package_database()->fetch_unique_qualified_package_name(
                                    PackageNamePart(*CommandLine::get_instance()->begin_parameters()))))));
        }
        else
            spec.reset(new PackageDepSpec(*CommandLine::get_instance()->begin_parameters(), pds_pm_unspecific));
    }
    catch (const AmbiguousPackageNameError & e)
    {
        cout << endl;
        cerr << "Query error:" << endl;
        cerr << "  * " << e.backtrace("\n  * ");
        cerr << "Ambiguous package name '" << e.name() << "'. Did you mean:" << endl;
        for (AmbiguousPackageNameError::OptionsIterator o(e.begin_options()),
                o_end(e.end_options()) ; o != o_end ; ++o)
            cerr << "    * " << colour(cl_package_name, *o) << endl;
        cerr << endl;
        return 4;
    }

    std::tr1::shared_ptr<PackageDatabaseEntryCollection> entries(env.package_database()->query(
                query::Matches(*spec), qo_order_by_version));
    int ret(0);

    if (entries->empty())
    {
        Log::get_instance()->message(ll_warning, lc_context, "No matches in package database for '"
                + stringify(*spec) + "'");
        return 1;
    }

    for (IndirectIterator<PackageDatabase::RepositoryIterator, const Repository>
            r(env.package_database()->begin_repositories()),
            r_end(env.package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (r->name() == RepositoryName("virtuals") || r->name() == RepositoryName("installed_virtuals"))
            continue;

        write_repository_header(stringify(*spec), stringify(r->name()));

        std::tr1::shared_ptr<const CategoryNamePartCollection> cat_names(r->category_names());
        for (CategoryNamePartCollection::Iterator c(cat_names->begin()), c_end(cat_names->end()) ;
                c != c_end ; ++c)
        {
            cerr << xterm_title("Checking " + stringify(*c) + " - adjutrix");

            if (CommandLine::get_instance()->a_category.specified())
                if (CommandLine::get_instance()->a_category.end_args() == std::find(
                            CommandLine::get_instance()->a_category.begin_args(),
                            CommandLine::get_instance()->a_category.end_args(),
                            stringify(*c)))
                    continue;

            std::tr1::shared_ptr<const QualifiedPackageNameCollection> pkg_names(r->package_names(*c));
            for (QualifiedPackageNameCollection::Iterator p(pkg_names->begin()), p_end(pkg_names->end()) ;
                    p != p_end ; ++p)
            {
                if (CommandLine::get_instance()->a_package.specified())
                    if (CommandLine::get_instance()->a_package.end_args() == std::find(
                                CommandLine::get_instance()->a_package.begin_args(),
                                CommandLine::get_instance()->a_package.end_args(),
                                stringify(p->package)))
                        continue;

                ret |= check_one_package(env, *r, *entries, *p);
            }
        }
    }

    return ret;
}


