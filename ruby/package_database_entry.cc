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
#include <paludis/package_database_entry.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_package_database_entry;

    VALUE
    package_database_entry_init(VALUE self, VALUE, VALUE, VALUE)
    {
        return self;
    }

    /*
     * call-seq:
     *     PackageDatabaseEntry.new (qualified_package_name, version_spec, repository_name) -> PackageDatabaseEntry
     *
     *  Create a new PackageDatabaseEntry.
     */
    VALUE
    package_database_entry_new(VALUE self, VALUE q, VALUE v, VALUE n)
    {
        VALUE argv[3] = { q, v, n };
        PackageDatabaseEntry * ptr(0);
        try
        {
            VersionSpec vv(value_to_version_spec(v));

            ptr = new PackageDatabaseEntry(QualifiedPackageName(StringValuePtr(q)), vv,
                    RepositoryName(StringValuePtr(n)));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<PackageDatabaseEntry>::free, ptr));
            rb_obj_call_init(tdata, 3, argv);
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
     *     version -> VersionSpec
     *
     * Fetch VersionSpec.
     */
    VALUE
    package_database_entry_version(VALUE self)
    {
        try
        {
            PackageDatabaseEntry * p;
            Data_Get_Struct(self, PackageDatabaseEntry, p);
            return version_spec_to_value(p->version);
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     version=
     *
     * Set package version.
     */
    VALUE
    package_database_entry_version_set(VALUE self, VALUE version)
    {
        try
        {
            PackageDatabaseEntry * p;
            Data_Get_Struct(self, PackageDatabaseEntry, p);
            p->version = value_to_version_spec(version);
            return self;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     name -> QualifiedPackageName
     *
     * Fetch package name
     */
    VALUE
    package_database_entry_name(VALUE self)
    {
        try
        {
            PackageDatabaseEntry * p;
            Data_Get_Struct(self, PackageDatabaseEntry, p);
            return qualified_package_name_to_value(p->name);
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     name=
     *
     * Set package name.
     */
    VALUE
    package_database_entry_name_set(VALUE self, VALUE qpn)
    {
        try
        {
            PackageDatabaseEntry * p;
            Data_Get_Struct(self, PackageDatabaseEntry, p);
            p->name = value_to_qualified_package_name(qpn);
            return self;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * Document-method: repository
     *
     * call-seq:
     *     repository -> String
     *
     * Fetch the repository name.
     */
    /*
     * Document-method: repository=
     *
     * call-seq:
     *     repository=
     *
     * Set package name.
     */
    template <typename T_, T_ PackageDatabaseEntry::* m_>
    struct EntryValue
    {
        static VALUE
        fetch(VALUE self)
        {
            PackageDatabaseEntry * p;
            Data_Get_Struct(self, PackageDatabaseEntry, p);
            return rb_str_new2(stringify(p->*m_).c_str());
        }

        static VALUE
        set(VALUE self, VALUE str)
        {
            try
            {
                PackageDatabaseEntry * p;
                Data_Get_Struct(self, PackageDatabaseEntry, p);
                p->*m_ =  T_ ((StringValuePtr(str)));
                return self;
            }
            catch (const std::exception & e)
            {
                exception_to_ruby_exception(e);
            }
        }
    };

    void do_register_package_database_entry()
    {
        /*
         * Document-class: Paludis::PackageDatabaseEntry
         *
         * Holds an entry in a PackageDatabase, and used to identify a specific version of a package in a particular repository.
         */
        c_package_database_entry = rb_define_class_under(paludis_module(), "PackageDatabaseEntry", rb_cObject);
        rb_define_singleton_method(c_package_database_entry, "new", RUBY_FUNC_CAST(&package_database_entry_new), 3);
        rb_define_method(c_package_database_entry, "initialize", RUBY_FUNC_CAST(&package_database_entry_init), 3);
        rb_define_method(c_package_database_entry, "==", RUBY_FUNC_CAST(&Common<PackageDatabaseEntry>::equal), 1);
        rb_define_method(c_package_database_entry, "to_s", RUBY_FUNC_CAST(&Common<PackageDatabaseEntry>::to_s), 0);
        rb_define_method(c_package_database_entry, "name", RUBY_FUNC_CAST(&package_database_entry_name), 0);
        rb_define_method(c_package_database_entry, "name=", RUBY_FUNC_CAST(&package_database_entry_name_set), 1);
        rb_define_method(c_package_database_entry, "version", RUBY_FUNC_CAST(&package_database_entry_version), 0);
        rb_define_method(c_package_database_entry, "version=", RUBY_FUNC_CAST(&package_database_entry_version_set), 1);
        rb_define_method(c_package_database_entry, "repository", RUBY_FUNC_CAST((&EntryValue<RepositoryName, &PackageDatabaseEntry::repository>::fetch)), 0);
        rb_define_method(c_package_database_entry, "repository=", RUBY_FUNC_CAST((&EntryValue<RepositoryName, &PackageDatabaseEntry::repository>::set)), 1);
    }
}

PackageDatabaseEntry
paludis::ruby::value_to_package_database_entry(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_package_database_entry))
    {
        PackageDatabaseEntry * v_ptr;
        Data_Get_Struct(v, PackageDatabaseEntry, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into PackageDatabaseEntry", rb_obj_classname(v));
    }
}

VALUE
paludis::ruby::package_database_entry_to_value(const PackageDatabaseEntry & d)
{
    PackageDatabaseEntry * m_ptr(0);
    try
    {
        m_ptr = new PackageDatabaseEntry(d);
        return Data_Wrap_Struct(c_package_database_entry, 0, &Common<PackageDatabaseEntry>::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }

}

RegisterRubyClass::Register paludis_ruby_register_package_database_entry PALUDIS_ATTRIBUTE((used))
    (&do_register_package_database_entry);

