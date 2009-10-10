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

    static VALUE c_dependencies_label;
    static VALUE c_dependencies_build_label;
    static VALUE c_dependencies_test_label;
    static VALUE c_dependencies_compile_against_label;
    static VALUE c_dependencies_fetch_label;
    static VALUE c_dependencies_install_label;
    static VALUE c_dependencies_post_label;
    static VALUE c_dependencies_recommendation_label;
    static VALUE c_dependencies_run_label;
    static VALUE c_dependencies_suggestion_label;

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

    struct DependenciesLabelToValue
    {
        VALUE value;
        std::tr1::shared_ptr<const DependenciesLabel> mm;

        DependenciesLabelToValue(const std::tr1::shared_ptr<const DependenciesLabel> & _m) :
            mm(_m)
        {
        }

        void visit(const DependenciesBuildLabel &)
        {
            value = Data_Wrap_Struct(c_dependencies_build_label, 0, &Common<std::tr1::shared_ptr<const DependenciesLabel> >::free,
                    new std::tr1::shared_ptr<const DependenciesLabel>(mm));
        }

        void visit(const DependenciesTestLabel &)
        {
            value = Data_Wrap_Struct(c_dependencies_test_label, 0, &Common<std::tr1::shared_ptr<const DependenciesLabel> >::free,
                    new std::tr1::shared_ptr<const DependenciesLabel>(mm));
        }

        void visit(const DependenciesRunLabel &)
        {
            value = Data_Wrap_Struct(c_dependencies_run_label, 0, &Common<std::tr1::shared_ptr<const DependenciesLabel> >::free,
                    new std::tr1::shared_ptr<const DependenciesLabel>(mm));
        }

        void visit(const DependenciesPostLabel &)
        {
            value = Data_Wrap_Struct(c_dependencies_post_label, 0, &Common<std::tr1::shared_ptr<const DependenciesLabel> >::free,
                    new std::tr1::shared_ptr<const DependenciesLabel>(mm));
        }

        void visit(const DependenciesInstallLabel &)
        {
            value = Data_Wrap_Struct(c_dependencies_install_label, 0, &Common<std::tr1::shared_ptr<const DependenciesLabel> >::free,
                    new std::tr1::shared_ptr<const DependenciesLabel>(mm));
        }

        void visit(const DependenciesFetchLabel &)
        {
            value = Data_Wrap_Struct(c_dependencies_fetch_label, 0, &Common<std::tr1::shared_ptr<const DependenciesLabel> >::free,
                    new std::tr1::shared_ptr<const DependenciesLabel>(mm));
        }

        void visit(const DependenciesSuggestionLabel &)
        {
            value = Data_Wrap_Struct(c_dependencies_suggestion_label, 0, &Common<std::tr1::shared_ptr<const DependenciesLabel> >::free,
                    new std::tr1::shared_ptr<const DependenciesLabel>(mm));
        }

        void visit(const DependenciesRecommendationLabel &)
        {
            value = Data_Wrap_Struct(c_dependencies_recommendation_label, 0, &Common<std::tr1::shared_ptr<const DependenciesLabel> >::free,
                    new std::tr1::shared_ptr<const DependenciesLabel>(mm));
        }

        void visit(const DependenciesCompileAgainstLabel &)
        {
            value = Data_Wrap_Struct(c_dependencies_compile_against_label, 0, &Common<std::tr1::shared_ptr<const DependenciesLabel> >::free,
                    new std::tr1::shared_ptr<const DependenciesLabel>(mm));
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
    dependencies_label_text(VALUE self)
    {
        std::tr1::shared_ptr<const DependenciesLabel> * ptr;
        Data_Get_Struct(self, std::tr1::shared_ptr<const DependenciesLabel>, ptr);
        return rb_str_new2((*ptr)->text().c_str());
    }

    VALUE
    empty_init(int, VALUE *, VALUE self)
    {
        return self;
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

        /*
         * Document-class: Paludis::DependenciesLabel
         *
         * Dependencies label base class.
         */
        c_dependencies_label = rb_define_class_under(paludis_module(), "DependenciesLabel", rb_cObject);
        rb_funcall(c_dependencies_label, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_dependencies_label, "text", RUBY_FUNC_CAST(&dependencies_label_text), 0);
        rb_define_method(c_dependencies_label, "to_s", RUBY_FUNC_CAST(&dependencies_label_text), 0);

        c_dependencies_build_label = rb_define_class_under(paludis_module(), "DependenciesBuildLabel", c_dependencies_label);
        c_dependencies_test_label = rb_define_class_under(paludis_module(), "DependenciesTestLabel", c_dependencies_label);
        c_dependencies_run_label = rb_define_class_under(paludis_module(), "DependenciesRunLabel", c_dependencies_label);
        c_dependencies_post_label = rb_define_class_under(paludis_module(), "DependenciesPostLabel", c_dependencies_label);
        c_dependencies_install_label = rb_define_class_under(paludis_module(), "DependenciesInstallLabel", c_dependencies_label);
        c_dependencies_compile_against_label = rb_define_class_under(paludis_module(), "DependenciesCompileAgainstLabel", c_dependencies_label);
        c_dependencies_fetch_label = rb_define_class_under(paludis_module(), "DependenciesFetchLabel", c_dependencies_label);
        c_dependencies_suggestion_label = rb_define_class_under(paludis_module(), "DependenciesSuggestionLabel", c_dependencies_label);
        c_dependencies_recommendation_label = rb_define_class_under(paludis_module(), "DependenciesRecommendationLabel", c_dependencies_label);

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

VALUE
paludis::ruby::dependencies_label_to_value(const std::tr1::shared_ptr<const DependenciesLabel> & m)
{
    try
    {
        DependenciesLabelToValue v(m);
        m->accept(v);
        return v.value;
    }
    catch (const std::exception & e)
    {
        exception_to_ruby_exception(e);
    }
}

std::tr1::shared_ptr<const DependenciesLabel>
paludis::ruby::value_to_dependencies_label(VALUE v)
{
    try
    {
        if (rb_obj_is_kind_of(v, c_dependencies_label))
        {
            std::tr1::shared_ptr<const DependenciesLabel> * ptr;
            Data_Get_Struct(v, std::tr1::shared_ptr<const DependenciesLabel>, ptr);
            return *ptr;
        }
        else
        {
            rb_raise(rb_eTypeError, "Can't convert %s into DependenciesLabel", rb_obj_classname(v));
        }
    }
    catch (const std::exception & e)
    {
        exception_to_ruby_exception(e);
    }
}

RegisterRubyClass::Register paludis_ruby_register_dep_label PALUDIS_ATTRIBUTE((used))
    (&do_register_dep_label);

