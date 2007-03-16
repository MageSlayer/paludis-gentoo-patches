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
#include <paludis/portage_dep_parser.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_portage_dep_parser;
    static VALUE c_portage_dep_parser_policy;

    VALUE
    portage_dep_parser_policy_to_value(const PortageDepParser::Policy & p)
    {
        PortageDepParser::Policy * p2 (new PortageDepParser::Policy(p));
        return Data_Wrap_Struct(c_portage_dep_parser_policy, 0, &Common<PortageDepParser::Policy>::free, p2);
    }

    PortageDepParser::Policy
    value_to_portage_dep_parser_policy(VALUE v)
    {
        if (rb_obj_is_kind_of(v, c_portage_dep_parser_policy))
        {
            PortageDepParser::Policy * v_ptr;
            Data_Get_Struct(v, PortageDepParser::Policy, v_ptr);
            return *v_ptr;
        }
        else
        {
            rb_raise(rb_eTypeError, "Can't convert %s into PortageDepParserPolicy", rb_obj_classname(v));
        }
    }

    /*
     * call-seq:
     *     PortageDepParser::parse(dep_string, spec_type, permit_any_deps, package_dep_parse_mode = PmPermissive) -> CompositeDepSpec
     *
     * Parse a given dependency string, and return an appropriate DepSpec tree.
     */
    VALUE
    portage_dep_parser_parse(VALUE, VALUE dep_string, VALUE policy)
    {
        try
        {
        return dep_spec_to_value(PortageDepParser::parse(StringValuePtr(dep_string),
                    value_to_portage_dep_parser_policy(policy)));
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     PortageDepParser::parse_depend(dep_string, package_dep_parse_mode) -> CompositeDepSpec
     *
     * Convenience wrapper for parsing depend strings.
     */
    VALUE
    portage_dep_parser_parse_depend(VALUE, VALUE string, VALUE parse_mode)
    {
        try
        {
            return dep_spec_to_value(PortageDepParser::parse_depend(StringValuePtr(string), static_cast<PackageDepSpecParseMode>(NUM2INT(parse_mode))));
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     PortageDepParser::parse_license(dep_string) -> CompositeDepSpec
     *
     * Convenience wrapper for parsing license strings.
     */
    VALUE
    portage_dep_parser_parse_license(VALUE, VALUE string)
    {
        try
        {
            return dep_spec_to_value(PortageDepParser::parse_license(StringValuePtr(string)));
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     Policy::text_is_text_dep_spec(permit_any_deps) -> Policy
     *
     * Returns a new policy for a PlainTextDepSpec
     */
    VALUE
    portage_dep_parser_policy_text_is_text_dep_spec(VALUE, VALUE permit_any_deps)
    {
        try
        {
            bool b(! (permit_any_deps == Qnil || permit_any_deps == Qfalse));
            return portage_dep_parser_policy_to_value(PortageDepParser::Policy::text_is_text_dep_spec(b));
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     Policy::text_is_text_dep_spec(permit_any_deps, parse_mode) -> Policy
     *
     * Returns a new policy for a PackageDepSpec
     */
    VALUE
    portage_dep_parser_policy_text_is_package_dep_spec(VALUE, VALUE permit_any_deps, VALUE parse_mode)
    {
        try
        {
            bool b(! (permit_any_deps == Qnil || permit_any_deps == Qfalse));
            return portage_dep_parser_policy_to_value(PortageDepParser::Policy::text_is_package_dep_spec(b,
                    static_cast<PackageDepSpecParseMode>(NUM2INT(parse_mode))));
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    void do_register_portage_dep_parser()
    {
        rb_require("singleton");

        /*
         * Document-module: Paludis::PortageDepParser
         *
         * The PortageDepParser module contains methods for converting string representations of a
         * dependency specification into a DepSpec instance.
         */
        c_portage_dep_parser = rb_define_module_under(paludis_module(), "PortageDepParser");
        rb_define_singleton_method(c_portage_dep_parser, "parse",
                RUBY_FUNC_CAST(&portage_dep_parser_parse), 2);
        rb_define_singleton_method(c_portage_dep_parser, "parse_depend",
                RUBY_FUNC_CAST(&portage_dep_parser_parse_depend), 2);
        rb_define_singleton_method(c_portage_dep_parser, "parse_license",
                RUBY_FUNC_CAST(&portage_dep_parser_parse_license), 1);

        /*
         * Document-class: Paludis::PortageDepParser::Policy
         *
         * The Policy class describes how to convert a string representation of a 
         * dependency specification into a DepSpec instance.
         */
        c_portage_dep_parser_policy = rb_define_class_under(c_portage_dep_parser, "Policy", rb_cObject);
        rb_funcall(c_portage_dep_parser_policy, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_singleton_method(c_portage_dep_parser_policy, "text_is_text_dep_spec",
                RUBY_FUNC_CAST(&portage_dep_parser_policy_text_is_text_dep_spec), 1);
        rb_define_singleton_method(c_portage_dep_parser_policy, "text_is_package_dep_spec",
                RUBY_FUNC_CAST(&portage_dep_parser_policy_text_is_package_dep_spec), 2);
    }
}

RegisterRubyClass::Register paludis_ruby_register_portage_dep_parser PALUDIS_ATTRIBUTE((used))
    (&do_register_portage_dep_parser);

