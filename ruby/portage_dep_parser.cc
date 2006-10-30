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
#include <paludis/portage_dep_parser.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_portage_dep_parser;

    VALUE
    portage_dep_parser_parse(int argc, VALUE * args, VALUE)
    {
        try
        {
            if (1 == argc)
                return dep_atom_to_value(PortageDepParser::parse(STR2CSTR(args[0])));
            else if (3 == argc)
            {
                bool b(! (args[2] == Qnil || args[2] == Qfalse));

                switch (NUM2INT(args[1]))
                {
                    case 17:
                        if (b)
                            return dep_atom_to_value(PortageDepParser::parse(STR2CSTR(args[0]),
                                        PortageDepParserPolicy<PackageDepAtom, true>::get_instance()));
                        else
                            return dep_atom_to_value(PortageDepParser::parse(STR2CSTR(args[0]),
                                        PortageDepParserPolicy<PackageDepAtom, false>::get_instance()));
                    case 23:
                        if (b)
                            return dep_atom_to_value(PortageDepParser::parse(STR2CSTR(args[0]),
                                        PortageDepParserPolicy<PlainTextDepAtom, true>::get_instance()));
                        else
                            return dep_atom_to_value(PortageDepParser::parse(STR2CSTR(args[0]),
                                        PortageDepParserPolicy<PlainTextDepAtom, false>::get_instance()));
                }

                rb_raise(rb_eArgError, "Bad value '%d' for PortageDepParser::parse parameter 2", NUM2INT(args[1]));
            }
            else
                rb_raise(rb_eArgError, "PortageDepParser::parse expects one or three arguments, but got %d", argc);
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    void do_register_portage_dep_parser()
    {
        rb_require("singleton");

        c_portage_dep_parser = rb_define_class_under(paludis_module(), "PortageDepParser", rb_cObject);
        rb_funcall(c_portage_dep_parser, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_singleton_method(c_portage_dep_parser, "parse",
                RUBY_FUNC_CAST(&portage_dep_parser_parse), -1);

        rb_define_const(c_portage_dep_parser, "PackageDepAtom", INT2FIX(17));
        rb_define_const(c_portage_dep_parser, "PlainTextDepAtom", INT2FIX(23));
    }
}

RegisterRubyClass::Register paludis_ruby_register_portage_dep_parser PALUDIS_ATTRIBUTE((used))
    (&do_register_portage_dep_parser);

