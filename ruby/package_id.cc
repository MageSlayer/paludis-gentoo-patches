/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Richard Brown <rbrown@gentoo.org>
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
#include <paludis/util/visitor-impl.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_package_id;
    static VALUE c_package_id_canonical_form;
    static VALUE c_metadata_key;
    static VALUE c_metadata_package_id_key;
    static VALUE c_metadata_string_key;
    static VALUE c_metadata_time_key;
    static VALUE c_metadata_contents_key;
    static VALUE c_metadata_keyword_name_collection_key;
    static VALUE c_metadata_use_flag_name_collection_key;
    static VALUE c_metadata_iuse_flag_collection_key;
    static VALUE c_metadata_inherited_collection_key;
    static VALUE c_metadata_key_type;

    struct V :
        ConstVisitor<MetadataKeyVisitorTypes>
        {
            VALUE value;
            tr1::shared_ptr<const MetadataKey> mm;

            V(tr1::shared_ptr<const MetadataKey> _m) :
                mm(_m)
            {
            }

            void visit(const MetadataPackageIDKey &)
            {
                value = Data_Wrap_Struct(c_metadata_package_id_key, 0, &Common<tr1::shared_ptr<const MetadataPackageIDKey> >::free,
                        new tr1::shared_ptr<const MetadataPackageIDKey>(tr1::static_pointer_cast<const MetadataPackageIDKey>(mm)));
            }

            void visit(const MetadataStringKey &)
            {
                value = Data_Wrap_Struct(c_metadata_string_key, 0, &Common<tr1::shared_ptr<const MetadataStringKey> >::free,
                        new tr1::shared_ptr<const MetadataStringKey>(tr1::static_pointer_cast<const MetadataStringKey>(mm)));
            }

            void visit(const MetadataTimeKey &)
            {
                value = Data_Wrap_Struct(c_metadata_time_key, 0, &Common<tr1::shared_ptr<const MetadataTimeKey> >::free,
                        new tr1::shared_ptr<const MetadataTimeKey>(tr1::static_pointer_cast<const MetadataTimeKey>(mm)));
            }

            void visit(const MetadataContentsKey &)
            {
                value = Data_Wrap_Struct(c_metadata_contents_key, 0, &Common<tr1::shared_ptr<const MetadataContentsKey> >::free,
                        new tr1::shared_ptr<const MetadataContentsKey>(tr1::static_pointer_cast<const MetadataContentsKey>(mm)));
            }

            void visit(const MetadataCollectionKey<KeywordNameCollection> &)
            {
                value = Data_Wrap_Struct(c_metadata_keyword_name_collection_key, 0, &Common<tr1::shared_ptr<const MetadataCollectionKey<KeywordNameCollection> > >::free,
                        new tr1::shared_ptr<const MetadataCollectionKey<KeywordNameCollection> >(tr1::static_pointer_cast<const MetadataCollectionKey<KeywordNameCollection> >(mm)));
            }

            void visit(const MetadataCollectionKey<UseFlagNameCollection> &)
            {
                value = Data_Wrap_Struct(c_metadata_use_flag_name_collection_key, 0, &Common<tr1::shared_ptr<const MetadataCollectionKey<UseFlagNameCollection> > >::free,
                        new tr1::shared_ptr<const MetadataCollectionKey<UseFlagNameCollection> >(tr1::static_pointer_cast<const MetadataCollectionKey<UseFlagNameCollection> >(mm)));
            }

            void visit(const MetadataCollectionKey<IUseFlagCollection> &)
            {
                value = Data_Wrap_Struct(c_metadata_iuse_flag_collection_key, 0, &Common<tr1::shared_ptr<const MetadataCollectionKey<IUseFlagCollection> > >::free,
                        new tr1::shared_ptr<const MetadataCollectionKey<IUseFlagCollection> >(tr1::static_pointer_cast<const MetadataCollectionKey<IUseFlagCollection> >(mm)));
            }

            void visit(const MetadataCollectionKey<InheritedCollection> &)
            {
                value = Data_Wrap_Struct(c_metadata_inherited_collection_key, 0, &Common<tr1::shared_ptr<const MetadataCollectionKey<InheritedCollection> > >::free,
                        new tr1::shared_ptr<const MetadataCollectionKey<InheritedCollection> >(tr1::static_pointer_cast<const MetadataCollectionKey<InheritedCollection> >(mm)));
            }

            void visit(const MetadataSpecTreeKey<LicenseSpecTree> &)
            {
                value = Qnil;
            }

            void visit(const MetadataSpecTreeKey<ProvideSpecTree> &)
            {
                value = Qnil;
            }

            void visit(const MetadataSpecTreeKey<DependencySpecTree> &)
            {
                value = Qnil;
            }

            void visit(const MetadataSpecTreeKey<RestrictSpecTree> &)
            {
                value = Qnil;
            }

            void visit(const MetadataSpecTreeKey<URISpecTree> &)
            {
                value = Qnil;
            }
        };

    VALUE
    metadata_key_to_value(tr1::shared_ptr<const MetadataKey> m)
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

    /*
     * call-seq:
     *     canonical_form(form) -> String
     *
     * Return this PackageID in a PackageIDCanonicalForm.
     */
    VALUE
    package_id_canonical_form(VALUE self, VALUE cf)
    {
        tr1::shared_ptr<const PackageID> * self_ptr;
        Data_Get_Struct(self, tr1::shared_ptr<const PackageID>, self_ptr);
        return rb_str_new2(((*self_ptr)->canonical_form(static_cast<PackageIDCanonicalForm>(NUM2INT(cf)))).c_str());
    }

    /*
     * Document-method: slot
     *
     * call-seq:
     *     slot -> String
     *
     * Our slot
     */
    /*
     * Document-method: name
     *
     * call-seq:
     *     name -> String
     *
     * Our name
     */
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
            tr1::shared_ptr<const S_> * self_ptr;
            Data_Get_Struct(self, tr1::shared_ptr<const S_>, self_ptr);
            return rb_str_new2(stringify(((**self_ptr).*m_)()).c_str());
        }
    };

    /*
     * Document-method: eapi
     *
     * call-seq:
     *     eapi -> EAPI
     *
     * Our EAPI.
     */
    VALUE
    package_id_eapi(VALUE self)
    {
        tr1::shared_ptr<const PackageID> * self_ptr;
        Data_Get_Struct(self, tr1::shared_ptr<const PackageID>, self_ptr);
        return eapi_to_value((*self_ptr)->eapi());
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
        tr1::shared_ptr<const PackageID> * self_ptr;
        Data_Get_Struct(self, tr1::shared_ptr<const PackageID>, self_ptr);
        return version_spec_to_value((*self_ptr)->version());
    }

    /*
     * call-seq:
     *     repoistory -> Repository
     *
     * Our Repository.
     */
    VALUE
    package_id_repository(VALUE self)
    {
        tr1::shared_ptr<const PackageID> * self_ptr;
        Data_Get_Struct(self, tr1::shared_ptr<const PackageID>, self_ptr);
        return repository_to_value((*self_ptr)->repository());
    }

    /*
     * Document-method: keywords_key
     *
     * call-seq:
     *     keywords_key -> MetadataCollectionKey
     *
     * Our keywords
     */
    /*
     * Document-method: use_key
     *
     * call-seq:
     *     use_key -> MetadataCollectionKey
     *
     * Our use flags
     */
    /*
     * Document-method: iuse_key
     *
     * call-seq:
     *     iuse_key -> MetadataCollectionKey
     *
     * Our iuse flags
     */
    /*
     * Document-method: inherited_key
     *
     * call-seq:
     *     inherited_key -> MetadataCollectionKey
     *
     * Our inherited
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
     * Document-method: contents_key
     *
     * call-seq:
     *     contents_key -> MetadataContentsKey
     *
     * Our Contents
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
     * Document-method: source_origin_key
     *
     * call-seq:
     *     source_origin_key -> MetadataStringKey
     *
     * Our source origin repository
     */
    /*
     * Document-method: binary_origin_key
     *
     * call-seq:
     *     binary_origin_key -> MetadataStringKey
     *
     * Our binary origin repository
     */
    template <typename T_, const tr1::shared_ptr<const T_> (PackageID::* m_) () const>
    struct KeyValue
    {
        static VALUE
        fetch(VALUE self)
        {
            tr1::shared_ptr<const PackageID> * self_ptr;
            Data_Get_Struct(self, tr1::shared_ptr<const PackageID>, self_ptr);
            tr1::shared_ptr<const MetadataKey> ptr = (((**self_ptr).*m_)());

            if (ptr)
            {
                return metadata_key_to_value(((**self_ptr).*m_)());
            }
            return Qnil;
        }
    };

    VALUE
    package_id_equal(VALUE self, VALUE other)
    {
        tr1::shared_ptr<const PackageID> * self_ptr;
        Data_Get_Struct(self, tr1::shared_ptr<const PackageID>, self_ptr);
        return (**self_ptr == *value_to_package_id(other));
    }

    /*
     * call-seq:
     *     type -> MetadataKeyType
     *
     * Our significance to a user.
     */
    VALUE
    metadata_key_type(VALUE self) {
        tr1::shared_ptr<const MetadataKey> * self_ptr;
        Data_Get_Struct(self, tr1::shared_ptr<const MetadataKey>, self_ptr);
        return INT2FIX((*self_ptr)->type());
    }

    /*
     * call-seq:
     *     value
     *
     * Our Value.
     * */
    VALUE
    metadata_package_id_key_value(VALUE self) {
        tr1::shared_ptr<const MetadataPackageIDKey> * self_ptr;
        Data_Get_Struct(self, tr1::shared_ptr<const MetadataPackageIDKey>, self_ptr);
        return package_id_to_value((*self_ptr)->value());
    }

    VALUE
    metadata_string_key_value(VALUE self) {
        tr1::shared_ptr<const MetadataStringKey> * self_ptr;
        Data_Get_Struct(self, tr1::shared_ptr<const MetadataStringKey>, self_ptr);
        return rb_str_new2((*self_ptr)->value().c_str());
    }

    VALUE
    metadata_time_key_value(VALUE self) {
        tr1::shared_ptr<const MetadataTimeKey> * self_ptr;
        Data_Get_Struct(self, tr1::shared_ptr<const MetadataTimeKey>, self_ptr);
        return rb_time_new((*self_ptr)->value(),0);
    }

    VALUE
    metadata_contents_key_value(VALUE self) {
        tr1::shared_ptr<const MetadataContentsKey> * self_ptr;
        Data_Get_Struct(self, tr1::shared_ptr<const MetadataContentsKey>, self_ptr);
        if ((*self_ptr)->value())
            return contents_to_value((*self_ptr)->value());
        return Qnil;
    }

    template <typename T_>
    struct CollectionValue
    {
        static VALUE
        fetch(VALUE self)
        {
            tr1::shared_ptr<const MetadataCollectionKey<T_> > * self_ptr;
            Data_Get_Struct(self, tr1::shared_ptr<const MetadataCollectionKey<T_> >, self_ptr);
            tr1::shared_ptr<const T_> c = (*self_ptr)->value();
            VALUE result (rb_ary_new());
            for (typename T_::Iterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                    rb_ary_push(result, rb_str_new2(stringify(*i).c_str()));
            return result;
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
        rb_define_method(c_package_id, "name", RUBY_FUNC_CAST((&BaseValue<QualifiedPackageName,PackageID,&PackageID::name>::fetch)), 0);
        rb_define_method(c_package_id, "version", RUBY_FUNC_CAST(&package_id_version), 0);
        rb_define_method(c_package_id, "slot", RUBY_FUNC_CAST((&BaseValue<SlotName,PackageID,&PackageID::slot>::fetch)), 0);
        rb_define_method(c_package_id, "repository", RUBY_FUNC_CAST(&package_id_repository), 0);
        rb_define_method(c_package_id, "eapi", RUBY_FUNC_CAST(&package_id_eapi), 0);
        rb_define_method(c_package_id, "==", RUBY_FUNC_CAST(&package_id_equal), 1);
        rb_define_method(c_package_id, "keywords_key", RUBY_FUNC_CAST((&KeyValue<MetadataCollectionKey<KeywordNameCollection>,&PackageID::keywords_key>::fetch)), 0);
        rb_define_method(c_package_id, "use_key", RUBY_FUNC_CAST((&KeyValue<MetadataCollectionKey<UseFlagNameCollection>,&PackageID::use_key>::fetch)), 0);
        rb_define_method(c_package_id, "iuse_key", RUBY_FUNC_CAST((&KeyValue<MetadataCollectionKey<IUseFlagCollection>,&PackageID::iuse_key>::fetch)), 0);
        rb_define_method(c_package_id, "inherited_key", RUBY_FUNC_CAST((&KeyValue<MetadataCollectionKey<InheritedCollection>,&PackageID::inherited_key>::fetch)), 0);
        rb_define_method(c_package_id, "short_description_key", RUBY_FUNC_CAST((&KeyValue<MetadataStringKey,&PackageID::short_description_key>::fetch)), 0);
        rb_define_method(c_package_id, "long_description_key", RUBY_FUNC_CAST((&KeyValue<MetadataStringKey,&PackageID::long_description_key>::fetch)), 0);
        rb_define_method(c_package_id, "contents_key", RUBY_FUNC_CAST((&KeyValue<MetadataContentsKey,&PackageID::contents_key>::fetch)), 0);
        rb_define_method(c_package_id, "installed_time_key", RUBY_FUNC_CAST((&KeyValue<MetadataTimeKey,&PackageID::installed_time_key>::fetch)), 0);
        rb_define_method(c_package_id, "source_origin_key", RUBY_FUNC_CAST((&KeyValue<MetadataStringKey,&PackageID::source_origin_key>::fetch)), 0);
        rb_define_method(c_package_id, "binary_origin_key", RUBY_FUNC_CAST((&KeyValue<MetadataStringKey,&PackageID::binary_origin_key>::fetch)), 0);

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

        /*
         * Document-class: Paludis::MetadataKey
         *
         * Base metadata class
         */
        c_metadata_key = rb_define_class_under(paludis_module(), "MetadataKey", rb_cObject);
        rb_funcall(c_metadata_key, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_metadata_key, "raw_name", RUBY_FUNC_CAST((&BaseValue<std::string,MetadataKey,&MetadataKey::raw_name>::fetch)), 0);
        rb_define_method(c_metadata_key, "human_name", RUBY_FUNC_CAST((&BaseValue<std::string,MetadataKey,&MetadataKey::human_name>::fetch)), 0);
        rb_define_method(c_metadata_key, "type", RUBY_FUNC_CAST(&metadata_key_type), 0);

        /*
         * Document-class: Paludis::MetadataKey
         *
         * Base metadata class
         */
        c_metadata_package_id_key = rb_define_class_under(paludis_module(), "MetadataPackageIDKey", c_metadata_key);
        rb_define_method(c_metadata_package_id_key, "value", RUBY_FUNC_CAST(&metadata_package_id_key_value), 0);

        /*
         * Document-class: Paludis::MetadataStringKey
         *
         * Metadata class for Strings
         */
        c_metadata_string_key = rb_define_class_under(paludis_module(), "MetadataStringKey", c_metadata_key);
        rb_define_method(c_metadata_string_key, "value", RUBY_FUNC_CAST(&metadata_string_key_value), 0);

        /*
         * Document-class: Paludis::MetadataTimeKey
         *
         * Metadata class for Time
         */
        c_metadata_time_key = rb_define_class_under(paludis_module(), "MetadataTimeKey", c_metadata_key);
        rb_define_method(c_metadata_time_key, "value", RUBY_FUNC_CAST(&metadata_time_key_value), 0);

        /*
         * Document-class: Paludis::MetadataContentsKey
         *
         * Metadata class for Contents
         */
        c_metadata_contents_key = rb_define_class_under(paludis_module(), "MetadataContentsKey", c_metadata_key);
        rb_define_method(c_metadata_contents_key, "value", RUBY_FUNC_CAST(&metadata_contents_key_value), 0);

        /*
         * Document-class: Paludis::MetadataKeywordNameCollectionKey
         *
         * Metadata class for Use flag names
         */
        c_metadata_keyword_name_collection_key = rb_define_class_under(paludis_module(), "MetadataKeywordNameCollectionKey", c_metadata_key);
        rb_define_method(c_metadata_keyword_name_collection_key, "value", RUBY_FUNC_CAST((&CollectionValue<KeywordNameCollection>::fetch)), 0);

        /*
         * Document-class: Paludis::MetadataUseFlagNameCollectionKey
         *
         * Metadata class for Use flag names
         */
        c_metadata_use_flag_name_collection_key = rb_define_class_under(paludis_module(), "MetadataUseFlagNameCollectionKey", c_metadata_key);
        rb_define_method(c_metadata_use_flag_name_collection_key, "value", RUBY_FUNC_CAST((&CollectionValue<UseFlagNameCollection>::fetch)), 0);

        /*
         * Document-class: Paludis::MetadataIUseFlagCollectionKey
         *
         * Metadata class for IUse flags
         */
        c_metadata_iuse_flag_collection_key = rb_define_class_under(paludis_module(), "MetadataIUseFlagCollectionKey", c_metadata_key);
        rb_define_method(c_metadata_iuse_flag_collection_key, "value", RUBY_FUNC_CAST((&CollectionValue<IUseFlagCollection>::fetch)), 0);

        /*
         * Document-class: Paludis::MetadataInheritedCollectionKey
         *
         * Metadata class for Inherited
         */
        c_metadata_inherited_collection_key = rb_define_class_under(paludis_module(), "MetadataInheritedCollectionKey", c_metadata_key);
        rb_define_method(c_metadata_inherited_collection_key, "value", RUBY_FUNC_CAST((&CollectionValue<InheritedCollection>::fetch)), 0);

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
paludis::ruby::package_id_to_value(tr1::shared_ptr<const PackageID> m)
{
    tr1::shared_ptr<const PackageID> * m_ptr(0);
    try
    {
        m_ptr = new tr1::shared_ptr<const PackageID>(m);
        return Data_Wrap_Struct(c_package_id, 0, &Common<tr1::shared_ptr<const PackageID> >::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

tr1::shared_ptr<const PackageID>
paludis::ruby::value_to_package_id(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_package_id))
    {
        tr1::shared_ptr<const PackageID> * v_ptr;
        Data_Get_Struct(v, tr1::shared_ptr<const PackageID>, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into PackageID", rb_obj_classname(v));
    }
}

RegisterRubyClass::Register paludis_ruby_register_package_id PALUDIS_ATTRIBUTE((used))
    (&do_register_package_id);

