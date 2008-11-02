/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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
    static VALUE c_filtered_generator;

    VALUE
    filtered_generator_init(int, VALUE *, VALUE self, VALUE, VALUE)
    {
        return self;
    }

    VALUE
    filtered_generator_new(VALUE self, VALUE generator, VALUE filter)
    {
        FilteredGenerator * ptr(0);
        try
        {
            Generator g(value_to_generator(generator));
            Filter f(value_to_filter(filter));
            ptr = new FilteredGenerator(g, f);
            VALUE data(Data_Wrap_Struct(self, 0, &Common<FilteredGenerator>::free, ptr));
            rb_obj_call_init(data, 2, &self);
            return data;
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
     * Add a new Filter.
     */
    VALUE
    filtered_generator_bar(VALUE self, VALUE other)
    {
        FilteredGenerator * ptr(0);
        try
        {
            FilteredGenerator g1(value_to_filtered_generator(self));
            Filter f1(value_to_filter(other));
            return filtered_generator_to_value(g1 | f1);
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     filter -> Filter
     *
     * Our Filter.
     */
    VALUE
    filtered_generator_filter(VALUE self)
    {
        FilteredGenerator * ptr(0);
        try
        {
            FilteredGenerator g1(value_to_filtered_generator(self));
            return filter_to_value(g1.filter());
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     generator -> Generator
     *
     * Our Generator.
     */
    VALUE
    filtered_generator_generator(VALUE self)
    {
        FilteredGenerator * ptr(0);
        try
        {
            FilteredGenerator g1(value_to_filtered_generator(self));
            return generator_to_value(g1.generator());
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    void do_register_filtered_generator()
    {
        /*
         * Document-class: Paludis::FilteredGenerator
         *
         * A combined Generator and Filter for an Environment selection.
         */
        c_filtered_generator = rb_define_class_under(paludis_module(), "FilteredGenerator", rb_cObject);
        rb_define_singleton_method(c_filtered_generator, "new", RUBY_FUNC_CAST(&filtered_generator_new), 2);
        rb_define_method(c_filtered_generator, "initialize", RUBY_FUNC_CAST(&filtered_generator_init), -1);
        rb_define_method(c_filtered_generator, "to_s", RUBY_FUNC_CAST(&Common<FilteredGenerator>::to_s), 0);
        rb_define_method(c_filtered_generator, "|", RUBY_FUNC_CAST(&filtered_generator_bar), 1);
        rb_define_method(c_filtered_generator, "filter", RUBY_FUNC_CAST(&filtered_generator_filter), 0);
        rb_define_method(c_filtered_generator, "generator", RUBY_FUNC_CAST(&filtered_generator_generator), 0);
    }
}

FilteredGenerator
paludis::ruby::value_to_filtered_generator(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_filtered_generator))
    {
        FilteredGenerator * f_ptr;
        Data_Get_Struct(v, FilteredGenerator, f_ptr);
        return *f_ptr;
    }
    else
    {
        Generator g(value_to_generator(v));
        return FilteredGenerator(g);
    }
}

VALUE
paludis::ruby::filtered_generator_to_value(const FilteredGenerator & v)
{
    FilteredGenerator * vv(new FilteredGenerator(v));
    return Data_Wrap_Struct(c_filtered_generator, 0, &Common<FilteredGenerator>::free, vv);
}

RegisterRubyClass::Register paludis_ruby_register_filtered_generator PALUDIS_ATTRIBUTE((used))
    (&do_register_filtered_generator);

