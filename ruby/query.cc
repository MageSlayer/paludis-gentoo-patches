/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Richard Brown <mynamewasgone@gmail.com>
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
#include <paludis/util/log.hh>
#include <paludis/query.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_query_module;
    static VALUE c_query;
    static VALUE c_matches;
    static VALUE c_package;
    static VALUE c_not_masked;
    static VALUE c_repository_has_installed_interface;
    static VALUE c_repository_has_installable_interface;
    static VALUE c_repository_has_uninstallable_interface;
    static VALUE c_installed_at_root;

    VALUE
    query_to_value(const Query & v)
    {
        Query * vv(new Query(v));
        return Data_Wrap_Struct(c_query, 0, &Common<Query>::free, vv);
    }

    VALUE
    query_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    /*
     * Document-method: &
     *
     * call-seq:
     *     &(other_query) -> Query
     *
     * Combine Queries
     */
    VALUE
    query_and(VALUE self, VALUE other)
    {
        Query * ptr;
        Data_Get_Struct(self, Query, ptr);
        try
        {
            return query_to_value((*ptr) & value_to_query(other));
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     Matches.new(pda)
     *
     * Create a new Matches Query object from the given PackageDepSpec.
     */
    VALUE
    matches_new(VALUE self, VALUE pda)
    {
        query::Matches * ptr(0);
        try
        {
            ptr = new query::Matches(*value_to_package_dep_spec(pda));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<query::Matches>::free, ptr));
            rb_obj_call_init(tdata, 1, &pda);
            return tdata;

        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     Package.new(qpn)
     *
     * Create a new Package Query object from the given QualifiedPackageName.
     */
    VALUE
    package_new(VALUE self, VALUE qpn)
    {
        query::Package * ptr(0);
        try
        {
            ptr = new query::Package(value_to_qualified_package_name(qpn));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<query::Package>::free, ptr));
            rb_obj_call_init(tdata, 1, &qpn);
            return tdata;

        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    template <typename A_>
    struct QueryNew
    {
        static VALUE
        query_new(VALUE self)
        {
            A_ * ptr = new A_();
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<A_>::free, ptr));
            rb_obj_call_init(tdata, 0, 0);
            return tdata;
        }
    };

    /*
     * call-seq:
     *     InstalledAtRoot.new(root)
     *
     *
     */
    VALUE
    installed_at_root_new(VALUE self, VALUE root)
    {
        query::InstalledAtRoot * ptr(0);
        try
        {
            ptr = new query::InstalledAtRoot(FSEntry(StringValuePtr(root)));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<query::InstalledAtRoot>::free, ptr));
            rb_obj_call_init(tdata, 1, &root);
            return tdata;

        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }


    void do_register_query()
    {
        /*
         * Document-module: Paludis::Query
         *
         * Collection of classes for use in querying package databases
         */
        c_query_module = rb_define_module_under(paludis_module(), "Query");

        /*
         * Document-class: Paludis::Query::Query
         *
         * Base query class.
         */
        c_query = rb_define_class_under(c_query_module, "Query", rb_cObject);
        rb_funcall(c_query, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_query, "initialize", RUBY_FUNC_CAST(&query_init), -1);
        rb_define_method(c_query, "&", RUBY_FUNC_CAST(&query_and), 1);

        /*
         * Document-class: Paludis::Query::Matches
         *
         * Query for packages that match a PackageDepSpec
         */
        c_matches = rb_define_class_under(c_query_module, "Matches", c_query);
        rb_define_singleton_method(c_matches, "new", RUBY_FUNC_CAST(&matches_new), 1);

        /*
         * Document-class: Paludis::Query::Package
         *
         * Query for packages that match a QualifiedPackageName
         */
        c_package = rb_define_class_under(c_query_module, "Package", c_query);
        rb_define_singleton_method(c_package, "new", RUBY_FUNC_CAST(&package_new), 1);

        c_not_masked = rb_define_class_under(c_query_module, "NotMasked", c_query);
        rb_define_singleton_method(c_not_masked, "new",
                RUBY_FUNC_CAST(&QueryNew<query::NotMasked>::query_new), 0);

        c_repository_has_installed_interface = rb_define_class_under(c_query_module,
                "RepositoryHasInstalledInterface", c_query);
        rb_define_singleton_method(c_repository_has_installed_interface, "new",
                RUBY_FUNC_CAST(&QueryNew<query::RepositoryHasInstalledInterface>::query_new), 0);

        c_repository_has_installable_interface = rb_define_class_under(c_query_module,
                "RepositoryHasInstallableInterface", c_query);
        rb_define_singleton_method(c_repository_has_installable_interface, "new",
                RUBY_FUNC_CAST(&QueryNew<query::RepositoryHasInstallableInterface>::query_new), 0);

        c_repository_has_uninstallable_interface = rb_define_class_under(c_query_module,
                "RepositoryHasUninstallableInterface", c_query);
        rb_define_singleton_method(c_repository_has_uninstallable_interface, "new",
                RUBY_FUNC_CAST(&QueryNew<query::RepositoryHasUninstallableInterface>::query_new), 0);

        c_installed_at_root = rb_define_class_under(c_query_module,
                "InstalledAtRoot", c_query);
        rb_define_singleton_method(c_installed_at_root, "new", RUBY_FUNC_CAST(&installed_at_root_new), 1);
    }
}

Query
paludis::ruby::value_to_query(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_query))
    {
        Query * v_ptr;
        Data_Get_Struct(v, Query, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into Query", rb_obj_classname(v));
    }
}

bool
paludis::ruby::is_kind_of_query(VALUE v)
{
    return rb_obj_is_kind_of(v, c_query);
}


RegisterRubyClass::Register paludis_ruby_register_query PALUDIS_ATTRIBUTE((used)) (&do_register_query);


