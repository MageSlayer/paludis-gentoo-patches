/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2011 Ciaran McCreesh
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
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/sequence.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

namespace
{
    static VALUE c_package_database;

    /*
     * call-seq:
     *     fetch_unique_qualified_package_name(package_name) -> QualifiedPackageName
     *     fetch_unique_qualified_package_name(package_name, filter) -> QualifiedPackageName
     *
     * Disambiguate a package name.  If a filter is specified, limit
     * the potential results to packages that match.
     */
    VALUE
    package_database_fetch_unique_qualified_package_name(int argc, VALUE *argv, VALUE self)
    {
        try
        {
            if (1 == argc || 2 == argc)
            {
                std::shared_ptr<PackageDatabase> * self_ptr;
                Data_Get_Struct(self, std::shared_ptr<PackageDatabase>, self_ptr);
                return qualified_package_name_to_value((*self_ptr)->fetch_unique_qualified_package_name(
                                PackageNamePart(StringValuePtr(argv[0])), 2 == argc ? value_to_filter(argv[1]) : filter::All()));
            }
            else
                rb_raise(rb_eArgError, "fetch_unique_qualified_package_name expects one or two arguments, but got %d",argc);
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
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
            std::shared_ptr<PackageDatabase> * self_ptr;
            Data_Get_Struct(self, std::shared_ptr<PackageDatabase>, self_ptr);

            if (rb_block_given_p())
            {
                for (PackageDatabase::RepositoryConstIterator r((*self_ptr)->begin_repositories()),
                        r_end((*self_ptr)->end_repositories()) ; r != r_end ; ++r)
                    rb_yield(repository_to_value(*r));
                return Qnil;
            }
            VALUE result(rb_ary_new());
            for (PackageDatabase::RepositoryConstIterator r((*self_ptr)->begin_repositories()),
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
            std::shared_ptr<PackageDatabase> * self_ptr;
            Data_Get_Struct(self, std::shared_ptr<PackageDatabase>, self_ptr);

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
            std::shared_ptr<PackageDatabase> * self_ptr;
            Data_Get_Struct(self, std::shared_ptr<PackageDatabase>, self_ptr);

            return (*self_ptr)->more_important_than(RepositoryName(StringValuePtr(name1)),
                    RepositoryName(StringValuePtr(name2))) ? Qtrue : Qfalse;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     has_repository_named?(repository_name) -> true or false
     *
     *  Do we have a named repository?
     */
    VALUE
    package_database_has_repository_named(VALUE self, VALUE name)
    {
        try
        {
            std::shared_ptr<PackageDatabase> * self_ptr;
            Data_Get_Struct(self, std::shared_ptr<PackageDatabase>, self_ptr);

            return ((*self_ptr)->has_repository_named(RepositoryName(StringValuePtr(name)))) ? true : false;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     add_repository(importance, repository) -> nil
     *
     *  Add a repository.
     */
    VALUE
    package_database_add_repository(VALUE self, VALUE importance, VALUE repo_v)
    {
        try
        {
            std::shared_ptr<PackageDatabase> * self_ptr;
            Data_Get_Struct(self, std::shared_ptr<PackageDatabase>, self_ptr);

            std::shared_ptr<Repository> repo(value_to_repository(repo_v));

            (*self_ptr)->add_repository(NUM2INT(importance), repo);
            return Qnil;
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
        rb_define_method(c_package_database, "fetch_unique_qualified_package_name",
                RUBY_FUNC_CAST(&package_database_fetch_unique_qualified_package_name), -1);
        rb_define_method(c_package_database, "repositories",
                RUBY_FUNC_CAST(&package_database_repositories), 0);
        rb_define_method(c_package_database, "fetch_repository",
                RUBY_FUNC_CAST(&package_database_fetch_repository), 1);
        rb_define_method(c_package_database, "more_important_than",
                RUBY_FUNC_CAST(&package_database_more_important_than), 2);
        rb_define_method(c_package_database, "has_repository_named?",
                RUBY_FUNC_CAST(&package_database_has_repository_named), 1);
        rb_define_method(c_package_database, "add_repository",
                RUBY_FUNC_CAST(&package_database_add_repository), 2);
    }
}

VALUE
paludis::ruby::package_database_to_value(std::shared_ptr<PackageDatabase> m)
{
    std::shared_ptr<PackageDatabase> * m_ptr(0);
    try
    {
        m_ptr = new std::shared_ptr<PackageDatabase>(m);
        return Data_Wrap_Struct(c_package_database, 0, &Common<std::shared_ptr<PackageDatabase> >::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

RegisterRubyClass::Register paludis_ruby_register_package_database PALUDIS_ATTRIBUTE((used))
    (&do_register_package_database);

