/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński
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
#include <paludis/spec_tree.hh>
#include <paludis/util/clone-impl.hh>

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

    static std::string format_plain(const CanFormat<T_> & self, const T_ & t)
    {
        return self.format(t, format::Plain());
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

    static std::string format_plain(const CanFormat<T_> & self, const T_ & t)
    {
        return self.format(t, format::Plain());
    }

    static std::string format_accepted(const CanFormat<T_> & self, const T_ & t)
    {
        return self.format(t, format::Accepted());
    }

    static std::string format_unaccepted(const CanFormat<T_> & self, const T_ & t)
    {
        return self.format(t, format::Unaccepted());
    }
};

template <typename T_>
struct PythonCanFormat<T_, format::ChoiceRoles> :
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

    static std::string format_plain(const CanFormat<T_> & self, const T_ & t)
    {
        return self.format(t, format::Plain());
    }

    static std::string format_enabled(const CanFormat<T_> & self, const T_ & t)
    {
        return self.format(t, format::Enabled());
    }

    static std::string format_disabled(const CanFormat<T_> & self, const T_ & t)
    {
        return self.format(t, format::Disabled());
    }

    static std::string format_forced(const CanFormat<T_> & self, const T_ & t)
    {
        return self.format(t, format::Forced());
    }

    static std::string format_masked(const CanFormat<T_> & self, const T_ & t)
    {
        return self.format(t, format::Masked());
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

    static std::string format_plain(const CanFormat<T_> & self, const T_ & t)
    {
        return self.format(t, format::Plain());
    }

    static std::string format_installed(const CanFormat<T_> & self, const T_ & t)
    {
        return self.format(t, format::Installed());
    }

    static std::string format_installable(const CanFormat<T_> & self, const T_ & t)
    {
        return self.format(t, format::Installable());
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
    public PythonCanFormat<std::string>,
    public PythonCanFormat<KeywordName>,
    public PythonCanFormat<PackageDepSpec>,
    public PythonCanFormat<BlockDepSpec>,
    public PythonCanFormat<FetchableURIDepSpec>,
    public PythonCanFormat<SimpleURIDepSpec>,
    public PythonCanFormat<DependenciesLabelsDepSpec>,
    public PythonCanFormat<URILabelsDepSpec>,
    public PythonCanFormat<PlainTextLabelDepSpec>,
    public PythonCanFormat<PlainTextDepSpec>,
    public PythonCanFormat<LicenseDepSpec>,
    public PythonCanFormat<ConditionalDepSpec>,
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
    bp::class_<PythonCanFormatWrapper<std::string>, boost::noncopyable>
        (
         "CanFormatString",
         "Descendents of this class implement the necessary methods to format a String.",
         bp::init<>("__init__()")
        )
        .def("format_string_plain", &PythonCanFormatWrapper<std::string>::format_plain)
        ;

    /**
     * CanFormatKeywordName
     */
    bp::class_<PythonCanFormatWrapper<KeywordName>, boost::noncopyable>
        (
         "CanFormatKeywordName",
         "Descendents of this class implement the necessary methods to format a KeywordName.",
         bp::init<>("__init__()")
        )
        .def("format_keyword_name_plain", &PythonCanFormatWrapper<KeywordName>::format_plain)
        .def("format_keyword_name_accepted", &PythonCanFormatWrapper<KeywordName>::format_accepted)
        .def("format_keyword_name_unaccepted", &PythonCanFormatWrapper<KeywordName>::format_unaccepted)
        ;

    /**
     * CanFormatPackageDepSpec
     */
    bp::class_<PythonCanFormatWrapper<PackageDepSpec>, boost::noncopyable>
        (
         "CanFormatPackageDepSpec",
         "Descendents of this class implement the necessary methods to format a PackageDepSpec.",
         bp::init<>("__init__()")
        )
        .def("format_package_dep_spec_plain", &PythonCanFormatWrapper<PackageDepSpec>::format_plain)
        .def("format_package_dep_spec_installed", &PythonCanFormatWrapper<PackageDepSpec>::format_installed)
        .def("format_package_dep_spec_installable", &PythonCanFormatWrapper<PackageDepSpec>::format_installable)
        ;

    /**
     * CanFormatBlockDepSpec
     */
    bp::class_<PythonCanFormatWrapper<BlockDepSpec>, boost::noncopyable>
        (
         "CanFormatBlockDepSpec",
         "Descendents of this class implement the necessary methods to format a BlockDepSpec.",
         bp::init<>("__init__()")
        )
        .def("format_block_dep_spec_plain", &PythonCanFormatWrapper<BlockDepSpec>::format_plain)
        ;

    /**
     * CanFormatFetchableURIDepSpec
     */
    bp::class_<PythonCanFormatWrapper<FetchableURIDepSpec>, boost::noncopyable>
        (
         "CanFormatFetchableURIDepSpec",
         "Descendents of this class implement the necessary methods to format a FetchableURIDepSpec.",
         bp::init<>("__init__()")
        )
        .def("format_fetchable_uri_dep_spec_plain", &PythonCanFormatWrapper<FetchableURIDepSpec>::format_plain)
        ;

    /**
     * CanFormatSimpleURIDepSpec
     */
    bp::class_<PythonCanFormatWrapper<SimpleURIDepSpec>, boost::noncopyable>
        (
         "CanFormatSimpleURIDepSpec",
         "Descendents of this class implement the necessary methods to format a SimpleURIDepSpec.",
         bp::init<>("__init__()")
        )
        .def("format_simple_uri_dep_spec_plain", &PythonCanFormatWrapper<SimpleURIDepSpec>::format_plain)
        ;

    /**
     * CanFormatDependenciesLabelsDepSpec
     */
    bp::class_<PythonCanFormatWrapper<DependenciesLabelsDepSpec>, boost::noncopyable>
        (
         "CanFormatDependenciesLabelsDepSpec",
         "Descendents of this class implement the necessary methods to format a DependenciesLabelsDepSpec.",
         bp::init<>("__init__()")
        )
        .def("format_dependencies_labels_dep_spec_plain",
                &PythonCanFormatWrapper<DependenciesLabelsDepSpec>::format_plain)
        ;

    /**
     * CanFormatURILabelsDepSpec
     */
    bp::class_<PythonCanFormatWrapper<URILabelsDepSpec>, boost::noncopyable>
        (
         "CanFormatURILabelsDepSpec",
         "Descendents of this class implement the necessary methods to format an URILabelsDepSpec.",
         bp::init<>("__init__()")
        )
        .def("format_uri_labels_dep_spec_plain", &PythonCanFormatWrapper<URILabelsDepSpec>::format_plain)
        ;

    /**
     * CanFormatPlainTextLabelDepSpec
     */
    bp::class_<PythonCanFormatWrapper<PlainTextLabelDepSpec>, boost::noncopyable>
        (
         "CanFormatPlainTextLabelDepSpec",
         "Descendents of this class implement the necessary methods to format a PlainTextLabelDepSpec.",
         bp::init<>("__init__()")
        )
        .def("format_plain_text_label_dep_spec_plain", &PythonCanFormatWrapper<PlainTextLabelDepSpec>::format_plain)
        ;

    /**
     * CanFormatPlainTextDepSpec
     */
    bp::class_<PythonCanFormatWrapper<PlainTextDepSpec>, boost::noncopyable>
        (
         "CanFormatPlainTextDepSpec",
         "Descendents of this class implement the necessary methods to format a PlainTextDepSpec.",
         bp::init<>("__init__()")
        )
        .def("format_plain_text_dep_spec_plain", &PythonCanFormatWrapper<PlainTextDepSpec>::format_plain)
        ;

    /**
     * CanFormatLicenseDepSpec
     */
    bp::class_<PythonCanFormatWrapper<LicenseDepSpec>, boost::noncopyable>
        (
         "CanFormatLicenseDepSpec",
         "Descendents of this class implement the necessary methods to format a LicenseDepSpec.",
         bp::init<>("__init__()")
        )
        .def("format_license_dep_spec_plain", &PythonCanFormatWrapper<LicenseDepSpec>::format_plain)
        .def("format_license_dep_spec_accepted", &PythonCanFormatWrapper<LicenseDepSpec>::format_accepted)
        .def("format_license_dep_spec_unaccepted", &PythonCanFormatWrapper<LicenseDepSpec>::format_unaccepted)
        ;

    /**
     * CanFormatConditionalDepSpec
     */
    bp::class_<PythonCanFormatWrapper<ConditionalDepSpec>, boost::noncopyable>
        (
         "CanFormatConditionalDepSpec",
         "Descendents of this class implement the necessary methods to format a ConditionalDepSpec.",
         bp::init<>("__init__()")
        )
        .def("format_conditional_dep_spec_plain", &PythonCanFormatWrapper<ConditionalDepSpec>::format_plain)
        .def("format_conditional_dep_spec_enabled", &PythonCanFormatWrapper<ConditionalDepSpec>::format_enabled)
        .def("format_conditional_dep_spec_disabled", &PythonCanFormatWrapper<ConditionalDepSpec>::format_disabled)
        .def("format_conditional_dep_spec_forced", &PythonCanFormatWrapper<ConditionalDepSpec>::format_forced)
        .def("format_conditional_dep_spec_masked", &PythonCanFormatWrapper<ConditionalDepSpec>::format_masked)
        ;

    /**
     * CanFormatNamedSetDepSpec
     */
    bp::class_<PythonCanFormatWrapper<NamedSetDepSpec>, boost::noncopyable>
        (
         "CanFormatNamedSetDepSpec",
         "Descendents of this class implement the necessary methods to format a NamedSetDepSpec.",
         bp::init<>("__init__()")
        )
        .def("format_named_set_dep_spec_plain", &PythonCanFormatWrapper<NamedSetDepSpec>::format_plain)
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
     * LicenseSpecTreeFormatter
     */
    bp::class_<LicenseSpecTree::ItemFormatter,
            bp::bases<
                CanFormat<ConditionalDepSpec>,
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
                CanFormat<ConditionalDepSpec>,
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
                CanFormat<ConditionalDepSpec>,
                CanFormat<PackageDepSpec>,
                CanFormat<NamedSetDepSpec>,
                CanFormat<DependenciesLabelsDepSpec> >,
            boost::noncopyable>
        (
         "DependencySpecTreeFormatter",
         "A formatter that can handle any formattable type found in a\n"
         "DependencySpecTree.",
         bp::no_init
        );

    /**
     * PlainTextSpecTreeFormatter
     */
    bp::class_<PlainTextSpecTree::ItemFormatter,
            bp::bases<
                CanFormat<ConditionalDepSpec>,
                CanFormat<PlainTextLabelDepSpec>,
                CanFormat<PlainTextDepSpec> >,
            boost::noncopyable>
        (
         "PlainTextSpecTreeFormatter",
         "A formatter that can handle any formattable type found in a\n"
         "PlainTextSpecTree.",
         bp::no_init
        );

    /**
     * SimpleURISpecTreeFormatter
     */
    bp::class_<SimpleURISpecTree::ItemFormatter,
            bp::bases<
                CanFormat<ConditionalDepSpec>,
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
                CanFormat<ConditionalDepSpec>,
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
    bp::implicitly_convertible<StringifyFormatter, Formatter<std::string> >();
    bp::implicitly_convertible<StringifyFormatter, Formatter<KeywordName> >();
    bp::implicitly_convertible<StringifyFormatter, LicenseSpecTree::ItemFormatter>();
    bp::implicitly_convertible<StringifyFormatter, ProvideSpecTree::ItemFormatter>();
    bp::implicitly_convertible<StringifyFormatter, DependencySpecTree::ItemFormatter>();
    bp::implicitly_convertible<StringifyFormatter, PlainTextSpecTree::ItemFormatter>();
    bp::implicitly_convertible<StringifyFormatter, SimpleURISpecTree::ItemFormatter>();
    bp::implicitly_convertible<StringifyFormatter, FetchableURISpecTree::ItemFormatter>();
    bp::class_<StringifyFormatter,
            bp::bases<
                CanFormat<std::string>,
                CanFormat<KeywordName>,
                CanFormat<PackageDepSpec>,
                CanFormat<BlockDepSpec>,
                CanFormat<FetchableURIDepSpec>,
                CanFormat<SimpleURIDepSpec>,
                CanFormat<DependenciesLabelsDepSpec>,
                CanFormat<URILabelsDepSpec>,
                CanFormat<PlainTextLabelDepSpec>,
                CanFormat<PlainTextDepSpec>,
                CanFormat<LicenseDepSpec>,
                CanFormat<ConditionalDepSpec>,
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
    bp::implicitly_convertible<PythonFormatterWrapper, Formatter<std::string> >();
    bp::implicitly_convertible<PythonFormatterWrapper, Formatter<KeywordName> >();
    bp::implicitly_convertible<PythonFormatterWrapper, LicenseSpecTree::ItemFormatter>();
    bp::implicitly_convertible<PythonFormatterWrapper, ProvideSpecTree::ItemFormatter>();
    bp::implicitly_convertible<PythonFormatterWrapper, DependencySpecTree::ItemFormatter>();
    bp::implicitly_convertible<PythonFormatterWrapper, PlainTextSpecTree::ItemFormatter>();
    bp::implicitly_convertible<PythonFormatterWrapper, SimpleURISpecTree::ItemFormatter>();
    bp::implicitly_convertible<PythonFormatterWrapper, FetchableURISpecTree::ItemFormatter>();
    bp::class_<PythonFormatterWrapper,
            bp::bases<
                CanFormat<std::string>,
                CanFormat<KeywordName>,
                CanFormat<PackageDepSpec>,
                CanFormat<BlockDepSpec>,
                CanFormat<FetchableURIDepSpec>,
                CanFormat<SimpleURIDepSpec>,
                CanFormat<DependenciesLabelsDepSpec>,
                CanFormat<URILabelsDepSpec>,
                CanFormat<PlainTextLabelDepSpec>,
                CanFormat<PlainTextDepSpec>,
                CanFormat<LicenseDepSpec>,
                CanFormat<ConditionalDepSpec>,
                CanFormat<NamedSetDepSpec>,
                CanSpace>,
            boost::noncopyable>
        (
         "PythonFormatter",
         bp::init<>("__init__()")
        );
}
