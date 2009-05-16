/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
 * Copyright (c) 2006, 2007, 2008 Richard Brown
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
#include <paludis/dep_list.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/options.hh>
#include <paludis/util/save.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <list>
#include <ruby.h>

#include "nice_names-nn.hh"

using namespace paludis;
using namespace paludis::ruby;

namespace
{
    static VALUE c_uri_label;
    static VALUE c_uri_mirrors_then_listed_label;
    static VALUE c_uri_mirrors_only_label;
    static VALUE c_uri_listed_only_label;
    static VALUE c_uri_listed_then_mirrors_label;
    static VALUE c_uri_local_mirrors_only_label;
    static VALUE c_uri_manual_only_label;

    struct URILabelToValue
    {
        VALUE value;
        std::tr1::shared_ptr<const URILabel> mm;

        URILabelToValue(const std::tr1::shared_ptr<const URILabel> & _m) :
            mm(_m)
        {
        }

        void visit(const URIMirrorsThenListedLabel &)
        {
            value = Data_Wrap_Struct(c_uri_mirrors_then_listed_label, 0, &Common<std::tr1::shared_ptr<const URILabel> >::free,
                    new std::tr1::shared_ptr<const URILabel>(mm));
        }

        void visit(const URIMirrorsOnlyLabel &)
        {
            value = Data_Wrap_Struct(c_uri_mirrors_only_label, 0, &Common<std::tr1::shared_ptr<const URILabel> >::free,
                    new std::tr1::shared_ptr<const URILabel>(mm));
        }

        void visit(const URIListedOnlyLabel &)
        {
            value = Data_Wrap_Struct(c_uri_listed_only_label, 0, &Common<std::tr1::shared_ptr<const URILabel> >::free,
                    new std::tr1::shared_ptr<const URILabel>(mm));
        }

        void visit(const URIListedThenMirrorsLabel &)
        {
            value = Data_Wrap_Struct(c_uri_listed_then_mirrors_label, 0, &Common<std::tr1::shared_ptr<const URILabel> >::free,
                    new std::tr1::shared_ptr<const URILabel>(mm));
        }

        void visit(const URILocalMirrorsOnlyLabel &)
        {
            value = Data_Wrap_Struct(c_uri_local_mirrors_only_label, 0, &Common<std::tr1::shared_ptr<const URILabel> >::free,
                    new std::tr1::shared_ptr<const URILabel>(mm));
        }

        void visit(const URIManualOnlyLabel &)
        {
            value = Data_Wrap_Struct(c_uri_manual_only_label, 0, &Common<std::tr1::shared_ptr<const URILabel> >::free,
                    new std::tr1::shared_ptr<const URILabel>(mm));
        }
    };

    /*
     * Document-method: text
     *
     * call-seq:
     *     text -> String
     *
     * Our text.
     */
    /*
     * Document-method: to_s
     *
     * call-seq:
     *     to_s -> String
     *
     * Fetch a string representation of ourself.
     */
    VALUE
    uri_label_text(VALUE self)
    {
        std::tr1::shared_ptr<const URILabel> * ptr;
        Data_Get_Struct(self, std::tr1::shared_ptr<const URILabel>, ptr);
        return rb_str_new2((*ptr)->text().c_str());
    }

    void do_register_dep_label()
    {
        /*
         * Document-class: Paludis::URILabel
         *
         * URI label base class.
         */
        c_uri_label = rb_define_class_under(paludis_module(), "URILabel", rb_cObject);
        rb_funcall(c_uri_label, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_uri_label, "text", RUBY_FUNC_CAST(&uri_label_text), 0);
        rb_define_method(c_uri_label, "to_s", RUBY_FUNC_CAST(&uri_label_text), 0);

        c_uri_mirrors_then_listed_label = rb_define_class_under(paludis_module(), "URIMirrorsThenListedLabel", c_uri_label);
        c_uri_mirrors_only_label = rb_define_class_under(paludis_module(), "URIMirrorsOnlyLabel", c_uri_label);
        c_uri_listed_only_label = rb_define_class_under(paludis_module(), "URIListedOnlyLabel", c_uri_label);
        c_uri_listed_then_mirrors_label = rb_define_class_under(paludis_module(), "URIListedThenMirrorsLabel", c_uri_label);
        c_uri_local_mirrors_only_label = rb_define_class_under(paludis_module(), "URILocalMirrorsOnlyLabel", c_uri_label);
        c_uri_manual_only_label = rb_define_class_under(paludis_module(), "URIManualOnlyLabel", c_uri_label);
    }
}

VALUE
paludis::ruby::uri_label_to_value(const std::tr1::shared_ptr<const URILabel> & m)
{
    try
    {
        URILabelToValue v(m);
        m->accept(v);
        return v.value;
    }
    catch (const std::exception & e)
    {
        exception_to_ruby_exception(e);
    }
}

RegisterRubyClass::Register paludis_ruby_register_dep_label PALUDIS_ATTRIBUTE((used))
    (&do_register_dep_label);

