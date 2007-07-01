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
#include <paludis/package_id.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

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
     * Document-method: description
     *
     * call-seq:
     *     name -> String
     *
     * Our name
     */
    template <typename T_, const T_ (PackageID::* m_) () const>
    struct BaseValue
    {
        static VALUE
        fetch(VALUE self)
        {
            tr1::shared_ptr<const PackageID> * self_ptr;
            Data_Get_Struct(self, tr1::shared_ptr<const PackageID>, self_ptr);
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

    VALUE
    package_id_equal(VALUE self, VALUE other)
    {
        tr1::shared_ptr<const PackageID> * self_ptr;
        Data_Get_Struct(self, tr1::shared_ptr<const PackageID>, self_ptr);
        return (**self_ptr == *value_to_package_id(other));
    }

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
        rb_define_method(c_package_id, "name", RUBY_FUNC_CAST((&BaseValue<QualifiedPackageName,&PackageID::name>::fetch)), 0);
        rb_define_method(c_package_id, "version", RUBY_FUNC_CAST(&package_id_version), 0);
        rb_define_method(c_package_id, "slot", RUBY_FUNC_CAST((&BaseValue<SlotName,&PackageID::slot>::fetch)), 0);
        rb_define_method(c_package_id, "repository", RUBY_FUNC_CAST(&package_id_repository), 0);
        rb_define_method(c_package_id, "eapi", RUBY_FUNC_CAST(&package_id_eapi), 0);
        rb_define_method(c_package_id, "==", RUBY_FUNC_CAST(&package_id_equal), 1);

        /*
         * Document-module: Paludis::PackageIDCanonicalForm
         *
         * How to order query results.
         */
        c_package_id_canonical_form = rb_define_module_under(paludis_module(), "PackageIDCanonicalForm");
        for (PackageIDCanonicalForm l(static_cast<PackageIDCanonicalForm>(0)), l_end(last_idcf) ; l != l_end ;
                l = static_cast<PackageIDCanonicalForm>(static_cast<int>(l) + 1))
            rb_define_const(c_package_id_canonical_form, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/package_id-se.hh, PackageIDCanonicalForm, c_package_id_canonical_form>
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


