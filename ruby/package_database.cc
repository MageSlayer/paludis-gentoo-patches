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

#include <paludis_ruby.hh>
#include <paludis/package_database.hh>
#include <paludis/util/stringify.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_package_database;
    static VALUE c_package_database_install_state;
    static VALUE c_package_database_query_order;

    /*
     * call-seq:
     *     favourite_repository -> String
     *
     * Fetch the name of our 'favourite' repository
     */
    VALUE
    package_database_favourite_repository(VALUE self)
    {
        try
        {
            std::tr1::shared_ptr<PackageDatabase> * self_ptr;
            Data_Get_Struct(self, std::tr1::shared_ptr<PackageDatabase>, self_ptr);
            return rb_str_new2(stringify((*self_ptr)->favourite_repository()).c_str());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     fetch_unique_qualified_package_name(package_name) -> QualifiedPackageName
     *
     * Disambiguate a package name
     */
    VALUE
    package_database_fetch_unique_qualified_package_name(VALUE self, VALUE pkg)
    {
        try
        {
            std::tr1::shared_ptr<PackageDatabase> * self_ptr;
            Data_Get_Struct(self, std::tr1::shared_ptr<PackageDatabase>, self_ptr);
            return rb_str_new2(stringify((*self_ptr)->fetch_unique_qualified_package_name(
                            PackageNamePart(StringValuePtr(pkg)))).c_str());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     query(query, query_order) -> Array
     *     query(spec, install_state, query_order) -> Array
     *
     *  Query the repository, the first argument is either a PackageDepSpec or a Query.
     *  Returns an array of PackageDatabaseEntry.
     */
    VALUE
    package_database_query(int argc, VALUE *argv, VALUE self)
    {
        std::tr1::shared_ptr<const PackageDatabaseEntryCollection> items;
        try
        {
            if (2 == argc && is_kind_of_query(argv[0]))
            {
                Query q = value_to_query(argv[0]);
                QueryOrder qo = static_cast<QueryOrder>(NUM2INT(argv[1]));

                std::tr1::shared_ptr<PackageDatabase> * self_ptr;
                Data_Get_Struct(self, std::tr1::shared_ptr<PackageDatabase>, self_ptr);

                items = ((*self_ptr)->query(q, qo));
            }
            else if (2 == argc || 3 == argc)
            {
                QueryOrder qo;
                Query q  = query::Matches(*value_to_package_dep_spec(argv[0]));
                InstallState is = static_cast<InstallState>(NUM2INT(argv[1]));

                switch (is)
                {
                    case is_installed_only:
                        q = q & query::RepositoryHasInstalledInterface();
                        break;

                    case is_installable_only:
                        q = q & query::RepositoryHasInstallableInterface();
                        break;

                    case is_any:
                    case last_is:
                        ;

                }
                if (2 ==argc)
                {
                    qo = qo_order_by_version;
                    rb_warn("Calling query with (PackageDepSpec, InstallState) has been deprecated");
                }
                else
                {
                    qo = static_cast<QueryOrder>(NUM2INT(argv[2]));
                    rb_warn("Calling query with (PackageDepSpec, InstallState, QUeryOrder) has been deprecated");
                }

                std::tr1::shared_ptr<PackageDatabase> * self_ptr;
                Data_Get_Struct(self, std::tr1::shared_ptr<PackageDatabase>, self_ptr);

                items = ((*self_ptr)->query(q, qo));

            }
            else
            {
                rb_raise(rb_eArgError, "query expects two or three arguments, but got %d",argc);
            }
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
        VALUE result(rb_ary_new());
        for (PackageDatabaseEntryCollection::Iterator i(items->begin()),
                i_end(items->end()) ; i != i_end ; ++i)
            rb_ary_push(result, package_database_entry_to_value(*i));
        return result;
    }

    /*
     * call-seq:
     *     repositories -> Array
     *     repositories {|repository| block } -> Nil
     *
     *  Returns all the repositories in the package database, either as an Array, or as
     *  the parameters to a block.
     */
    VALUE
    package_database_repositories(VALUE self)
    {
        try
        {
            std::tr1::shared_ptr<PackageDatabase> * self_ptr;
            Data_Get_Struct(self, std::tr1::shared_ptr<PackageDatabase>, self_ptr);

            if (rb_block_given_p())
            {
                for (PackageDatabase::RepositoryIterator r((*self_ptr)->begin_repositories()),
                        r_end((*self_ptr)->end_repositories()) ; r != r_end ; ++r)
                    rb_yield(repository_to_value(*r));
                return Qnil;
            }
            VALUE result(rb_ary_new());
            for (PackageDatabase::RepositoryIterator r((*self_ptr)->begin_repositories()),
                    r_end((*self_ptr)->end_repositories()) ; r != r_end ; ++r)
                rb_ary_push(result, repository_to_value(*r));

            return result;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     fetch_repository(repository_name) -> Repository
     *
     *  Fetch a named repository.
     */
    VALUE
    package_database_fetch_repository(VALUE self, VALUE name)
    {
        try
        {
            std::tr1::shared_ptr<PackageDatabase> * self_ptr;
            Data_Get_Struct(self, std::tr1::shared_ptr<PackageDatabase>, self_ptr);

            return repository_to_value((*self_ptr)->fetch_repository(RepositoryName(StringValuePtr(name))));
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     more_important_than(repository_name_a, repository_name_b) -> bool
     *
     * True if repository_name_a is more important than repository_name_b .
     */
    VALUE
    package_database_more_important_than(VALUE self, VALUE name1, VALUE name2)
    {
        try
        {
            std::tr1::shared_ptr<PackageDatabase> * self_ptr;
            Data_Get_Struct(self, std::tr1::shared_ptr<PackageDatabase>, self_ptr);

            return (*self_ptr)->more_important_than(RepositoryName(StringValuePtr(name1)),
                    RepositoryName(StringValuePtr(name2))) ? Qtrue : Qfalse;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    void do_register_package_database()
    {
        /*
         * Document-class: Paludis::PackageDatabase
         *
         * A PackageDatabase can be queried for Package instances.
         */
        c_package_database = rb_define_class_under(paludis_module(), "PackageDatabase", rb_cObject);
        rb_funcall(c_package_database, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_package_database, "favourite_repository", RUBY_FUNC_CAST(&package_database_favourite_repository), 0);
        rb_define_method(c_package_database, "fetch_unique_qualified_package_name",
                RUBY_FUNC_CAST(&package_database_fetch_unique_qualified_package_name), 1);
        rb_define_method(c_package_database, "query",
                RUBY_FUNC_CAST(&package_database_query), -1);
        rb_define_method(c_package_database, "repositories",
                RUBY_FUNC_CAST(&package_database_repositories), 0);
        rb_define_method(c_package_database, "fetch_repository",
                RUBY_FUNC_CAST(&package_database_fetch_repository), 1);
        rb_define_method(c_package_database, "more_important_than",
                RUBY_FUNC_CAST(&package_database_more_important_than), 2);

        /*
         * Document-module: Paludis::InstallState
         *
         * Do we want only installed packages, only installable packages, or any package when querying?
         */
        c_package_database_install_state = rb_define_class_under(paludis_module(), "InstallState", rb_cObject);
        for (InstallState l(static_cast<InstallState>(0)), l_end(last_is) ; l != l_end ;
                l = static_cast<InstallState>(static_cast<int>(l) + 1))
            rb_define_const(c_package_database_install_state, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/package_database.hh, InstallState, c_package_database_install_state>

        /*
         * Document-module: Paludis::QueryOrder
         *
         * How to order query results.
         */
        c_package_database_query_order = rb_define_class_under(paludis_module(), "QueryOrder", rb_cObject);
        for (QueryOrder l(static_cast<QueryOrder>(0)), l_end(last_qo) ; l != l_end ;
                l = static_cast<QueryOrder>(static_cast<int>(l) + 1))
            rb_define_const(c_package_database_query_order, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/package_database.hh, QueryOrder, c_package_database_query_order>
    }
}

VALUE
paludis::ruby::package_database_to_value(std::tr1::shared_ptr<PackageDatabase> m)
{
    std::tr1::shared_ptr<PackageDatabase> * m_ptr(0);
    try
    {
        m_ptr = new std::tr1::shared_ptr<PackageDatabase>(m);
        return Data_Wrap_Struct(c_package_database, 0, &Common<std::tr1::shared_ptr<PackageDatabase> >::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

RegisterRubyClass::Register paludis_ruby_register_package_database PALUDIS_ATTRIBUTE((used))
    (&do_register_package_database);

