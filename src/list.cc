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

#include "colour.hh"
#include "list.hh"
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <paludis/paludis.hh>
#include <paludis/util/log.hh>
#include <paludis/util/visitor.hh>

namespace p = paludis;

int
do_list_repositories()
{
    int ret_code(1);

    p::Context context("When performing list-repositories action from command line:");
    p::Environment * const env(p::DefaultEnvironment::get_instance());

    for (p::IndirectIterator<p::PackageDatabase::RepositoryIterator, const p::Repository>
            r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        if (CommandLine::get_instance()->a_repository.specified())
            if (CommandLine::get_instance()->a_repository.args_end() == std::find(
                        CommandLine::get_instance()->a_repository.args_begin(),
                        CommandLine::get_instance()->a_repository.args_end(),
                        stringify(r->name())))
                continue;

        ret_code = 0;

        std::cout << "* " << colour(cl_package_name, r->name()) << std::endl;

        p::RepositoryInfo::ConstPointer ii(r->info(false));
        for (p::RepositoryInfo::SectionIterator i(ii->begin_sections()),
                i_end(ii->end_sections()) ; i != i_end ; ++i)
        {
            std::cout << "    " << colour(cl_heading, i->heading() + ":") << std::endl;
            for (p::RepositoryInfoSection::KeyValueIterator k(i->begin_kvs()),
                    k_end(i->end_kvs()) ; k != k_end ; ++k)
                std::cout << "        " << std::setw(22) << std::left << (p::stringify(k->first) + ":")
                    << std::setw(0) << " " << k->second << std::endl;
            std::cout << std::endl;
        }
    }

    return ret_code;
}

int
do_list_categories()
{
    int ret_code(1);

    p::Context context("When performing list-categories action from command line:");
    p::Environment * const env(p::DefaultEnvironment::get_instance());

    std::map<p::CategoryNamePart, std::list<p::RepositoryName> > cats;

    for (p::IndirectIterator<p::PackageDatabase::RepositoryIterator, const p::Repository>
            r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        if (CommandLine::get_instance()->a_repository.specified())
            if (CommandLine::get_instance()->a_repository.args_end() == std::find(
                        CommandLine::get_instance()->a_repository.args_begin(),
                        CommandLine::get_instance()->a_repository.args_end(),
                        stringify(r->name())))
                continue;

        p::CategoryNamePartCollection::ConstPointer cat_names(r->category_names());
        for (p::CategoryNamePartCollection::Iterator c(cat_names->begin()), c_end(cat_names->end()) ;
                c != c_end ; ++c)
            cats[*c].push_back(r->name());
    }

    for (std::map<p::CategoryNamePart, std::list<p::RepositoryName > >::const_iterator
            c(cats.begin()), c_end(cats.end()) ; c != c_end ; ++c)
    {
        if (CommandLine::get_instance()->a_category.specified())
            if (CommandLine::get_instance()->a_category.args_end() == std::find(
                        CommandLine::get_instance()->a_category.args_begin(),
                        CommandLine::get_instance()->a_category.args_end(),
                        stringify(c->first)))
                continue;

        ret_code = 0;

        std::cout << "* " << colour(cl_package_name, c->first) << std::endl;
        std::cout << "    " << std::setw(22) << std::left << "found in:" <<
            std::setw(0) << " " << p::join(c->second.begin(), c->second.end(), ", ") << std::endl;
        std::cout << std::endl;
    }

    return ret_code;
}

int
do_list_packages()
{
    int ret_code(1);

    p::Context context("When performing list-packages action from command line:");
    p::Environment * const env(p::DefaultEnvironment::get_instance());

    std::map<p::QualifiedPackageName, std::list<p::RepositoryName> > pkgs;

    for (p::IndirectIterator<p::PackageDatabase::RepositoryIterator, const p::Repository>
            r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        if (CommandLine::get_instance()->a_repository.specified())
            if (CommandLine::get_instance()->a_repository.args_end() == std::find(
                        CommandLine::get_instance()->a_repository.args_begin(),
                        CommandLine::get_instance()->a_repository.args_end(),
                        stringify(r->name())))
                continue;

        p::CategoryNamePartCollection::ConstPointer cat_names(r->category_names());
        for (p::CategoryNamePartCollection::Iterator c(cat_names->begin()), c_end(cat_names->end()) ;
                c != c_end ; ++c)
        {
            if (CommandLine::get_instance()->a_category.specified())
                if (CommandLine::get_instance()->a_category.args_end() == std::find(
                            CommandLine::get_instance()->a_category.args_begin(),
                            CommandLine::get_instance()->a_category.args_end(),
                            stringify(*c)))
                    continue;

            p::QualifiedPackageNameCollection::ConstPointer pkg_names(r->package_names(*c));
            for (p::QualifiedPackageNameCollection::Iterator p(pkg_names->begin()), p_end(pkg_names->end()) ;
                    p != p_end ; ++p)
                pkgs[*p].push_back(r->name());
        }
    }

    for (std::map<p::QualifiedPackageName, std::list<p::RepositoryName > >::const_iterator
            p(pkgs.begin()), p_end(pkgs.end()) ; p != p_end ; ++p)
    {
        if (CommandLine::get_instance()->a_package.specified())
            if (CommandLine::get_instance()->a_package.args_end() == std::find(
                        CommandLine::get_instance()->a_package.args_begin(),
                        CommandLine::get_instance()->a_package.args_end(),
                        stringify(p->first.get<p::qpn_package>())))
                continue;

        ret_code = 0;

        std::cout << "* " << colour(cl_package_name, p->first) << std::endl;
        std::cout << "    " << std::setw(22) << std::left << "found in:" <<
            std::setw(0) << " " << p::join(p->second.begin(), p->second.end(), ", ") << std::endl;
        std::cout << std::endl;
    }

    return ret_code;
}

namespace
{
    /**
    * Print dependency atoms as returned by do_package_set("security").
    *
    * \ingroup grpvulnerabilitiesprinter
    */
    class VulnerabilitiesPrinter :
        public p::DepAtomVisitorTypes::ConstVisitor
    {
        private:
            unsigned _size;

        public:
            /**
             * Constructor.
             */
            VulnerabilitiesPrinter() :
                _size(0)
            {
            }

            /// \name Visit functions
            ///{
            void visit(const p::AllDepAtom * const);

            void visit(const p::AnyDepAtom * const);

            void visit(const p::UseDepAtom * const);

            void visit(const p::PackageDepAtom * const);

            void visit(const p::PlainTextDepAtom * const);

            void visit(const p::BlockDepAtom * const);
            ///}

            /**
             * Return number of visited atoms.
             */
            unsigned size() const
            {
                return _size;
            }
    };

    void
    VulnerabilitiesPrinter::visit(const p::AllDepAtom * const a)
    {
        std::for_each(a->begin(), a->end(), p::accept_visitor(this));
    }

    void
    VulnerabilitiesPrinter::visit(const p::AnyDepAtom * const a)
    {
        std::for_each(a->begin(), a->end(), p::accept_visitor(this));
    }

    void
    VulnerabilitiesPrinter::visit(const p::UseDepAtom * const a)
    {
        p::Log::get_instance()->message(p::ll_warning, p::lc_no_context,
                "UseDepAtom encounter in do_package_set(\"security\").");
        std::for_each(a->begin(), a->end(), p::accept_visitor(this));
    }

    void
    VulnerabilitiesPrinter::visit(const p::PackageDepAtom * const a)
    {
        p::QualifiedPackageName q(a->package());

        std::string c(p::stringify(q.get<p::qpn_category>()));
        if (CommandLine::get_instance()->a_category.specified())
            if (CommandLine::get_instance()->a_category.args_end() == std::find(
                            CommandLine::get_instance()->a_category.args_begin(),
                            CommandLine::get_instance()->a_category.args_end(),
                            c))
                    return;

        std::string p(p::stringify(q.get<p::qpn_package>()));
        if (CommandLine::get_instance()->a_package.specified())
            if (CommandLine::get_instance()->a_package.args_end() == std::find(
                            CommandLine::get_instance()->a_package.args_begin(),
                            CommandLine::get_instance()->a_package.args_end(),
                            p))
                    return;

        std::cout << "* " << colour(cl_package_name, p::stringify(q));
        if (0 != a->tag())
            std::cout << "-" << p::stringify(*a->version_spec_ptr());
        if (0 != a->tag())
            std::cout << " " << colour(cl_tag, "<" + a->tag()->short_text() + ">");
        std::cout << std::endl;
        ++_size;
    }

    void
    VulnerabilitiesPrinter::visit(const p::PlainTextDepAtom * const)
    {
    }

    void
    VulnerabilitiesPrinter::visit(const p::BlockDepAtom * const)
    {
    }
}

int
do_list_vulnerabilities()
{
    int ret_code = 0;

    p::Context context("When performing list-vulnerabilities action from command line:");
    p::Environment * const env(p::DefaultEnvironment::get_instance());
    p::PackageSetOptions opts(true);

    p::CompositeDepAtom::Pointer vulnerabilities(new p::AllDepAtom);

    for (p::IndirectIterator<p::PackageDatabase::RepositoryIterator, const p::Repository>
            r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        if (CommandLine::get_instance()->a_repository.specified())
            if (CommandLine::get_instance()->a_repository.args_end() == std::find(
                        CommandLine::get_instance()->a_repository.args_begin(),
                        CommandLine::get_instance()->a_repository.args_end(),
                        stringify(r->name())))
                continue;

        if (! r->get_interface<p::repo_sets>())
                continue;

        p::DepAtom::Pointer dep = r->get_interface<p::repo_sets>()->package_set("security", opts);
        if (0 != dep)
            vulnerabilities->add_child(dep);
    }

    VulnerabilitiesPrinter vp;
    std::for_each(vulnerabilities->begin(), vulnerabilities->end(), p::accept_visitor(&vp));

    if (vp.size() == 0)
        ret_code = 1;

    return ret_code;
}

