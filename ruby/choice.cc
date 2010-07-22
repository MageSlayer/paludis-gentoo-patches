/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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
#include <paludis/choice.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

namespace
{
    static VALUE c_choice;
    static VALUE c_choices;
    static VALUE c_choice_value;

    /*
     * call-seq:
     *     each {|choice| block } -> Nil
     *
     * Our Choice children.
     */
    VALUE
    choices_each(VALUE self_v)
    {
        std::shared_ptr<const Choices> self(value_to_choices(self_v));
        for (Choices::ConstIterator k(self->begin()), k_end(self->end()) ;
                k != k_end ; ++k)
        {
            VALUE val(choice_to_value(*k));
            if (Qnil != val)
                rb_yield(val);
        }
        return Qnil;
    }

    /*
     * call-seq:
     *     has_matching_contains_every_value_prefix?(prefix) -> true or false
     *
     * Do we have a Choice subkey with contains_every_value true and a prefix matching
     * this name?
     *
     * 0-based EAPIs don't require things like userland_GNU in IUSE. So if you're looking
     * for a flag and don't find it, check this method before issuing a QA notice.
     */
    VALUE
    choices_has_matching_contains_every_value_prefix(VALUE self_v, VALUE arg_v)
    {
        try
        {
            std::shared_ptr<const Choices> self(value_to_choices(self_v));
            std::string arg(StringValuePtr(arg_v));
            return bool_to_value(self->has_matching_contains_every_value_prefix(ChoiceNameWithPrefix(arg)));
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     find_by_name_with_prefix(prefix) -> ChoiceValue or Nil
     *
     * Find a ChoiceValue that has a particular prefix and name.
     *
     * Returns nil for no match.
     *
     * This is a convenient way of getting a particular use flag's details. Calling this
     * method with, say, "nls" or "ruby" will get the value for that flag without having
     * to hunt around in all the subkeys manually. Prefixes work too, e.g. "linguas_en" for
     * 0-based EAPIs or "linguas:en" for exheres EAPIs.
     */
    VALUE
    choices_find_by_name_with_prefix(VALUE self_v, VALUE arg_v)
    {
        try
        {
            std::shared_ptr<const Choices> self(value_to_choices(self_v));
            std::string arg(StringValuePtr(arg_v));
            std::shared_ptr<const ChoiceValue> result(self->find_by_name_with_prefix(ChoiceNameWithPrefix(arg)));
            if (result)
                return choice_value_to_value(result);
            else
                return Qnil;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     each {|choice_value| block } -> Nil
     *
     * Our ChoiceValue children.
     */
    VALUE
    choice_each(VALUE self_v)
    {
        std::shared_ptr<const Choice> self(value_to_choice(self_v));
        for (Choice::ConstIterator k(self->begin()), k_end(self->end()) ;
                k != k_end ; ++k)
        {
            VALUE val(choice_value_to_value(*k));
            if (Qnil != val)
                rb_yield(val);
        }
        return Qnil;
    }

    /*
     * call-seq:
     *     raw_name -> String
     *
     * Our raw name, for example 'USE' or 'LINGUAS'.
     */
    FAKE_RDOC_METHOD(choice_raw_name)

    /*
     * call-seq:
     *     human_name -> String
     *
     * A human-readable name (often the same as raw_name).
     */
    FAKE_RDOC_METHOD(choice_human_name)

    /*
     * call-seq:
     *     prefix -> String
     *
     * The prefix for our ChoiceValue children.
     *
     * An empty string for USE and ARCH, 'linguas' for LINGUAS etc.
     */
    FAKE_RDOC_METHOD(choice_prefix)

    template <typename R_, const R_ (Choice::* r_) () const>
    struct ChoiceStringishMembers
    {
        static VALUE fetch(VALUE self_v)
        {
            try
            {
                std::shared_ptr<const Choice> self(value_to_choice(self_v));
                return rb_str_new2(stringify(((*self).*r_)()).c_str());
            }
            catch (const std::exception & e)
            {
                exception_to_ruby_exception(e);
            }
        }
    };

    /*
     * call-seq:
     *     contains_every_value? -> true or false
     *
     * If true, pretend that we contain every possible value and that any value not listed
     * as a child exists and is not enabled.
     *
     * For pesky 0-based EAPIs that don't require things like userland_GNU in IUSE, and that
     * don't have a comprehensive list of possible values.
     */
    FAKE_RDOC_METHOD(choice_contains_every_value)

    /*
     * call-seq:
     *     hidden? -> true or false
     *
     * If true, this option should not usually be shown visually to a user.
     */
    FAKE_RDOC_METHOD(choice_hidden)

    /*
     * call-seq:
     *     show_with_no_prefix? -> true or false
     *
     * If true, hint that we're better not displaying our prefix to the user.
     *
     * This is used by --pretend --install and --query to avoid showing a Use:
     * prefix before a list of use flag names.
     */
    FAKE_RDOC_METHOD(choice_show_with_no_prefix)

    /*
     * call-seq:
     *     consider_added_or_changed? -> true or false
     *
     * If false, do not consider flags in this section for 'added' or 'changed'
     * detection.
     *
     * Used by build_options.
     */
    FAKE_RDOC_METHOD(choice_consider_added_or_changed)

    template <typename R_, R_ (Choice::* r_) () const>
    struct ChoiceBoolishMembers
    {
        static VALUE fetch(VALUE self_v)
        {
            try
            {
                std::shared_ptr<const Choice> self(value_to_choice(self_v));
                return bool_to_value(((*self).*r_)());
            }
            catch (const std::exception & e)
            {
                exception_to_ruby_exception(e);
            }
        }
    };

    /*
     * call-seq:
     *     unprefixed_name -> String
     *
     * Our name, without an prefix (for example, 'nls' or 'en').
     */
    FAKE_RDOC_METHOD(choice_value_unprefixed_name)

    /*
     * call-seq:
     *     name_with_prefix -> String
     *
     * Our name, with prefix if there is one (for example, 'nls' or 'linguas_en').
     */
    FAKE_RDOC_METHOD(choice_value_name_with_prefix)

    /*
     * call-seq:
     *     description -> String
     *
     * The flag's description, or an empty string.
     */
    FAKE_RDOC_METHOD(choice_value_description)

    template <typename R_, const R_ (ChoiceValue::* r_) () const>
    struct ChoiceValueStringishMembers
    {
        static VALUE fetch(VALUE self_v)
        {
            try
            {
                std::shared_ptr<const ChoiceValue> self(value_to_choice_value(self_v));
                return rb_str_new2(stringify(((*self).*r_)()).c_str());
            }
            catch (const std::exception & e)
            {
                exception_to_ruby_exception(e);
            }
        }
    };

    /*
     * call-seq:
     *     enabled? -> true or false
     *
     * Is this flag enabled?
     */
    FAKE_RDOC_METHOD(choice_value_enabled)

    /*
     * call-seq:
     *     enabled_by_default? -> true or false
     *
     * Would this flag be enabled by default (i.e. before considering
     * any overrides from the Environment)?
     */
    FAKE_RDOC_METHOD(choice_value_enabled_by_default)

    /*
     * call-seq:
     *     locked? -> true or false
     *
     * Is this flag locked (forced or masked)?
     */
    FAKE_RDOC_METHOD(choice_value_locked)

    /*
     * call-seq:
     *     explicitly_listed? -> true or false
     *
     * Is this flag explicitly listed?
     *
     * Use this to avoid showing things like LINGUAS values that aren't listed
     * in IUSE but that end up as a ChoiceValue anyway.
     */
    FAKE_RDOC_METHOD(choice_value_explicitly_listed)

    template <typename R_, R_ (ChoiceValue::* r_) () const>
    struct ChoiceValueBoolishMembers
    {
        static VALUE fetch(VALUE self_v)
        {
            try
            {
                std::shared_ptr<const ChoiceValue> self(value_to_choice_value(self_v));
                return bool_to_value(((*self).*r_)());
            }
            catch (const std::exception & e)
            {
                exception_to_ruby_exception(e);
            }
        }
    };

    void do_register_choice()
    {
        /*
         * Document-class: Paludis::Choices
         *
         * A collection of Choice objects for a PackageID.
         */
        c_choices = rb_define_class_under(paludis_module(), "Choices", rb_cObject);
        rb_funcall(c_choices, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_choices, "each", RUBY_FUNC_CAST(&choices_each), 0);
        rb_include_module(c_choices, rb_mEnumerable);
        rb_define_method(c_choices, "find_by_name_with_prefix", RUBY_FUNC_CAST(&choices_find_by_name_with_prefix), 1);
        rb_define_method(c_choices, "has_matching_contains_every_value_prefix?", RUBY_FUNC_CAST(&choices_has_matching_contains_every_value_prefix), 1);

        /*
         * Document-class: Paludis::Choice
         *
         * A collection of ChoiceValue objects for a PackageID's Choices.
         */
        c_choice = rb_define_class_under(paludis_module(), "Choice", rb_cObject);
        rb_funcall(c_choice, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_choice, "raw_name", RDOC_IS_STUPID(choice_raw_name,
                    (&ChoiceStringishMembers<std::string, &Choice::raw_name>::fetch)), 0);
        rb_define_method(c_choice, "human_name", RDOC_IS_STUPID(choice_human_name,
                    (&ChoiceStringishMembers<std::string, &Choice::human_name>::fetch)), 0);
        rb_define_method(c_choice, "prefix", RDOC_IS_STUPID(choice_prefix,
                    (&ChoiceStringishMembers<ChoicePrefixName, &Choice::prefix>::fetch)), 0);
        rb_define_method(c_choice, "contains_every_value?", RDOC_IS_STUPID(choice_contains_every_value,
                    (&ChoiceBoolishMembers<bool, &Choice::contains_every_value>::fetch)), 0);
        rb_define_method(c_choice, "hidden?", RDOC_IS_STUPID(choice_hidden,
                    (&ChoiceBoolishMembers<bool, &Choice::hidden>::fetch)), 0);
        rb_define_method(c_choice, "show_with_no_prefix?", RDOC_IS_STUPID(choice_show_with_no_prefix,
                    (&ChoiceBoolishMembers<bool, &Choice::show_with_no_prefix>::fetch)), 0);
        rb_define_method(c_choice, "consider_added_or_changed?", RDOC_IS_STUPID(choice_consider_added_or_changed,
                    (&ChoiceBoolishMembers<bool, &Choice::consider_added_or_changed>::fetch)), 0);
        rb_define_method(c_choice, "each", RUBY_FUNC_CAST(&choice_each), 0);
        rb_include_module(c_choice, rb_mEnumerable);

        /*
         * Document-class: Paludis::ChoiceValue
         *
         * A single ChoiceValue object for a Choice.
         */
        c_choice_value = rb_define_class_under(paludis_module(), "ChoiceValue", rb_cObject);
        rb_funcall(c_choice_value, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_choice_value, "unprefixed_name", RDOC_IS_STUPID(choice_value_unprefixed_name,
                    (&ChoiceValueStringishMembers<UnprefixedChoiceName, &ChoiceValue::unprefixed_name>::fetch)), 0);
        rb_define_method(c_choice_value, "name_with_prefix", RDOC_IS_STUPID(choice_value_name_with_prefix,
                    (&ChoiceValueStringishMembers<ChoiceNameWithPrefix, &ChoiceValue::name_with_prefix>::fetch)), 0);
        rb_define_method(c_choice_value, "enabled?", RDOC_IS_STUPID(choice_value_enabled,
                    (&ChoiceValueBoolishMembers<bool, &ChoiceValue::enabled>::fetch)), 0);
        rb_define_method(c_choice_value, "enabled_by_default?", RDOC_IS_STUPID(choice_value_enabled_by_default,
                    (&ChoiceValueBoolishMembers<bool, &ChoiceValue::enabled_by_default>::fetch)), 0);
        rb_define_method(c_choice_value, "locked?", RDOC_IS_STUPID(choice_value_locked,
                    (&ChoiceValueBoolishMembers<bool, &ChoiceValue::locked>::fetch)), 0);
        rb_define_method(c_choice_value, "description", RDOC_IS_STUPID(choice_value_description,
                    (&ChoiceValueStringishMembers<std::string, &ChoiceValue::description>::fetch)), 0);
        rb_define_method(c_choice_value, "explicitly_listed?", RDOC_IS_STUPID(choice_value_explicitly_listed,
                    (&ChoiceValueBoolishMembers<bool, &ChoiceValue::explicitly_listed>::fetch)), 0);
    }
}

VALUE
paludis::ruby::choices_to_value(const std::shared_ptr<const Choices> & m)
{
    std::shared_ptr<const Choices> * m_ptr(0);
    try
    {
        m_ptr = new std::shared_ptr<const Choices>(m);
        return Data_Wrap_Struct(c_choices, 0, &Common<std::shared_ptr<const Choices> >::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

std::shared_ptr<const Choices>
paludis::ruby::value_to_choices(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_choices))
    {
        std::shared_ptr<const Choices> * v_ptr;
        Data_Get_Struct(v, std::shared_ptr<const Choices>, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into Choices", rb_obj_classname(v));
    }
}

VALUE
paludis::ruby::choice_to_value(const std::shared_ptr<const Choice> & m)
{
    std::shared_ptr<const Choice> * m_ptr(0);
    try
    {
        m_ptr = new std::shared_ptr<const Choice>(m);
        return Data_Wrap_Struct(c_choice, 0, &Common<std::shared_ptr<const Choice> >::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

std::shared_ptr<const Choice>
paludis::ruby::value_to_choice(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_choice))
    {
        std::shared_ptr<const Choice> * v_ptr;
        Data_Get_Struct(v, std::shared_ptr<const Choice>, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into Choice", rb_obj_classname(v));
    }
}

VALUE
paludis::ruby::choice_value_to_value(const std::shared_ptr<const ChoiceValue> & m)
{
    std::shared_ptr<const ChoiceValue> * m_ptr(0);
    try
    {
        m_ptr = new std::shared_ptr<const ChoiceValue>(m);
        return Data_Wrap_Struct(c_choice_value, 0, &Common<std::shared_ptr<const ChoiceValue> >::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

std::shared_ptr<const ChoiceValue>
paludis::ruby::value_to_choice_value(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_choice_value))
    {
        std::shared_ptr<const ChoiceValue> * v_ptr;
        Data_Get_Struct(v, std::shared_ptr<const ChoiceValue>, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into ChoiceValue", rb_obj_classname(v));
    }
}

RegisterRubyClass::Register paludis_ruby_register_choice PALUDIS_ATTRIBUTE((used))
    (&do_register_choice);

