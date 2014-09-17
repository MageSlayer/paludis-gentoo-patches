/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Richard Brown
 * Copyright (c) 2007, 2008, 2009, 2010, 2011, 2014 Ciaran McCreesh
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
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/mask.hh>
#include <paludis/util/set.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

namespace
{
    static VALUE c_package_id;
    static VALUE c_package_id_canonical_form;

    /*
     * call-seq:
     *     canonical_form(form) -> String
     *
     * Return this PackageID in a PackageIDCanonicalForm.
     */
    VALUE
    package_id_canonical_form(VALUE self, VALUE cf)
    {
        std::shared_ptr<const PackageID> * self_ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageID>, self_ptr);
        try
        {
            return rb_str_new2(((*self_ptr)->canonical_form(static_cast<PackageIDCanonicalForm>(NUM2INT(cf)))).c_str());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }

        return Qnil;
    }

    /*
     * call-seq:
     *     name -> QualifiedPackageName
     *
     * Our name.
     */
    VALUE
    package_id_name(VALUE self)
    {
        std::shared_ptr<const PackageID> * self_ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageID>, self_ptr);
        try
        {
            return qualified_package_name_to_value((*self_ptr)->name());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }

        return Qnil;
    }

    /*
     * call-seq:
     *     uniquely_identifying_spec -> PackageDepSpec
     *
     * Our name.
     */
    VALUE
    package_id_uniquely_identifying_spec(VALUE self)
    {
        std::shared_ptr<const PackageID> * self_ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageID>, self_ptr);
        try
        {
            return package_dep_spec_to_value((*self_ptr)->uniquely_identifying_spec());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }

        return Qnil;
    }

    /*
     * call-seq:
     *     supports_action(action_test) -> true or false
     *
     * Returns whether we support an action.
     */
    VALUE
    package_id_supports_action(VALUE self, VALUE test)
    {
        std::shared_ptr<const PackageID> * self_ptr;
        std::shared_ptr<const SupportsActionTestBase> test_ptr(value_to_supports_action_test_base(test));
        Data_Get_Struct(self, std::shared_ptr<const PackageID>, self_ptr);
        try
        {
            return (*self_ptr)->supports_action(*test_ptr) ? Qtrue : Qfalse;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }

        return Qnil;
    }

    /*
     * call-seq:
     *     perform_action(action) -> Nil
     *
     * Perform an action.
     */
    VALUE
    package_id_perform_action(VALUE self, VALUE test)
    {
        std::shared_ptr<const PackageID> * self_ptr;
        std::shared_ptr<Action> a_ptr(value_to_action(test));
        Data_Get_Struct(self, std::shared_ptr<const PackageID>, self_ptr);
        try
        {
            (*self_ptr)->perform_action(*a_ptr);
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }

        return Qnil;
    }

    /*
     * call-seq:
     *     version -> VersionSpec
     *
     * Our VersionSpec.
     */
    VALUE
    package_id_version(VALUE self)
    {
        std::shared_ptr<const PackageID> * self_ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageID>, self_ptr);
        try
        {
            return version_spec_to_value((*self_ptr)->version());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }

        return Qnil;
    }

    /*
     * call-seq:
     *     repository_name -> String
     *
     * Our Repository Name.
     */
    VALUE
    package_id_repository_name(VALUE self)
    {
        std::shared_ptr<const PackageID> * self_ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageID>, self_ptr);
        try
        {
            return rb_str_new2(stringify((*self_ptr)->repository_name()).c_str());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }

        return Qnil;
    }

    /*
     * call-seq:
     *     [String] -> MetadataKey or Nil
     *
     * The named metadata key.
     */
    VALUE
    package_id_subscript(VALUE self, VALUE raw_name)
    {
        std::shared_ptr<const PackageID> * self_ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageID>, self_ptr);
        try
        {
            PackageID::MetadataConstIterator it((*self_ptr)->find_metadata(StringValuePtr(raw_name)));
            if ((*self_ptr)->end_metadata() == it)
                return Qnil;
            return metadata_key_to_value(*it);
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }

        return Qnil;
    }

    /*
     * call-seq:
     *     each_metadata {|key| block } -> Nil
     *
     * Our metadata.
     */
    VALUE
    package_id_each_metadata(VALUE self)
    {
        std::shared_ptr<const PackageID> * self_ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageID>, self_ptr);
        try
        {
            for (PackageID::MetadataConstIterator it((*self_ptr)->begin_metadata()),
                    it_end((*self_ptr)->end_metadata()); it_end != it; ++it)
            {
                VALUE val(metadata_key_to_value(*it));
                if (Qnil != val)
                    rb_yield(val);
            }
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }

        return Qnil;
    }

    /*
     * call-seq:
     *     masks -> Array
     *
     * Our masks.
     */
    VALUE
    package_id_masks(VALUE self)
    {
        std::shared_ptr<const PackageID> * self_ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageID>, self_ptr);
        VALUE result(rb_ary_new());
        try
        {
            for (PackageID::MasksConstIterator it((*self_ptr)->begin_masks()),
                    it_end((*self_ptr)->end_masks()); it_end != it; ++it)
            {
                rb_ary_push(result, mask_to_value(*it));
            }
            return result;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }

        return Qnil;
    }

    /*
     * call-seq:
     *     overridden_masks -> Array
     *
     * Our overridden masks.
     */
    VALUE
    package_id_overridden_masks(VALUE self)
    {
        std::shared_ptr<const PackageID> * self_ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageID>, self_ptr);
        VALUE result(rb_ary_new());
        try
        {
            for (PackageID::OverriddenMasksConstIterator it((*self_ptr)->begin_overridden_masks()),
                    it_end((*self_ptr)->end_overridden_masks()); it_end != it; ++it)
            {
                rb_ary_push(result, overridden_mask_to_value(*it));
            }
            return result;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }

        return Qnil;
    }

    /*
     * Document-method: contents
     *
     * call-seq:
     *     contents -> Contents
     *
     * Our Contents
     */
    VALUE
    package_id_contents(VALUE self)
    {
        std::shared_ptr<const PackageID> * self_ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageID>, self_ptr);
        try
        {
            auto contents((*self_ptr)->contents());
            if (! contents)
                return Qnil;
            else
                return contents_to_value(contents);
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }

        return Qnil;
    }

    /*
     * Document-method: masked?
     *
     * call-seq:
     *     masked? -> true or false
     *
     * Do we have any masks?
     */
    template <bool (PackageID::* m_) () const>
    struct PackageIDBool
    {
        static VALUE
        fetch(VALUE self)
        {
            std::shared_ptr<const PackageID> * self_ptr;
            Data_Get_Struct(self, std::shared_ptr<const PackageID>, self_ptr);
            try
            {
                return (self_ptr->get()->*m_)() ? Qtrue : Qfalse;
            }
            catch (const std::exception &e)
            {
                exception_to_ruby_exception(e);
            }

            return Qnil;
        }
    };

    /*
     * Document-method: keywords_key
     *
     * call-seq:
     *     keywords_key -> MetadataCollectionKey
     *
     * Our keywords
     */
    /*
     * Document-method: short_description_key
     *
     * call-seq:
     *     short_description_key -> MetadataStringKey
     *
     * Our short description
     */
    /*
     * Document-method: long_description_key
     *
     * call-seq:
     *     long_description_key -> MetadataStringKey
     *
     * Our long description
     */
    /*
     * Document-method: installed_time_key
     *
     * call-seq:
     *     installed_time_key -> MetadataTimeKey
     *
     * Our installed time
     */
    /*
     * Document-method: from_repositories_key
     *
     * call-seq:
     *     from_repositories_key -> MetadataCollectionKey
     *
     * Our source origin repository
     */
    /*
     * Document-method: dependencies_key
     *
     * call-seq:
     *     dependencies_key -> MetadataDependencySpecTreeKey
     *
     * Our dependencies
     */
    /*
     * Document-method: build_dependencies_key
     *
     * call-seq:
     *     build_dependencies_key -> MetadataDependencySpecTreeKey
     *
     * Our build dependencies
     */
    /*
     * Document-method: run_dependencies_key
     *
     * call-seq:
     *     run_dependencies_key -> MetadataDependencySpecTreeKey
     *
     * Our run dependencies
     */
    /*
     * Document-method: post_dependencies_key
     *
     * call-seq:
     *     post_dependencies_key -> MetadataDependencySpecTreeKey
     *
     * Our post dependencies
     */
    /*
     * Document-method: homepage_key
     *
     * call-seq:
     *     homepage_key -> MetadataSimpleURISpecTreeKey
     *
     * Our homepage
     */
    /*
     * Document-method: fetches_key
     *
     * call-seq:
     *     fetches_key -> MetadataFetchableURISpecTreeKey
     *
     * Things we fetch
     */
    /*
     * Document-method: choices_key
     *
     * call-seq:
     *     choices_key -> MetadataChoicesKey
     *
     * Our choices
     */
    template <typename T_, const std::shared_ptr<const T_> (PackageID::* m_) () const>
    struct KeyValue
    {
        static VALUE
        fetch(VALUE self)
        {
            try
            {
                std::shared_ptr<const PackageID> * self_ptr;
                Data_Get_Struct(self, std::shared_ptr<const PackageID>, self_ptr);
                std::shared_ptr<const MetadataKey> ptr = (((**self_ptr).*m_)());

                if (ptr)
                {
                    return metadata_key_to_value(((**self_ptr).*m_)());
                }
            }
            catch (const std::exception & e)
            {
                exception_to_ruby_exception(e);
            }

            return Qnil;
        }
    };

    void do_register_package_id()
    {
        /*
         * Document-class: Paludis::PackageID
         *
         * Metadata about a package.
         */
        c_package_id = rb_define_class_under(paludis_module(), "PackageID", rb_cObject);
        rb_funcall(c_package_id, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_package_id, "canonical_form", RUBY_FUNC_CAST(&package_id_canonical_form), 1);
        rb_define_method(c_package_id, "name", RUBY_FUNC_CAST(&package_id_name), 0);
        rb_define_method(c_package_id, "version", RUBY_FUNC_CAST(&package_id_version), 0);
        rb_define_method(c_package_id, "repository_name", RUBY_FUNC_CAST(&package_id_repository_name), 0);
        rb_define_method(c_package_id, "==", RUBY_FUNC_CAST(&Common<std::shared_ptr<const PackageID> >::equal_via_ptr), 1);
        rb_define_method(c_package_id, "[]", RUBY_FUNC_CAST(&package_id_subscript), 1);
        rb_define_method(c_package_id, "to_s", RUBY_FUNC_CAST(&Common<std::shared_ptr<const PackageID> >::to_s_via_ptr), 0);
        rb_define_method(c_package_id, "hash", RUBY_FUNC_CAST(&Common<std::shared_ptr<const PackageID> >::hash_via_ptr), 0);
        rb_define_method(c_package_id, "eql?", RUBY_FUNC_CAST(&Common<std::shared_ptr<const PackageID> >::equal_via_ptr), 1);
        rb_define_method(c_package_id, "supports_action", RUBY_FUNC_CAST(&package_id_supports_action), 1);
        rb_define_method(c_package_id, "perform_action", RUBY_FUNC_CAST(&package_id_perform_action), 1);
        rb_define_method(c_package_id, "each_metadata", RUBY_FUNC_CAST(&package_id_each_metadata), 0);

        rb_define_method(c_package_id, "masks", RUBY_FUNC_CAST(&package_id_masks), 0);
        rb_define_method(c_package_id, "overridden_masks", RUBY_FUNC_CAST(&package_id_overridden_masks), 0);
        rb_define_method(c_package_id, "masked?", RUBY_FUNC_CAST((&PackageIDBool<&PackageID::masked>::fetch)), 0);

        rb_define_method(c_package_id, "keywords_key", RUBY_FUNC_CAST((&KeyValue<MetadataCollectionKey<KeywordNameSet>,&PackageID::keywords_key>::fetch)), 0);
        rb_define_method(c_package_id, "dependencies_key", RUBY_FUNC_CAST((
                        &KeyValue<MetadataSpecTreeKey<DependencySpecTree>, &PackageID::dependencies_key>::fetch)), 0);
        rb_define_method(c_package_id, "build_dependencies_key", RUBY_FUNC_CAST((
                        &KeyValue<MetadataSpecTreeKey<DependencySpecTree>, &PackageID::build_dependencies_key>::fetch)), 0);
        rb_define_method(c_package_id, "run_dependencies_key", RUBY_FUNC_CAST((
                        &KeyValue<MetadataSpecTreeKey<DependencySpecTree>, &PackageID::run_dependencies_key>::fetch)), 0);
        rb_define_method(c_package_id, "post_dependencies_key", RUBY_FUNC_CAST((
                        &KeyValue<MetadataSpecTreeKey<DependencySpecTree>, &PackageID::post_dependencies_key>::fetch)), 0);
        rb_define_method(c_package_id, "homepage_key", RUBY_FUNC_CAST((
                        &KeyValue<MetadataSpecTreeKey<SimpleURISpecTree>, &PackageID::homepage_key>::fetch)), 0);
        rb_define_method(c_package_id, "short_description_key", RUBY_FUNC_CAST((&KeyValue<MetadataValueKey<std::string>,
                        &PackageID::short_description_key>::fetch)), 0);
        rb_define_method(c_package_id, "long_description_key", RUBY_FUNC_CAST((&KeyValue<MetadataValueKey<std::string>,
                        &PackageID::long_description_key>::fetch)), 0);
        rb_define_method(c_package_id, "installed_time_key", RUBY_FUNC_CAST((&KeyValue<MetadataTimeKey,&PackageID::installed_time_key>::fetch)), 0);
        rb_define_method(c_package_id, "from_repositories_key", RUBY_FUNC_CAST((&KeyValue<MetadataCollectionKey<Set<std::string> >,
                        &PackageID::from_repositories_key>::fetch)), 0);
        rb_define_method(c_package_id, "fs_location_key", RUBY_FUNC_CAST((
                        &KeyValue<MetadataValueKey<FSPath>, &PackageID::fs_location_key>::fetch)), 0);
        rb_define_method(c_package_id, "fetches_key", RUBY_FUNC_CAST((
                        &KeyValue<MetadataSpecTreeKey<FetchableURISpecTree>, &PackageID::fetches_key>::fetch)), 0);
        rb_define_method(c_package_id, "choices_key", RUBY_FUNC_CAST((
                        &KeyValue<MetadataValueKey<std::shared_ptr<const Choices> >, &PackageID::choices_key>::fetch)), 0);
        rb_define_method(c_package_id, "slot_key", RUBY_FUNC_CAST((
                        &KeyValue<MetadataValueKey<Slot>, &PackageID::slot_key>::fetch)), 0);
        rb_define_method(c_package_id, "behaviours_key", RUBY_FUNC_CAST((
                        &KeyValue<MetadataCollectionKey<Set<std::string> >, &PackageID::behaviours_key>::fetch)), 0);
        rb_define_method(c_package_id, "uniquely_identifying_spec", RUBY_FUNC_CAST(&package_id_uniquely_identifying_spec), 0);
        rb_define_method(c_package_id, "contents", RUBY_FUNC_CAST(&package_id_contents), 0);

        /*
         * Document-module: Paludis::PackageIDCanonicalForm
         *
         * How to generate PackageID.canonical_form
         */
        c_package_id_canonical_form = rb_define_module_under(paludis_module(), "PackageIDCanonicalForm");
        for (PackageIDCanonicalForm l(static_cast<PackageIDCanonicalForm>(0)), l_end(last_idcf) ; l != l_end ;
                l = static_cast<PackageIDCanonicalForm>(static_cast<int>(l) + 1))
            rb_define_const(c_package_id_canonical_form, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/package_id-se.hh, PackageIDCanonicalForm, c_package_id_canonical_form>

    }
}

VALUE
paludis::ruby::package_id_to_value(std::shared_ptr<const PackageID> m)
{
    std::shared_ptr<const PackageID> * m_ptr(0);
    try
    {
        m_ptr = new std::shared_ptr<const PackageID>(m);
        return Data_Wrap_Struct(c_package_id, 0, &Common<std::shared_ptr<const PackageID> >::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

std::shared_ptr<const PackageID>
paludis::ruby::value_to_package_id(VALUE v, bool nil_ok)
{
    if (nil_ok && rb_obj_is_kind_of(v, rb_cNilClass))
        return std::shared_ptr<const PackageID>();

    if (rb_obj_is_kind_of(v, c_package_id))
    {
        std::shared_ptr<const PackageID> * v_ptr;
        Data_Get_Struct(v, std::shared_ptr<const PackageID>, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into PackageID", rb_obj_classname(v));
    }
}

RegisterRubyClass::Register paludis_ruby_register_package_id PALUDIS_ATTRIBUTE((used))
    (&do_register_package_id);

