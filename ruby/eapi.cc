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
#include <paludis/eapi.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_eapi;
    static VALUE c_supported_eapi;
    static VALUE c_eapi_data;

    VALUE
    supported_eapi_to_value(tr1::shared_ptr<const SupportedEAPI> m)
    {
        tr1::shared_ptr<const SupportedEAPI> * m_ptr = new tr1::shared_ptr<const SupportedEAPI>(m);
        return Data_Wrap_Struct(c_supported_eapi, 0, &Common<tr1::shared_ptr<const SupportedEAPI> >::free, m_ptr);
    }

    VALUE
    eapi_name(VALUE self)
    {
        EAPI * p;
        Data_Get_Struct(self, EAPI, p);
        return rb_str_new2((p->name).c_str());
    }

    VALUE
    eapi_supported(VALUE self)
    {
        EAPI * p;
        Data_Get_Struct(self, EAPI, p);
        if (p->supported)
            return supported_eapi_to_value((p->supported));
        return Qnil;
    }
    /*
     * Document-method: package_dep_spec_parse_mode
     *
     * call-seq:
     *     package_dep_spec_parse_mode -> PackageDepSpecParseMode
     *
     * Our PackageDepSpecParseMode.
     */
    /*
     * Document-method: strict_package_dep_spec_parse_mode
     *
     * call-seq:
     *     strict_package_dep_spec_parse_mode -> PackageDepSpecParseMode
     *
     * Our strict PackageDepSpecParseMode.
     */
    /*
     * Document-method: iuse_flag_parse_mode
     *
     * call-seq:
     *     iuse_flag_parse_mode -> IUseFlagParseMode
     *
     * Our IUseFlagParseMode.
     */
    /*
     * Document-method: strict_iuse_flag_parse_mode
     *
     * call-seq:
     *     strict_iuse_flag_parse_mode -> IUseFlagParseMode
     *
     * Our strict IUseFlagParseMode.
     */
    template <typename T_, T_ SupportedEAPI::* m_>
    struct ParseModeMember
    {
        static VALUE
        fetch(VALUE self)
        {
            tr1::shared_ptr<SupportedEAPI> * p;
            Data_Get_Struct(self, tr1::shared_ptr<SupportedEAPI>, p);
            return INT2FIX((**p).*m_);
        }
    };

    /*
     * Document-method: breaks_portage?
     *
     * call_seq:
     *     breaks_portage? -> true or false
     *
     * Does this EAPI break portage?
     */
    /*
     * Document-method: has_pretend_phase?
     *
     * call_seq:
     *     has_pretend_phase? -> true or false
     *
     * Does this EAPI have a pretend phase?
     */
    template <bool SupportedEAPI::* m_>
    struct BoolMember
    {
        static VALUE
        fetch(VALUE self)
        {
            tr1::shared_ptr<SupportedEAPI> * p;
            Data_Get_Struct(self, tr1::shared_ptr<SupportedEAPI>, p);
            return ((**p).*m_) ? Qtrue : Qfalse;
        }
    };

    /*
     * call-seq:
     *     EAPIData.eapi_from_string(eapi_string) -> EAPI
     *
     * Make an EAPI.
     */
    VALUE
    eapi_data_eapi_from_string(VALUE, VALUE eapi)
    {
        return eapi_to_value(EAPIData::get_instance()->eapi_from_string(stringify(StringValuePtr(eapi))));
    }

    /*
     * call-seq:
     *     EAPIData.unknown_eapi -> EAPI
     *
     * Make the unknown EAPI.
     */
    VALUE
    eapi_data_unknown_eapi()
    {
        return eapi_to_value(EAPIData::get_instance()->unknown_eapi());
    }

    void do_register_eapi()
    {
        /*
         * Document-class: Paludis::EAPI
         *
         * Information about an EAPI.
         */
        c_eapi = rb_define_class_under(paludis_module(), "EAPI", rb_cObject);
        rb_funcall(c_eapi, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_eapi, "name", RUBY_FUNC_CAST(&eapi_name), 0);
        rb_define_method(c_eapi, "supported", RUBY_FUNC_CAST(&eapi_supported), 0);

        c_supported_eapi = rb_define_class_under(paludis_module(), "SupportedEAPI", rb_cObject);
        rb_funcall(c_supported_eapi, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_supported_eapi, "package_dep_spec_parse_mode",
                RUBY_FUNC_CAST((&ParseModeMember<PackageDepSpecParseMode, &SupportedEAPI::package_dep_spec_parse_mode>::fetch)), 0);
        rb_define_method(c_supported_eapi, "strict_package_dep_spec_parse_mode",
                RUBY_FUNC_CAST((&ParseModeMember<PackageDepSpecParseMode, &SupportedEAPI::strict_package_dep_spec_parse_mode>::fetch)), 0);
        rb_define_method(c_supported_eapi, "iuse_flag_parse_mode",
                RUBY_FUNC_CAST((&ParseModeMember<IUseFlagParseMode, &SupportedEAPI::iuse_flag_parse_mode>::fetch)), 0);
        rb_define_method(c_supported_eapi, "strict_iuse_flag_parse_mode",
                RUBY_FUNC_CAST((&ParseModeMember<IUseFlagParseMode, &SupportedEAPI::strict_iuse_flag_parse_mode>::fetch)), 0);
        rb_define_method(c_supported_eapi, "breaks_portage?",
                RUBY_FUNC_CAST((&BoolMember<&SupportedEAPI::breaks_portage>::fetch)), 0);
        rb_define_method(c_supported_eapi, "has_pretend_phase?",
                RUBY_FUNC_CAST((&BoolMember<&SupportedEAPI::has_pretend_phase>::fetch)), 0);

        rb_require("singleton");

        c_eapi_data = rb_define_class_under(paludis_module(), "EAPIData", rb_cObject);
        rb_funcall(rb_const_get(rb_cObject, rb_intern("Singleton")), rb_intern("included"), 1, c_eapi_data);
        rb_define_method(c_eapi_data, "eapi_from_string", RUBY_FUNC_CAST(&eapi_data_eapi_from_string), 1);
        rb_define_method(c_eapi_data, "unknown_eapi", RUBY_FUNC_CAST(&eapi_data_unknown_eapi), 0);
    }
}

VALUE
paludis::ruby::eapi_to_value(const EAPI & v)
{
    EAPI  * m_ptr = new EAPI(v);
    return Data_Wrap_Struct(c_eapi, 0, &Common<EAPI>::free, m_ptr);
}

RegisterRubyClass::Register paludis_ruby_register_eapi PALUDIS_ATTRIBUTE((used))
    (&do_register_eapi);

