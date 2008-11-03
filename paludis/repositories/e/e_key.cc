/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#include <paludis/util/pretty_print.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/set.hh>
#include <paludis/util/join.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/member_iterator-impl.hh>

#include <paludis/contents.hh>
#include <paludis/repository.hh>
#include <paludis/environment.hh>
#include <paludis/stringify_formatter-impl.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/choice.hh>
#include <paludis/elike_choices.hh>

#include <tr1/functional>
#include <list>
#include <vector>
#include <fstream>
#include <set>
#include <map>

using namespace paludis;
using namespace paludis::erepository;

EMutableRepositoryMaskInfoKey::EMutableRepositoryMaskInfoKey(const std::tr1::shared_ptr<const ERepositoryID> &,
        const std::string & r, const std::string & h, const std::tr1::shared_ptr<const RepositoryMaskInfo> & v, const MetadataKeyType t) :
    MetadataValueKey<std::tr1::shared_ptr<const RepositoryMaskInfo> > (r, h, t),
    _value(v)
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

namespace paludis
{
    template <>
    struct Implementation<EDependenciesKey>
    {
        const Environment * const env;
        const std::tr1::shared_ptr<const ERepositoryID> id;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable std::tr1::shared_ptr<const DependencySpecTree::ConstItem> value;
        mutable std::tr1::function<void () throw ()> value_used;
        const std::tr1::shared_ptr<const DependencyLabelSequence> labels;

        Implementation(
                const Environment * const e,
                const std::tr1::shared_ptr<const ERepositoryID> & i, const std::string & v,
                const std::tr1::shared_ptr<const DependencyLabelSequence> & s) :
            env(e),
            id(i),
            string_value(v),
            labels(s)
        {
        }
    };
}

EDependenciesKey::EDependenciesKey(
        const Environment * const e,
        const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const std::string & v,
        const std::tr1::shared_ptr<const DependencyLabelSequence> & l, const MetadataKeyType t) :
    MetadataSpecTreeKey<DependencySpecTree>(r, h, t),
    PrivateImplementationPattern<EDependenciesKey>(new Implementation<EDependenciesKey>(e, id, v, l)),
    _imp(PrivateImplementationPattern<EDependenciesKey>::_imp)
{
}

EDependenciesKey::~EDependenciesKey()
{
}

const std::tr1::shared_ptr<const DependencySpecTree::ConstItem>
EDependenciesKey::value() const
{
    Lock l(_imp->value_mutex);
    if (_imp->value)
    {
        if (_imp->value_used)
        {
            _imp->value_used();
            _imp->value_used = std::tr1::function<void () throw ()>();
        }
        return _imp->value;
    }

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = parse_depend(_imp->string_value, _imp->env, _imp->id, *_imp->id->eapi());
    return _imp->value;
}

const std::tr1::shared_ptr<const DependencyLabelSequence>
EDependenciesKey::initial_labels() const
{
    return _imp->labels;
}

std::string
EDependenciesKey::pretty_print(const DependencySpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, true, true);
    value()->accept(p);
    return stringify(p);
}

std::string
EDependenciesKey::pretty_print_flat(const DependencySpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, false, true);
    value()->accept(p);
    return stringify(p);
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
        mutable std::tr1::shared_ptr<const LicenseSpecTree::ConstItem> value;
        mutable std::tr1::function<void () throw ()> value_used;

        Implementation(const Environment * const e,
                const std::tr1::shared_ptr<const ERepositoryID> & i, const std::string & v) :
            env(e),
            id(i),
            string_value(v)
        {
        }
    };
}

ELicenseKey::ELicenseKey(
        const Environment * const e,
        const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataSpecTreeKey<LicenseSpecTree>(r, h, t),
    PrivateImplementationPattern<ELicenseKey>(new Implementation<ELicenseKey>(e, id, v)),
    _imp(PrivateImplementationPattern<ELicenseKey>::_imp)
{
}

ELicenseKey::~ELicenseKey()
{
}

const std::tr1::shared_ptr<const LicenseSpecTree::ConstItem>
ELicenseKey::value() const
{
    Lock l(_imp->value_mutex);
    if (_imp->value)
    {
        if (_imp->value_used)
        {
            _imp->value_used();
            _imp->value_used = std::tr1::function<void () throw ()>();
        }
        return _imp->value;
    }

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = parse_license(_imp->string_value, _imp->env, _imp->id, *_imp->id->eapi());
    return _imp->value;
}

std::string
ELicenseKey::pretty_print(const LicenseSpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, true, true);
    value()->accept(p);
    return stringify(p);
}

std::string
ELicenseKey::pretty_print_flat(const LicenseSpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, false, true);
    value()->accept(p);
    return stringify(p);
}

namespace paludis
{
    template <>
    struct Implementation<EFetchableURIKey>
    {
        const Environment * const env;
        const std::tr1::shared_ptr<const ERepositoryID> id;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable std::tr1::shared_ptr<const FetchableURISpecTree::ConstItem> value;
        mutable std::tr1::shared_ptr<const URILabel> initial_label;

        Implementation(const Environment * const e, const std::tr1::shared_ptr<const ERepositoryID> & i, const std::string & v) :
            env(e),
            id(i),
            string_value(v)
        {
        }
    };
}

EFetchableURIKey::EFetchableURIKey(const Environment * const e,
        const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataSpecTreeKey<FetchableURISpecTree>(r, h, t),
    PrivateImplementationPattern<EFetchableURIKey>(new Implementation<EFetchableURIKey>(e, id, v)),
    _imp(PrivateImplementationPattern<EFetchableURIKey>::_imp)
{
}

EFetchableURIKey::~EFetchableURIKey()
{
}

const std::tr1::shared_ptr<const FetchableURISpecTree::ConstItem>
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
    value()->accept(p);
    return stringify(p);
}

std::string
EFetchableURIKey::pretty_print_flat(const FetchableURISpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, false, true);
    value()->accept(p);
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
            _imp->id->restrict_key()->value()->accept(f);
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

namespace paludis
{
    template <>
    struct Implementation<ESimpleURIKey>
    {
        const Environment * const env;
        const std::tr1::shared_ptr<const ERepositoryID> id;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable std::tr1::shared_ptr<const SimpleURISpecTree::ConstItem> value;

        Implementation(const Environment * const e, const std::tr1::shared_ptr<const ERepositoryID> & i, const std::string & v) :
            env(e),
            id(i),
            string_value(v)
        {
        }
    };
}

ESimpleURIKey::ESimpleURIKey(const Environment * const e,
        const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataSpecTreeKey<SimpleURISpecTree>(r, h, t),
    PrivateImplementationPattern<ESimpleURIKey>(new Implementation<ESimpleURIKey>(e, id, v)),
    _imp(PrivateImplementationPattern<ESimpleURIKey>::_imp)
{
}

ESimpleURIKey::~ESimpleURIKey()
{
}

const std::tr1::shared_ptr<const SimpleURISpecTree::ConstItem>
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
    value()->accept(p);
    return stringify(p);
}

std::string
ESimpleURIKey::pretty_print_flat(const SimpleURISpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, false, true);
    value()->accept(p);
    return stringify(p);
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
        mutable std::tr1::shared_ptr<const PlainTextSpecTree::ConstItem> value;

        Implementation(const Environment * const e, const std::tr1::shared_ptr<const ERepositoryID> & i, const std::string & v) :
            env(e),
            id(i),
            string_value(v)
        {
        }
    };
}

EPlainTextSpecKey::EPlainTextSpecKey(const Environment * const e,
        const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataSpecTreeKey<PlainTextSpecTree>(r, h, t),
    PrivateImplementationPattern<EPlainTextSpecKey>(new Implementation<EPlainTextSpecKey>(e, id, v)),
    _imp(PrivateImplementationPattern<EPlainTextSpecKey>::_imp)
{
}

EPlainTextSpecKey::~EPlainTextSpecKey()
{
}

const std::tr1::shared_ptr<const PlainTextSpecTree::ConstItem>
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
    value()->accept(p);
    return stringify(p);
}

std::string
EPlainTextSpecKey::pretty_print_flat(const PlainTextSpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, false, true);
    value()->accept(p);
    return stringify(p);
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
        mutable std::tr1::shared_ptr<const PlainTextSpecTree::ConstItem> value;

        Implementation(const Environment * const e, const std::tr1::shared_ptr<const ERepositoryID> & i, const std::string & v) :
            env(e),
            id(i),
            string_value(v)
        {
        }
    };
}

EMyOptionsKey::EMyOptionsKey(const Environment * const e,
        const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataSpecTreeKey<PlainTextSpecTree>(r, h, t),
    PrivateImplementationPattern<EMyOptionsKey>(new Implementation<EMyOptionsKey>(e, id, v)),
    _imp(PrivateImplementationPattern<EMyOptionsKey>::_imp)
{
}

EMyOptionsKey::~EMyOptionsKey()
{
}

const std::tr1::shared_ptr<const PlainTextSpecTree::ConstItem>
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
    value()->accept(p);
    return stringify(p);
}

std::string
EMyOptionsKey::pretty_print_flat(const PlainTextSpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, false, true);
    value()->accept(p);
    return stringify(p);
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
        mutable std::tr1::shared_ptr<const ProvideSpecTree::ConstItem> value;

        Implementation(const Environment * const e, const std::tr1::shared_ptr<const ERepositoryID> & i, const std::string & v) :
            env(e),
            id(i),
            string_value(v)
        {
        }
    };
}

EProvideKey::EProvideKey(const Environment * const e, const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataSpecTreeKey<ProvideSpecTree>(r, h, t),
    PrivateImplementationPattern<EProvideKey>(new Implementation<EProvideKey>(e, id, v)),
    _imp(PrivateImplementationPattern<EProvideKey>::_imp)
{
}

EProvideKey::~EProvideKey()
{
}

const std::tr1::shared_ptr<const ProvideSpecTree::ConstItem>
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
    value()->accept(p);
    return stringify(p);
}

std::string
EProvideKey::pretty_print_flat(const ProvideSpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, _imp->id, ff, 0, false, true);
    value()->accept(p);
    return stringify(p);
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
        mutable std::tr1::function<void () throw ()> value_used;

        Implementation(const std::tr1::shared_ptr<const ERepositoryID> & i, const Environment * const e, const std::string & v) :
            id(i),
            env(e),
            string_value(v)
        {
        }
    };
}

EKeywordsKey::EKeywordsKey(const Environment * const e, const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataCollectionKey<KeywordNameSet>(r, h, t),
    PrivateImplementationPattern<EKeywordsKey>(new Implementation<EKeywordsKey>(id, e, v)),
    _imp(PrivateImplementationPattern<EKeywordsKey>::_imp)
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
    {
        if (_imp->value_used)
        {
            _imp->value_used();
            _imp->value_used = std::tr1::function<void () throw ()>();
        }
        return _imp->value;
    }

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

namespace paludis
{
    template <>
    struct Implementation<EStringSetKey>
    {
        const std::tr1::shared_ptr<const ERepositoryID> id;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable std::tr1::shared_ptr<Set<std::string> > value;

        Implementation(const std::tr1::shared_ptr<const ERepositoryID> & i, const std::string & v) :
            id(i),
            string_value(v)
        {
        }
    };
}

EStringSetKey::EStringSetKey(const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataCollectionKey<Set<std::string> >(r, h, t),
    PrivateImplementationPattern<EStringSetKey>(new Implementation<EStringSetKey>(id, v)),
    _imp(PrivateImplementationPattern<EStringSetKey>::_imp)
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

        Implementation(const std::tr1::shared_ptr<const ERepositoryID> & i, const FSEntry & v) :
            id(i),
            filename(v)
        {
        }
    };
}

EContentsKey::EContentsKey(const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const FSEntry & v, const MetadataKeyType t) :
    MetadataValueKey<std::tr1::shared_ptr<const Contents> > (r, h, t),
    PrivateImplementationPattern<EContentsKey>(new Implementation<EContentsKey>(id, v)),
    _imp(PrivateImplementationPattern<EContentsKey>::_imp)
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

    std::ifstream ff(stringify(f).c_str());
    if (! ff)
        throw ConfigurationError("Could not read '" + stringify(f) + "'");

    std::string line;
    unsigned line_number(0);
    while (std::getline(ff, line))
    {
        ++line_number;

        std::vector<std::string> tokens;
        if (! VDBContentsTokeniser::tokenise(line, std::back_inserter(tokens)))
        {
            Log::get_instance()->message("e.contents.broken", ll_warning, lc_no_context) << "CONTENTS has broken line " <<
                line_number << ", skipping";
            continue;
        }

        if ("obj" == tokens.at(0))
            _imp->value->add(std::tr1::shared_ptr<ContentsEntry>(new ContentsFileEntry(tokens.at(1))));
        else if ("dir" == tokens.at(0))
            _imp->value->add(std::tr1::shared_ptr<ContentsEntry>(new ContentsDirEntry(tokens.at(1))));
        else if ("misc" == tokens.at(0))
            _imp->value->add(std::tr1::shared_ptr<ContentsEntry>(new ContentsMiscEntry(tokens.at(1))));
        else if ("fif" == tokens.at(0))
            _imp->value->add(std::tr1::shared_ptr<ContentsEntry>(new ContentsFifoEntry(tokens.at(1))));
        else if ("dev" == tokens.at(0))
            _imp->value->add(std::tr1::shared_ptr<ContentsEntry>(new ContentsDevEntry(tokens.at(1))));
        else if ("sym" == tokens.at(0))
            _imp->value->add(std::tr1::shared_ptr<ContentsEntry>(new ContentsSymEntry(tokens.at(1), tokens.at(2))));
    }

    return _imp->value;
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

        Implementation(const std::tr1::shared_ptr<const ERepositoryID> & i, const FSEntry & v) :
            id(i),
            filename(v)
        {
        }
    };
}

EMTimeKey::EMTimeKey(const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & r, const std::string & h, const FSEntry & v, const MetadataKeyType t) :
    MetadataTimeKey(r, h, t),
    PrivateImplementationPattern<EMTimeKey>(new Implementation<EMTimeKey>(id, v)),
    _imp(PrivateImplementationPattern<EMTimeKey>::_imp)
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

namespace paludis
{
    template <>
    struct Implementation<EChoicesKey>
    {
        mutable Mutex mutex;
        mutable std::tr1::shared_ptr<Choices> value;

        const Environment * const env;
        const std::tr1::shared_ptr<const ERepositoryID> id;
        const std::tr1::shared_ptr<const ERepository> maybe_e_repository;

        Implementation(const Environment * const e, const std::tr1::shared_ptr<const ERepositoryID> & i,
                const std::tr1::shared_ptr<const ERepository> & p) :
            env(e),
            id(i),
            maybe_e_repository(p)
        {
        }
    };
}

EChoicesKey::EChoicesKey(
        const Environment * const e,
        const std::tr1::shared_ptr<const ERepositoryID> & i,
        const std::string & r, const std::string & h, const MetadataKeyType t,
        const std::tr1::shared_ptr<const ERepository> & p) :
    MetadataValueKey<std::tr1::shared_ptr<const Choices> > (r, h, t),
    PrivateImplementationPattern<EChoicesKey>(new Implementation<EChoicesKey>(e, i, p)),
    _imp(PrivateImplementationPattern<EChoicesKey>::_imp)
{
}

EChoicesKey::~EChoicesKey()
{
}

namespace
{
    struct IsExpand
    {
        ChoiceNameWithPrefix flag;
        std::string delim;

        IsExpand(const ChoiceNameWithPrefix & f, const std::string & d) :
            flag(f),
            delim(d)
        {
        }

        bool operator() (const std::string & s) const
        {
            std::string lower_s;
            std::transform(s.begin(), s.end(), std::back_inserter(lower_s), &::tolower);
            lower_s.append(delim);
            return (0 == flag.data().compare(0, lower_s.length(), lower_s, 0, lower_s.length()));
        }
    };

    struct MyOptionsFinder :
        ConstVisitor<PlainTextSpecTree>
    {
        typedef std::map<std::string, std::string> Annotations;
        typedef std::map<UnprefixedChoiceName, Annotations> Values;
        typedef std::map<ChoicePrefixName, Values> Prefixes;

        Prefixes prefixes;
        std::list<ChoicePrefixName> current_prefix_stack;

        MyOptionsFinder()
        {
            current_prefix_stack.push_front(ChoicePrefixName(""));
        }

        void visit_leaf(const PlainTextDepSpec & s)
        {
            Context context("When handling item '" + stringify(s) + "':");

            Prefixes::iterator p(prefixes.find(*current_prefix_stack.begin()));
            if (p == prefixes.end())
                p = prefixes.insert(std::make_pair(*current_prefix_stack.begin(), Values())).first;

            UnprefixedChoiceName n(s.text());
            Values::iterator v(p->second.find(n));
            if (v == p->second.end())
                v = p->second.insert(std::make_pair(n, Annotations())).first;

            if (s.annotations_key())
            {
                for (MetadataSectionKey::MetadataConstIterator m(s.annotations_key()->begin_metadata()),
                        m_end(s.annotations_key()->end_metadata()) ;
                        m != m_end ; ++m)
                {
                    const MetadataValueKey<std::string> * mm(visitor_cast<const MetadataValueKey<std::string> >(**m));
                    if (! mm)
                    {
                        Log::get_instance()->message("e_key.myoptions.strange_annotation", ll_warning, lc_context)
                            << "Don't know how to handle annotation '" << (*m)->raw_name() << "'";
                        continue;
                    }

                    Annotations::iterator a(v->second.find(mm->raw_name()));
                    if ((a != v->second.end()) && a->second != mm->value())
                        Log::get_instance()->message("e_key.myoptions.duplicate", ll_qa, lc_context)
                            << "Annotation '" << mm->raw_name() << "' set to both '" << a->second << "' and '"
                            << mm->value() << "'";
                    else
                        v->second.insert(make_pair(mm->raw_name(), mm->value()));
                }
            }
        }

        void visit_leaf(const PlainTextLabelDepSpec & s)
        {
            *current_prefix_stack.begin() = ChoicePrefixName(s.label());
        }

        void visit_sequence(const ConditionalDepSpec &,
                PlainTextSpecTree::ConstSequenceIterator cur,
                PlainTextSpecTree::ConstSequenceIterator end)
        {
            if (cur != end)
                throw InternalError(PALUDIS_HERE, "Don't know how to handle conditionals here yet");
        }

        void visit_sequence(const AllDepSpec &,
                PlainTextSpecTree::ConstSequenceIterator cur,
                PlainTextSpecTree::ConstSequenceIterator end)
        {
            current_prefix_stack.push_front(*current_prefix_stack.begin());
            std::for_each(cur, end, accept_visitor(*this));
            current_prefix_stack.pop_front();
        }
    };

    std::tr1::shared_ptr<ChoiceValue> make_myoption(
            const std::tr1::shared_ptr<const ERepositoryID> & id,
            std::tr1::shared_ptr<Choice> & choice,
            MyOptionsFinder::Values::const_iterator & v,
            const Tribool s,
            const bool b)
    {
        std::string description;
        for (MyOptionsFinder::Annotations::const_iterator a(v->second.begin()), a_end(v->second.end()) ;
                a != a_end ; ++a)
        {
            if (a->first == id->eapi()->supported()->annotations()->myoptions_description())
                description = a->second;
            else
                Log::get_instance()->message("e_key.myoptions.unknown", ll_qa, lc_context)
                    << "Unknown annotation '" << a->first << "' = '" << a->second << "'";
        }
        return id->make_choice_value(choice, v->first, s, b, description, false);
    }
}

#include <iostream>
#include <paludis/util/join.hh>
#include <paludis/util/member_iterator-impl.hh>

const std::tr1::shared_ptr<const Choices>
EChoicesKey::value() const
{
    Lock l(_imp->mutex);
    if (_imp->value)
        return _imp->value;

    Context context("When making Choices key for '" + stringify(*_imp->id) + "':");

    _imp->value.reset(new Choices);
    if (! _imp->id->eapi()->supported())
        return _imp->value;

    std::tr1::shared_ptr<Choice> use(new Choice(
                _imp->id->eapi()->supported()->ebuild_environment_variables()->env_use(),
                _imp->id->eapi()->supported()->ebuild_environment_variables()->env_use(),
                ChoicePrefixName(""),
                false,
                false,
                true,
                true));
    _imp->value->add(use);

    bool has_fancy_test_flag(false);

    std::tr1::shared_ptr<const Set<std::string> > hidden;
    if (_imp->id->raw_use_expand_hidden_key())
        hidden = _imp->id->raw_use_expand_hidden_key()->value();

    if (_imp->id->raw_myoptions_key())
    {
        Context local_context("When using raw_myoptions_key to populate choices:");

        /* yay. myoptions is easy. */
        MyOptionsFinder myoptions;
        _imp->id->raw_myoptions_key()->value()->accept(myoptions);

        if (_imp->id->raw_use_expand_key())
            for (Set<std::string>::ConstIterator u(_imp->id->raw_use_expand_key()->value()->begin()),
                    u_end(_imp->id->raw_use_expand_key()->value()->end()) ;
                    u != u_end ; ++u)
            {
                Context local_local_context("When using raw_use_expand_key value '" + *u + "' to populate choices:");

                std::string lower_u;
                std::transform(u->begin(), u->end(), std::back_inserter(lower_u), &::tolower);
                std::tr1::shared_ptr<Choice> exp(new Choice(stringify(*u), lower_u, ChoicePrefixName(lower_u),
                            false, hidden ? hidden->end() != hidden->find(*u) : false, false, true));
                _imp->value->add(exp);

                MyOptionsFinder::Prefixes::iterator p(myoptions.prefixes.find(ChoicePrefixName(lower_u)));
                if (myoptions.prefixes.end() != p)
                {
                    for (MyOptionsFinder::Values::const_iterator v(p->second.begin()), v_end(p->second.end()) ;
                            v != v_end ; ++v)
                        exp->add(make_myoption(_imp->id, exp, v, indeterminate, true));
                    myoptions.prefixes.erase(p);
                }
            }

        MyOptionsFinder::Prefixes::iterator p(myoptions.prefixes.find(ChoicePrefixName("")));
        if (myoptions.prefixes.end() != p)
        {
            Context local_local_context("When using empty prefix to populate choices:");
            for (MyOptionsFinder::Values::const_iterator v(p->second.begin()), v_end(p->second.end()) ;
                    v != v_end ; ++v)
                use->add(make_myoption(_imp->id, use, v, indeterminate, true));
            myoptions.prefixes.erase(p);
        }

        if (! myoptions.prefixes.empty())
        {
            Log::get_instance()->message("e.myoptions_key.invalid", ll_warning, lc_context) << "Key '" << raw_name() << "' for '"
                << *_imp->id << "' uses unknown prefixes { '" << join(first_iterator(myoptions.prefixes.begin()),
                        first_iterator(myoptions.prefixes.end()), "', '") << "' }";
        }
    }
    else
    {
        /* ugh. iuse and all that mess. */
        Context local_context("When using raw_iuse_key and raw_use_key to populate choices:");

        std::map<ChoiceNameWithPrefix, Tribool> i_values;
        std::string delim(1, _imp->id->eapi()->supported()->ebuild_options()->use_expand_separator());

        if (_imp->id->raw_iuse_key())
        {
            for (Set<std::string>::ConstIterator u(_imp->id->raw_iuse_key()->value()->begin()), u_end(_imp->id->raw_iuse_key()->value()->end()) ;
                    u != u_end ; ++u)
            {
                std::pair<ChoiceNameWithPrefix, Tribool> flag(parse_iuse(_imp->id->eapi(), *u));
                if (_imp->id->raw_use_expand_key() &&
                        _imp->id->raw_use_expand_key()->value()->end() != std::find_if(
                            _imp->id->raw_use_expand_key()->value()->begin(),
                            _imp->id->raw_use_expand_key()->value()->end(),
                            IsExpand(flag.first, delim)))
                    i_values.insert(flag);
                else if (stringify(flag.first) == _imp->id->eapi()->supported()->choices_options()->fancy_test_flag())
                {
                    /* have to add this right at the end, after build_options is there */
                    has_fancy_test_flag = true;
                }
                else
                    use->add(_imp->id->make_choice_value(use, UnprefixedChoiceName(stringify(flag.first)), flag.second, true, "", false));
            }

            /* pain in the ass: installed packages with DEPEND="x86? ( blah )" need to work,
             * even if x86 isn't listed in IUSE. */
            if (_imp->id->raw_use_key())
            {
                for (Set<std::string>::ConstIterator u(_imp->id->raw_use_key()->value()->begin()), u_end(_imp->id->raw_use_key()->value()->end()) ;
                        u != u_end ; ++u)
                {
                    if (_imp->id->raw_iuse_key()->value()->end() != _imp->id->raw_iuse_key()->value()->find(*u))
                        continue;

                    std::pair<ChoiceNameWithPrefix, Tribool> flag(ChoiceNameWithPrefix("x"), indeterminate);
                    if (0 == u->compare(0, 1, "-", 0, 1))
                        flag = std::make_pair(ChoiceNameWithPrefix(u->substr(1)), false);
                    else
                        flag = std::make_pair(ChoiceNameWithPrefix(*u), true);

                    if (_imp->id->raw_use_expand_key() &&
                            _imp->id->raw_use_expand_key()->value()->end() != std::find_if(
                                _imp->id->raw_use_expand_key()->value()->begin(),
                                _imp->id->raw_use_expand_key()->value()->end(),
                                IsExpand(flag.first, delim)))
                    {
                        /* don't need to worry */
                    }
                    else
                        use->add(_imp->id->make_choice_value(use, UnprefixedChoiceName(stringify(flag.first)), flag.second, false, "", false));
                }
            }
        }

        std::string env_arch(_imp->id->eapi()->supported()->ebuild_environment_variables()->env_arch());
        if ((! env_arch.empty()) && _imp->maybe_e_repository)
        {
            std::tr1::shared_ptr<Choice> arch(new Choice(env_arch, env_arch, ChoicePrefixName(""), false, true, false, false));
            _imp->value->add(arch);

            for (Set<UnprefixedChoiceName>::ConstIterator a(_imp->maybe_e_repository->arch_flags()->begin()), a_end(_imp->maybe_e_repository->arch_flags()->end()) ;
                    a != a_end ; ++a)
                arch->add(_imp->id->make_choice_value(arch, *a, indeterminate, false, "", false));
        }

        if (_imp->id->raw_use_expand_key())
        {
            for (Set<std::string>::ConstIterator u(_imp->id->raw_use_expand_key()->value()->begin()),
                    u_end(_imp->id->raw_use_expand_key()->value()->end()) ;
                    u != u_end ; ++u)
            {
                std::string lower_u;
                std::transform(u->begin(), u->end(), std::back_inserter(lower_u), &::tolower);
                std::tr1::shared_ptr<Choice> exp(new Choice(stringify(*u), lower_u, ChoicePrefixName(lower_u),
                            ! _imp->id->eapi()->supported()->ebuild_options()->require_use_expand_in_iuse(),
                            hidden ? hidden->end() != hidden->find(*u) : false, false, true));
                _imp->value->add(exp);

                std::set<UnprefixedChoiceName> values;

                if (! _imp->id->eapi()->supported()->ebuild_options()->require_use_expand_in_iuse())
                {
                    std::tr1::shared_ptr<const Set<UnprefixedChoiceName> >
                        e_values(_imp->env->known_choice_value_names(_imp->id, exp)),
                        r_values;

                    if (_imp->maybe_e_repository)
                        r_values = _imp->maybe_e_repository->profile()->known_choice_value_names(_imp->id, exp);
                    else
                        r_values = make_shared_ptr(new Set<UnprefixedChoiceName>);

                    std::set_union(e_values->begin(), e_values->end(), r_values->begin(), r_values->end(),
                            std::inserter(values, values.begin()));
                }

                for (std::map<ChoiceNameWithPrefix, Tribool>::const_iterator i(i_values.begin()), i_end(i_values.end()) ;
                        i != i_end ; ++i)
                    if (IsExpand(i->first, delim)(*u))
                        values.insert(UnprefixedChoiceName(i->first.data().substr(u->length() + delim.length())));

                for (std::set<UnprefixedChoiceName>::const_iterator v(values.begin()), v_end(values.end()) ;
                        v != v_end ; ++v)
                {
                    std::map<ChoiceNameWithPrefix, Tribool>::const_iterator i(i_values.find(ChoiceNameWithPrefix(lower_u + delim + stringify(*v))));
                    if (i_values.end() != i)
                        exp->add(_imp->id->make_choice_value(exp, *v, i->second, true, "", false));
                    else
                        exp->add(_imp->id->make_choice_value(exp, *v, indeterminate, false, "", false));
                }
            }
        }
    }

    _imp->id->add_build_options(_imp->value);

    if (has_fancy_test_flag)
    {
        std::tr1::shared_ptr<const ChoiceValue> choice;

        choice = _imp->value->find_by_name_with_prefix(ELikeRecommendedTestsChoiceValue::canonical_name_with_prefix());
        if (! choice)
            choice = _imp->value->find_by_name_with_prefix(ELikeOptionalTestsChoiceValue::canonical_name_with_prefix());
        if (choice)
            use->add(_imp->id->make_choice_value(use, UnprefixedChoiceName(_imp->id->eapi()->supported()->choices_options()->fancy_test_flag()),
                        choice->enabled(), true, "", true));
    }

    return _imp->value;
}

