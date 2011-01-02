/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/vdb_contents_tokeniser.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/myoption.hh>
#include <paludis/repositories/e/spec_tree_pretty_printer.hh>

#include <paludis/util/pretty_print.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/log.hh>
#include <paludis/util/mutex.hh>
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
#include <list>
#include <vector>

using namespace paludis;
using namespace paludis::erepository;

EMutableRepositoryMaskInfoKey::EMutableRepositoryMaskInfoKey(const std::shared_ptr<const ERepositoryID> &,
        const std::string & r, const std::string & h, const std::shared_ptr<const RepositoryMaskInfo> & v, const MetadataKeyType t) :
    _value(v),
    _r(r),
    _h(h),
    _t(t)
{
}

EMutableRepositoryMaskInfoKey::~EMutableRepositoryMaskInfoKey()
{
}

const std::shared_ptr<const RepositoryMaskInfo>
EMutableRepositoryMaskInfoKey::value() const
{
    return _value;
}

void
EMutableRepositoryMaskInfoKey::set_value(const std::shared_ptr<const RepositoryMaskInfo> & v)
{
    _value = v;
}

const std::string
EMutableRepositoryMaskInfoKey::raw_name() const
{
    return _r;
}

const std::string
EMutableRepositoryMaskInfoKey::human_name() const
{
    return _h;
}

MetadataKeyType
EMutableRepositoryMaskInfoKey::type() const
{
    return _t;
}

namespace paludis
{
    template <>
    struct Imp<EDependenciesKey>
    {
        const Environment * const env;
        const std::shared_ptr<const ERepositoryID> id;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable std::shared_ptr<const DependencySpecTree> value;
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
    Pimp<EDependenciesKey>(e, id, v, l, r, h, t)
{
}

EDependenciesKey::~EDependenciesKey()
{
}

const std::shared_ptr<const DependencySpecTree>
EDependenciesKey::value() const
{
    Lock l(_imp->value_mutex);
    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = parse_depend(_imp->string_value, _imp->env, _imp->id, *_imp->id->eapi());
    return _imp->value;
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
    value()->top()->accept(p);
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
        const std::shared_ptr<const ERepositoryID> id;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable std::shared_ptr<const LicenseSpecTree> value;

        const std::shared_ptr<const EAPIMetadataVariable> variable;
        const MetadataKeyType type;

        Imp(const Environment * const e,
                const std::shared_ptr<const ERepositoryID> & i, const std::string & v,
                const std::shared_ptr<const EAPIMetadataVariable> & m, const MetadataKeyType t) :
            env(e),
            id(i),
            string_value(v),
            variable(m),
            type(t)
        {
        }
    };
}

ELicenseKey::ELicenseKey(
        const Environment * const e,
        const std::shared_ptr<const ERepositoryID> & id,
        const std::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v, const MetadataKeyType t) :
    Pimp<ELicenseKey>(e, id, v, m, t)
{
}

ELicenseKey::~ELicenseKey()
{
}

const std::shared_ptr<const LicenseSpecTree>
ELicenseKey::value() const
{
    Lock l(_imp->value_mutex);
    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = parse_license(_imp->string_value, _imp->env, _imp->id, *_imp->id->eapi());
    return _imp->value;
}

const std::string
ELicenseKey::pretty_print_value(
        const PrettyPrinter & pretty_printer,
        const PrettyPrintOptions & options) const
{
    SpecTreePrettyPrinter p(pretty_printer, options);
    value()->top()->accept(p);
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

        mutable Mutex value_mutex;
        mutable std::shared_ptr<const FetchableURISpecTree> value;
        mutable std::shared_ptr<const URILabel> initial_label;

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
    Pimp<EFetchableURIKey>(e, id, m, v, t)
{
}

EFetchableURIKey::~EFetchableURIKey()
{
}

const std::shared_ptr<const FetchableURISpecTree>
EFetchableURIKey::value() const
{
    Lock l(_imp->value_mutex);

    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = parse_fetchable_uri(_imp->string_value, _imp->env, _imp->id, *_imp->id->eapi());
    return _imp->value;
}

const std::string
EFetchableURIKey::pretty_print_value(
        const PrettyPrinter & pretty_printer,
        const PrettyPrintOptions & options) const
{
    SpecTreePrettyPrinter p(pretty_printer, options);
    value()->top()->accept(p);
    return stringify(p);
}

const std::shared_ptr<const URILabel>
EFetchableURIKey::initial_label() const
{
    Lock l(_imp->value_mutex);

    if (! _imp->initial_label)
    {
        DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec> f(_imp->env, _imp->id);
        if (_imp->id->restrict_key())
            _imp->id->restrict_key()->value()->top()->accept(f);
        for (DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec>::ConstIterator i(f.begin()), i_end(f.end()) ;
                i != i_end ; ++i)
        {
            if (_imp->id->eapi()->supported()->ebuild_options()->restrict_fetch()->end() !=
                    std::find(_imp->id->eapi()->supported()->ebuild_options()->restrict_fetch()->begin(),
                        _imp->id->eapi()->supported()->ebuild_options()->restrict_fetch()->end(), (*i)->text()))
                _imp->initial_label = *parse_uri_label("default-restrict-fetch:", *_imp->id->eapi())->begin();

            else if (_imp->id->eapi()->supported()->ebuild_options()->restrict_mirror()->end() !=
                    std::find(_imp->id->eapi()->supported()->ebuild_options()->restrict_mirror()->begin(),
                        _imp->id->eapi()->supported()->ebuild_options()->restrict_mirror()->end(), (*i)->text()))
                _imp->initial_label = *parse_uri_label("default-restrict-mirror:", *_imp->id->eapi())->begin();

            else if (_imp->id->eapi()->supported()->ebuild_options()->restrict_primaryuri()->end() !=
                    std::find(_imp->id->eapi()->supported()->ebuild_options()->restrict_primaryuri()->begin(),
                        _imp->id->eapi()->supported()->ebuild_options()->restrict_primaryuri()->end(), (*i)->text()))
                _imp->initial_label = *parse_uri_label("default-restrict-primaryuri:", *_imp->id->eapi())->begin();
        }

        if (! _imp->initial_label)
            _imp->initial_label = *parse_uri_label("default:", *_imp->id->eapi())->begin();
    }

    return _imp->initial_label;
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
        const std::shared_ptr<const ERepositoryID> id;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable std::shared_ptr<const SimpleURISpecTree> value;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Imp(const Environment * const e, const std::shared_ptr<const ERepositoryID> & i,
                const std::string & v,
                const std::string & r, const std::string & h, const MetadataKeyType t) :
            env(e),
            id(i),
            string_value(v),
            raw_name(r),
            human_name(h),
            type(t)
        {
        }
    };
}

ESimpleURIKey::ESimpleURIKey(const Environment * const e,
        const std::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    Pimp<ESimpleURIKey>(e, id, v, r, h, t)
{
}

ESimpleURIKey::~ESimpleURIKey()
{
}

const std::shared_ptr<const SimpleURISpecTree>
ESimpleURIKey::value() const
{
    Lock l(_imp->value_mutex);

    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = parse_simple_uri(_imp->string_value, _imp->env, _imp->id, *_imp->id->eapi());
    return _imp->value;
}

const std::string
ESimpleURIKey::pretty_print_value(
        const PrettyPrinter & pretty_printer,
        const PrettyPrintOptions & options) const
{
    SpecTreePrettyPrinter p(pretty_printer, options);
    value()->top()->accept(p);
    return stringify(p);
}

const std::string
ESimpleURIKey::raw_name() const
{
    return _imp->raw_name;
}

const std::string
ESimpleURIKey::human_name() const
{
    return _imp->human_name;
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
        const std::shared_ptr<const ERepositoryID> id;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable std::shared_ptr<const PlainTextSpecTree> value;

        const std::shared_ptr<const EAPIMetadataVariable> variable;
        const MetadataKeyType type;

        Imp(const Environment * const e, const std::shared_ptr<const ERepositoryID> & i, const std::string & v,
                const std::shared_ptr<const EAPIMetadataVariable> & m,
                const MetadataKeyType t) :
            env(e),
            id(i),
            string_value(v),
            variable(m),
            type(t)
        {
        }
    };
}

EPlainTextSpecKey::EPlainTextSpecKey(const Environment * const e,
        const std::shared_ptr<const ERepositoryID> & id,
        const std::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v, const MetadataKeyType t) :
    Pimp<EPlainTextSpecKey>(e, id, v, m, t)
{
}

EPlainTextSpecKey::~EPlainTextSpecKey()
{
}

const std::shared_ptr<const PlainTextSpecTree>
EPlainTextSpecKey::value() const
{
    Lock l(_imp->value_mutex);

    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = parse_plain_text(_imp->string_value, _imp->env, _imp->id, *_imp->id->eapi());
    return _imp->value;
}

const std::string
EPlainTextSpecKey::pretty_print_value(
        const PrettyPrinter & pretty_printer,
        const PrettyPrintOptions & options) const
{
    SpecTreePrettyPrinter p(pretty_printer, options);
    value()->top()->accept(p);
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
        const std::shared_ptr<const ERepositoryID> id;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable std::shared_ptr<const PlainTextSpecTree> value;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Imp(const Environment * const e, const std::shared_ptr<const ERepositoryID> & i, const std::string & v,
                const std::string & r, const std::string & h, const MetadataKeyType t) :
            env(e),
            id(i),
            string_value(v),
            raw_name(r),
            human_name(h),
            type(t)
        {
        }
    };
}

EMyOptionsKey::EMyOptionsKey(const Environment * const e,
        const std::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    Pimp<EMyOptionsKey>(e, id, v, r, h, t)
{
}

EMyOptionsKey::~EMyOptionsKey()
{
}

const std::shared_ptr<const PlainTextSpecTree>
EMyOptionsKey::value() const
{
    Lock l(_imp->value_mutex);

    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = parse_myoptions(_imp->string_value, _imp->env, _imp->id, *_imp->id->eapi());
    return _imp->value;
}

const std::string
EMyOptionsKey::pretty_print_value(
        const PrettyPrinter & pretty_printer,
        const PrettyPrintOptions & options) const
{
    SpecTreePrettyPrinter p(pretty_printer, options);
    value()->top()->accept(p);
    return stringify(p);
}

const std::string
EMyOptionsKey::raw_name() const
{
    return _imp->raw_name;
}

const std::string
EMyOptionsKey::human_name() const
{
    return _imp->human_name;
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
        const std::shared_ptr<const ERepositoryID> id;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable std::shared_ptr<const RequiredUseSpecTree> value;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Imp(const Environment * const e, const std::shared_ptr<const ERepositoryID> & i, const std::string & v,
                const std::string & r, const std::string & h, const MetadataKeyType t) :
            env(e),
            id(i),
            string_value(v),
            raw_name(r),
            human_name(h),
            type(t)
        {
        }
    };
}

ERequiredUseKey::ERequiredUseKey(const Environment * const e,
        const std::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    Pimp<ERequiredUseKey>(e, id, v, r, h, t)
{
}

ERequiredUseKey::~ERequiredUseKey()
{
}

const std::shared_ptr<const RequiredUseSpecTree>
ERequiredUseKey::value() const
{
    Lock l(_imp->value_mutex);

    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = parse_required_use(_imp->string_value, _imp->env, _imp->id, *_imp->id->eapi());
    return _imp->value;
}

const std::string
ERequiredUseKey::pretty_print_value(
        const PrettyPrinter & pretty_printer,
        const PrettyPrintOptions & options) const
{
    SpecTreePrettyPrinter p(pretty_printer, options);
    value()->top()->accept(p);
    return stringify(p);
}

const std::string
ERequiredUseKey::raw_name() const
{
    return _imp->raw_name;
}

const std::string
ERequiredUseKey::human_name() const
{
    return _imp->human_name;
}

MetadataKeyType
ERequiredUseKey::type() const
{
    return _imp->type;
}

namespace paludis
{
    template <>
    struct Imp<EProvideKey>
    {
        const Environment * const env;
        const std::shared_ptr<const ERepositoryID> id;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable std::shared_ptr<const ProvideSpecTree> value;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Imp(const Environment * const e, const std::shared_ptr<const ERepositoryID> & i, const std::string & v,
                const std::string & r, const std::string & h, const MetadataKeyType t) :
            env(e),
            id(i),
            string_value(v),
            raw_name(r),
            human_name(h),
            type(t)
        {
        }
    };
}

EProvideKey::EProvideKey(const Environment * const e, const std::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    Pimp<EProvideKey>(e, id, v, r, h, t)
{
}

EProvideKey::~EProvideKey()
{
}

const std::shared_ptr<const ProvideSpecTree>
EProvideKey::value() const
{
    Lock l(_imp->value_mutex);

    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = parse_provide(_imp->string_value, _imp->env, _imp->id, *_imp->id->eapi());
    return _imp->value;
}

const std::string
EProvideKey::pretty_print_value(
        const PrettyPrinter & pretty_printer,
        const PrettyPrintOptions & options) const
{
    SpecTreePrettyPrinter p(pretty_printer, options);
    value()->top()->accept(p);
    return stringify(p);
}

const std::string
EProvideKey::raw_name() const
{
    return _imp->raw_name;
}

const std::string
EProvideKey::human_name() const
{
    return _imp->human_name;
}

MetadataKeyType
EProvideKey::type() const
{
    return _imp->type;
}

namespace paludis
{
    template <>
    struct Imp<EContentsKey>
    {
        const std::shared_ptr<const ERepositoryID> id;
        const FSPath filename;
        mutable Mutex value_mutex;
        mutable std::shared_ptr<Contents> value;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Imp(const std::shared_ptr<const ERepositoryID> & i, const FSPath & v,
                const std::string & r, const std::string & h, const MetadataKeyType & t) :
            id(i),
            filename(v),
            raw_name(r),
            human_name(h),
            type(t)
        {
        }
    };
}

EContentsKey::EContentsKey(const std::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const FSPath & v, const MetadataKeyType t) :
    Pimp<EContentsKey>(id, v, r, h, t)
{
}

EContentsKey::~EContentsKey()
{
}

const std::shared_ptr<const Contents>
EContentsKey::value() const
{
    Lock l(_imp->value_mutex);

    if (_imp->value)
        return _imp->value;

    Context context("When creating contents for VDB key '" + stringify(*_imp->id) + "' from '" + stringify(_imp->filename) + "':");

    _imp->value = std::make_shared<Contents>();

    FSPath f(_imp->filename);
    if (! f.stat().is_regular_file_or_symlink_to_regular_file())
    {
        Log::get_instance()->message("e.contents.not_a_file", ll_warning, lc_context) << "CONTENTS lookup failed for request for '" <<
                *_imp->id << "' using '" << _imp->filename << "'";
        return _imp->value;
    }

    SafeIFStream ff(f);

    std::string line;
    unsigned line_number(0);
    while (std::getline(ff, line))
    {
        ++line_number;

        std::vector<std::string> tokens;
        if (! VDBContentsTokeniser::tokenise(line, std::back_inserter(tokens)))
        {
            Log::get_instance()->message("e.contents.broken", ll_warning, lc_context) << "CONTENTS has broken line '" <<
                line_number << "', skipping";
            continue;
        }

        if ("obj" == tokens.at(0))
        {
            std::shared_ptr<ContentsEntry> e(std::make_shared<ContentsFileEntry>(FSPath(tokens.at(1))));
            e->add_metadata_key(std::make_shared<LiteralMetadataTimeKey>("mtime", "mtime", mkt_normal,
                            Timestamp(destringify<time_t>(tokens.at(3)), 0)));
            e->add_metadata_key(std::make_shared<LiteralMetadataValueKey<std::string>>("md5", "md5", mkt_normal, tokens.at(2)));
            _imp->value->add(e);
        }
        else if ("dir" == tokens.at(0))
        {
            std::shared_ptr<ContentsEntry> e(std::make_shared<ContentsDirEntry>(FSPath(tokens.at(1))));
            _imp->value->add(e);
        }
        else if ("sym" == tokens.at(0))
        {
            std::shared_ptr<ContentsEntry> e(std::make_shared<ContentsSymEntry>(FSPath(tokens.at(1)), tokens.at(2)));
            e->add_metadata_key(std::make_shared<LiteralMetadataTimeKey>("mtime", "mtime", mkt_normal,
                            Timestamp(destringify<time_t>(tokens.at(3)), 0)));
            _imp->value->add(e);
        }
        else if ("misc" == tokens.at(0) || "fif" == tokens.at(0) || "dev" == tokens.at(0))
            _imp->value->add(std::shared_ptr<ContentsEntry>(std::make_shared<ContentsOtherEntry>(FSPath(tokens.at(1)))));
        else
            Log::get_instance()->message("e.contents.unknown", ll_warning, lc_context) << "CONTENTS has unsupported entry type '" <<
                tokens.at(0) << "', skipping";
    }

    return _imp->value;
}

const std::string
EContentsKey::raw_name() const
{
    return _imp->raw_name;
}

const std::string
EContentsKey::human_name() const
{
    return _imp->human_name;
}

MetadataKeyType
EContentsKey::type() const
{
    return _imp->type;
}

namespace paludis
{
    template <>
    struct Imp<EMTimeKey>
    {
        const std::shared_ptr<const ERepositoryID> id;
        const FSPath filename;
        mutable Mutex value_mutex;
        mutable std::shared_ptr<Timestamp> value;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Imp(const std::shared_ptr<const ERepositoryID> & i, const FSPath & v,
                const std::string & r, const std::string & h, const MetadataKeyType t) :
            id(i),
            filename(v),
            raw_name(r),
            human_name(h),
            type(t)
        {
        }
    };
}

EMTimeKey::EMTimeKey(const std::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const FSPath & v, const MetadataKeyType t) :
    Pimp<EMTimeKey>(id, v, r, h, t)
{
}

EMTimeKey::~EMTimeKey()
{
}

Timestamp
EMTimeKey::value() const
{
    Lock l(_imp->value_mutex);

    if (_imp->value)
        return *_imp->value;

    _imp->value = std::make_shared<Timestamp>(Timestamp::now());

    try
    {
        *_imp->value = _imp->filename.stat().mtim();
    }
    catch (const FSError & e)
    {
        Log::get_instance()->message("e.contents.mtime_failure", ll_warning, lc_context) << "Couldn't get mtime for '"
            << _imp->filename << "' for ID '" << *_imp->id << "' due to exception '" << e.message()
            << "' (" << e.what() << ")";
    }

    return *_imp->value;
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

