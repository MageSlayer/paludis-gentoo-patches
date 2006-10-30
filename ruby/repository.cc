/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/repository.hh>
#include <paludis/util/stringify.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_repository;
    static VALUE c_repository_info;
    static VALUE c_repository_info_section;

    VALUE
    repository_name(VALUE self)
    {
        try
        {
            Repository::ConstPointer * self_ptr;
            Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
            return rb_str_new2(stringify((*self_ptr)->name()).c_str());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    repository_has_category_named(VALUE self, VALUE cat)
    {
        try
        {
            Repository::ConstPointer * self_ptr;
            Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
            return (*self_ptr)->has_category_named(CategoryNamePart(STR2CSTR(cat))) ? Qtrue : Qfalse;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    repository_has_package_named(VALUE self, VALUE name)
    {
        try
        {
            Repository::ConstPointer * self_ptr;
            Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
            return (*self_ptr)->has_package_named(QualifiedPackageName(STR2CSTR(name))) ? Qtrue : Qfalse;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    repository_has_version(VALUE self, VALUE name, VALUE version)
    {
        try
        {
            Repository::ConstPointer * self_ptr;
            Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
            return (*self_ptr)->has_version(QualifiedPackageName(STR2CSTR(name)),
                    value_to_version_spec(version)) ? Qtrue : Qfalse;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    repository_category_names(VALUE self)
    {
        try
        {
            Repository::ConstPointer * self_ptr;
            Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
            VALUE result(rb_ary_new());
            CategoryNamePartCollection::ConstPointer c((*self_ptr)->category_names());
            for (CategoryNamePartCollection::Iterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                rb_ary_push(result, rb_str_new2(stringify(*i).c_str()));
            return result;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    repository_package_names(VALUE self, VALUE cat)
    {
        try
        {
            Repository::ConstPointer * self_ptr;
            Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
            CategoryNamePart category(STR2CSTR(cat));

            VALUE result(rb_ary_new());
            QualifiedPackageNameCollection::ConstPointer c((*self_ptr)->package_names(category));
            for (QualifiedPackageNameCollection::Iterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                rb_ary_push(result, rb_str_new2(stringify(*i).c_str()));
            return result;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    repository_version_specs(VALUE self, VALUE qpn)
    {
        try
        {
            Repository::ConstPointer * self_ptr;
            Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
            QualifiedPackageName q(STR2CSTR(qpn));

            VALUE result(rb_ary_new());
            VersionSpecCollection::ConstPointer c((*self_ptr)->version_specs(q));
            for (VersionSpecCollection::Iterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                rb_ary_push(result, version_spec_to_value(*i));
            return result;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    repository_version_metadata(VALUE self, VALUE name, VALUE version)
    {
        try
        {
            Repository::ConstPointer * self_ptr;
            Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
            return version_metadata_to_value((*self_ptr)->version_metadata(QualifiedPackageName(STR2CSTR(name)),
                        value_to_version_spec(version)));
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    template <typename T_, T_ * RepositoryCapabilities::* m_>
    struct Interface
    {
        static VALUE fetch(VALUE self)
        {
            Repository::ConstPointer * self_ptr;
            Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
            return ((**self_ptr).*m_) ? self : Qnil;
        }
    };

    VALUE
    repository_info(VALUE self, VALUE verbose)
    {
        Repository::ConstPointer * self_ptr;
        Data_Get_Struct(self, Repository::ConstPointer, self_ptr);

        RepositoryInfo::ConstPointer * p = new RepositoryInfo::ConstPointer((*self_ptr)->info(Qfalse == verbose));
        return Data_Wrap_Struct(c_repository_info, 0, &Common<RepositoryInfo::ConstPointer>::free, p);
    }

    VALUE
    repository_info_sections(VALUE self)
    {
        try
        {
            RepositoryInfo::ConstPointer * self_ptr;
            Data_Get_Struct(self, RepositoryInfo::ConstPointer, self_ptr);

            VALUE result(rb_ary_new());
            for (RepositoryInfo::SectionIterator i((*self_ptr)->begin_sections()),
                    i_end((*self_ptr)->end_sections()) ; i != i_end ; ++i)
            {
                RepositoryInfoSection::ConstPointer * s(new RepositoryInfoSection::ConstPointer(*i));
                rb_ary_push(result, Data_Wrap_Struct(c_repository_info_section, 0, &Common<RepositoryInfo::ConstPointer>::free, s));
            }
            return result;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    repository_info_section_kvs(VALUE self)
    {
        try
        {
            RepositoryInfoSection::ConstPointer * self_ptr;
            Data_Get_Struct(self, RepositoryInfoSection::ConstPointer, self_ptr);

            VALUE result(rb_hash_new());
            for (RepositoryInfoSection::KeyValueIterator i((*self_ptr)->begin_kvs()),
                    i_end((*self_ptr)->end_kvs()) ; i != i_end ; ++i)
                rb_hash_aset(result, rb_str_new2(i->first.c_str()), rb_str_new2(i->second.c_str()));
            return result;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    void do_register_repository()
    {
        c_repository = rb_define_class_under(paludis_module(), "Repository", rb_cObject);
        rb_funcall(c_repository, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_repository, "name", RUBY_FUNC_CAST(&repository_name), 0);

        rb_define_method(c_repository, "has_category_named?", RUBY_FUNC_CAST(&repository_has_category_named), 1);
        rb_define_method(c_repository, "has_package_named?", RUBY_FUNC_CAST(&repository_has_package_named), 1);
        rb_define_method(c_repository, "has_version?", RUBY_FUNC_CAST(&repository_has_version), 2);

        rb_define_method(c_repository, "category_names", RUBY_FUNC_CAST(&repository_category_names), 0);
        rb_define_method(c_repository, "package_names", RUBY_FUNC_CAST(&repository_package_names), 1);
        rb_define_method(c_repository, "version_specs", RUBY_FUNC_CAST(&repository_version_specs), 1);

        rb_define_method(c_repository, "version_metadata", RUBY_FUNC_CAST(&repository_version_metadata), 2);

        rb_define_method(c_repository, "installable_interface", RUBY_FUNC_CAST((&Interface<RepositoryInstallableInterface,
                        &Repository::installable_interface>::fetch)), 0);
        rb_define_method(c_repository, "installed_interface", RUBY_FUNC_CAST((&Interface<RepositoryInstalledInterface,
                        &Repository::installed_interface>::fetch)), 0);
        rb_define_method(c_repository, "mask_interface", RUBY_FUNC_CAST((&Interface<RepositoryMaskInterface,
                        &Repository::mask_interface>::fetch)), 0);
        rb_define_method(c_repository, "news_interface", RUBY_FUNC_CAST((&Interface<RepositoryNewsInterface,
                        &Repository::news_interface>::fetch)), 0);
        rb_define_method(c_repository, "sets_interface", RUBY_FUNC_CAST((&Interface<RepositorySetsInterface,
                        &Repository::sets_interface>::fetch)), 0);
        rb_define_method(c_repository, "syncable_interface", RUBY_FUNC_CAST((&Interface<RepositorySyncableInterface,
                        &Repository::syncable_interface>::fetch)), 0);
        rb_define_method(c_repository, "uninstallable_interface", RUBY_FUNC_CAST((&Interface<RepositoryUninstallableInterface,
                        &Repository::uninstallable_interface>::fetch)), 0);
        rb_define_method(c_repository, "use_interface", RUBY_FUNC_CAST((&Interface<RepositoryUseInterface,
                        &Repository::use_interface>::fetch)), 0);
        rb_define_method(c_repository, "world_interface", RUBY_FUNC_CAST((&Interface<RepositoryWorldInterface,
                        &Repository::world_interface>::fetch)), 0);
        rb_define_method(c_repository, "mirrors_interface", RUBY_FUNC_CAST((&Interface<RepositoryMirrorsInterface,
                        &Repository::mirrors_interface>::fetch)), 0);
        rb_define_method(c_repository, "environment_variable_interface", RUBY_FUNC_CAST((&Interface<RepositoryEnvironmentVariableInterface,
                        &Repository::environment_variable_interface>::fetch)), 0);
        rb_define_method(c_repository, "provides_interface", RUBY_FUNC_CAST((&Interface<RepositoryProvidesInterface,
                        &Repository::provides_interface>::fetch)), 0);
        rb_define_method(c_repository, "virtuals_interface", RUBY_FUNC_CAST((&Interface<RepositoryVirtualsInterface,
                        &Repository::virtuals_interface>::fetch)), 0);

        rb_define_method(c_repository, "info", RUBY_FUNC_CAST(&repository_info), 1);

        c_repository_info = rb_define_class_under(paludis_module(), "RepositoryInfo", rb_cObject);
        rb_funcall(c_repository_info, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_repository_info, "sections", RUBY_FUNC_CAST(&repository_info_sections), 0);

        c_repository_info_section = rb_define_class_under(paludis_module(), "RepositoryInfoSection", rb_cObject);
        rb_funcall(c_repository_info_section, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_repository_info_section, "kvs", RUBY_FUNC_CAST(&repository_info_section_kvs), 0);
    }
}

VALUE
paludis::ruby::repository_to_value(Repository::ConstPointer m)
{
    Repository::ConstPointer * m_ptr(0);
    try
    {
        m_ptr = new Repository::ConstPointer(m);
        return Data_Wrap_Struct(c_repository, 0, &Common<Repository::ConstPointer>::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

RegisterRubyClass::Register paludis_ruby_register_repository PALUDIS_ATTRIBUTE((used))
    (&do_register_repository);


