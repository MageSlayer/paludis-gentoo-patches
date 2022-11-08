/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
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

#include <paludis/repositories/e/e_key.hh>
#include <paludis/repositories/e/ebuild_id.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/parse_uri_label.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/myoption.hh>
#include <paludis/repositories/e/spec_tree_pretty_printer.hh>

#include <paludis/util/pretty_print.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/join.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/fs_error.hh>

#include <paludis/contents.hh>
#include <paludis/repository.hh>
#include <paludis/environment.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/call_pretty_printer.hh>

#include <algorithm>
#include <functional>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Imp<EDependenciesKey>
    {
        const Environment * const env;
        const std::shared_ptr<const ERepositoryID> id;
        const std::string string_value;
        const std::shared_ptr<const DependenciesLabelSequence> labels;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Imp(
                const Environment * const e,
                const std::shared_ptr<const ERepositoryID> & i, const std::string & v,
                const std::shared_ptr<const DependenciesLabelSequence> & s,
                const std::string & r, const std::string & h, const MetadataKeyType & t) :
            env(e),
            id(i),
            string_value(v),
            labels(s),
            raw_name(r),
            human_name(h),
            type(t)
        {
        }
    };
}

EDependenciesKey::EDependenciesKey(
        const Environment * const e,
        const std::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const std::string & v,
        const std::shared_ptr<const DependenciesLabelSequence> & l, const MetadataKeyType t) :
    _imp(e, id, v, l, r, h, t)
{
}

EDependenciesKey::~EDependenciesKey() = default;

const std::shared_ptr<const DependencySpecTree>
EDependenciesKey::parse_value() const
{
    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    return parse_depend(_imp->string_value, _imp->env, *_imp->id->eapi(), _imp->id->is_installed());
}

const std::shared_ptr<const DependenciesLabelSequence>
EDependenciesKey::initial_labels() const
{
    return _imp->labels;
}

const std::string
EDependenciesKey::pretty_print_value(
        const PrettyPrinter & pretty_printer,
        const PrettyPrintOptions & options) const
{
    SpecTreePrettyPrinter p(pretty_printer, options);
    parse_value()->top()->accept(p);
    return stringify(p);
}

const std::string
EDependenciesKey::raw_name() const
{
    return _imp->raw_name;
}

const std::string
EDependenciesKey::human_name() const
{
    return _imp->human_name;
}

MetadataKeyType
EDependenciesKey::type() const
{
    return _imp->type;
}

namespace paludis
{
    template <>
    struct Imp<ELicenseKey>
    {
        const Environment * const env;
        const std::string string_value;

        const std::shared_ptr<const EAPIMetadataVariable> variable;
        const std::shared_ptr<const EAPI> eapi;
        const MetadataKeyType type;
        const bool is_installed;

        Imp(const Environment * const e,
                const std::string & v,
                const std::shared_ptr<const EAPIMetadataVariable> & m,
                const std::shared_ptr<const EAPI> & p,
                const MetadataKeyType t,
                bool i) :
            env(e),
            string_value(v),
            variable(m),
            eapi(p),
            type(t),
            is_installed(i)
        {
        }
    };
}

ELicenseKey::ELicenseKey(
        const Environment * const e,
        const std::shared_ptr<const EAPIMetadataVariable> & m,
        const std::shared_ptr<const EAPI> & p,
        const std::string & v, const MetadataKeyType t,
        const bool i) :
    _imp(e, v, m, p, t, i)
{
}

ELicenseKey::~ELicenseKey() = default;

const std::shared_ptr<const LicenseSpecTree>
ELicenseKey::parse_value() const
{
    Context context("When parsing metadata key '" + raw_name() + "':");
    return parse_license(_imp->string_value, _imp->env, *_imp->eapi, _imp->is_installed);
}

const std::string
ELicenseKey::pretty_print_value(
        const PrettyPrinter & pretty_printer,
        const PrettyPrintOptions & options) const
{
    SpecTreePrettyPrinter p(pretty_printer, options);
    parse_value()->top()->accept(p);
    return stringify(p);
}

const std::string
ELicenseKey::raw_name() const
{
    return _imp->variable->name();
}

const std::string
ELicenseKey::human_name() const
{
    return _imp->variable->description();
}

MetadataKeyType
ELicenseKey::type() const
{
    return _imp->type;
}

namespace paludis
{
    template <>
    struct Imp<EFetchableURIKey>
    {
        const Environment * const env;
        const std::shared_ptr<const ERepositoryID> id;
        const std::shared_ptr<const EAPIMetadataVariable> variable;
        const std::string string_value;
        const MetadataKeyType type;

        Imp(const Environment * const e, const std::shared_ptr<const ERepositoryID> & i,
                const std::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v,
                const MetadataKeyType t) :
            env(e),
            id(i),
            variable(m),
            string_value(v),
            type(t)
        {
        }
    };
}

EFetchableURIKey::EFetchableURIKey(const Environment * const e,
        const std::shared_ptr<const ERepositoryID> & id,
        const std::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v, const MetadataKeyType t) :
    _imp(e, id, m, v, t)
{
}

EFetchableURIKey::~EFetchableURIKey() = default;

const std::shared_ptr<const FetchableURISpecTree>
EFetchableURIKey::parse_value() const
{
    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    return parse_fetchable_uri(_imp->string_value, _imp->env, *_imp->id->eapi(), _imp->id->is_installed());
}

const std::string
EFetchableURIKey::pretty_print_value(
        const PrettyPrinter & pretty_printer,
        const PrettyPrintOptions & options) const
{
    SpecTreePrettyPrinter p(pretty_printer, options);
    parse_value()->top()->accept(p);
    return stringify(p);
}

const std::shared_ptr<const URILabel>
EFetchableURIKey::initial_label() const
{
    std::shared_ptr<const URILabel> result;

    DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec> f(_imp->env, _imp->id);
    if (_imp->id->restrict_key())
        _imp->id->restrict_key()->parse_value()->top()->accept(f);

    for (const auto & i : f)
    {
        if (_imp->id->eapi()->supported()->ebuild_options()->restrict_fetch()->end() !=
                std::find(_imp->id->eapi()->supported()->ebuild_options()->restrict_fetch()->begin(),
                    _imp->id->eapi()->supported()->ebuild_options()->restrict_fetch()->end(), i->text()))
            result = *parse_uri_label("default-restrict-fetch:", *_imp->id->eapi())->begin();

        else if (_imp->id->eapi()->supported()->ebuild_options()->restrict_mirror()->end() !=
                std::find(_imp->id->eapi()->supported()->ebuild_options()->restrict_mirror()->begin(),
                    _imp->id->eapi()->supported()->ebuild_options()->restrict_mirror()->end(), i->text()))
            result = *parse_uri_label("default-restrict-mirror:", *_imp->id->eapi())->begin();

        else if (_imp->id->eapi()->supported()->ebuild_options()->restrict_primaryuri()->end() !=
                std::find(_imp->id->eapi()->supported()->ebuild_options()->restrict_primaryuri()->begin(),
                    _imp->id->eapi()->supported()->ebuild_options()->restrict_primaryuri()->end(), i->text()))
            result = *parse_uri_label("default-restrict-primaryuri:", *_imp->id->eapi())->begin();
    }

    if (! result)
        result = *parse_uri_label("default:", *_imp->id->eapi())->begin();

    return result;
}

const std::string
EFetchableURIKey::raw_name() const
{
    return _imp->variable->name();
}

const std::string
EFetchableURIKey::human_name() const
{
    return _imp->variable->description();
}

MetadataKeyType
EFetchableURIKey::type() const
{
    return _imp->type;
}

namespace paludis
{
    template <>
    struct Imp<ESimpleURIKey>
    {
        const Environment * const env;
        const std::string string_value;

        const std::shared_ptr<const EAPIMetadataVariable> variable;
        const std::shared_ptr<const EAPI> eapi;
        const MetadataKeyType type;
        const bool is_installed;

        Imp(const Environment * const e, const std::string & v,
                const std::shared_ptr<const EAPIMetadataVariable> & m,
                const std::shared_ptr<const EAPI> & p,
                const MetadataKeyType t, bool i) :
            env(e),
            string_value(v),
            variable(m),
            eapi(p),
            type(t),
            is_installed(i)
        {
        }
    };
}

ESimpleURIKey::ESimpleURIKey(const Environment * const e,
        const std::shared_ptr<const EAPIMetadataVariable> & m,
        const std::shared_ptr<const EAPI> & p,
        const std::string & v, const MetadataKeyType t,
        const bool i) :
    _imp(e, v, m, p, t, i)
{
}

ESimpleURIKey::~ESimpleURIKey() = default;

const std::shared_ptr<const SimpleURISpecTree>
ESimpleURIKey::parse_value() const
{
    return parse_simple_uri(_imp->string_value, _imp->env, *_imp->eapi, _imp->is_installed);
}

const std::string
ESimpleURIKey::pretty_print_value(
        const PrettyPrinter & pretty_printer,
        const PrettyPrintOptions & options) const
{
    SpecTreePrettyPrinter p(pretty_printer, options);
    parse_value()->top()->accept(p);
    return stringify(p);
}

const std::string
ESimpleURIKey::raw_name() const
{
    return _imp->variable->name();
}

const std::string
ESimpleURIKey::human_name() const
{
    return _imp->variable->description();
}

MetadataKeyType
ESimpleURIKey::type() const
{
    return _imp->type;
}

namespace paludis
{
    template <>
    struct Imp<EPlainTextSpecKey>
    {
        const Environment * const env;
        const std::string string_value;

        const std::shared_ptr<const EAPIMetadataVariable> variable;
        const std::shared_ptr<const EAPI> eapi;
        const MetadataKeyType type;
        const bool is_installed;

        Imp(const Environment * const e, const std::string & v,
                const std::shared_ptr<const EAPIMetadataVariable> & m,
                const std::shared_ptr<const EAPI> & p,
                const MetadataKeyType t, bool i) :
            env(e),
            string_value(v),
            variable(m),
            eapi(p),
            type(t),
            is_installed(i)
        {
        }
    };
}

EPlainTextSpecKey::EPlainTextSpecKey(const Environment * const e,
        const std::shared_ptr<const EAPIMetadataVariable> & m,
        const std::shared_ptr<const EAPI> & p,
        const std::string & v, const MetadataKeyType t, bool i) :
    _imp(e, v, m, p, t, i)
{
}

EPlainTextSpecKey::~EPlainTextSpecKey() = default;

const std::shared_ptr<const PlainTextSpecTree>
EPlainTextSpecKey::parse_value() const
{
    Context context("When parsing metadata key '" + raw_name() + "':");
    return parse_plain_text(_imp->string_value, _imp->env, *_imp->eapi, _imp->is_installed);
}

const std::string
EPlainTextSpecKey::pretty_print_value(
        const PrettyPrinter & pretty_printer,
        const PrettyPrintOptions & options) const
{
    SpecTreePrettyPrinter p(pretty_printer, options);
    parse_value()->top()->accept(p);
    return stringify(p);
}

const std::string
EPlainTextSpecKey::raw_name() const
{
    return _imp->variable->name();
}

const std::string
EPlainTextSpecKey::human_name() const
{
    return _imp->variable->description();
}

MetadataKeyType
EPlainTextSpecKey::type() const
{
    return _imp->type;
}

namespace paludis
{
    template <>
    struct Imp<EMyOptionsKey>
    {
        const Environment * const env;
        const std::string string_value;

        const std::shared_ptr<const EAPIMetadataVariable> variable;
        const std::shared_ptr<const EAPI> eapi;
        const MetadataKeyType type;
        const bool is_installed;

        Imp(const Environment * const e,
                const std::string & v,
                const std::shared_ptr<const EAPIMetadataVariable> & m,
                const std::shared_ptr<const EAPI> & p,
                const MetadataKeyType t, const bool i) :
            env(e),
            string_value(v),
            variable(m),
            eapi(p),
            type(t),
            is_installed(i)
        {
        }
    };
}

EMyOptionsKey::EMyOptionsKey(
        const Environment * const e,
        const std::shared_ptr<const EAPIMetadataVariable> & m,
        const std::shared_ptr<const EAPI> & p,
        const std::string & v, const MetadataKeyType t,
        const bool i) :
    _imp(e, v, m, p, t, i)
{
}

EMyOptionsKey::~EMyOptionsKey() = default;

const std::shared_ptr<const PlainTextSpecTree>
EMyOptionsKey::parse_value() const
{
    Context context("When parsing metadata key '" + raw_name() + "':");
    return parse_myoptions(_imp->string_value, _imp->env, *_imp->eapi, _imp->is_installed);
}

const std::string
EMyOptionsKey::pretty_print_value(
        const PrettyPrinter & pretty_printer,
        const PrettyPrintOptions & options) const
{
    SpecTreePrettyPrinter p(pretty_printer, options);
    parse_value()->top()->accept(p);
    return stringify(p);
}

const std::string
EMyOptionsKey::raw_name() const
{
    return _imp->variable->name();
}

const std::string
EMyOptionsKey::human_name() const
{
    return _imp->variable->description();
}

MetadataKeyType
EMyOptionsKey::type() const
{
    return _imp->type;
}

namespace paludis
{
    template <>
    struct Imp<ERequiredUseKey>
    {
        const Environment * const env;
        const std::string string_value;

        const std::shared_ptr<const EAPIMetadataVariable> variable;
        const std::shared_ptr<const EAPI> eapi;
        const MetadataKeyType type;
        const bool is_installed;

        Imp(const Environment * const e,
                const std::string & v,
                const std::shared_ptr<const EAPIMetadataVariable> & m,
                const std::shared_ptr<const EAPI> & p,
                const MetadataKeyType t,
                bool i) :
            env(e),
            string_value(v),
            variable(m),
            eapi(p),
            type(t),
            is_installed(i)
        {
        }
    };
}

ERequiredUseKey::ERequiredUseKey(
        const Environment * const e,
        const std::shared_ptr<const EAPIMetadataVariable> & m,
        const std::shared_ptr<const EAPI> & p,
        const std::string & v, const MetadataKeyType t,
        const bool i) :
    _imp(e, v, m, p, t, i)
{
}

ERequiredUseKey::~ERequiredUseKey() = default;

const std::shared_ptr<const RequiredUseSpecTree>
ERequiredUseKey::parse_value() const
{
    Context context("When parsing metadata key '" + raw_name() + "':");
    return parse_required_use(_imp->string_value, _imp->env, *_imp->eapi, _imp->is_installed);
}

const std::string
ERequiredUseKey::pretty_print_value(
        const PrettyPrinter & pretty_printer,
        const PrettyPrintOptions & options) const
{
    SpecTreePrettyPrinter p(pretty_printer, options);
    parse_value()->top()->accept(p);
    return stringify(p);
}

const std::string
ERequiredUseKey::raw_name() const
{
    return _imp->variable->name();
}

const std::string
ERequiredUseKey::human_name() const
{
    return _imp->variable->description();
}

MetadataKeyType
ERequiredUseKey::type() const
{
    return _imp->type;
}

namespace
{
    Timestamp get_mtime(const FSPath & v)
    {
        try
        {
            FSStat s(v);
            if (s.exists())
                return s.mtim();
            else
            {
                Log::get_instance()->message("e.contents.mtime_failure", ll_warning, lc_context) << "Couldn't get mtime for '"
                    << v << "' because it does not exist";
                return Timestamp::now();
            }
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.contents.mtime_failure", ll_warning, lc_context) << "Couldn't get mtime for '"
                << v << "' due to exception '" << e.message() << "' (" << e.what() << ")";
            return Timestamp::now();
        }
    }
}

namespace paludis
{
    template <>
    struct Imp<EMTimeKey>
    {
        Timestamp value;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Imp(const FSPath & v,
                const std::string & r, const std::string & h, const MetadataKeyType t) :
            value(get_mtime(v)),
            raw_name(r),
            human_name(h),
            type(t)
        {
        }
    };
}

EMTimeKey::EMTimeKey(const std::string & r, const std::string & h, const FSPath & v, const MetadataKeyType t) :
    _imp(v, r, h, t)
{
}

EMTimeKey::~EMTimeKey() = default;

Timestamp
EMTimeKey::parse_value() const
{
    return _imp->value;
}

const std::string
EMTimeKey::raw_name() const
{
    return _imp->raw_name;
}

const std::string
EMTimeKey::human_name() const
{
    return _imp->human_name;
}

MetadataKeyType
EMTimeKey::type() const
{
    return _imp->type;
}

