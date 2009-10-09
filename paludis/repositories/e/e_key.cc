/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/repositories/e/vdb_contents_tokeniser.hh>
#include <paludis/repositories/e/e_repository_profile.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/myoption.hh>

#include <paludis/util/pretty_print.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/join.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/destringify.hh>

#include <paludis/contents.hh>
#include <paludis/repository.hh>
#include <paludis/environment.hh>
#include <paludis/stringify_formatter-impl.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/literal_metadata_key.hh>

#include <tr1/functional>
#include <list>
#include <vector>

using namespace paludis;
using namespace paludis::erepository;

EMutableRepositoryMaskInfoKey::EMutableRepositoryMaskInfoKey(const std::tr1::shared_ptr<const ERepositoryID> &,
        const std::string & r, const std::string & h, const std::tr1::shared_ptr<const RepositoryMaskInfo> & v, const MetadataKeyType t) :
    _value(v),
    _r(r),
    _h(h),
    _t(t)
{
}

EMutableRepositoryMaskInfoKey::~EMutableRepositoryMaskInfoKey()
{
}

const std::tr1::shared_ptr<const RepositoryMaskInfo>
EMutableRepositoryMaskInfoKey::value() const
{
    return _value;
}

void
EMutableRepositoryMaskInfoKey::set_value(const std::tr1::shared_ptr<const RepositoryMaskInfo> & v)
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
    struct Implementation<EDependenciesKey>
    {
        const Environment * const env;
        const std::tr1::shared_ptr<const ERepositoryID> id;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable std::tr1::shared_ptr<const DependencySpecTree> value;
        const std::tr1::shared_ptr<const DependenciesLabelSequence> labels;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Implementation(
                const Environment * const e,
                const std::tr1::shared_ptr<const ERepositoryID> & i, const std::string & v,
                const std::tr1::shared_ptr<const DependenciesLabelSequence> & s,
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
        const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const std::string & v,
        const std::tr1::shared_ptr<const DependenciesLabelSequence> & l, const MetadataKeyType t) :
    PrivateImplementationPattern<EDependenciesKey>(new Implementation<EDependenciesKey>(e, id, v, l, r, h, t))
{
}

EDependenciesKey::~EDependenciesKey()
{
}

const std::tr1::shared_ptr<const DependencySpecTree>
EDependenciesKey::value() const
{
    Lock l(_imp->value_mutex);
    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = parse_depend(_imp->string_value, _imp->env, _imp->id, *_imp->id->eapi());
    return _imp->value;
}

const std::tr1::shared_ptr<const DependenciesLabelSequence>
EDependenciesKey::initial_labels() const
{
    return _imp->labels;
}

std::string
EDependenciesKey::pretty_print(const DependencySpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, true, true);
    value()->root()->accept(p);
    return stringify(p);
}

std::string
EDependenciesKey::pretty_print_flat(const DependencySpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, false, true);
    value()->root()->accept(p);
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
    struct Implementation<ELicenseKey>
    {
        const Environment * const env;
        const std::tr1::shared_ptr<const ERepositoryID> id;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable std::tr1::shared_ptr<const LicenseSpecTree> value;

        const std::tr1::shared_ptr<const EAPIMetadataVariable> variable;
        const MetadataKeyType type;

        Implementation(const Environment * const e,
                const std::tr1::shared_ptr<const ERepositoryID> & i, const std::string & v,
                const std::tr1::shared_ptr<const EAPIMetadataVariable> & m, const MetadataKeyType t) :
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
        const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::tr1::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v, const MetadataKeyType t) :
    PrivateImplementationPattern<ELicenseKey>(new Implementation<ELicenseKey>(e, id, v, m, t))
{
}

ELicenseKey::~ELicenseKey()
{
}

const std::tr1::shared_ptr<const LicenseSpecTree>
ELicenseKey::value() const
{
    Lock l(_imp->value_mutex);
    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = parse_license(_imp->string_value, _imp->env, _imp->id, *_imp->id->eapi());
    return _imp->value;
}

std::string
ELicenseKey::pretty_print(const LicenseSpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, true, true);
    value()->root()->accept(p);
    return stringify(p);
}

std::string
ELicenseKey::pretty_print_flat(const LicenseSpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, false, true);
    value()->root()->accept(p);
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
    struct Implementation<EFetchableURIKey>
    {
        const Environment * const env;
        const std::tr1::shared_ptr<const ERepositoryID> id;
        const std::tr1::shared_ptr<const EAPIMetadataVariable> variable;
        const std::string string_value;
        const MetadataKeyType type;

        mutable Mutex value_mutex;
        mutable std::tr1::shared_ptr<const FetchableURISpecTree> value;
        mutable std::tr1::shared_ptr<const URILabel> initial_label;

        Implementation(const Environment * const e, const std::tr1::shared_ptr<const ERepositoryID> & i,
                const std::tr1::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v,
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
        const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::tr1::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v, const MetadataKeyType t) :
    PrivateImplementationPattern<EFetchableURIKey>(new Implementation<EFetchableURIKey>(e, id, m, v, t))
{
}

EFetchableURIKey::~EFetchableURIKey()
{
}

const std::tr1::shared_ptr<const FetchableURISpecTree>
EFetchableURIKey::value() const
{
    Lock l(_imp->value_mutex);

    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = parse_fetchable_uri(_imp->string_value, _imp->env, _imp->id, *_imp->id->eapi());
    return _imp->value;
}

std::string
EFetchableURIKey::pretty_print(const FetchableURISpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, true, true);
    value()->root()->accept(p);
    return stringify(p);
}

std::string
EFetchableURIKey::pretty_print_flat(const FetchableURISpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, false, true);
    value()->root()->accept(p);
    return stringify(p);
}

const std::tr1::shared_ptr<const URILabel>
EFetchableURIKey::initial_label() const
{
    Lock l(_imp->value_mutex);

    if (! _imp->initial_label)
    {
        DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec> f(_imp->env);
        if (_imp->id->restrict_key())
            _imp->id->restrict_key()->value()->root()->accept(f);
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
    struct Implementation<ESimpleURIKey>
    {
        const Environment * const env;
        const std::tr1::shared_ptr<const ERepositoryID> id;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable std::tr1::shared_ptr<const SimpleURISpecTree> value;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Implementation(const Environment * const e, const std::tr1::shared_ptr<const ERepositoryID> & i,
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
        const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    PrivateImplementationPattern<ESimpleURIKey>(new Implementation<ESimpleURIKey>(e, id, v, r, h, t))
{
}

ESimpleURIKey::~ESimpleURIKey()
{
}

const std::tr1::shared_ptr<const SimpleURISpecTree>
ESimpleURIKey::value() const
{
    Lock l(_imp->value_mutex);

    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = parse_simple_uri(_imp->string_value, _imp->env, _imp->id, *_imp->id->eapi());
    return _imp->value;
}

std::string
ESimpleURIKey::pretty_print(const SimpleURISpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, true, true);
    value()->root()->accept(p);
    return stringify(p);
}

std::string
ESimpleURIKey::pretty_print_flat(const SimpleURISpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, false, true);
    value()->root()->accept(p);
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
    struct Implementation<EPlainTextSpecKey>
    {
        const Environment * const env;
        const std::tr1::shared_ptr<const ERepositoryID> id;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable std::tr1::shared_ptr<const PlainTextSpecTree> value;

        const std::tr1::shared_ptr<const EAPIMetadataVariable> variable;
        const MetadataKeyType type;

        Implementation(const Environment * const e, const std::tr1::shared_ptr<const ERepositoryID> & i, const std::string & v,
                const std::tr1::shared_ptr<const EAPIMetadataVariable> & m,
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
        const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::tr1::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v, const MetadataKeyType t) :
    PrivateImplementationPattern<EPlainTextSpecKey>(new Implementation<EPlainTextSpecKey>(e, id, v, m, t))
{
}

EPlainTextSpecKey::~EPlainTextSpecKey()
{
}

const std::tr1::shared_ptr<const PlainTextSpecTree>
EPlainTextSpecKey::value() const
{
    Lock l(_imp->value_mutex);

    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = parse_plain_text(_imp->string_value, _imp->env, _imp->id, *_imp->id->eapi());
    return _imp->value;
}

std::string
EPlainTextSpecKey::pretty_print(const PlainTextSpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, true, true);
    value()->root()->accept(p);
    return stringify(p);
}

std::string
EPlainTextSpecKey::pretty_print_flat(const PlainTextSpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, false, true);
    value()->root()->accept(p);
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
    struct Implementation<EMyOptionsKey>
    {
        const Environment * const env;
        const std::tr1::shared_ptr<const ERepositoryID> id;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable std::tr1::shared_ptr<const PlainTextSpecTree> value;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Implementation(const Environment * const e, const std::tr1::shared_ptr<const ERepositoryID> & i, const std::string & v,
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
        const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    PrivateImplementationPattern<EMyOptionsKey>(new Implementation<EMyOptionsKey>(e, id, v, r, h, t))
{
}

EMyOptionsKey::~EMyOptionsKey()
{
}

const std::tr1::shared_ptr<const PlainTextSpecTree>
EMyOptionsKey::value() const
{
    Lock l(_imp->value_mutex);

    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = parse_myoptions(_imp->string_value, _imp->env, _imp->id, *_imp->id->eapi());
    return _imp->value;
}

std::string
EMyOptionsKey::pretty_print(const PlainTextSpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, true, true);
    value()->root()->accept(p);
    return stringify(p);
}

std::string
EMyOptionsKey::pretty_print_flat(const PlainTextSpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, false, true);
    value()->root()->accept(p);
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
    struct Implementation<EProvideKey>
    {
        const Environment * const env;
        const std::tr1::shared_ptr<const ERepositoryID> id;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable std::tr1::shared_ptr<const ProvideSpecTree> value;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Implementation(const Environment * const e, const std::tr1::shared_ptr<const ERepositoryID> & i, const std::string & v,
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

EProvideKey::EProvideKey(const Environment * const e, const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    PrivateImplementationPattern<EProvideKey>(new Implementation<EProvideKey>(e, id, v, r, h, t))
{
}

EProvideKey::~EProvideKey()
{
}

const std::tr1::shared_ptr<const ProvideSpecTree>
EProvideKey::value() const
{
    Lock l(_imp->value_mutex);

    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = parse_provide(_imp->string_value, _imp->env, _imp->id, *_imp->id->eapi());
    return _imp->value;
}

std::string
EProvideKey::pretty_print(const ProvideSpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, true, true);
    value()->root()->accept(p);
    return stringify(p);
}

std::string
EProvideKey::pretty_print_flat(const ProvideSpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, false, true);
    value()->root()->accept(p);
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
    struct Implementation<EKeywordsKey>
    {
        const std::tr1::shared_ptr<const ERepositoryID> id;
        const Environment * const env;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable std::tr1::shared_ptr<KeywordNameSet> value;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Implementation(const std::tr1::shared_ptr<const ERepositoryID> & i, const Environment * const e, const std::string & v,
                const std::string & r, const std::string & h, const MetadataKeyType t) :
            id(i),
            env(e),
            string_value(v),
            raw_name(r),
            human_name(h),
            type(t)
        {
        }
    };
}

EKeywordsKey::EKeywordsKey(const Environment * const e, const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    PrivateImplementationPattern<EKeywordsKey>(new Implementation<EKeywordsKey>(id, e, v, r, h, t))
{
}

EKeywordsKey::~EKeywordsKey()
{
}

const std::tr1::shared_ptr<const KeywordNameSet>
EKeywordsKey::value() const
{
    Lock l(_imp->value_mutex);
    if (_imp->value)
        return _imp->value;

    _imp->value.reset(new KeywordNameSet);
    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    tokenise_whitespace(_imp->string_value, create_inserter<KeywordName>(_imp->value->inserter()));
    return _imp->value;
}

std::string
EKeywordsKey::pretty_print_flat(const Formatter<KeywordName> & f) const
{
    std::string result;
    for (KeywordNameSet::ConstIterator i(value()->begin()), i_end(value()->end()) ;
            i != i_end ; ++i)
    {
        if (! result.empty())
            result.append(" ");

        std::tr1::shared_ptr<KeywordNameSet> k(new KeywordNameSet);
        k->insert(*i);
        if (_imp->env->accept_keywords(k, *_imp->id))
            result.append(f.format(*i, format::Accepted()));
        else
            result.append(f.format(*i, format::Unaccepted()));
    }

    return result;
}

const std::string
EKeywordsKey::raw_name() const
{
    return _imp->raw_name;
}

const std::string
EKeywordsKey::human_name() const
{
    return _imp->human_name;
}

MetadataKeyType
EKeywordsKey::type() const
{
    return _imp->type;
}

namespace paludis
{
    template <>
    struct Implementation<EStringSetKey>
    {
        const std::tr1::shared_ptr<const ERepositoryID> id;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable std::tr1::shared_ptr<Set<std::string> > value;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Implementation(const std::tr1::shared_ptr<const ERepositoryID> & i, const std::string & v,
                const std::string & r, const std::string & h, const MetadataKeyType t) :
            id(i),
            string_value(v),
            raw_name(r),
            human_name(h),
            type(t)
        {
        }
    };
}

EStringSetKey::EStringSetKey(const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    PrivateImplementationPattern<EStringSetKey>(new Implementation<EStringSetKey>(id, v, r, h, t))
{
}

EStringSetKey::~EStringSetKey()
{
}

const std::tr1::shared_ptr<const Set<std::string> >
EStringSetKey::value() const
{
    Lock l(_imp->value_mutex);

    if (_imp->value)
        return _imp->value;

    _imp->value.reset(new Set<std::string>);
    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    tokenise_whitespace(_imp->string_value, _imp->value->inserter());
    return _imp->value;
}

const std::string
EStringSetKey::raw_name() const
{
    return _imp->raw_name;
}

const std::string
EStringSetKey::human_name() const
{
    return _imp->human_name;
}

MetadataKeyType
EStringSetKey::type() const
{
    return _imp->type;
}

namespace
{
    std::string format_string(const std::string & i, const Formatter<std::string> & f)
    {
        return f.format(i, format::Plain());
    }
}

std::string
EStringSetKey::pretty_print_flat(const Formatter<std::string> & f) const
{
    using namespace std::tr1::placeholders;
    return join(value()->begin(), value()->end(), " ", std::tr1::bind(&format_string, _1, f));
}

namespace paludis
{
    template <>
    struct Implementation<EContentsKey>
    {
        const std::tr1::shared_ptr<const ERepositoryID> id;
        const FSEntry filename;
        mutable Mutex value_mutex;
        mutable std::tr1::shared_ptr<Contents> value;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Implementation(const std::tr1::shared_ptr<const ERepositoryID> & i, const FSEntry & v,
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

EContentsKey::EContentsKey(const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const FSEntry & v, const MetadataKeyType t) :
    PrivateImplementationPattern<EContentsKey>(new Implementation<EContentsKey>(id, v, r, h, t))
{
}

EContentsKey::~EContentsKey()
{
}

const std::tr1::shared_ptr<const Contents>
EContentsKey::value() const
{
    Lock l(_imp->value_mutex);

    if (_imp->value)
        return _imp->value;

    Context context("When creating contents for VDB key '" + stringify(*_imp->id) + "' from '" + stringify(_imp->filename) + "':");

    _imp->value.reset(new Contents);

    FSEntry f(_imp->filename);
    if (! f.is_regular_file_or_symlink_to_regular_file())
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
            Log::get_instance()->message("e.contents.broken", ll_warning, lc_context) << "CONTENTS has broken line " <<
                line_number << ", skipping";
            continue;
        }

        if ("obj" == tokens.at(0))
        {
            std::tr1::shared_ptr<ContentsEntry> e(new ContentsFileEntry(tokens.at(1)));
            e->add_metadata_key(make_shared_ptr(new LiteralMetadataTimeKey("mtime", "mtime", mkt_normal, destringify<time_t>(tokens.at(3)))));
            e->add_metadata_key(make_shared_ptr(new LiteralMetadataValueKey<std::string>("md5", "md5", mkt_normal, tokens.at(2))));
            _imp->value->add(e);
        }
        else if ("dir" == tokens.at(0))
        {
            std::tr1::shared_ptr<ContentsEntry> e(new ContentsDirEntry(tokens.at(1)));
            _imp->value->add(e);
        }
        else if ("sym" == tokens.at(0))
        {
            std::tr1::shared_ptr<ContentsEntry> e(new ContentsSymEntry(tokens.at(1), tokens.at(2)));
            e->add_metadata_key(make_shared_ptr(new LiteralMetadataTimeKey("mtime", "mtime", mkt_normal, destringify<time_t>(tokens.at(3)))));
            _imp->value->add(e);
        }
        else if ("misc" == tokens.at(0) || "fif" == tokens.at(0) || "dev" == tokens.at(0))
            _imp->value->add(std::tr1::shared_ptr<ContentsEntry>(new ContentsOtherEntry(tokens.at(1))));
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
    struct Implementation<EMTimeKey>
    {
        const std::tr1::shared_ptr<const ERepositoryID> id;
        const FSEntry filename;
        mutable Mutex value_mutex;
        mutable std::tr1::shared_ptr<time_t> value;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Implementation(const std::tr1::shared_ptr<const ERepositoryID> & i, const FSEntry & v,
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

EMTimeKey::EMTimeKey(const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const FSEntry & v, const MetadataKeyType t) :
    PrivateImplementationPattern<EMTimeKey>(new Implementation<EMTimeKey>(id, v, r, h, t))
{
}

EMTimeKey::~EMTimeKey()
{
}

time_t
EMTimeKey::value() const
{
    Lock l(_imp->value_mutex);

    if (_imp->value)
        return *_imp->value;

    _imp->value.reset(new time_t(0));

    try
    {
        *_imp->value = _imp->filename.mtime();
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

namespace paludis
{
    template <>
    struct Implementation<ESlotKey>
    {
        const SlotName value;
        const std::tr1::shared_ptr<const EAPIMetadataVariable> variable;
        const MetadataKeyType type;

        Implementation(const SlotName & v, const std::tr1::shared_ptr<const EAPIMetadataVariable> & m, const MetadataKeyType t) :
            value(v),
            variable(m),
            type(t)
        {
        }
    };
}

ESlotKey::ESlotKey(const std::tr1::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v, const MetadataKeyType t) :
    PrivateImplementationPattern<ESlotKey>(new Implementation<ESlotKey>(SlotName(v), m, t))
{
}

ESlotKey::~ESlotKey()
{
}

const SlotName
ESlotKey::value() const
{
    return _imp->value;
}

const std::string
ESlotKey::raw_name() const
{
    return _imp->variable->name();
}

const std::string
ESlotKey::human_name() const
{
    return _imp->variable->description();
}

MetadataKeyType
ESlotKey::type() const
{
    return _imp->type;
}

