/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Richard Brown
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/slot.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/timestamp.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

namespace
{
    static VALUE c_metadata_key;
    static VALUE c_metadata_package_id_key;
    static VALUE c_metadata_string_key;
    static VALUE c_metadata_slot_key;
    static VALUE c_metadata_size_key;
    static VALUE c_metadata_time_key;
    static VALUE c_metadata_choices_key;
    static VALUE c_metadata_keyword_name_set_key;
    static VALUE c_metadata_string_set_key;
    static VALUE c_metadata_string_string_map_key;
    static VALUE c_metadata_string_sequence_key;
    static VALUE c_metadata_package_id_sequence_key;
    static VALUE c_metadata_fsentry_key;
    static VALUE c_metadata_fsentry_sequence_key;
    static VALUE c_metadata_maintainers_key;
    static VALUE c_metadata_key_type;
    static VALUE c_metadata_license_spec_tree_key;
    static VALUE c_metadata_fetchable_uri_spec_tree_key;
    static VALUE c_metadata_simple_uri_spec_tree_key;
    static VALUE c_metadata_dependency_spec_tree_key;
    static VALUE c_metadata_plain_text_spec_tree_key;
    static VALUE c_metadata_required_use_spec_tree_key;
    static VALUE c_metadata_section_key;

    /*
     * Document-method: raw_name
     *
     * call-seq:
     *     raw_name -> String
     *
     * Our raw name
     */
    /*
     * Document-method: human_name
     *
     * call-seq:
     *     human_name -> String
     *
     * Our human name
     */
    template <typename T_, typename S_, const T_ (S_::* m_) () const>
    struct BaseValue
    {
        static VALUE
        fetch(VALUE self)
        {
            try
            {
                std::shared_ptr<const MetadataKey> * self_ptr;
                Data_Get_Struct(self, std::shared_ptr<const MetadataKey>, self_ptr);
                return rb_str_new2(stringify(((*std::static_pointer_cast<const S_>(*self_ptr)).*m_)()).c_str());
            }
            catch (const std::exception & e)
            {
                exception_to_ruby_exception(e);
            }
        }
    };

    struct V
    {
        VALUE value;
        std::shared_ptr<const MetadataKey> mm;

        V(std::shared_ptr<const MetadataKey> _m) :
            mm(_m)
        {
        }

        void visit(const MetadataValueKey<std::shared_ptr<const PackageID> > &)
        {
            value = Data_Wrap_Struct(c_metadata_package_id_key, 0, &Common<std::shared_ptr<const MetadataKey> >::free,
                    new std::shared_ptr<const MetadataKey>(mm));
        }

        void visit(const MetadataValueKey<std::string> &)
        {
            value = Data_Wrap_Struct(c_metadata_string_key, 0, &Common<std::shared_ptr<const MetadataKey> >::free,
                    new std::shared_ptr<const MetadataKey>(mm));
        }

        void visit(const MetadataValueKey<Slot> &)
        {
            value = Data_Wrap_Struct(c_metadata_slot_key, 0, &Common<std::shared_ptr<const MetadataKey> >::free,
                    new std::shared_ptr<const MetadataKey>(mm));
        }

        void visit(const MetadataValueKey<long> &)
        {
            value = Data_Wrap_Struct(c_metadata_size_key, 0, &Common<std::shared_ptr<const MetadataKey> >::free,
                    new std::shared_ptr<const MetadataKey>(mm));
        }

        void visit(const MetadataValueKey<bool> &)
        {
            value = Data_Wrap_Struct(c_metadata_size_key, 0, &Common<std::shared_ptr<const MetadataKey> >::free,
                    new std::shared_ptr<const MetadataKey>(mm));
        }

        void visit(const MetadataTimeKey &)
        {
            value = Data_Wrap_Struct(c_metadata_time_key, 0, &Common<std::shared_ptr<const MetadataKey> >::free,
                    new std::shared_ptr<const MetadataKey>(mm));
        }

        void visit(const MetadataValueKey<std::shared_ptr<const Choices> > &)
        {
            value = Data_Wrap_Struct(c_metadata_choices_key, 0, &Common<std::shared_ptr<const MetadataKey> >::free,
                    new std::shared_ptr<const MetadataKey>(mm));
        }

        void visit(const MetadataValueKey<FSPath> &)
        {
            value = Data_Wrap_Struct(c_metadata_fsentry_key, 0, &Common<std::shared_ptr<const MetadataKey> >::free,
                    new std::shared_ptr<const MetadataKey>(mm));
        }

        void visit(const MetadataSectionKey &)
        {
            value = Data_Wrap_Struct(c_metadata_section_key, 0, &Common<std::shared_ptr<const MetadataKey> >::free,
                    new std::shared_ptr<const MetadataKey>(mm));
        }

        void visit(const MetadataCollectionKey<KeywordNameSet> &)
        {
            value = Data_Wrap_Struct(c_metadata_keyword_name_set_key, 0, &Common<std::shared_ptr<const MetadataKey> >::free,
                    new std::shared_ptr<const MetadataKey>(mm));
        }

        void visit(const MetadataCollectionKey<Set<std::string> > &)
        {
            value = Data_Wrap_Struct(c_metadata_string_set_key, 0, &Common<std::shared_ptr<const MetadataKey> >::free,
                    new std::shared_ptr<const MetadataKey>(mm));
        }

        void visit(const MetadataCollectionKey<Map<std::string, std::string> > &)
        {
            value = Data_Wrap_Struct(c_metadata_string_string_map_key, 0, &Common<std::shared_ptr<const MetadataKey> >::free,
                    new std::shared_ptr<const MetadataKey>(mm));
        }

        void visit(const MetadataCollectionKey<Sequence<std::string> > &)
        {
            value = Data_Wrap_Struct(c_metadata_string_sequence_key, 0, &Common<std::shared_ptr<const MetadataKey> >::free,
                    new std::shared_ptr<const MetadataKey>(mm));
        }

        void visit(const MetadataCollectionKey<FSPathSequence> &)
        {
            value = Data_Wrap_Struct(c_metadata_fsentry_sequence_key, 0, &Common<std::shared_ptr<const MetadataKey> >::free,
                    new std::shared_ptr<const MetadataKey>(mm));
        }

        void visit(const MetadataCollectionKey<Maintainers> &)
        {
            value = Data_Wrap_Struct(c_metadata_maintainers_key, 0, &Common<std::shared_ptr<const MetadataKey> >::free,
                    new std::shared_ptr<const MetadataKey>(mm));
        }

        void visit(const MetadataCollectionKey<PackageIDSequence> &)
        {
            value = Data_Wrap_Struct(c_metadata_package_id_sequence_key, 0, &Common<std::shared_ptr<const MetadataKey> >::free,
                    new std::shared_ptr<const MetadataKey>(mm));
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> &)
        {
            value = Data_Wrap_Struct(c_metadata_license_spec_tree_key, 0, &Common<std::shared_ptr<const MetadataKey> >::free,
                    new std::shared_ptr<const MetadataKey>(mm));
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> &)
        {
            value = Data_Wrap_Struct(c_metadata_dependency_spec_tree_key, 0, &Common<std::shared_ptr<const MetadataKey> >::free,
                    new std::shared_ptr<const MetadataKey>(mm));
        }

        void visit(const MetadataSpecTreeKey<PlainTextSpecTree> &)
        {
            value = Data_Wrap_Struct(c_metadata_plain_text_spec_tree_key, 0, &Common<std::shared_ptr<const MetadataKey> >::free,
                    new std::shared_ptr<const MetadataKey>(mm));
        }

        void visit(const MetadataSpecTreeKey<RequiredUseSpecTree> &)
        {
            value = Data_Wrap_Struct(c_metadata_required_use_spec_tree_key, 0, &Common<std::shared_ptr<const MetadataKey> >::free,
                    new std::shared_ptr<const MetadataKey>(mm));
        }

        void visit(const MetadataSpecTreeKey<FetchableURISpecTree> &)
        {
            value = Data_Wrap_Struct(c_metadata_fetchable_uri_spec_tree_key, 0, &Common<std::shared_ptr<const MetadataKey> >::free,
                    new std::shared_ptr<const MetadataKey>(mm));
        }

        void visit(const MetadataSpecTreeKey<SimpleURISpecTree> &)
        {
            value = Data_Wrap_Struct(c_metadata_simple_uri_spec_tree_key, 0, &Common<std::shared_ptr<const MetadataKey> >::free,
                    new std::shared_ptr<const MetadataKey>(mm));
        }
    };

    /*
     * call-seq:
     *     type -> MetadataKeyType
     *
     * Our significance to a user.
     */
    VALUE
    metadata_key_type(VALUE self)
    {
        std::shared_ptr<const MetadataKey> * self_ptr;
        Data_Get_Struct(self, std::shared_ptr<const MetadataKey>, self_ptr);
        return INT2FIX((*self_ptr)->type());
    }

    /*
     * call-seq:
     *     parse_value -> PackageID
     *
     * Our Value.
     * */
    VALUE
    metadata_package_id_key_value(VALUE self)
    {
        try
        {
            std::shared_ptr<const MetadataKey> * self_ptr;
            Data_Get_Struct(self, std::shared_ptr<const MetadataKey>, self_ptr);
            return package_id_to_value((std::static_pointer_cast<const MetadataValueKey<std::shared_ptr<const PackageID> > >(*self_ptr))->parse_value());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     parse_value -> String
     *
     * Our Value.
     * */
    VALUE
    metadata_string_key_value(VALUE self)
    {
        try
        {
            std::shared_ptr<const MetadataKey> * self_ptr;
            Data_Get_Struct(self, std::shared_ptr<const MetadataKey>, self_ptr);
            return rb_str_new2((std::static_pointer_cast<const MetadataValueKey<std::string> >(*self_ptr))->parse_value().c_str());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     parse_value -> String
     *
     * Our Value.
     * */
    VALUE
    metadata_slot_key_value(VALUE self)
    {
        try
        {
            std::shared_ptr<const MetadataKey> * self_ptr;
            Data_Get_Struct(self, std::shared_ptr<const MetadataKey>, self_ptr);
            return rb_str_new2(stringify((std::static_pointer_cast<const MetadataValueKey<Slot> >(*self_ptr))->parse_value().raw_value()).c_str());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     parse_value -> Numeric
     *
     * Our Value.
     * */
    VALUE
    metadata_size_key_value(VALUE self)
    {
        try
        {
            std::shared_ptr<const MetadataKey> * self_ptr;
            Data_Get_Struct(self, std::shared_ptr<const MetadataKey>, self_ptr);
            return LONG2NUM((std::static_pointer_cast<const MetadataValueKey<long> >(*self_ptr))->parse_value());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     parse_value -> String
     *
     * Our Value.
     * */
    VALUE
    metadata_fsentry_key_value(VALUE self)
    {
        try
        {
            std::shared_ptr<const MetadataKey> * self_ptr;
            Data_Get_Struct(self, std::shared_ptr<const MetadataKey>, self_ptr);
            return rb_str_new2(stringify((std::static_pointer_cast<const MetadataValueKey<FSPath> >(*self_ptr))->parse_value()).c_str());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     parse_value -> Time
     *
     * Our Value.
     * */
    VALUE
    metadata_time_key_value(VALUE self)
    {
        try
        {
            std::shared_ptr<const MetadataKey> * self_ptr;
            Data_Get_Struct(self, std::shared_ptr<const MetadataKey>, self_ptr);
            return rb_time_new((std::static_pointer_cast<const MetadataTimeKey>(*self_ptr))->parse_value().seconds(), 0);
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     parse_value -> Choices
     *
     * Our Value.
     * */
    VALUE
    metadata_choices_key_value(VALUE self)
    {
        try
        {
            std::shared_ptr<const MetadataKey> * self_ptr;
            Data_Get_Struct(self, std::shared_ptr<const MetadataKey>, self_ptr);
            return choices_to_value(std::static_pointer_cast<const MetadataValueKey<std::shared_ptr<const Choices> > >(*self_ptr)->parse_value());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     parse_value -> Array
     *
     * Our Value.
     * */
    template <typename T_>
    struct SetValue
    {
        static VALUE
        fetch(VALUE self)
        {
            try
            {
                std::shared_ptr<const MetadataKey> * self_ptr;
                Data_Get_Struct(self, std::shared_ptr<const MetadataKey>, self_ptr);
                std::shared_ptr<const T_> c = std::static_pointer_cast<const MetadataCollectionKey<T_> >(*self_ptr)->parse_value();
                VALUE result (rb_ary_new());
                for (const auto & item : *c)
                    rb_ary_push(result, rb_str_new2(stringify(item).c_str()));
                return result;
            }
            catch (const std::exception & e)
            {
                exception_to_ruby_exception(e);
            }
        }
    };

    /*
     * call-seq:
     *     parse_value -> Hash
     *
     * Our Value.
     * */
    template <typename T_>
    struct MapValue
    {
        static VALUE
        fetch(VALUE self)
        {
            try
            {
                std::shared_ptr<const MetadataKey> * self_ptr;
                Data_Get_Struct(self, std::shared_ptr<const MetadataKey>, self_ptr);
                std::shared_ptr<const T_> c = std::static_pointer_cast<const MetadataCollectionKey<T_> >(*self_ptr)->parse_value();
                VALUE result (rb_hash_new());
                for (const auto & item : *c)
                    rb_hash_aset(result, rb_str_new2(stringify(item.first).c_str()), rb_str_new2(stringify(item.second).c_str()));
                return result;
            }
            catch (const std::exception & e)
            {
                exception_to_ruby_exception(e);
            }
        }
    };

    /*
     * call-seq:
     *     parse_value -> Array
     *
     * Our Value.
     * */
    template <>
    struct SetValue<PackageIDSequence>
    {
        static VALUE
        fetch(VALUE self)
        {
            try
            {
                std::shared_ptr<const MetadataKey> * self_ptr;
                Data_Get_Struct(self, std::shared_ptr<const MetadataKey>, self_ptr);
                std::shared_ptr<const PackageIDSequence> c = std::static_pointer_cast<const MetadataCollectionKey<PackageIDSequence> >(
                        *self_ptr)->parse_value();
                VALUE result (rb_ary_new());
                for (const auto & id : *c)
                    rb_ary_push(result, package_id_to_value(id));
                return result;
            }
            catch (const std::exception & e)
            {
                exception_to_ruby_exception(e);
            }
        }
    };

    /*
     * call-seq:
     *     parse_value -> spec tree
     *
     * Our Value.
     * */
    template <typename T_>
    struct SpecTreeValue
    {
        static VALUE
        fetch(VALUE self)
        {
            try
            {
                std::shared_ptr<const MetadataKey> * self_ptr;
                Data_Get_Struct(self, std::shared_ptr<const MetadataKey>, self_ptr);
                std::shared_ptr<const T_> c = std::static_pointer_cast<const MetadataSpecTreeKey<T_> >(*self_ptr)->parse_value();
                return dep_tree_to_value<T_>(c);
            }
            catch (const std::exception & e)
            {
                exception_to_ruby_exception(e);
            }
        }
    };

    /*
     * call-seq:
     *     initial_label -> URILabel
     *
     * Return the initial label to use when deciding the behaviour of
     * individual items in the heirarchy.
     */
    VALUE
    metadata_fetchable_uri_spec_tree_key_initial_label(VALUE self)
    {
        try
        {
            std::shared_ptr<const MetadataKey> * self_ptr;
            Data_Get_Struct(self, std::shared_ptr<const MetadataKey>, self_ptr);
            return uri_label_to_value((std::static_pointer_cast<const MetadataSpecTreeKey<FetchableURISpecTree> >(*self_ptr))->initial_label());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     initial_labels -> Array of DependenciesLabel
     *
     * Return the initial labels to use when deciding the behaviour of
     * individual items in the heirarchy.
     */
    VALUE
    metadata_dependency_spec_tree_key_initial_labels(VALUE self)
    {
        try
        {
            std::shared_ptr<const MetadataKey> * self_ptr;
            Data_Get_Struct(self, std::shared_ptr<const MetadataKey>, self_ptr);
            const MetadataSpecTreeKey<DependencySpecTree> * real_self(visitor_cast<
                    const MetadataSpecTreeKey<DependencySpecTree> >(**self_ptr));

            VALUE result(rb_ary_new());

            for (const auto & label : *real_self->initial_labels())
                rb_ary_push(result, dependencies_label_to_value(label));

            return result;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     each_metadata {|key| block } -> Nil
     *
     * Our metadata.
     */
    VALUE
    metadata_section_key_each_metadata(VALUE self)
    {
        std::shared_ptr<const MetadataKey> * self_ptr;
        Data_Get_Struct(self, std::shared_ptr<const MetadataKey>, self_ptr);
        std::shared_ptr<const MetadataSectionKey> c = std::static_pointer_cast<const MetadataSectionKey>(*self_ptr);
        for (const auto & key : c->metadata())
        {
            VALUE val(metadata_key_to_value(key));
            if (Qnil != val)
                rb_yield(val);
        }
        return Qnil;
    }

    /*
     * call-seq:
     *     [String] -> MetadataKey or Nil
     *
     * The named metadata key
     */
    VALUE
    metadata_section_key_subscript(VALUE self, VALUE raw_name)
    {
        std::shared_ptr<const MetadataKey> * self_ptr;
        Data_Get_Struct(self, std::shared_ptr<const MetadataKey>, self_ptr);
        std::shared_ptr<const MetadataSectionKey> c = std::static_pointer_cast<const MetadataSectionKey>(*self_ptr);
        MetadataSectionKey::MetadataConstIterator it((c)->find_metadata(StringValuePtr(raw_name)));
        if (c->end_metadata() == it)
            return Qnil;
        return metadata_key_to_value(*it);
    }

    void do_register_metadata_key()
    {
        /*
         * Document-class: Paludis::MetadataKey
         *
         * Base metadata class, subclasses contain a "parse_value" to return the contents of the key.
         */
        c_metadata_key = rb_define_class_under(paludis_module(), "MetadataKey", rb_cObject);
        rb_funcall(c_metadata_key, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_metadata_key, "raw_name", RUBY_FUNC_CAST((&BaseValue<std::string,MetadataKey,&MetadataKey::raw_name>::fetch)), 0);
        rb_define_method(c_metadata_key, "human_name", RUBY_FUNC_CAST((&BaseValue<std::string,MetadataKey,&MetadataKey::human_name>::fetch)), 0);
        rb_define_method(c_metadata_key, "type", RUBY_FUNC_CAST(&metadata_key_type), 0);

        /*
         * Document-class: Paludis::MetadataPackageIDKey
         *
         * Metadata class for a PackageId.
         */
        c_metadata_package_id_key = rb_define_class_under(paludis_module(), "MetadataPackageIDKey", c_metadata_key);
        rb_define_method(c_metadata_package_id_key, "parse_value", RUBY_FUNC_CAST(&metadata_package_id_key_value), 0);

        /*
         * Document-class: Paludis::MetadataStringKey
         *
         * Metadata class for Strings.
         */
        c_metadata_string_key = rb_define_class_under(paludis_module(), "MetadataStringKey", c_metadata_key);
        rb_define_method(c_metadata_string_key, "parse_value", RUBY_FUNC_CAST(&metadata_string_key_value), 0);

        /*
         * Document-class: Paludis::MetadataSlotNameKey
         *
         * Metadata class for SlotNames.
         */
        c_metadata_slot_key = rb_define_class_under(paludis_module(), "MetadataSlotNameKey", c_metadata_key);
        rb_define_method(c_metadata_slot_key, "parse_value", RUBY_FUNC_CAST(&metadata_slot_key_value), 0);

        /*
         * Document-class: Paludis::MetadataSizeKey
         *
         * Metadata class for file sizes.
         */
        c_metadata_size_key = rb_define_class_under(paludis_module(), "MetadataSizeKey", c_metadata_key);
        rb_define_method(c_metadata_size_key, "parse_value", RUBY_FUNC_CAST(&metadata_size_key_value), 0);

        /*
         * Document-class: Paludis::MetadataFSPathKey
         *
         * Metadata class for FSPath.
         */
        c_metadata_fsentry_key = rb_define_class_under(paludis_module(), "MetadataFSPathKey", c_metadata_key);
        rb_define_method(c_metadata_fsentry_key, "parse_value", RUBY_FUNC_CAST(&metadata_fsentry_key_value), 0);

        /*
         * Document-class: Paludis::MetadataTimeKey
         *
         * Metadata class for Time.
         */
        c_metadata_time_key = rb_define_class_under(paludis_module(), "MetadataTimeKey", c_metadata_key);
        rb_define_method(c_metadata_time_key, "parse_value", RUBY_FUNC_CAST(&metadata_time_key_value), 0);

        /*
         * Document-class: Paludis::MetadataChoicesKey
         *
         * Metadata class for Choices.
         */
        c_metadata_choices_key = rb_define_class_under(paludis_module(), "MetadataChoicesKey", c_metadata_key);
        rb_define_method(c_metadata_choices_key, "parse_value", RUBY_FUNC_CAST(&metadata_choices_key_value), 0);

        /*
         * Document-class: Paludis::MetadataKeywordNameSetKey
         *
         * Metadata class for keywords.
         */
        c_metadata_keyword_name_set_key = rb_define_class_under(paludis_module(), "MetadataKeywordNameSetKey", c_metadata_key);
        rb_define_method(c_metadata_keyword_name_set_key, "parse_value", RUBY_FUNC_CAST((&SetValue<KeywordNameSet>::fetch)), 0);

        /*
         * Document-class: Paludis::MetadataPackageIDSequenceKey
         *
         * Metadata class for package IDs.
         */
        c_metadata_package_id_sequence_key = rb_define_class_under(paludis_module(), "MetadataPackageIDSequenceKey", c_metadata_key);
        rb_define_method(c_metadata_package_id_sequence_key, "parse_value", RUBY_FUNC_CAST((&SetValue<PackageIDSequence>::fetch)), 0);

        /*
         * Document-class: Paludis::MetadataFSPathSequenceKey
         *
         * Metadata class for filesystem sequences.
         */
        c_metadata_fsentry_sequence_key = rb_define_class_under(paludis_module(), "MetadataFSPathSequenceKey", c_metadata_key);
        rb_define_method(c_metadata_fsentry_sequence_key, "parse_value", RUBY_FUNC_CAST((&SetValue<Maintainers>::fetch)), 0);

        /*
         * Document-class: Paludis::MetadataMaintainersKey
         *
         * Metadata class for maintainers.
         */
        c_metadata_maintainers_key = rb_define_class_under(paludis_module(), "MetadataMaintainersKey", c_metadata_key);
        rb_define_method(c_metadata_maintainers_key, "parse_value", RUBY_FUNC_CAST((&SetValue<FSPathSequence>::fetch)), 0);

        /*
         * Document-class: Paludis::MetadataStringSetKey
         *
         * Metadata class for String sets.
         */
        c_metadata_string_set_key = rb_define_class_under(paludis_module(), "MetadataStringSetKey", c_metadata_key);
        rb_define_method(c_metadata_string_set_key, "parse_value", RUBY_FUNC_CAST((&SetValue<Set<std::string> >::fetch)), 0);

        /*
         * Document-class: Paludis::MetadataStringStringMapKey
         *
         * Metadata class for String to String maps.
         */
        c_metadata_string_string_map_key = rb_define_class_under(paludis_module(), "MetadataStringStringMapKey", c_metadata_key);
        rb_define_method(c_metadata_string_string_map_key, "parse_value", RUBY_FUNC_CAST((&MapValue<Map<std::string, std::string> >::fetch)), 0);

        /*
         * Document-class: Paludis::MetadataStringSequenceKey
         *
         * Metadata class for String sequences.
         */
        c_metadata_string_sequence_key = rb_define_class_under(paludis_module(), "MetadataStringSequenceKey", c_metadata_key);
        rb_define_method(c_metadata_string_sequence_key, "parse_value", RUBY_FUNC_CAST((&SetValue<Sequence<std::string> >::fetch)), 0);

        /*
         * Document-class: Paludis::MetadataLicenseSpecTreeKey
         *
         * Metadata class for license specs.
         */
        c_metadata_license_spec_tree_key = rb_define_class_under(paludis_module(), "MetadataLicenseSpecTreeKey", c_metadata_key);
        rb_define_method(c_metadata_license_spec_tree_key, "parse_value", RUBY_FUNC_CAST((&SpecTreeValue<LicenseSpecTree>::fetch)), 0);

        /*
         * Document-class: Paludis::MetadataPlainTextSpecTreeKey
         *
         * Metadata class for restrict specs.
         */
        c_metadata_plain_text_spec_tree_key = rb_define_class_under(paludis_module(), "MetadataPlainTextSpecTreeKey", c_metadata_key);
        rb_define_method(c_metadata_plain_text_spec_tree_key, "parse_value", RUBY_FUNC_CAST((&SpecTreeValue<PlainTextSpecTree>::fetch)), 0);

        /*
         * Document-class: Paludis::MetadataRequiredUseSpecTreeKey
         *
         * Metadata class for restrict specs.
         */
        c_metadata_required_use_spec_tree_key = rb_define_class_under(paludis_module(), "MetadataRequiredUseSpecTreeKey", c_metadata_key);
        rb_define_method(c_metadata_required_use_spec_tree_key, "parse_value", RUBY_FUNC_CAST((&SpecTreeValue<RequiredUseSpecTree>::fetch)), 0);

        /*
         * Document-class: Paludis::MetadataDependencySpecTreeKey
         *
         * Metadata class for dependency specs.
         */
        c_metadata_dependency_spec_tree_key = rb_define_class_under(paludis_module(), "MetadataDependencySpecTreeKey", c_metadata_key);
        rb_define_method(c_metadata_dependency_spec_tree_key, "parse_value", RUBY_FUNC_CAST((&SpecTreeValue<DependencySpecTree>::fetch)), 0);
        rb_define_method(c_metadata_dependency_spec_tree_key, "initial_labels", RUBY_FUNC_CAST(&metadata_dependency_spec_tree_key_initial_labels), 0);

        /*
         * Document-class: Paludis::MetadataFetchableURISpecTreeKey
         *
         * Metadata class for fetchable uri specs.
         */
        c_metadata_fetchable_uri_spec_tree_key = rb_define_class_under(paludis_module(), "MetadataFetchableURISpecTreeKey", c_metadata_key);
        rb_define_method(c_metadata_fetchable_uri_spec_tree_key, "parse_value", RUBY_FUNC_CAST((&SpecTreeValue<FetchableURISpecTree>::fetch)), 0);
        rb_define_method(c_metadata_fetchable_uri_spec_tree_key, "initial_label", RUBY_FUNC_CAST(&metadata_fetchable_uri_spec_tree_key_initial_label), 0);

        /*
         * Document-class: Paludis::MetadataSimpleURISpecTreeKey
         *
         * Metadata class for simple uri specs.
         */
        c_metadata_simple_uri_spec_tree_key = rb_define_class_under(paludis_module(), "MetadataSimpleURISpecTreeKey", c_metadata_key);
        rb_define_method(c_metadata_simple_uri_spec_tree_key, "parse_value", RUBY_FUNC_CAST((&SpecTreeValue<SimpleURISpecTree>::fetch)), 0);

        /*
         * Document-class: Paludis::MetadataSectionKey
         *
         * A MetadataSectionKey is a MetadataKey that holds a number of other
         * MetadataKey instances.
         */
        c_metadata_section_key = rb_define_class_under(paludis_module(), "MetadataSectionKey", c_metadata_key);
        rb_define_method(c_metadata_section_key, "each_metadata", RUBY_FUNC_CAST(&metadata_section_key_each_metadata), 0);
        rb_define_method(c_metadata_section_key, "[]", RUBY_FUNC_CAST(&metadata_section_key_subscript), 1);

        /*
         * Document-module: Paludis::MetadataKeyType
         *
         * The significance of a MetadataKey to a user.
         */
        c_metadata_key_type = rb_define_module_under(paludis_module(), "MetadataKeyType");
        for (MetadataKeyType l(static_cast<MetadataKeyType>(0)), l_end(last_mkt) ; l != l_end ;
                l = static_cast<MetadataKeyType>(static_cast<int>(l) + 1))
            rb_define_const(c_metadata_key_type, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/metadata_key-se.hh, MetadataKeyType, c_metadata_key_type>
    }
}

VALUE
paludis::ruby::metadata_key_to_value(std::shared_ptr<const MetadataKey> m)
{
    try
    {
        V v(m);
        m->accept(v);
        return v.value;
    }
    catch (const std::exception & e)
    {
        exception_to_ruby_exception(e);
    }
}

RegisterRubyClass::Register paludis_ruby_metadata_key PALUDIS_ATTRIBUTE((used))
    (&do_register_metadata_key);

