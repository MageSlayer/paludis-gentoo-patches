/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 * Copyright (c) 2007 Alexander H. Færøy <eroyf@gentoo.org>
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
#include <paludis/version_metadata.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/util/stringify.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_version_metadata;

    template <typename T_>
    VALUE version_metadata_get_interface(VALUE self, T_ * (VersionMetadataCapabilities::* m))
    {
        std::tr1::shared_ptr<const VersionMetadata> * self_ptr;
        Data_Get_Struct(self, std::tr1::shared_ptr<const VersionMetadata>, self_ptr);
        return ((**self_ptr).*m) ? self : Qnil;
    }

    /*
     * call-seq:
     *     ebuild_interface -> self or Nil
     *
     * Returns self if the VersionMetadata supports the interface, otherwise Nil.
     */
    VALUE version_metadata_ebuild_interface(VALUE self)
    {
        return version_metadata_get_interface(self, &VersionMetadata::ebuild_interface);
    }

    /*
     * call-seq:
     *     cran_interface -> self or Nil
     *
     * Returns self if the VersionMetadata supports the interface, otherwise Nil.
     */
    VALUE version_metadata_cran_interface(VALUE self)
    {
        return version_metadata_get_interface(self, &VersionMetadata::cran_interface);
    }

    /*
     * call-seq:
     *     virtual_interface -> self or Nil
     *
     * Returns self if the VersionMetadata supports the interface, otherwise Nil.
     */
    VALUE version_metadata_virtual_interface(VALUE self)
    {
        return version_metadata_get_interface(self, &VersionMetadata::virtual_interface);
    }

    /*
     * call-seq:
     *     deps_interface -> self or Nil
     *
     * Returns self if the VersionMetadata supports the interface, otherwise Nil.
     */
    VALUE version_metadata_deps_interface(VALUE self)
    {
        return version_metadata_get_interface(self, &VersionMetadata::deps_interface);
    }

    /*
     * call-seq:
     *     license_interface -> self or Nil
     *
     * Returns self if the VersionMetadata supports the interface, otherwise Nil.
     */
    VALUE version_metadata_license_interface(VALUE self)
    {
        return version_metadata_get_interface(self, &VersionMetadata::license_interface);
    }

    /*
     * call-seq:
     *     origins_interface -> self or Nil
     *
     * Returns self if the VersionMetadata supports the interface, otherwise Nil.
     */
    VALUE version_metadata_origins_interface(VALUE self)
    {
        return version_metadata_get_interface(self, &VersionMetadata::origins_interface);
    }

    /*
     * call-seq:
     *     license
     *
     * Fetch our license, as a DepSpec structure, or Nil if we don't support
     * license_interface.
     */
    VALUE
    version_metadata_license(VALUE self)
    {
        std::tr1::shared_ptr<const VersionMetadata> * self_ptr;
        Data_Get_Struct(self, std::tr1::shared_ptr<const VersionMetadata>, self_ptr);
        if ((*self_ptr)->license_interface)
            return dep_spec_to_value((*self_ptr)->license_interface->license());
        else
            return Qnil;
    }

#ifdef CIARANM_REMOVED_THIS
    /*
     * call-seq:
     *     license_string
     *
     * Fetch our license, as a string, or Nil if we don't support license_interface.
     */
    VALUE
    version_metadata_license_string(VALUE self)
    {
        std::tr1::shared_ptr<const VersionMetadata> * self_ptr;
        Data_Get_Struct(self, std::tr1::shared_ptr<const VersionMetadata>, self_ptr);
        if ((*self_ptr)->license_interface)
            return rb_str_new2((*self_ptr)->license_interface->license_string.c_str());
        else
            return Qnil;
    }
#endif

    /*
     * Document-method: slot
     *
     * call-seq:
     *     slot -> String
     *
     * Our slot
     */
    /*
     * Document-method: eapi
     *
     * call-seq:
     *     eapi -> String
     *
     * Our eapi
     */
#ifdef CIARANM_REMOVED_THIS
    /*
     * Document-method: homepage
     *
     * call-seq:
     *     homepage -> String
     *
     * Our homepage
     */
#endif
    /*
     * Document-method: description
     *
     * call-seq:
     *     description -> String
     *
     * Our description
     */
    template <typename T_, T_ VersionMetadataBase::* m_>
    struct BaseValue
    {
        static VALUE
        fetch(VALUE self)
        {
            std::tr1::shared_ptr<const VersionMetadata> * self_ptr;
            Data_Get_Struct(self, std::tr1::shared_ptr<const VersionMetadata>, self_ptr);
            return rb_str_new2(stringify((**self_ptr).*m_).c_str());
        }
    };

    /*
     * call-seq:
     *     interactive? -> true or false
     *
     * Are we interactive.
     */
    VALUE
    version_metadata_interactive(VALUE self)
    {
        std::tr1::shared_ptr<const VersionMetadata> * self_ptr;
        Data_Get_Struct(self, std::tr1::shared_ptr<const VersionMetadata>, self_ptr);
        return (*self_ptr)-> interactive ? Qtrue : Qfalse;
    }

#ifdef CIARANM_REMOVED_THIS
    /*
     * Document-method: provide_string
     *
     * call-seq:
     *     provide_string -> String
     *
     * Fetches the package provide_string, if ebuild_interface is not Nil.
     */
    /*
     * Document-method: src_uri_string
     *
     * call-seq:
     *     src_uri_string -> String
     *
     * Fetches the package src_uri, if ebuild_interface is not Nil.
     */
    /*
     * Document-method: restrict_string
     *
     * call-seq:
     *     restrict_string -> String
     *
     * Fetches the package restrict_string, if ebuild_interface is not Nil.
     */
    /*
     * Document-method: eclass_keywords
     *
     * call-seq:
     *     eclass_keywords -> String
     *
     * Fetches the package eclass_keywords, if ebuild_interface is not Nil.
     */
    /*
     * Document-method: iuse
     *
     * call-seq:
     *     iuse -> String
     *
     * Fetches the package iuse, if ebuild_interface is not Nil.
     */
    /*
     * Document-method: inherited
     *
     * call-seq:
     *     inherited -> String
     *
     * Fetches the package inherited, if ebuild_interface is not Nil.
     */
    template <typename T_, T_ VersionMetadataEbuildInterface::* m_>
    struct EbuildValue
    {
        static VALUE
        fetch(VALUE self)
        {
            std::tr1::shared_ptr<const VersionMetadata> * self_ptr;
            Data_Get_Struct(self, std::tr1::shared_ptr<const VersionMetadata>, self_ptr);
            if ((*self_ptr)->ebuild_interface)
                return rb_str_new2(stringify((*self_ptr)->ebuild_interface->*m_).c_str());
            else
                return Qnil;
        }
    };
#endif

    /*
     * Document-method: package
     *
     * call-seq:
     *     package -> String
     *
     * Fetches the package name, if cran_interface is not Nil
     */
    /*
     * Document-method: version
     *
     * call-seq:
     *     version -> String
     *
     * Fetches the package version, if ebin_interface is not Nil
     */
    template <typename T_, T_ VersionMetadataCRANInterface::* m_>
    struct CRANValue
    {
        static VALUE
        fetch(VALUE self)
        {
            std::tr1::shared_ptr<const VersionMetadata> * self_ptr;
            Data_Get_Struct(self, std::tr1::shared_ptr<const VersionMetadata>, self_ptr);
            if ((*self_ptr)->cran_interface)
                return rb_str_new2(stringify((*self_ptr)->cran_interface->*m_).c_str());
            else
                return Qnil;
        }
    };

    /*
     * Document-method: build_depend
     *
     * call-seq:
     *     build_depend -> DepSpec
     *
     * Fetches build_depend information as a DepSpec, or Nil if we have no deps
     * interface.
     */
    /*
     * Document-method: run_depend
     *
     * call-seq:
     *     run_depend -> DepSpec
     *
     * Fetches run_depend information as a DepSpec, or Nil if we have no deps
     * interface.
     */
    /*
     * Document-method: suggested_depend
     *
     * call-seq:
     *     suggested_depend -> DepSpec
     *
     * Fetches sugest_depend information as a DepSpec, or Nil if we have no deps
     * interface.
     */
    /*
     * Document-method: post_depend
     *
     * call-seq:
     *     post_depend -> DepSpec
     *
     * Fetches post_depend information as a DepSpec, or Nil if we have no deps
     * interface.
     */
    template <std::tr1::shared_ptr<const DepSpec> (VersionMetadataDepsInterface::* m_) () const>
    struct DependValue
    {
        static VALUE
        fetch(VALUE self)
        {
            std::tr1::shared_ptr<const VersionMetadata> * self_ptr;
            Data_Get_Struct(self, std::tr1::shared_ptr<const VersionMetadata>, self_ptr);
            if ((*self_ptr)->deps_interface)
                return dep_spec_to_value(((*self_ptr)->deps_interface->*m_)());
            else
                return Qnil;
        }
    };

#ifdef CIARANM_REMOVED_THIS
    /*
     * Document-method: build_depend_string
     *
     * call-seq:
     *     build_depend_string -> String
     *
     * Fetches build_depend information as a String, or Nil if we have no deps
     * interface
     */
    /*
     * Document-method: run_depend_string
     *
     * call-seq:
     *     run_depend_string -> String
     *
     * Fetches run_depend information as a String, or Nil if we have no deps
     * interface
     */
    /*
     * Document-method: suggested_depend_string
     *
     * call-seq:
     *     suggested_depend_string -> String
     *
     * Fetches suggested_depend information as a String, or Nil if we have no
     * deps interface
     */
    /*
     * Document-method: post_depend_string
     *
     * call-seq:
     *     post_depend_string -> String
     *
     * Fetches post_depend information as a String, or Nil if we have no deps
     * interface
     */
    template <std::string VersionMetadataDepsInterface::* m_>
    struct DependValueString
    {
        static VALUE
        fetch(VALUE self)
        {
            std::tr1::shared_ptr<const VersionMetadata> * self_ptr;
            Data_Get_Struct(self, std::tr1::shared_ptr<const VersionMetadata>, self_ptr);
            if ((*self_ptr)->deps_interface)
                return rb_str_new2((((*self_ptr)->deps_interface)->*m_).c_str());
            else
                return Qnil;
        }
    };
#endif

    /*
     * Document-method: origin_source
     *
     * call-seq:
     *     origin_source -> PackageDatabaseEntry
     *
     * Returnd the PackageDatabaseEntry from which the package was installed.
     */
    template <std::tr1::shared_ptr<PackageDatabaseEntry> VersionMetadataOriginsInterface::* m_>
    struct VMOrigins
    {
        static VALUE
        fetch(VALUE self)
        {
            std::tr1::shared_ptr<const VersionMetadata> * self_ptr;
            Data_Get_Struct(self, std::tr1::shared_ptr<const VersionMetadata>, self_ptr);
            if ((*self_ptr)->origins_interface && (((*self_ptr)->origins_interface)->*m_))
                return package_database_entry_to_value(*(((*self_ptr)->origins_interface)->*m_));
            else
                return Qnil;
        }
    };

    /*
     * call-seq:
     *     virtual_for -> PackageDatabaseEntry
     *
     * Fetch package we are a virtual for, if virtual_interface is not Nil.
     */
    VALUE version_metadata_virtual_for(VALUE self)
    {
        std::tr1::shared_ptr<const VersionMetadata> * self_ptr;
        Data_Get_Struct(self, std::tr1::shared_ptr<const VersionMetadata>, self_ptr);
        if ((*self_ptr)->virtual_interface)
            return package_database_entry_to_value((*self_ptr)->virtual_interface->virtual_for);
        else
            return Qnil;

    }

    /*
     * call-seq:
     *     is_bundle? -> true or false
     *
     * Are we a bundle? True or false, if virtual_interface is not Nil.
     */
    VALUE version_metadata_is_bundle(VALUE self)
    {
        std::tr1::shared_ptr<const VersionMetadata> * self_ptr;
        Data_Get_Struct(self, std::tr1::shared_ptr<const VersionMetadata>, self_ptr);
        if ((*self_ptr)->cran_interface)
            return ((*self_ptr)->cran_interface->is_bundle) ? Qtrue : Qfalse;
        else
            return Qnil;

    }

#ifdef CIARANM_REMOVED_THIS
    /*
     * call-seq:
     *     keywords_string -> String
     *
     * Fetches the package keywords, if ebuild_interface or cran_interface is not Nil.
     */
    VALUE version_metadata_keywords_string(VALUE self)
    {
        std::tr1::shared_ptr<const VersionMetadata> * self_ptr;
        Data_Get_Struct(self, std::tr1::shared_ptr<const VersionMetadata>, self_ptr);
        if ((*self_ptr)->ebuild_interface)
            return rb_str_new2(((*self_ptr)->ebuild_interface->keywords_string).c_str());
        if ((*self_ptr)->cran_interface)
            return rb_str_new2(((*self_ptr)->cran_interface->keywords_string).c_str());
        else
            return Qnil;
    }
#endif

    void do_register_version_metadata()
    {
        /*
         * Document-class: Paludis::VersionMetadata
         *
         * Metadata about a package version.
         */
        c_version_metadata = rb_define_class_under(paludis_module(), "VersionMetadata", rb_cObject);
        rb_funcall(c_version_metadata, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_version_metadata, "ebuild_interface", RUBY_FUNC_CAST(&version_metadata_ebuild_interface), 0);
        rb_define_method(c_version_metadata, "virtual_interface", RUBY_FUNC_CAST(&version_metadata_virtual_interface), 0);
        rb_define_method(c_version_metadata, "cran_interface", RUBY_FUNC_CAST(&version_metadata_cran_interface), 0);
        rb_define_method(c_version_metadata, "license_interface", RUBY_FUNC_CAST(&version_metadata_license_interface), 0);
        rb_define_method(c_version_metadata, "deps_interface", RUBY_FUNC_CAST(&version_metadata_deps_interface), 0);
        rb_define_method(c_version_metadata, "origins_interface", RUBY_FUNC_CAST(&version_metadata_origins_interface), 0);

        rb_define_method(c_version_metadata, "license", RUBY_FUNC_CAST(&version_metadata_license), 0);
#ifdef CIARANM_REMOVED_THIS
        rb_define_method(c_version_metadata, "license_string", RUBY_FUNC_CAST(&version_metadata_license_string), 0);
#endif

        rb_define_method(c_version_metadata, "slot", RUBY_FUNC_CAST((&BaseValue<SlotName, &VersionMetadataBase::slot>::fetch)), 0);
        rb_define_method(c_version_metadata, "eapi", RUBY_FUNC_CAST((&BaseValue<std::string, &VersionMetadataBase::eapi>::fetch)), 0);
#ifdef CIARANM_REMOVED_THIS
        rb_define_method(c_version_metadata, "homepage", RUBY_FUNC_CAST((&BaseValue<std::string, &VersionMetadataBase::homepage>::fetch)), 0);
#endif
        rb_define_method(c_version_metadata, "description", RUBY_FUNC_CAST((&BaseValue<std::string,
                        &VersionMetadataBase::description>::fetch)), 0);
        rb_define_method(c_version_metadata, "interactive?", RUBY_FUNC_CAST(&version_metadata_interactive), 0);

#ifdef CIARANM_REMOVED_THIS
        rb_define_method(c_version_metadata, "provide_string", RUBY_FUNC_CAST((&EbuildValue<std::string,
                        &VersionMetadataEbuildInterface::provide_string>::fetch)), 0);
        rb_define_method(c_version_metadata, "src_uri_string", RUBY_FUNC_CAST((&EbuildValue<std::string,
                        &VersionMetadataEbuildInterface::src_uri_string>::fetch)), 0);
        rb_define_method(c_version_metadata, "restrict_string", RUBY_FUNC_CAST((&EbuildValue<std::string,
                        &VersionMetadataEbuildInterface::restrict_string>::fetch)), 0);
        rb_define_method(c_version_metadata, "eclass_keywords", RUBY_FUNC_CAST((&EbuildValue<std::string,
                        &VersionMetadataEbuildInterface::eclass_keywords>::fetch)), 0);
        rb_define_method(c_version_metadata, "iuse", RUBY_FUNC_CAST((&EbuildValue<std::string,
                        &VersionMetadataEbuildInterface::iuse>::fetch)), 0);
        rb_define_method(c_version_metadata, "inherited", RUBY_FUNC_CAST((&EbuildValue<std::string,
                        &VersionMetadataEbuildInterface::inherited>::fetch)), 0);
#endif

        rb_define_method(c_version_metadata, "build_depend", RUBY_FUNC_CAST((&DependValue<
                        &VersionMetadataDepsInterface::build_depend>::fetch)), 0);
        rb_define_method(c_version_metadata, "run_depend", RUBY_FUNC_CAST((&DependValue<
                        &VersionMetadataDepsInterface::run_depend>::fetch)), 0);
        rb_define_method(c_version_metadata, "suggested_depend", RUBY_FUNC_CAST((&DependValue<
                        &VersionMetadataDepsInterface::suggested_depend>::fetch)), 0);
        rb_define_method(c_version_metadata, "post_depend", RUBY_FUNC_CAST((&DependValue<
                        &VersionMetadataDepsInterface::post_depend>::fetch)), 0);

#ifdef CIARANM_REMOVED_THIS
        rb_define_method(c_version_metadata, "build_depend_string", RUBY_FUNC_CAST((&DependValueString<
                        &VersionMetadataDepsInterface::build_depend_string>::fetch)), 0);
        rb_define_method(c_version_metadata, "run_depend_string", RUBY_FUNC_CAST((&DependValueString<
                        &VersionMetadataDepsInterface::run_depend_string>::fetch)), 0);
        rb_define_method(c_version_metadata, "suggested_depend_string", RUBY_FUNC_CAST((&DependValueString<
                        &VersionMetadataDepsInterface::suggested_depend_string>::fetch)), 0);
        rb_define_method(c_version_metadata, "post_depend_string", RUBY_FUNC_CAST((&DependValueString<
                        &VersionMetadataDepsInterface::post_depend_string>::fetch)), 0);
#endif

        rb_define_method(c_version_metadata, "origin_source", RUBY_FUNC_CAST((&VMOrigins<
                        &VersionMetadataOriginsInterface::source>::fetch)), 0);
        rb_define_method(c_version_metadata, "origin_binary", RUBY_FUNC_CAST((&VMOrigins<
                        &VersionMetadataOriginsInterface::binary>::fetch)), 0);

        rb_define_method(c_version_metadata, "virtual_for", RUBY_FUNC_CAST(&version_metadata_virtual_for), 0);

        rb_define_method(c_version_metadata, "package", RUBY_FUNC_CAST((&CRANValue<std::string,
                        &VersionMetadataCRANInterface::package>::fetch)), 0);
        rb_define_method(c_version_metadata, "version", RUBY_FUNC_CAST((&CRANValue<std::string,
                        &VersionMetadataCRANInterface::version>::fetch)), 0);
        rb_define_method(c_version_metadata, "is_bundle?", RUBY_FUNC_CAST(&version_metadata_is_bundle), 0);

#ifdef CIARANM_REMOVED_THIS
        rb_define_method(c_version_metadata, "keywords_string", RUBY_FUNC_CAST(&version_metadata_keywords_string), 0);
#endif

    }
}

VALUE
paludis::ruby::version_metadata_to_value(std::tr1::shared_ptr<const VersionMetadata> m)
{
    std::tr1::shared_ptr<const VersionMetadata> * m_ptr(0);
    try
    {
        m_ptr = new std::tr1::shared_ptr<const VersionMetadata>(m);
        return Data_Wrap_Struct(c_version_metadata, 0, &Common<std::tr1::shared_ptr<const VersionMetadata> >::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

std::tr1::shared_ptr<const VersionMetadata>
paludis::ruby::value_to_version_metadata(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_version_metadata))
    {
        std::tr1::shared_ptr<const VersionMetadata> * v_ptr;
        Data_Get_Struct(v, std::tr1::shared_ptr<const VersionMetadata>, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into VersionMetadata", rb_obj_classname(v));
    }
}

RegisterRubyClass::Register paludis_ruby_register_version_metadata PALUDIS_ATTRIBUTE((used))
    (&do_register_version_metadata);


