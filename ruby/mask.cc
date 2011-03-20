/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Richard Brown
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
#include <paludis/mask.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

namespace
{
    static VALUE c_mask;
    static VALUE c_user_mask;
    static VALUE c_unaccepted_mask;
    static VALUE c_repository_mask;
    static VALUE c_unsupported_mask;
    static VALUE c_association_mask;
    static VALUE c_overridden_mask;
    static VALUE c_mask_override_reason;

    struct V
    {
        VALUE value;
        std::shared_ptr<const Mask> mm;

        V(std::shared_ptr<const Mask> _m) :
            mm(_m)
        {
        }

        void visit(const UserMask &)
        {
            value = Data_Wrap_Struct(c_user_mask, 0, &Common<std::shared_ptr<const Mask> >::free,
                    new std::shared_ptr<const Mask>(mm));
        }

        void visit(const UnacceptedMask &)
        {
            value = Data_Wrap_Struct(c_unaccepted_mask, 0, &Common<std::shared_ptr<const Mask> >::free,
                    new std::shared_ptr<const Mask>(mm));
        }

        void visit(const UnsupportedMask &)
        {
            value = Data_Wrap_Struct(c_unsupported_mask, 0, &Common<std::shared_ptr<const Mask> >::free,
                    new std::shared_ptr<const Mask>(mm));
        }

        void visit(const RepositoryMask &)
        {
            value = Data_Wrap_Struct(c_repository_mask, 0, &Common<std::shared_ptr<const Mask> >::free,
                    new std::shared_ptr<const Mask>(mm));
        }

        void visit(const AssociationMask &)
        {
            value = Data_Wrap_Struct(c_association_mask, 0, &Common<std::shared_ptr<const Mask> >::free,
                    new std::shared_ptr<const Mask>(mm));
        }
    };

    VALUE
    mask_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    /*
     * call-seq:
     *     key -> String
     *
     * A single character key, which can be used by clients if they need a very compact way of representing a mask.
     */
    VALUE
    mask_key(VALUE self)
    {
        std::shared_ptr<const Mask> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const Mask>, ptr);
        char c = (*ptr)->key();
        const char* c_ptr = &c;
        return rb_str_new2(c_ptr);
    }

    /*
     * Document-method: description
     *
     * call-seq:
     *     description -> String
     *
     * A description of the mask.
     */
    /*
     * Document-method: explanation
     *
     * call-seq:
     *     explanation -> String
     *
     * An explanation of why we are unsupported.
     */
    template <typename T_, const std::string (T_::* m_) () const>
    struct MaskStringKey
    {
        static VALUE
        fetch(VALUE self)
        {
            std::shared_ptr<const Mask> * ptr;
            Data_Get_Struct(self, std::shared_ptr<const Mask>, ptr);
            std::shared_ptr<const T_> cast_ptr(std::static_pointer_cast<const T_>(*ptr));
            return rb_str_new2(((*cast_ptr).*m_)().c_str());
        }
    };

    /*
     * Document-method: unaccepted_key
     *
     * call-seq:
     *     unaccepted_key -> String
     *
     * Fetch the name of the metadata key that is not accepted.
     */
    /*
     * Document-method: mask_key
     *
     * call-seq:
     *     mask_key -> String
     *
     * Fetch the name of a metadata key explaining the mask.
     */
    template <typename T_, const std::string (T_::* m_) () const>
    struct MaskMetadataKey
    {
        static VALUE
        fetch(VALUE self)
        {
            std::shared_ptr<const Mask> * ptr;
            Data_Get_Struct(self, std::shared_ptr<const Mask>, ptr);
            std::shared_ptr<const T_> cast_ptr(std::static_pointer_cast<const T_>(*ptr));
            return rb_str_new2(((*cast_ptr).*m_)().c_str());
        }
    };

    /*
     * call-seq:
     *     associated_package_spec -> PackageDepSpec
     *
     * Fetch the associated package.
     */
    VALUE
    association_mask_associated_package(VALUE self)
    {
        std::shared_ptr<const Mask> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const Mask>, ptr);
        std::shared_ptr<const AssociationMask> cast_ptr(std::static_pointer_cast<const AssociationMask>(*ptr));
        return package_dep_spec_to_value((cast_ptr)->associated_package_spec());
    }

    /*
     * call-seq:
     *     mask -> Mask
     */
    VALUE
    overridden_mask_mask(VALUE self)
    {
        std::shared_ptr<const OverriddenMask> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const OverriddenMask>, ptr);
        return mask_to_value((*ptr)->mask());
    }

    /*
     * call-seq:
     *     override_reason -> MaskOverrideReason
     */
    VALUE
    overridden_mask_override_reason(VALUE self)
    {
        std::shared_ptr<const OverriddenMask> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const OverriddenMask>, ptr);
        return INT2FIX((*ptr)->override_reason());
    }


    void do_register_mask()
    {
        /*
         * Document-class: Paludis::Mask
         *
         * A mask represents one reason why a PackageID is masked (not installable).
         * A basic Mask has:
         *
         * * A single character key, which can be used by clients if they need a very compact way of representing a mask.
         *
         * * A description.
         *
         * Subclasses provide additional information.
         */
        c_mask = rb_define_class_under(paludis_module(), "Mask", rb_cObject);
        rb_funcall(c_mask, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_mask, "key", RUBY_FUNC_CAST(&mask_key), 0);
        rb_define_method(c_mask, "description",
                RUBY_FUNC_CAST((&MaskStringKey<Mask,&Mask::description>::fetch)), 0);
        rb_define_method(c_mask, "initialize", RUBY_FUNC_CAST(&mask_init), -1);

        /*
         * Document-class: Paludis::UserMask
         *
         * A UserMask is a Mask due to user configuration.
         */
        c_user_mask = rb_define_class_under(paludis_module(), "UserMask", c_mask);

        /*
         * Document-class: Paludis::UnacceptedMask
         *
         * An UnacceptedMask is a Mask that signifies that a particular value or combination of values in
         * (for example) a MetadataCollectionKey or MetadataSpecTreeKey is not accepted by user configuration.
         */
        c_unaccepted_mask = rb_define_class_under(paludis_module(), "UnacceptedMask", c_mask);
        rb_define_method(c_unaccepted_mask, "unaccepted_key_name",
                RUBY_FUNC_CAST((&MaskMetadataKey<UnacceptedMask,&UnacceptedMask::unaccepted_key_name>::fetch)), 0);

        /*
         * Document-class: Paludis::RepositoryMask
         *
         * A RepositoryMask is a Mask that signifies that a PackageID has been marked as masked by a Repository.
         */
        c_repository_mask = rb_define_class_under(paludis_module(), "RepositoryMask", c_mask);

        /*
         * Document-class: Paludis::UnsupportedMask
         *
         *  An UnsupportedMask is a Mask that signifies that a PackageID is not supported, for example because
         *  it is broken or because it uses an unrecognised EAPI.
         */
        c_unsupported_mask = rb_define_class_under(paludis_module(), "UnsupportedMask", c_mask);
        rb_define_method(c_unsupported_mask, "explanation",
                RUBY_FUNC_CAST((&MaskStringKey<UnsupportedMask,&UnsupportedMask::explanation>::fetch)), 0);

        /*
         * Document-class: Paludis::AssociationMask
         *
         * An AssociationMask is a Mask that signifies that a PackageID is masked because of its association with
         * another PackageID that is itself masked.
         *
         * This is used by old-style virtuals. If the provider of a virtual is masked then the virtual itself is
         * masked by association.
         */
        c_association_mask = rb_define_class_under(paludis_module(), "AssociationMask", c_mask);
        rb_define_method(c_association_mask, "associated_package_spec", RUBY_FUNC_CAST(&association_mask_associated_package), 0);

        /*
         * Document-class: Paludis::OverriddenMask
         *
         * An OverriddenMask holds a Mask and an explanation of why it has been overridden.
         */
        c_overridden_mask = rb_define_class_under(paludis_module(), "OverriddenMask", rb_cObject);
        rb_define_method(c_overridden_mask, "mask", RUBY_FUNC_CAST(&overridden_mask_mask), 0);
        rb_define_method(c_overridden_mask, "override_reason", RUBY_FUNC_CAST(&overridden_mask_override_reason), 0);

        /*
         * Document-module: Paludis::MaskOverrideReason
         *
         * The reason an OverriddenMask is overridden.
         */
        c_mask_override_reason = rb_define_module_under(paludis_module(), "MaskOverrideReason");
        for (MaskOverrideReason r(static_cast<MaskOverrideReason>(0)), r_end(last_mro) ; r != r_end ;
                r = static_cast<MaskOverrideReason>(static_cast<int>(r) + 1))
            rb_define_const(c_mask_override_reason, value_case_to_RubyCase(stringify(r)).c_str(), INT2FIX(r));

        // cc_enum_special<paludis/mask.hh, MaskOverrideReason, c_mask_override_reason>
    }
}

VALUE
paludis::ruby::mask_to_value(std::shared_ptr<const Mask> m)
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

VALUE
paludis::ruby::overridden_mask_to_value(std::shared_ptr<const OverriddenMask> m)
{
    return Data_Wrap_Struct(c_overridden_mask, 0, &Common<std::shared_ptr<const OverriddenMask> >::free,
            new std::shared_ptr<const OverriddenMask>(m));
}

RegisterRubyClass::Register paludis_ruby_register_mask PALUDIS_ATTRIBUTE((used))
    (&do_register_mask);

