/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2011 Ciaran McCreesh
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

using namespace paludis;
using namespace paludis::ruby;

namespace
{
    static VALUE c_generator_module;
    static VALUE c_generator;
    static VALUE c_generator_all;
    static VALUE c_generator_matches;
    static VALUE c_generator_package;
    static VALUE c_generator_in_repository;
    static VALUE c_generator_from_repository;
    static VALUE c_generator_category;
    static VALUE c_generator_intersection;
    static VALUE c_generator_some_ids_might_support_action;

    VALUE
    generator_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    VALUE
    generator_all_new(VALUE self)
    {
        Generator * ptr(0);
        try
        {
            ptr = new generator::All();
            VALUE data(Data_Wrap_Struct(self, 0, &Common<Generator>::free, ptr));
            rb_obj_call_init(data, 0, &self);
            return data;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    /*
     * Document-method: &
     *
     * call-seq:
     *     &(other_generator) -> Generator
     *
     * Combine with another Generator using a set intersection.
     */
    VALUE
    generator_ampersand(VALUE self, VALUE other)
    {
        Generator * ptr(0);
        try
        {
            Generator g1(value_to_generator(self));
            Generator g2(value_to_generator(other));
            return generator_to_value(g1 & g2);
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    /*
     * Document-method: |
     *
     * call-seq:
     *     |(filter) -> FilteredGenerator
     *
     * Combine with a Filter to produce a FilteredGenerator.
     */
    VALUE
    generator_bar(VALUE self, VALUE other)
    {
        Generator * ptr(0);
        try
        {
            Generator g1(value_to_generator(self));
            Filter f1(value_to_filter(other));
            return filtered_generator_to_value(g1 | f1);
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    generator_matches_new(VALUE self, VALUE spec_v, VALUE id_v, VALUE options_v)
    {
        Generator * ptr(0);
        try
        {
            std::shared_ptr<const PackageDepSpec> spec(value_to_package_dep_spec(spec_v));
            std::shared_ptr<const PackageID> id(value_to_package_id(id_v, true));
            MatchPackageOptions options(value_to_match_package_options(options_v));
            ptr = new generator::Matches(*spec, id, options);
            VALUE data(Data_Wrap_Struct(self, 0, &Common<Generator>::free, ptr));
            rb_obj_call_init(data, 2, &spec_v);
            return data;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    generator_category_new(VALUE self, VALUE name_v)
    {
        Generator * ptr(0);
        try
        {
            CategoryNamePart name(StringValuePtr(name_v));
            ptr = new generator::Category(name);
            VALUE data(Data_Wrap_Struct(self, 0, &Common<Generator>::free, ptr));
            rb_obj_call_init(data, 1, &name_v);
            return data;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    generator_package_new(VALUE self, VALUE name_v)
    {
        Generator * ptr(0);
        try
        {
            QualifiedPackageName name(value_to_qualified_package_name(name_v));
            ptr = new generator::Package(name);
            VALUE data(Data_Wrap_Struct(self, 0, &Common<Generator>::free, ptr));
            rb_obj_call_init(data, 1, &name_v);
            return data;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    generator_in_repository_new(VALUE self, VALUE name_v)
    {
        Generator * ptr(0);
        try
        {
            RepositoryName name(StringValuePtr(name_v));
            ptr = new generator::InRepository(name);
            VALUE data(Data_Wrap_Struct(self, 0, &Common<Generator>::free, ptr));
            rb_obj_call_init(data, 1, &name_v);
            return data;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    generator_from_repository_new(VALUE self, VALUE name_v)
    {
        Generator * ptr(0);
        try
        {
            RepositoryName name(StringValuePtr(name_v));
            ptr = new generator::FromRepository(name);
            VALUE data(Data_Wrap_Struct(self, 0, &Common<Generator>::free, ptr));
            rb_obj_call_init(data, 1, &name_v);
            return data;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    generator_intersection_new(VALUE self, VALUE f1_v, VALUE f2_v)
    {
        Generator * ptr(0);
        try
        {
            Generator f1(value_to_generator(f1_v));
            Generator f2(value_to_generator(f2_v));
            ptr = new generator::Intersection(f1, f2);
            VALUE data(Data_Wrap_Struct(self, 0, &Common<Generator>::free, ptr));
            rb_obj_call_init(data, 2, &f1_v);
            return data;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     new(class) -> Filter
     *
     * Create a Generator that accepts packages that might support the
     * specified Action subclass.
     */
    VALUE
    generator_some_ids_might_support_action_new(VALUE self, VALUE action_class)
    {
        Generator * ptr(0);
        try
        {
            if (Qtrue == rb_funcall2(action_class, rb_intern("<="), 1, install_action_value_ptr()))
                ptr = new generator::SomeIDsMightSupportAction<InstallAction>();
            else if (Qtrue == rb_funcall2(action_class, rb_intern("<="), 1, pretend_action_value_ptr()))
                ptr = new generator::SomeIDsMightSupportAction<PretendAction>();
            else if (Qtrue == rb_funcall2(action_class, rb_intern("<="), 1, config_action_value_ptr()))
                ptr = new generator::SomeIDsMightSupportAction<ConfigAction>();
            else if (Qtrue == rb_funcall2(action_class, rb_intern("<="), 1, fetch_action_value_ptr()))
                ptr = new generator::SomeIDsMightSupportAction<FetchAction>();
            else if (Qtrue == rb_funcall2(action_class, rb_intern("<="), 1, info_action_value_ptr()))
                ptr = new generator::SomeIDsMightSupportAction<InfoAction>();
            else if (Qtrue == rb_funcall2(action_class, rb_intern("<="), 1, pretend_fetch_action_value_ptr()))
                ptr = new generator::SomeIDsMightSupportAction<PretendFetchAction>();
            else
                rb_raise(rb_eTypeError, "Can't convert %s into an Action subclass", rb_obj_classname(action_class));

            VALUE data(Data_Wrap_Struct(self, 0, &Common<Generator>::free, ptr));
            rb_obj_call_init(data, 1, &action_class);
            return data;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    void do_register_generator()
    {
        /*
         * Document-module: Paludis::Generator
         *
         * Collection of classes that produce results for an
         * Environment selection.
         */
        c_generator_module = rb_define_module_under(paludis_module(), "Generator");

        /*
         * Document-class: Paludis::Generator::Generator
         *
         * Generator for an Environment selection.
         */
        c_generator = rb_define_class_under(c_generator_module, "Generator", rb_cObject);
        rb_funcall(c_generator, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_generator, "initialize", RUBY_FUNC_CAST(&generator_init), -1);
        rb_define_method(c_generator, "to_s", RUBY_FUNC_CAST(&Common<Generator>::to_s), 0);
        rb_define_method(c_generator, "&", RUBY_FUNC_CAST(&generator_ampersand), 1);
        rb_define_method(c_generator, "|", RUBY_FUNC_CAST(&generator_bar), 1);

        /*
         * Document-class: Paludis::Generator::All
         *
         * Generate all packages.
         */
        c_generator_all = rb_define_class_under(c_generator_module, "All", c_generator);
        rb_define_singleton_method(c_generator_all, "new", RUBY_FUNC_CAST(&generator_all_new), 0);

        /*
         * Document-class: Paludis::Generator::Matches
         *
         * Generate matching packages.
         */
        c_generator_matches = rb_define_class_under(c_generator_module, "Matches", c_generator);
        rb_define_singleton_method(c_generator_matches, "new", RUBY_FUNC_CAST(&generator_matches_new), 3);

        /*
         * Document-class: Paludis::Generator::Intersection
         *
         * Generate packages from the intersection of two other Generator instances.
         */
        c_generator_intersection = rb_define_class_under(c_generator_module, "Intersection", c_generator);
        rb_define_singleton_method(c_generator_intersection, "new", RUBY_FUNC_CAST(&generator_intersection_new), 2);

        /*
         * Document-class: Paludis::Generator::Package
         *
         * Generate all named packages.
         */
        c_generator_package = rb_define_class_under(c_generator_module, "Package", c_generator);
        rb_define_singleton_method(c_generator_package, "new", RUBY_FUNC_CAST(&generator_package_new), 1);

        /*
         * Document-class: Paludis::Generator::Category
         *
         * Generate all packages in a given category.
         */
        c_generator_category = rb_define_class_under(c_generator_module, "Category", c_generator);
        rb_define_singleton_method(c_generator_category, "new", RUBY_FUNC_CAST(&generator_category_new), 1);

        /*
         * Document-class: Paludis::Generator::InRepository
         *
         * Generate all packages in a given repository.
         */
        c_generator_in_repository = rb_define_class_under(c_generator_module, "InRepository", c_generator);
        rb_define_singleton_method(c_generator_in_repository, "new", RUBY_FUNC_CAST(&generator_in_repository_new), 1);

        /*
         * Document-class: Paludis::Generator::FromRepository
         *
         * Generate all packages originally from a given repository.
         */
        c_generator_from_repository = rb_define_class_under(c_generator_module, "FromRepository", c_generator);
        rb_define_singleton_method(c_generator_from_repository, "new", RUBY_FUNC_CAST(&generator_from_repository_new), 1);

        /*
         * Document-class: Paludis::Generator::SomeIDsMightSupportAction
         *
         * Generate all packages that might support a particular action.
         */
        c_generator_some_ids_might_support_action = rb_define_class_under(c_generator_module, "SomeIDsMightSupportAction", c_generator);
        rb_define_singleton_method(c_generator_some_ids_might_support_action, "new", RUBY_FUNC_CAST(&generator_some_ids_might_support_action_new), 1);
    }
}

Generator
paludis::ruby::value_to_generator(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_generator))
    {
        Generator * f_ptr;
        Data_Get_Struct(v, Generator, f_ptr);
        return *f_ptr;
    }
    else
        rb_raise(rb_eTypeError, "Can't convert %s into Generator", rb_obj_classname(v));
}

VALUE
paludis::ruby::generator_to_value(const Generator & v)
{
    Generator * vv(new Generator(v));
    return Data_Wrap_Struct(c_generator, 0, &Common<Generator>::free, vv);
}

RegisterRubyClass::Register paludis_ruby_register_generator PALUDIS_ATTRIBUTE((used))
    (&do_register_generator);

