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
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>

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
};

template <typename T_>
struct PythonCanFormat<T_, format::PackageRoles> :
    CanFormat<T_>,
    PythonCanFormatBase<T_>
{
    virtual std::string format(const T_ & t, const format::Plain & f) const
    {
        return do_format(t, f);
    }

    virtual std::string format(const T_ & t, const format::Installed & f) const
    {
        return do_format(t, f);
    }

    virtual std::string format(const T_ & t, const format::Installable & f) const
    {
        return do_format(t, f);
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
     * CanFormatUseFlagName
     */
    bp::class_<PythonCanFormatWrapper<UseFlagName>, boost::noncopyable>
        (
         "CanFormatUseFlagName",
         "Descendents of this class implement the necessary methods to format a UseFlagName.",
         bp::init<>("__init__()")
        );

    /**
     * CanFormatIUseFlag
     */
    bp::class_<PythonCanFormatWrapper<IUseFlag>, boost::noncopyable>
        (
         "CanFormatIUseFlag",
         "Descendents of this class implement the necessary methods to format a IUseFlag.",
         bp::init<>("__init__()")
        );

    /**
     * CanFormatKeywordName
     */
    bp::class_<PythonCanFormatWrapper<KeywordName>, boost::noncopyable>
        (
         "CanFormatKeywordName",
         "Descendents of this class implement the necessary methods to format a KeywordName.",
         bp::init<>("__init__()")
        );

    /**
     * CanFormatPackageDepSpec
     */
    bp::class_<PythonCanFormatWrapper<PackageDepSpec>, boost::noncopyable>
        (
         "CanFormatPackageDepSpec",
         "Descendents of this class implement the necessary methods to format a PackageDepSpec.",
         bp::init<>("__init__()")
        );

    /**
     * CanFormatBlockDepSpec
     */
    bp::class_<PythonCanFormatWrapper<BlockDepSpec>, boost::noncopyable>
        (
         "CanFormatBlockDepSpec",
         "Descendents of this class implement the necessary methods to format a BlockDepSpec.",
         bp::init<>("__init__()")
        );

    /**
     * CanFormatFetchableURIDepSpec
     */
    bp::class_<PythonCanFormatWrapper<FetchableURIDepSpec>, boost::noncopyable>
        (
         "CanFormatFetchableURIDepSpec",
         "Descendents of this class implement the necessary methods to format a FetchableURIDepSpec.",
         bp::init<>("__init__()")
        );

    /**
     * CanFormatSimpleURIDepSpec
     */
    bp::class_<PythonCanFormatWrapper<SimpleURIDepSpec>, boost::noncopyable>
        (
         "CanFormatSimpleURIDepSpec",
         "Descendents of this class implement the necessary methods to format a SimpleURIDepSpec.",
         bp::init<>("__init__()")
        );

    /**
     * CanFormatDependencyLabelsDepSpec
     */
    bp::class_<PythonCanFormatWrapper<DependencyLabelsDepSpec>, boost::noncopyable>
        (
         "CanFormatDependencyLabelsDepSpec",
         "Descendents of this class implement the necessary methods to format a DependencyLabelsDepSpec.",
         bp::init<>("__init__()")
        );

    /**
     * CanFormatURILabelsDepSpec
     */
    bp::class_<PythonCanFormatWrapper<URILabelsDepSpec>, boost::noncopyable>
        (
         "CanFormatURILabelsDepSpec",
         "Descendents of this class implement the necessary methods to format an URILabelsDepSpec.",
         bp::init<>("__init__()")
        );

    /**
     * CanFormatPlainTextDepSpec
     */
    bp::class_<PythonCanFormatWrapper<PlainTextDepSpec>, boost::noncopyable>
        (
         "CanFormatPlainTextDepSpec",
         "Descendents of this class implement the necessary methods to format a PlainTextDepSpec.",
         bp::init<>("__init__()")
        );

    /**
     * CanFormatLicenseDepSpec
     */
    bp::class_<PythonCanFormatWrapper<LicenseDepSpec>, boost::noncopyable>
        (
         "CanFormatLicenseDepSpec",
         "Descendents of this class implement the necessary methods to format a LicenseDepSpec.",
         bp::init<>("__init__()")
        );

    /**
     * CanFormatUseDepSpec
     */
    bp::class_<PythonCanFormatWrapper<UseDepSpec>, boost::noncopyable>
        (
         "CanFormatUseDepSpec",
         "Descendents of this class implement the necessary methods to format a UseDepSpec.",
         bp::init<>("__init__()")
        );

    /**
     * CanFormatNamedSetDepSpec
     */
    bp::class_<PythonCanFormatWrapper<NamedSetDepSpec>, boost::noncopyable>
        (
         "CanFormatNamedSetDepSpec",
         "Descendents of this class implement the necessary methods to format a NamedSetDepSpec.",
         bp::init<>("__init__()")
        );

    /**
     * CanSpace
     */
    bp::class_<PythonCanSpaceWrapper, boost::noncopyable>
        (
         "CanSpace",
         "Descendents of this class implement the necessary methods to format whitespace.",
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
