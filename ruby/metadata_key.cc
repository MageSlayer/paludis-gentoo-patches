/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Richard Brown
 * Copyright (c) 2007 Ciaran McCreesh
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
#include <paludis/util/visitor-impl.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/mask.hh>
#include <paludis/util/set.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_metadata_key;
    static VALUE c_metadata_package_id_key;
    static VALUE c_metadata_string_key;
    static VALUE c_metadata_time_key;
    static VALUE c_metadata_contents_key;
    static VALUE c_metadata_repository_mask_info_key;
    static VALUE c_metadata_keyword_name_set_key;
    static VALUE c_metadata_use_flag_name_set_key;
    static VALUE c_metadata_iuse_flag_set_key;
    static VALUE c_metadata_string_set_key;
    static VALUE c_metadata_package_id_sequence_key;
    static VALUE c_metadata_fsentry_sequence_key;
    static VALUE c_metadata_key_type;
    static VALUE c_metadata_provide_spec_tree_key;
    static VALUE c_metadata_license_spec_tree_key;
    static VALUE c_metadata_fetchable_uri_spec_tree_key;
    static VALUE c_metadata_simple_uri_spec_tree_key;
    static VALUE c_metadata_dependency_spec_tree_key;
    static VALUE c_metadata_restrict_spec_tree_key;
    static VALUE c_repository_mask_info;

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

        void visit(const MetadataFSEntryKey &)
        {
            value = Qnil;
        }

        void visit(const MetadataSectionKey &)
        {
            value = Qnil;
        }

        void visit(const MetadataRepositoryMaskInfoKey &)
        {
            value = Data_Wrap_Struct(c_metadata_repository_mask_info_key, 0, &Common<tr1::shared_ptr<const MetadataContentsKey> >::free,
                    new tr1::shared_ptr<const MetadataRepositoryMaskInfoKey>(tr1::static_pointer_cast<const MetadataRepositoryMaskInfoKey>(mm)));
        }

        void visit(const MetadataSetKey<KeywordNameSet> &)
        {
            value = Data_Wrap_Struct(c_metadata_keyword_name_set_key, 0, &Common<tr1::shared_ptr<const MetadataSetKey<KeywordNameSet> > >::free,
                    new tr1::shared_ptr<const MetadataSetKey<KeywordNameSet> >(tr1::static_pointer_cast<const MetadataSetKey<KeywordNameSet> >(mm)));
        }

        void visit(const MetadataSetKey<UseFlagNameSet> &)
        {
            value = Data_Wrap_Struct(c_metadata_use_flag_name_set_key, 0, &Common<tr1::shared_ptr<const MetadataSetKey<UseFlagNameSet> > >::free,
                    new tr1::shared_ptr<const MetadataSetKey<UseFlagNameSet> >(tr1::static_pointer_cast<const MetadataSetKey<UseFlagNameSet> >(mm)));
        }

        void visit(const MetadataSetKey<IUseFlagSet> &)
        {
            value = Data_Wrap_Struct(c_metadata_iuse_flag_set_key, 0, &Common<tr1::shared_ptr<const MetadataSetKey<IUseFlagSet> > >::free,
                    new tr1::shared_ptr<const MetadataSetKey<IUseFlagSet> >(tr1::static_pointer_cast<const MetadataSetKey<IUseFlagSet> >(mm)));
        }

        void visit(const MetadataSetKey<Set<std::string> > &)
        {
            value = Data_Wrap_Struct(c_metadata_string_set_key, 0, &Common<tr1::shared_ptr<const MetadataSetKey<Set<std::string> > > >::free,
                    new tr1::shared_ptr<const MetadataSetKey<Set<std::string> > >(
                        tr1::static_pointer_cast<const MetadataSetKey<Set<std::string> > >(mm)));
        }

        void visit(const MetadataSetKey<FSEntrySequence> &)
        {
            value = Data_Wrap_Struct(c_metadata_fsentry_sequence_key, 0, &Common<tr1::shared_ptr<const FSEntrySequence> >::free,
                    new tr1::shared_ptr<const MetadataSetKey<FSEntrySequence> >(
                        tr1::static_pointer_cast<const MetadataSetKey<FSEntrySequence> >(mm)));
        }

        void visit(const MetadataSetKey<PackageIDSequence> &)
        {
            value = Data_Wrap_Struct(c_metadata_package_id_sequence_key, 0, &Common<tr1::shared_ptr<const MetadataSetKey<PackageIDSequence> > >::free,
                    new tr1::shared_ptr<const MetadataSetKey<PackageIDSequence> >
                    (tr1::static_pointer_cast<const MetadataSetKey<PackageIDSequence> >(mm)));
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> &)
        {
            value = Data_Wrap_Struct(c_metadata_license_spec_tree_key, 0,
                    &Common<tr1::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> > >::free,
                    new tr1::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> >(
                        tr1::static_pointer_cast<const MetadataSpecTreeKey<LicenseSpecTree> >(mm)));
        }

        void visit(const MetadataSpecTreeKey<ProvideSpecTree> &)
        {
            value = Data_Wrap_Struct(c_metadata_provide_spec_tree_key, 0,
                    &Common<tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> > >::free,
                    new tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >(
                        tr1::static_pointer_cast<const MetadataSpecTreeKey<ProvideSpecTree> >(mm)));
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> &)
        {
            value = Data_Wrap_Struct(c_metadata_dependency_spec_tree_key, 0,
                    &Common<tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > >::free,
                    new tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >(
                        tr1::static_pointer_cast<const MetadataSpecTreeKey<DependencySpecTree> >(mm)));
        }

        void visit(const MetadataSpecTreeKey<RestrictSpecTree> &)
        {
            value = Data_Wrap_Struct(c_metadata_restrict_spec_tree_key, 0,
                    &Common<tr1::shared_ptr<const MetadataSpecTreeKey<RestrictSpecTree> > >::free,
                    new tr1::shared_ptr<const MetadataSpecTreeKey<RestrictSpecTree> >(
                        tr1::static_pointer_cast<const MetadataSpecTreeKey<RestrictSpecTree> >(mm)));
        }

        void visit(const MetadataSpecTreeKey<FetchableURISpecTree> &)
        {
            value = Data_Wrap_Struct(c_metadata_fetchable_uri_spec_tree_key, 0,
                    &Common<tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> > >::free,
                    new tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >(
                        tr1::static_pointer_cast<const MetadataSpecTreeKey<FetchableURISpecTree> >(mm)));
        }

        void visit(const MetadataSpecTreeKey<SimpleURISpecTree> &)
        {
            value = Data_Wrap_Struct(c_metadata_simple_uri_spec_tree_key, 0,
                    &Common<tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > >::free,
                    new tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >(
                        tr1::static_pointer_cast<const MetadataSpecTreeKey<SimpleURISpecTree> >(mm)));
        }
    };

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

    VALUE
    metadata_repository_mask_info_key_value(VALUE self) {
        tr1::shared_ptr<const MetadataRepositoryMaskInfoKey> * self_ptr;
        Data_Get_Struct(self, tr1::shared_ptr<const MetadataRepositoryMaskInfoKey>, self_ptr);
        if ((*self_ptr)->value())
            return repository_mask_info_to_value((*self_ptr)->value());
        return Qnil;
    }

    template <typename T_>
    struct SetValue
    {
        static VALUE
        fetch(VALUE self)
        {
            tr1::shared_ptr<const MetadataSetKey<T_> > * self_ptr;
            Data_Get_Struct(self, tr1::shared_ptr<const MetadataSetKey<T_> >, self_ptr);
            tr1::shared_ptr<const T_> c = (*self_ptr)->value();
            VALUE result (rb_ary_new());
            for (typename T_::ConstIterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                    rb_ary_push(result, rb_str_new2(stringify(*i).c_str()));
            return result;
        }
    };

    template <>
    struct SetValue<PackageIDSequence>
    {
        static VALUE
        fetch(VALUE self)
        {
            tr1::shared_ptr<const MetadataSetKey<PackageIDSequence> > * self_ptr;
            Data_Get_Struct(self, tr1::shared_ptr<const MetadataSetKey<PackageIDSequence> >, self_ptr);
            tr1::shared_ptr<const PackageIDSequence> c = (*self_ptr)->value();
            VALUE result (rb_ary_new());
            for (PackageIDSequence::ConstIterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                rb_ary_push(result, package_id_to_value(*i));
            return result;
        }
    };

    VALUE
    repository_mask_info_mask_file(VALUE self)
    {
        tr1::shared_ptr<const RepositoryMaskInfo> * ptr;
        Data_Get_Struct(self, tr1::shared_ptr<const RepositoryMaskInfo>, ptr);
        return rb_str_new2(stringify((*ptr)->mask_file).c_str());
    }

    VALUE
    repository_mask_info_comment(VALUE self)
    {
        tr1::shared_ptr<const RepositoryMaskInfo> * ptr;
        Data_Get_Struct(self, tr1::shared_ptr<const RepositoryMaskInfo>, ptr);
        VALUE result(rb_ary_new());
        for (Sequence<std::string>::ConstIterator it((*ptr)->comment->begin()),
                 it_end((*ptr)->comment->end()); it_end != it; ++it)
            rb_ary_push(result, rb_str_new2(it->c_str()));
        return result;
    }

    void do_register_metadata_key()
    {
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
         * Document-class: Paludis::MetadataRepositoryMaskInfoKey
         *
         * Metadata class for RepositoryMaskInfo
         */
        c_metadata_repository_mask_info_key = rb_define_class_under(paludis_module(), "MetadataRepositoryMaskInfoKey", c_metadata_key);
        rb_define_method(c_metadata_repository_mask_info_key, "value", RUBY_FUNC_CAST(&metadata_repository_mask_info_key_value), 0);

        /*
         * Document-class: Paludis::MetadataKeywordNameSetKey
         *
         * Metadata class for Use flag names
         */
        c_metadata_keyword_name_set_key = rb_define_class_under(paludis_module(), "MetadataKeywordNameSetKey", c_metadata_key);
        rb_define_method(c_metadata_keyword_name_set_key, "value", RUBY_FUNC_CAST((&SetValue<KeywordNameSet>::fetch)), 0);

        /*
         * Document-class: Paludis::MetadataUseFlagNameSetKey
         *
         * Metadata class for Use flag names
         */
        c_metadata_use_flag_name_set_key = rb_define_class_under(paludis_module(), "MetadataUseFlagNameSetKey", c_metadata_key);
        rb_define_method(c_metadata_use_flag_name_set_key, "value", RUBY_FUNC_CAST((&SetValue<UseFlagNameSet>::fetch)), 0);

        /*
         * Document-class: Paludis::MetadataPackageIDSequenceKey
         *
         * Metadata class for package IDs
         */
        c_metadata_package_id_sequence_key = rb_define_class_under(paludis_module(), "MetadataPackageIDSequenceKey", c_metadata_key);
        rb_define_method(c_metadata_package_id_sequence_key, "value", RUBY_FUNC_CAST((&SetValue<PackageIDSequence>::fetch)), 0);

        /*
         * Document-class: Paludis::MetadataPackageIDSequenceKey
         *
         * Metadata class for filesystem sequences
         */
        c_metadata_fsentry_sequence_key = rb_define_class_under(paludis_module(), "MetadataFSEntrySequenceKey", c_metadata_key);
        rb_define_method(c_metadata_fsentry_sequence_key, "value", RUBY_FUNC_CAST((&SetValue<FSEntrySequence>::fetch)), 0);

        /*
         * Document-class: Paludis::MetadataIUseFlagSetKey
         *
         * Metadata class for IUse flags
         */
        c_metadata_iuse_flag_set_key = rb_define_class_under(paludis_module(), "MetadataIUseFlagSetKey", c_metadata_key);
        rb_define_method(c_metadata_iuse_flag_set_key, "value", RUBY_FUNC_CAST((&SetValue<IUseFlagSet>::fetch)), 0);

        /*
         * Document-class: Paludis::MetadataStringSetKey
         *
         * Metadata class for String sets
         */
        c_metadata_string_set_key = rb_define_class_under(paludis_module(), "MetadataStringSetKey", c_metadata_key);
        rb_define_method(c_metadata_string_set_key, "value", RUBY_FUNC_CAST((&SetValue<Set<std::string> >::fetch)), 0);

        /*
         * Document-class: Paludis::MetadataLicenseSpecTreeKey
         *
         * Metadata class for license specs
         */
        c_metadata_license_spec_tree_key = rb_define_class_under(paludis_module(), "MetadataLicenseSpecTreeKey", c_metadata_key);

        /*
         * Document-class: Paludis::MetadataProvideSpecTreeKey
         *
         * Metadata class for provide specs
         */
        c_metadata_provide_spec_tree_key = rb_define_class_under(paludis_module(), "MetadataProvideSpecTreeKey", c_metadata_key);

        /*
         * Document-class: Paludis::MetadataRestrictSpecTreeKey
         *
         * Metadata class for restrict specs
         */
        c_metadata_restrict_spec_tree_key = rb_define_class_under(paludis_module(), "MetadataRestrictSpecTreeKey", c_metadata_key);

        /*
         * Document-class: Paludis::MetadataDependencySpecTreeKey
         *
         * Metadata class for dependency specs
         */
        c_metadata_dependency_spec_tree_key = rb_define_class_under(paludis_module(), "MetadataDependencySpecTreeKey", c_metadata_key);

        /*
         * Document-class: Paludis::MetadataFetchableURISpecTreeKey
         *
         * Metadata class for fetchable uri specs
         */
        c_metadata_fetchable_uri_spec_tree_key = rb_define_class_under(paludis_module(), "MetadataFetchableURISpecTreeKey", c_metadata_key);

        /*
         * Document-class: Paludis::MetadataSimpleURISpecTreeKey
         *
         * Metadata class for simple uri specs
         */
        c_metadata_simple_uri_spec_tree_key = rb_define_class_under(paludis_module(), "MetadataSimpleURISpecTreeKey", c_metadata_key);

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

        /*
         * Document-class: Paludis::RepositoryMaskInfo
         *
         * Information about a RepositoryMask.
         */
        c_repository_mask_info = rb_define_class_under(paludis_module(), "RepositoryMaskInfo", rb_cObject);
        rb_funcall(c_repository_mask_info, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_repository_mask_info, "mask_file", RUBY_FUNC_CAST(&repository_mask_info_mask_file), 0);
        rb_define_method(c_repository_mask_info, "comment", RUBY_FUNC_CAST(&repository_mask_info_comment), 0);
    }
}

VALUE
paludis::ruby::repository_mask_info_to_value(tr1::shared_ptr<const RepositoryMaskInfo> m)
{
    tr1::shared_ptr<const RepositoryMaskInfo> * m_ptr(0);
    try
    {
        m_ptr = new tr1::shared_ptr<const RepositoryMaskInfo>(m);
        return Data_Wrap_Struct(c_repository_mask_info, 0, &Common<tr1::shared_ptr<const RepositoryMaskInfo> >::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

VALUE
paludis::ruby::metadata_key_to_value(tr1::shared_ptr<const MetadataKey> m)
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


