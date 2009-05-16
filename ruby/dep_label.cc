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

    static VALUE c_dependency_label;
    static VALUE c_dependency_system_label;
    static VALUE c_dependency_type_label;
    static VALUE c_dependency_suggest_label;
    static VALUE c_dependency_abis_label;
    static VALUE c_dependency_host_label;
    static VALUE c_dependency_target_label;
    static VALUE c_dependency_build_label;
    static VALUE c_dependency_run_label;
    static VALUE c_dependency_post_label;
    static VALUE c_dependency_compile_label;
    static VALUE c_dependency_install_label;
    static VALUE c_dependency_suggested_label;
    static VALUE c_dependency_recommended_label;
    static VALUE c_dependency_required_label;
    static VALUE c_dependency_any_label;
    static VALUE c_dependency_mine_label;
    static VALUE c_dependency_primary_label;
    static VALUE c_dependency_abi_label;

    static VALUE c_active_dependency_labels;

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

    struct RealDependencyLabelToValue
    {
        VALUE value;
        std::tr1::shared_ptr<const DependencyLabel> mm;

        RealDependencyLabelToValue(const std::tr1::shared_ptr<const DependencyLabel> & _m) :
            mm(_m)
        {
            value = Qnil;
        }

        void visit(const DependencyHostLabel &)
        {
            value = Data_Wrap_Struct(c_dependency_host_label, 0,
                    &Common<std::tr1::shared_ptr<const DependencyLabel> >::free,
                    new std::tr1::shared_ptr<const DependencyLabel>(mm));
        }

        void visit(const DependencyTargetLabel &)
        {
            value = Data_Wrap_Struct(c_dependency_target_label, 0,
                    &Common<std::tr1::shared_ptr<const DependencyLabel> >::free,
                    new std::tr1::shared_ptr<const DependencyLabel>(mm));
        }

        void visit(const DependencyBuildLabel &)
        {
            value = Data_Wrap_Struct(c_dependency_build_label, 0,
                    &Common<std::tr1::shared_ptr<const DependencyLabel> >::free,
                    new std::tr1::shared_ptr<const DependencyLabel>(mm));
        }

        void visit(const DependencyRunLabel &)
        {
            value = Data_Wrap_Struct(c_dependency_run_label, 0,
                    &Common<std::tr1::shared_ptr<const DependencyLabel> >::free,
                    new std::tr1::shared_ptr<const DependencyLabel>(mm));
        }

        void visit(const DependencyPostLabel &)
        {
            value = Data_Wrap_Struct(c_dependency_post_label, 0,
                    &Common<std::tr1::shared_ptr<const DependencyLabel> >::free,
                    new std::tr1::shared_ptr<const DependencyLabel>(mm));
        }

        void visit(const DependencyCompileLabel &)
        {
            value = Data_Wrap_Struct(c_dependency_compile_label, 0,
                    &Common<std::tr1::shared_ptr<const DependencyLabel> >::free,
                    new std::tr1::shared_ptr<const DependencyLabel>(mm));
        }

        void visit(const DependencyInstallLabel &)
        {
            value = Data_Wrap_Struct(c_dependency_install_label, 0,
                    &Common<std::tr1::shared_ptr<const DependencyLabel> >::free,
                    new std::tr1::shared_ptr<const DependencyLabel>(mm));
        }

        void visit(const DependencySuggestedLabel &)
        {
            value = Data_Wrap_Struct(c_dependency_suggested_label, 0,
                    &Common<std::tr1::shared_ptr<const DependencyLabel> >::free,
                    new std::tr1::shared_ptr<const DependencyLabel>(mm));
        }

        void visit(const DependencyRecommendedLabel &)
        {
            value = Data_Wrap_Struct(c_dependency_recommended_label, 0,
                    &Common<std::tr1::shared_ptr<const DependencyLabel> >::free,
                    new std::tr1::shared_ptr<const DependencyLabel>(mm));
        }

        void visit(const DependencyRequiredLabel &)
        {
            value = Data_Wrap_Struct(c_dependency_required_label, 0,
                    &Common<std::tr1::shared_ptr<const DependencyLabel> >::free,
                    new std::tr1::shared_ptr<const DependencyLabel>(mm));
        }

        void visit(const DependencyAnyLabel &)
        {
            value = Data_Wrap_Struct(c_dependency_any_label, 0,
                    &Common<std::tr1::shared_ptr<const DependencyLabel> >::free,
                    new std::tr1::shared_ptr<const DependencyLabel>(mm));
        }

        void visit(const DependencyMineLabel &)
        {
            value = Data_Wrap_Struct(c_dependency_mine_label, 0,
                    &Common<std::tr1::shared_ptr<const DependencyLabel> >::free,
                    new std::tr1::shared_ptr<const DependencyLabel>(mm));
        }

        void visit(const DependencyPrimaryLabel &)
        {
            value = Data_Wrap_Struct(c_dependency_primary_label, 0,
                    &Common<std::tr1::shared_ptr<const DependencyLabel> >::free,
                    new std::tr1::shared_ptr<const DependencyLabel>(mm));
        }

        void visit(const DependencyABILabel &)
        {
            value = Data_Wrap_Struct(c_dependency_abi_label, 0,
                    &Common<std::tr1::shared_ptr<const DependencyLabel> >::free,
                    new std::tr1::shared_ptr<const DependencyLabel>(mm));
        }
    };

    struct DependencyLabelToValue
    {
        VALUE value;
        std::tr1::shared_ptr<const DependencyLabel> mm;

        DependencyLabelToValue(const std::tr1::shared_ptr<const DependencyLabel> & _m) :
            mm(_m)
        {
        }

        void visit(const DependencySystemLabel & label)
        {
            RealDependencyLabelToValue v(mm);
            label.accept(v);
            value = v.value;
        }

        void visit(const DependencyTypeLabel & label)
        {
            RealDependencyLabelToValue v(mm);
            label.accept(v);
            value = v.value;
        }

        void visit(const DependencySuggestLabel & label)
        {
            RealDependencyLabelToValue v(mm);
            label.accept(v);
            value = v.value;
        }

        void visit(const DependencyABIsLabel & label)
        {
            RealDependencyLabelToValue v(mm);
            label.accept(v);
            value = v.value;
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
    dependency_label_text(VALUE self)
    {
        std::tr1::shared_ptr<const DependencyLabel> * ptr;
        Data_Get_Struct(self, std::tr1::shared_ptr<const DependencyLabel>, ptr);
        return rb_str_new2((*ptr)->text().c_str());
    }

    VALUE
    empty_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    VALUE
    active_dependency_labels_new(int argc, VALUE *argv, VALUE self)
    {
        std::tr1::shared_ptr<ActiveDependencyLabels> * ptr(0);
        try
        {
            if (1 == argc && rb_obj_is_kind_of(argv[0], c_active_dependency_labels))
            {
                ptr = new std::tr1::shared_ptr<ActiveDependencyLabels>(
                        new ActiveDependencyLabels(*value_to_active_dependency_labels(argv[0])));
            }
            else if (1 == argc && rb_obj_is_kind_of(argv[0], *dependency_labels_dep_spec_value_ptr()))
            {
                ptr = new std::tr1::shared_ptr<ActiveDependencyLabels>(
                        new ActiveDependencyLabels(*value_to_dependency_labels_dep_spec(argv[0])));
            }
            else if (1 == argc && rb_obj_is_kind_of(argv[0], rb_cArray))
            {
                std::tr1::shared_ptr<DependencyLabelSequence> seq(new DependencyLabelSequence);

                for (int i(0) ; i < RARRAY_LEN(argv[0]) ; ++i)
                {
                    VALUE entry(rb_ary_entry(argv[0], i));
                    seq->push_back(value_to_dependency_label(entry));
                }

                ptr = new std::tr1::shared_ptr<ActiveDependencyLabels>(new ActiveDependencyLabels(*seq));
            }
            else if (2 == argc && rb_obj_is_kind_of(argv[0], c_active_dependency_labels) &&
                    rb_obj_is_kind_of(argv[1], *dependency_labels_dep_spec_value_ptr()))
            {
                ptr = new std::tr1::shared_ptr<ActiveDependencyLabels>(
                        new ActiveDependencyLabels(*value_to_active_dependency_labels(argv[0]),
                            *value_to_dependency_labels_dep_spec(argv[1])));
            }
            else
            {
                rb_raise(rb_eArgError, "ActiveDependencyLabels expects one or two arguments, but got %d",argc);
            }

            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<std::tr1::shared_ptr<ActiveDependencyLabels> >::free, ptr));
            rb_obj_call_init(tdata, argc, argv);
            return tdata;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    template <typename T_, const std::tr1::shared_ptr<const T_> (ActiveDependencyLabels::* f_) () const>
    struct ActiveDependencyLabelsLabels
    {
        static VALUE
        get(VALUE self)
        {
            try
            {
                std::tr1::shared_ptr<ActiveDependencyLabels> * ptr;
                Data_Get_Struct(self, std::tr1::shared_ptr<ActiveDependencyLabels>, ptr);
                std::tr1::shared_ptr<const T_> v(((**ptr).*(f_))());

                VALUE result(rb_ary_new());
                for (typename T_::ConstIterator i(v->begin()), i_end(v->end()) ;
                        i != i_end ; ++i)
                    rb_ary_push(result, dependency_label_to_value(*i));
                return result;
            }
            catch (const std::exception & e)
            {
                exception_to_ruby_exception(e);
            }
        }
    };

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

        c_dependency_label = rb_define_class_under(paludis_module(), "DependencyLabel", rb_cObject);
        rb_funcall(c_dependency_label, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_dependency_label, "text", RUBY_FUNC_CAST(&dependency_label_text), 0);
        rb_define_method(c_dependency_label, "to_s", RUBY_FUNC_CAST(&dependency_label_text), 0);

        c_dependency_system_label = rb_define_class_under(paludis_module(), "DependencySystemLabel", c_dependency_label);
        rb_funcall(c_dependency_system_label, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        c_dependency_type_label = rb_define_class_under(paludis_module(), "DependencyTypeLabel", c_dependency_label);
        rb_funcall(c_dependency_type_label, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        c_dependency_suggest_label = rb_define_class_under(paludis_module(), "DependencySuggestLabel", c_dependency_label);
        rb_funcall(c_dependency_suggest_label, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        c_dependency_abis_label = rb_define_class_under(paludis_module(), "DependencyABIsLabel", c_dependency_label);
        rb_funcall(c_dependency_abis_label, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        c_dependency_host_label = rb_define_class_under(paludis_module(), "DependencyHostLabel", c_dependency_system_label);
        rb_funcall(c_dependency_host_label, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        c_dependency_target_label = rb_define_class_under(paludis_module(), "DependencyTargetLabel", c_dependency_system_label);
        rb_funcall(c_dependency_target_label, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        c_dependency_build_label = rb_define_class_under(paludis_module(), "DependencyBuildLabel", c_dependency_type_label);
        rb_funcall(c_dependency_build_label, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        c_dependency_run_label = rb_define_class_under(paludis_module(), "DependencyRunLabel", c_dependency_type_label);
        rb_funcall(c_dependency_run_label, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        c_dependency_post_label = rb_define_class_under(paludis_module(), "DependencyPostLabel", c_dependency_type_label);
        rb_funcall(c_dependency_post_label, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        c_dependency_compile_label = rb_define_class_under(paludis_module(), "DependencyCompileLabel", c_dependency_type_label);
        rb_funcall(c_dependency_compile_label, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        c_dependency_install_label = rb_define_class_under(paludis_module(), "DependencyInstallLabel", c_dependency_type_label);
        rb_funcall(c_dependency_install_label, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        c_dependency_suggested_label = rb_define_class_under(paludis_module(), "DependencySuggestedLabel", c_dependency_suggest_label);
        rb_funcall(c_dependency_suggested_label, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        c_dependency_recommended_label = rb_define_class_under(paludis_module(), "DependencyRecommendedLabel", c_dependency_suggest_label);
        rb_funcall(c_dependency_recommended_label, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        c_dependency_required_label = rb_define_class_under(paludis_module(), "DependencyRequiredLabel", c_dependency_suggest_label);
        rb_funcall(c_dependency_required_label, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        c_dependency_any_label = rb_define_class_under(paludis_module(), "DependencyAnyLabel", c_dependency_abis_label);
        rb_funcall(c_dependency_any_label, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        c_dependency_mine_label = rb_define_class_under(paludis_module(), "DependencyMineLabel", c_dependency_abis_label);
        rb_funcall(c_dependency_mine_label, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        c_dependency_primary_label = rb_define_class_under(paludis_module(), "DependencyPrimaryLabel", c_dependency_abis_label);
        rb_funcall(c_dependency_primary_label, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        c_dependency_abi_label = rb_define_class_under(paludis_module(), "DependencyAbiLabel", c_dependency_abis_label);
        rb_funcall(c_dependency_abi_label, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        c_active_dependency_labels = rb_define_class_under(paludis_module(), "ActiveDependencyLabels", rb_cObject);
        rb_define_singleton_method(c_active_dependency_labels, "new", RUBY_FUNC_CAST(&active_dependency_labels_new), -1);
        rb_define_method(c_active_dependency_labels, "initialize", RUBY_FUNC_CAST(&empty_init), -1);
        rb_define_method(c_active_dependency_labels, "system_labels", RUBY_FUNC_CAST((
                        ActiveDependencyLabelsLabels<DependencySystemLabelSequence,
                        &ActiveDependencyLabels::system_labels>::get)), 0);
        rb_define_method(c_active_dependency_labels, "type_labels", RUBY_FUNC_CAST((
                        ActiveDependencyLabelsLabels<DependencyTypeLabelSequence,
                        &ActiveDependencyLabels::type_labels>::get)), 0);
        rb_define_method(c_active_dependency_labels, "abi_labels", RUBY_FUNC_CAST((
                        ActiveDependencyLabelsLabels<DependencyABIsLabelSequence,
                        &ActiveDependencyLabels::abi_labels>::get)), 0);
        rb_define_method(c_active_dependency_labels, "suggest_labels", RUBY_FUNC_CAST((
                        ActiveDependencyLabelsLabels<DependencySuggestLabelSequence,
                        &ActiveDependencyLabels::suggest_labels>::get)), 0);
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
paludis::ruby::dependency_label_to_value(const std::tr1::shared_ptr<const DependencyLabel> & m)
{
    try
    {
        DependencyLabelToValue v(m);
        m->accept(v);
        return v.value;
    }
    catch (const std::exception & e)
    {
        exception_to_ruby_exception(e);
    }
}

std::tr1::shared_ptr<ActiveDependencyLabels>
paludis::ruby::value_to_active_dependency_labels(VALUE v)
{
    try
    {
        if (rb_obj_is_kind_of(v, c_active_dependency_labels))
        {
            std::tr1::shared_ptr<ActiveDependencyLabels> * ptr;
            Data_Get_Struct(v, std::tr1::shared_ptr<ActiveDependencyLabels>, ptr);
            return *ptr;
        }
        else
        {
            rb_raise(rb_eTypeError, "Can't convert %s into ActiveDependencyLabels", rb_obj_classname(v));
        }
    }
    catch (const std::exception & e)
    {
        exception_to_ruby_exception(e);
    }
}

std::tr1::shared_ptr<const DependencyLabel>
paludis::ruby::value_to_dependency_label(VALUE v)
{
    try
    {
        if (rb_obj_is_kind_of(v, c_dependency_label))
        {
            std::tr1::shared_ptr<const DependencyLabel> * ptr;
            Data_Get_Struct(v, std::tr1::shared_ptr<const DependencyLabel>, ptr);
            return *ptr;
        }
        else
        {
            rb_raise(rb_eTypeError, "Can't convert %s into DependencyLabel", rb_obj_classname(v));
        }
    }
    catch (const std::exception & e)
    {
        exception_to_ruby_exception(e);
    }
}

RegisterRubyClass::Register paludis_ruby_register_dep_label PALUDIS_ATTRIBUTE((used))
    (&do_register_dep_label);

