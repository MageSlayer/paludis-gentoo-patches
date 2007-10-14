/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
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

#define BOOST_PYTHON_MAX_BASES 20

#include <python/paludis_python.hh>
#include <python/exception.hh>
#include <python/nice_names-nn.hh>

#include <paludis/formatter.hh>
#include <paludis/stringify_formatter.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/dep_tree.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

template <typename T_>
struct PythonCanFormatBase
{
    virtual bp::override get_override(const char *) const = 0;

    virtual ~PythonCanFormatBase()
    {
    }

    template <typename F_>
    std::string do_format(const T_ & t, const F_ &) const
    {
        Lock l(get_mutex());

        std::string func_name(std::string("format_") + LowercaseNiceNames<T_>::name
                     + "_" + LowercaseNiceNames<F_>::name);

        if (bp::override f = this->get_override(func_name.c_str()))
            return f(t);
        else
            throw PythonMethodNotImplemented(std::string("CanFormat") + NiceNames<T_>::name, func_name);
    }
};

template <typename T_, typename C_ = typename format::CategorySelector<T_>::Category >
struct PythonCanFormat;

template <typename T_>
struct PythonCanFormat<T_, format::PlainRoles> :
    CanFormat<T_>,
    PythonCanFormatBase<T_>
{
    std::string format(const T_ & t, const format::Plain & f) const
    {
        return do_format(t, f);
    }

    std::string format_plain(const T_ & t) const
    {
        return format(t, format::Plain());
    }
};

template <typename T_>
struct PythonCanFormat<T_, format::AcceptableRoles> :
    CanFormat<T_>,
    PythonCanFormatBase<T_>
{
    std::string format(const T_ & t, const format::Plain & f) const
    {
        return do_format(t, f);
    }

    std::string format(const T_ & t, const format::Accepted & f) const
    {
        return do_format(t, f);
    }

    std::string format(const T_ & t, const format::Unaccepted & f) const
    {
        return do_format(t, f);
    }

    std::string format_plain(const T_ & t) const
    {
        return format(t, format::Plain());
    }

    std::string format_accepted(const T_ & t) const
    {
        return format(t, format::Accepted());
    }

    std::string format_unaccepted(const T_ & t) const
    {
        return format(t, format::Unaccepted());
    }
};

template <typename T_>
struct PythonCanFormat<T_, format::UseRoles> :
    CanFormat<T_>,
    PythonCanFormatBase<T_>
{
    std::string format(const T_ & t, const format::Plain & f) const
    {
        return do_format(t, f);
    }

    std::string format(const T_ & t, const format::Enabled & f) const
    {
        return do_format(t, f);
    }

    std::string format(const T_ & t, const format::Disabled & f) const
    {
        return do_format(t, f);
    }

    std::string format(const T_ & t, const format::Forced & f) const
    {
        return do_format(t, f);
    }

    std::string format(const T_ & t, const format::Masked & f) const
    {
        return do_format(t, f);
    }

    std::string format_plain(const T_ & t) const
    {
        return format(t, format::Plain());
    }

    std::string format_enabled(const T_ & t) const
    {
        return format(t, format::Enabled());
    }

    std::string format_disabled(const T_ & t) const
    {
        return format(t, format::Disabled());
    }

    std::string format_forced(const T_ & t) const
    {
        return format(t, format::Forced());
    }

    std::string format_masked(const T_ & t) const
    {
        return format(t, format::Masked());
    }
};

template <typename T_>
struct PythonCanFormat<T_, format::IUseRoles> :
    CanFormat<T_>,
    PythonCanFormatBase<T_>
{
    template <typename F_>
    std::string do_decorate(const T_ & t, const std::string & s, const F_ &) const
    {
        Lock l(get_mutex());

        std::string func_name(std::string("decorate_") + LowercaseNiceNames<T_>::name
                     + "_" + LowercaseNiceNames<F_>::name);

        if (bp::override f = this->get_override(func_name.c_str()))
            return f(t, s);
        else
            throw PythonMethodNotImplemented(std::string("CanFormat") + NiceNames<T_>::name, func_name);
    }

    std::string format(const T_ & t, const format::Plain & f) const
    {
        return do_format(t, f);
    }

    std::string format(const T_ & t, const format::Enabled & f) const
    {
        return do_format(t, f);
    }

    std::string format(const T_ & t, const format::Disabled & f) const
    {
        return do_format(t, f);
    }

    std::string format(const T_ & t, const format::Forced & f) const
    {
        return do_format(t, f);
    }

    std::string format(const T_ & t, const format::Masked & f) const
    {
        return do_format(t, f);
    }

    std::string decorate(const T_ & t, const std::string & s, const format::Added & f) const
    {
        return do_decorate(t, s, f);
    }

    std::string decorate(const T_ & t, const std::string & s, const format::Changed & f) const
    {
        return do_decorate(t, s, f);
    }

    std::string format_plain(const T_ & t) const
    {
        return format(t, format::Plain());
    }

    std::string format_enabled(const T_ & t) const
    {
        return format(t, format::Enabled());
    }

    std::string format_disabled(const T_ & t) const
    {
        return format(t, format::Disabled());
    }

    std::string format_forced(const T_ & t) const
    {
        return format(t, format::Forced());
    }

    std::string format_masked(const T_ & t) const
    {
        return format(t, format::Masked());
    }

    std::string decorate_added(const T_ & t, const std::string & s) const
    {
        return decorate(t, s, format::Added());
    }

    std::string decorate_changed(const T_ & t, const std::string & s) const
    {
        return decorate(t, s, format::Changed());
    }
};

template <typename T_>
struct PythonCanFormat<T_, format::PackageRoles> :
    CanFormat<T_>,
    PythonCanFormatBase<T_>
{
    std::string format(const T_ & t, const format::Plain & f) const
    {
        return do_format(t, f);
    }

    std::string format(const T_ & t, const format::Installed & f) const
    {
        return do_format(t, f);
    }

    std::string format(const T_ & t, const format::Installable & f) const
    {
        return do_format(t, f);
    }

    std::string format_plain(const T_ & t) const
    {
        return format(t, format::Plain());
    }

    std::string format_installed(const T_ & t) const
    {
        return format(t, format::Installed());
    }

    std::string format_installable(const T_ & t) const
    {
        return format(t, format::Installable());
    }
};

template <typename T_>
struct PythonCanFormatWrapper:
    PythonCanFormat<T_>,
    bp::wrapper<CanFormat<T_> >
{
    bp::override get_override(const char * name) const
    {
        return bp::wrapper<CanFormat<T_> >::get_override(name);
    }
};

struct PythonCanSpace :
    CanSpace
{
    virtual bp::override get_override(const char *) const = 0;

    std::string newline() const
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("newline"))
            return f();
        else
            throw PythonMethodNotImplemented(std::string("CanSpace"), "newline");
    }

    std::string indent(const int i) const
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("indent"))
            return f(i);
        else
            throw PythonMethodNotImplemented(std::string("CanSpace"), "indent");
    }
};

struct PythonCanSpaceWrapper :
    PythonCanSpace,
    bp::wrapper<CanSpace>
{
    bp::override get_override(const char * name) const
    {
        return bp::wrapper<CanSpace>::get_override(name);
    }
};

class PythonFormatterWrapper :
    public PythonCanFormat<UseFlagName>,
    public PythonCanFormat<IUseFlag>,
    public PythonCanFormat<KeywordName>,
    public PythonCanFormat<PackageDepSpec>,
    public PythonCanFormat<BlockDepSpec>,
    public PythonCanFormat<FetchableURIDepSpec>,
    public PythonCanFormat<SimpleURIDepSpec>,
    public PythonCanFormat<DependencyLabelsDepSpec>,
    public PythonCanFormat<URILabelsDepSpec>,
    public PythonCanFormat<PlainTextDepSpec>,
    public PythonCanFormat<LicenseDepSpec>,
    public PythonCanFormat<UseDepSpec>,
    public PythonCanFormat<NamedSetDepSpec>,
    public PythonCanSpace,
    public bp::wrapper<PythonFormatterWrapper>
{
    bp::override get_override(const char * name) const
    {
        return bp::wrapper<PythonFormatterWrapper>::get_override(name);
    }
};

void expose_formatter()
{

    /**
     * CanFormatString
     */
    std::string (PythonCanFormatWrapper<std::string>::* format_string_plain)
        (const std::string &) const = &PythonCanFormatWrapper<std::string>::format_plain;

    bp::class_<PythonCanFormatWrapper<std::string>, boost::noncopyable>
        (
         "CanFormatString",
         "Descendents of this class implement the necessary methods to format a String.",
         bp::init<>("__init__()")
        )
        .def("format_string_plain", format_string_plain)
        ;

    /**
     * CanFormatUseFlagName
     */
    std::string (PythonCanFormatWrapper<UseFlagName>::* format_use_flag_name_plain)
        (const UseFlagName &) const = &PythonCanFormatWrapper<UseFlagName>::format_plain;
    std::string (PythonCanFormatWrapper<UseFlagName>::* format_use_flag_name_enabled)
        (const UseFlagName &) const = &PythonCanFormatWrapper<UseFlagName>::format_enabled;
    std::string (PythonCanFormatWrapper<UseFlagName>::* format_use_flag_name_disabled)
        (const UseFlagName &) const = &PythonCanFormatWrapper<UseFlagName>::format_disabled;
    std::string (PythonCanFormatWrapper<UseFlagName>::* format_use_flag_name_forced)
        (const UseFlagName &) const = &PythonCanFormatWrapper<UseFlagName>::format_forced;
    std::string (PythonCanFormatWrapper<UseFlagName>::* format_use_flag_name_masked)
        (const UseFlagName &) const = &PythonCanFormatWrapper<UseFlagName>::format_masked;

    bp::class_<PythonCanFormatWrapper<UseFlagName>, boost::noncopyable>
        (
         "CanFormatUseFlagName",
         "Descendents of this class implement the necessary methods to format a UseFlagName.",
         bp::init<>("__init__()")
        )
        .def("format_use_flag_name_plain", format_use_flag_name_plain)
        .def("format_use_flag_name_enabled", format_use_flag_name_enabled)
        .def("format_use_flag_name_disabled", format_use_flag_name_disabled)
        .def("format_use_flag_name_forced", format_use_flag_name_forced)
        .def("format_use_flag_name_masked", format_use_flag_name_masked)
        ;

    /**
     * CanFormatIUseFlag
     */
    std::string (PythonCanFormatWrapper<IUseFlag>::* format_iuse_flag_plain)
        (const IUseFlag &) const = &PythonCanFormatWrapper<IUseFlag>::format_plain;
    std::string (PythonCanFormatWrapper<IUseFlag>::* format_iuse_flag_enabled)
        (const IUseFlag &) const = &PythonCanFormatWrapper<IUseFlag>::format_enabled;
    std::string (PythonCanFormatWrapper<IUseFlag>::* format_iuse_flag_disabled)
        (const IUseFlag &) const = &PythonCanFormatWrapper<IUseFlag>::format_disabled;
    std::string (PythonCanFormatWrapper<IUseFlag>::* format_iuse_flag_forced)
        (const IUseFlag &) const = &PythonCanFormatWrapper<IUseFlag>::format_forced;
    std::string (PythonCanFormatWrapper<IUseFlag>::* format_iuse_flag_masked)
        (const IUseFlag &) const = &PythonCanFormatWrapper<IUseFlag>::format_masked;
    std::string (PythonCanFormatWrapper<IUseFlag>::* decorate_iuse_flag_added)
        (const IUseFlag &, const std::string &) const = &PythonCanFormatWrapper<IUseFlag>::decorate_added;
    std::string (PythonCanFormatWrapper<IUseFlag>::* decorate_iuse_flag_changed)
        (const IUseFlag &, const std::string &) const = &PythonCanFormatWrapper<IUseFlag>::decorate_changed;

    bp::class_<PythonCanFormatWrapper<IUseFlag>, boost::noncopyable>
        (
         "CanFormatIUseFlag",
         "Descendents of this class implement the necessary methods to format a IUseFlag.",
         bp::init<>("__init__()")
        )
        .def("format_iuse_flag_plain", format_iuse_flag_plain)
        .def("format_iuse_flag_enabled", format_iuse_flag_enabled)
        .def("format_iuse_flag_disabled", format_iuse_flag_disabled)
        .def("format_iuse_flag_forced", format_iuse_flag_forced)
        .def("format_iuse_flag_masked", format_iuse_flag_masked)
        .def("decorate_iuse_flag_added", decorate_iuse_flag_added)
        .def("decorate_iuse_flag_changed", decorate_iuse_flag_changed)
        ;

    /**
     * CanFormatKeywordName
     */
    std::string (PythonCanFormatWrapper<KeywordName>::* format_keyword_name_plain)
        (const KeywordName &) const = &PythonCanFormatWrapper<KeywordName>::format_plain;
    std::string (PythonCanFormatWrapper<KeywordName>::* format_keyword_name_accepted)
        (const KeywordName &) const = &PythonCanFormatWrapper<KeywordName>::format_accepted;
    std::string (PythonCanFormatWrapper<KeywordName>::* format_keyword_name_unaccepted)
        (const KeywordName &) const = &PythonCanFormatWrapper<KeywordName>::format_unaccepted;

    bp::class_<PythonCanFormatWrapper<KeywordName>, boost::noncopyable>
        (
         "CanFormatKeywordName",
         "Descendents of this class implement the necessary methods to format a KeywordName.",
         bp::init<>("__init__()")
        )
        .def("format_keyword_name_plain", format_keyword_name_plain)
        .def("format_keyword_name_accepted", format_keyword_name_accepted)
        .def("format_keyword_name_unaccepted", format_keyword_name_unaccepted)
        ;

    /**
     * CanFormatPackageDepSpec
     */
    std::string (PythonCanFormatWrapper<PackageDepSpec>::* format_package_dep_spec_plain)
        (const PackageDepSpec &) const = &PythonCanFormatWrapper<PackageDepSpec>::format_plain;
    std::string (PythonCanFormatWrapper<PackageDepSpec>::* format_package_dep_spec_installed)
        (const PackageDepSpec &) const = &PythonCanFormatWrapper<PackageDepSpec>::format_installed;
    std::string (PythonCanFormatWrapper<PackageDepSpec>::* format_package_dep_spec_installable)
        (const PackageDepSpec &) const = &PythonCanFormatWrapper<PackageDepSpec>::format_installable;

    bp::class_<PythonCanFormatWrapper<PackageDepSpec>, boost::noncopyable>
        (
         "CanFormatPackageDepSpec",
         "Descendents of this class implement the necessary methods to format a PackageDepSpec.",
         bp::init<>("__init__()")
        )
        .def("format_package_dep_spec_plain", format_package_dep_spec_plain)
        .def("format_package_dep_spec_installed", format_package_dep_spec_installed)
        .def("format_package_dep_spec_installable", format_package_dep_spec_installable)
        ;

    /**
     * CanFormatBlockDepSpec
     */
    std::string (PythonCanFormatWrapper<BlockDepSpec>::* format_block_dep_spec_plain)
        (const BlockDepSpec &) const = &PythonCanFormatWrapper<BlockDepSpec>::format_plain;

    bp::class_<PythonCanFormatWrapper<BlockDepSpec>, boost::noncopyable>
        (
         "CanFormatBlockDepSpec",
         "Descendents of this class implement the necessary methods to format a BlockDepSpec.",
         bp::init<>("__init__()")
        )
        .def("format_block_dep_spec_plain", format_block_dep_spec_plain)
        ;

    /**
     * CanFormatFetchableURIDepSpec
     */
    std::string (PythonCanFormatWrapper<FetchableURIDepSpec>::* format_fetchable_uri_dep_spec_plain)
        (const FetchableURIDepSpec &) const = &PythonCanFormatWrapper<FetchableURIDepSpec>::format_plain;

    bp::class_<PythonCanFormatWrapper<FetchableURIDepSpec>, boost::noncopyable>
        (
         "CanFormatFetchableURIDepSpec",
         "Descendents of this class implement the necessary methods to format a FetchableURIDepSpec.",
         bp::init<>("__init__()")
        )
        .def("format_fetchable_uri_dep_spec_plain", format_fetchable_uri_dep_spec_plain)
        ;

    /**
     * CanFormatSimpleURIDepSpec
     */
    std::string (PythonCanFormatWrapper<SimpleURIDepSpec>::* format_simple_uri_dep_spec_plain)
        (const SimpleURIDepSpec &) const = &PythonCanFormatWrapper<SimpleURIDepSpec>::format_plain;

    bp::class_<PythonCanFormatWrapper<SimpleURIDepSpec>, boost::noncopyable>
        (
         "CanFormatSimpleURIDepSpec",
         "Descendents of this class implement the necessary methods to format a SimpleURIDepSpec.",
         bp::init<>("__init__()")
        )
        .def("format_simple_uri_dep_spec_plain", format_simple_uri_dep_spec_plain)
        ;

    /**
     * CanFormatDependencyLabelsDepSpec
     */
    std::string (PythonCanFormatWrapper<DependencyLabelsDepSpec>::* format_dependency_labels_dep_spec_plain)
        (const DependencyLabelsDepSpec &) const = &PythonCanFormatWrapper<DependencyLabelsDepSpec>::format_plain;

    bp::class_<PythonCanFormatWrapper<DependencyLabelsDepSpec>, boost::noncopyable>
        (
         "CanFormatDependencyLabelsDepSpec",
         "Descendents of this class implement the necessary methods to format a DependencyLabelsDepSpec.",
         bp::init<>("__init__()")
        )
        .def("format_dependency_labels_dep_spec_plain", format_dependency_labels_dep_spec_plain)
        ;

    /**
     * CanFormatURILabelsDepSpec
     */
    std::string (PythonCanFormatWrapper<URILabelsDepSpec>::* format_uri_labels_dep_spec_plain)
        (const URILabelsDepSpec &) const = &PythonCanFormatWrapper<URILabelsDepSpec>::format_plain;

    bp::class_<PythonCanFormatWrapper<URILabelsDepSpec>, boost::noncopyable>
        (
         "CanFormatURILabelsDepSpec",
         "Descendents of this class implement the necessary methods to format an URILabelsDepSpec.",
         bp::init<>("__init__()")
        )
        .def("format_uri_labels_dep_spec_plain", format_uri_labels_dep_spec_plain)
        ;

    /**
     * CanFormatPlainTextDepSpec
     */
    std::string (PythonCanFormatWrapper<PlainTextDepSpec>::* format_plain_text_dep_spec_plain)
        (const PlainTextDepSpec &) const = &PythonCanFormatWrapper<PlainTextDepSpec>::format_plain;

    bp::class_<PythonCanFormatWrapper<PlainTextDepSpec>, boost::noncopyable>
        (
         "CanFormatPlainTextDepSpec",
         "Descendents of this class implement the necessary methods to format a PlainTextDepSpec.",
         bp::init<>("__init__()")
        )
        .def("format_plain_text_dep_spec_plain", format_plain_text_dep_spec_plain)
        ;

    /**
     * CanFormatLicenseDepSpec
     */
    std::string (PythonCanFormatWrapper<LicenseDepSpec>::* format_license_dep_spec_plain)
        (const LicenseDepSpec &) const = &PythonCanFormatWrapper<LicenseDepSpec>::format_plain;
    std::string (PythonCanFormatWrapper<LicenseDepSpec>::* format_license_dep_spec_accepted)
        (const LicenseDepSpec &) const = &PythonCanFormatWrapper<LicenseDepSpec>::format_accepted;
    std::string (PythonCanFormatWrapper<LicenseDepSpec>::* format_license_dep_spec_unaccepted)
        (const LicenseDepSpec &) const = &PythonCanFormatWrapper<LicenseDepSpec>::format_unaccepted;

    bp::class_<PythonCanFormatWrapper<LicenseDepSpec>, boost::noncopyable>
        (
         "CanFormatLicenseDepSpec",
         "Descendents of this class implement the necessary methods to format a LicenseDepSpec.",
         bp::init<>("__init__()")
        )
        .def("format_license_dep_spec_plain", format_license_dep_spec_plain)
        .def("format_license_dep_spec_accepted", format_license_dep_spec_accepted)
        .def("format_license_dep_spec_unaccepted", format_license_dep_spec_unaccepted)
        ;

    /**
     * CanFormatUseDepSpec
     */
    std::string (PythonCanFormatWrapper<UseDepSpec>::* format_use_dep_spec_plain)
        (const UseDepSpec &) const = &PythonCanFormatWrapper<UseDepSpec>::format_plain;
    std::string (PythonCanFormatWrapper<UseDepSpec>::* format_use_dep_spec_enabled)
        (const UseDepSpec &) const = &PythonCanFormatWrapper<UseDepSpec>::format_enabled;
    std::string (PythonCanFormatWrapper<UseDepSpec>::* format_use_dep_spec_disabled)
        (const UseDepSpec &) const = &PythonCanFormatWrapper<UseDepSpec>::format_disabled;
    std::string (PythonCanFormatWrapper<UseDepSpec>::* format_use_dep_spec_forced)
        (const UseDepSpec &) const = &PythonCanFormatWrapper<UseDepSpec>::format_forced;
    std::string (PythonCanFormatWrapper<UseDepSpec>::* format_use_dep_spec_masked)
        (const UseDepSpec &) const = &PythonCanFormatWrapper<UseDepSpec>::format_masked;

    bp::class_<PythonCanFormatWrapper<UseDepSpec>, boost::noncopyable>
        (
         "CanFormatUseDepSpec",
         "Descendents of this class implement the necessary methods to format a UseDepSpec.",
         bp::init<>("__init__()")
        )
        .def("format_use_dep_spec_plain", format_use_dep_spec_plain)
        .def("format_use_dep_spec_enabled", format_use_dep_spec_enabled)
        .def("format_use_dep_spec_disabled", format_use_dep_spec_disabled)
        .def("format_use_dep_spec_forced", format_use_dep_spec_forced)
        .def("format_use_dep_spec_masked", format_use_dep_spec_masked)
        ;

    /**
     * CanFormatNamedSetDepSpec
     */
    std::string (PythonCanFormatWrapper<NamedSetDepSpec>::* format_named_set_dep_spec_plain)
        (const NamedSetDepSpec &) const = &PythonCanFormatWrapper<NamedSetDepSpec>::format_plain;

    bp::class_<PythonCanFormatWrapper<NamedSetDepSpec>, boost::noncopyable>
        (
         "CanFormatNamedSetDepSpec",
         "Descendents of this class implement the necessary methods to format a NamedSetDepSpec.",
         bp::init<>("__init__()")
        )
        .def("format_named_set_dep_spec_plain", format_named_set_dep_spec_plain)
        ;

    /**
     * CanSpace
     */
    bp::class_<PythonCanSpaceWrapper, boost::noncopyable>
        (
         "CanSpace",
         "Descendents of this class implement the necessary methods to format whitespace.",
         bp::init<>("__init__()")
        )
        .def("newline", &CanSpace::newline,
                "newline() -> string\n"
                "Output a newline."
            )

        .def("indent", &CanSpace::newline,
                "indent(int) -> string\n"
                "Output an indent marker of the specified indent level."
            )
        ;

    /**
     * StringFormatter
     */
    bp::class_<Formatter<std::string>, bp::bases<CanFormat<std::string> >, boost::noncopyable>
        (
         "StringFormatter",
         "A formatter that can handle Strings.",
         bp::no_init
        );

    /**
     * KeywordNameFormatter
     */
    bp::class_<Formatter<KeywordName>, bp::bases<CanFormat<KeywordName> >, boost::noncopyable>
        (
         "KeywordNameFormatter",
         "A formatter that can handle KeywordNames.",
         bp::no_init
        );

    /**
     * UseFlagNameFormatter
     */
    bp::class_<Formatter<UseFlagName>, bp::bases<CanFormat<UseFlagName> >, boost::noncopyable>
        (
         "UseFlagNameFormatter",
         "A formatter that can handle UseFlagNames.",
         bp::no_init
        );

    /**
     * IUseFlagFormatter
     */
    bp::class_<Formatter<IUseFlag>, bp::bases<CanFormat<IUseFlag> >, boost::noncopyable>
        (
         "IUseFlagFormatter",
         "A formatter that can handle IUseFlags.",
         bp::no_init
        );

    /**
     * LicenseSpecTreeFormatter
     */
    bp::class_<LicenseSpecTree::ItemFormatter,
            bp::bases<
                CanFormat<UseDepSpec>,
                CanFormat<LicenseDepSpec> >,
            boost::noncopyable>
        (
         "LicenseSpecTreeFormatter",
         "A formatter that can handle any formattable type found in a\n"
         "LicenseSpecTree.",
         bp::no_init
        );

    /**
     * ProvideSpecTreeFormatter
     */
    bp::class_<ProvideSpecTree::ItemFormatter,
            bp::bases<
                CanFormat<UseDepSpec>,
                CanFormat<PackageDepSpec> >,
            boost::noncopyable>
        (
         "ProvideSpecTreeFormatter",
         "A formatter that can handle any formattable type found in a\n"
         "ProvideSpecTree.",
         bp::no_init
        );

    /**
     * DependencySpecTreeFormatter
     */
    bp::class_<DependencySpecTree::ItemFormatter,
            bp::bases<
                CanFormat<UseDepSpec>,
                CanFormat<PackageDepSpec>,
                CanFormat<NamedSetDepSpec>,
                CanFormat<DependencyLabelsDepSpec> >,
            boost::noncopyable>
        (
         "DependencySpecTreeFormatter",
         "A formatter that can handle any formattable type found in a\n"
         "DependencySpecTree.",
         bp::no_init
        );

    /**
     * RestrictSpecTreeFormatter
     */
    bp::class_<RestrictSpecTree::ItemFormatter,
            bp::bases<
                CanFormat<UseDepSpec>,
                CanFormat<PlainTextDepSpec> >,
            boost::noncopyable>
        (
         "RestrictSpecTreeFormatter",
         "A formatter that can handle any formattable type found in a\n"
         "RestrictSpecTree.",
         bp::no_init
        );

    /**
     * SimpleURISpecTreeFormatter
     */
    bp::class_<SimpleURISpecTree::ItemFormatter,
            bp::bases<
                CanFormat<UseDepSpec>,
                CanFormat<SimpleURIDepSpec> >,
            boost::noncopyable>
        (
         "SimpleURISpecTreeFormatter",
         "A formatter that can handle any formattable type found in a\n"
         "SimpleURISpecTree.",
         bp::no_init
        );

    /**
     * FetchableURISpecTreeFormatter
     */
    bp::class_<FetchableURISpecTree::ItemFormatter,
            bp::bases<
                CanFormat<UseDepSpec>,
                CanFormat<FetchableURIDepSpec>,
                CanFormat<URILabelsDepSpec> >,
            boost::noncopyable>
        (
         "FetchableURISpecTreeFormatter",
         "A formatter that can handle any formattable type found in a\n"
         "FetchableURISpecTree.",
         bp::no_init
        );

    /**
     * StringifyFormatter
     */
    bp::class_<StringifyFormatter,
            bp::bases<
                CanFormat<UseFlagName>,
                CanFormat<IUseFlag>,
                CanFormat<KeywordName>,
                CanFormat<PackageDepSpec>,
                CanFormat<BlockDepSpec>,
                CanFormat<FetchableURIDepSpec>,
                CanFormat<SimpleURIDepSpec>,
                CanFormat<DependencyLabelsDepSpec>,
                CanFormat<URILabelsDepSpec>,
                CanFormat<PlainTextDepSpec>,
                CanFormat<LicenseDepSpec>,
                CanFormat<UseDepSpec>,
                CanFormat<NamedSetDepSpec>,
                CanSpace>,
            boost::noncopyable>
        (
         "StringifyFormatter",
         "A StringifyFormatter is a Formatter that implements every format function\n"
         "by calling paludis::stringify().\n\n"

         "Indenting is done via simple spaces; newlines are done via a newline\n"
         "character. Again, when used as a wrapper, this can be overridden by the\n"
         "wrapped class.\n\n"

         "This class can be subclassed in Python.",
         bp::init<>("__init__()")
        );

    /**
     * PythonFormatter
     */
    bp::class_<PythonFormatterWrapper,
            bp::bases<
                CanFormat<UseFlagName>,
                CanFormat<IUseFlag>,
                CanFormat<KeywordName>,
                CanFormat<PackageDepSpec>,
                CanFormat<BlockDepSpec>,
                CanFormat<FetchableURIDepSpec>,
                CanFormat<SimpleURIDepSpec>,
                CanFormat<DependencyLabelsDepSpec>,
                CanFormat<URILabelsDepSpec>,
                CanFormat<PlainTextDepSpec>,
                CanFormat<LicenseDepSpec>,
                CanFormat<UseDepSpec>,
                CanFormat<NamedSetDepSpec>,
                CanSpace>,
            boost::noncopyable>
        (
         "PythonFormatter",
         bp::init<>("__init__()")
        );
}
