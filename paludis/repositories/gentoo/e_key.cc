/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/repositories/gentoo/e_key.hh>
#include <paludis/repositories/gentoo/ebuild_id.hh>

#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/iterator.hh>

#include <paludis/portage_dep_parser.hh>
#include <paludis/eapi.hh>

#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

#include <list>

using namespace paludis;
using namespace paludis::erepository;

EStringKey::EStringKey(const tr1::shared_ptr<const PackageID> &,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataStringKey(r, h, t),
    _value(v)
{
}

EStringKey::~EStringKey()
{
}

const std::string
EStringKey::value() const
{
    return _value;
}

namespace paludis
{
    template <>
    struct Implementation<EDependenciesKey>
    {
        const tr1::shared_ptr<const PackageID> id;
        const std::string string_value;
        mutable tr1::shared_ptr<const DependencySpecTree::ConstItem> value;

        Implementation(const tr1::shared_ptr<const PackageID> & i, const std::string & v) :
            id(i),
            string_value(v)
        {
        }
    };
}

EDependenciesKey::EDependenciesKey(const tr1::shared_ptr<const PackageID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataSpecTreeKey<DependencySpecTree>(r, h, t),
    PrivateImplementationPattern<EDependenciesKey>(new Implementation<EDependenciesKey>(id, v)),
    _imp(PrivateImplementationPattern<EDependenciesKey>::_imp.get())
{
}

EDependenciesKey::~EDependenciesKey()
{
}

const tr1::shared_ptr<const DependencySpecTree::ConstItem>
EDependenciesKey::value() const
{
    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = PortageDepParser::parse_depend(_imp->string_value, *_imp->id->eapi());
    return _imp->value;
}

namespace paludis
{
    template <>
    struct Implementation<ELicenseKey>
    {
        const tr1::shared_ptr<const PackageID> id;
        const std::string string_value;
        mutable tr1::shared_ptr<const LicenseSpecTree::ConstItem> value;

        Implementation(const tr1::shared_ptr<const PackageID> & i, const std::string & v) :
            id(i),
            string_value(v)
        {
        }
    };
}

ELicenseKey::ELicenseKey(const tr1::shared_ptr<const PackageID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataSpecTreeKey<LicenseSpecTree>(r, h, t),
    PrivateImplementationPattern<ELicenseKey>(new Implementation<ELicenseKey>(id, v)),
    _imp(PrivateImplementationPattern<ELicenseKey>::_imp.get())
{
}

ELicenseKey::~ELicenseKey()
{
}

const tr1::shared_ptr<const LicenseSpecTree::ConstItem>
ELicenseKey::value() const
{
    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = PortageDepParser::parse_license(_imp->string_value, *_imp->id->eapi());
    return _imp->value;
}

namespace paludis
{
    template <>
    struct Implementation<EURIKey>
    {
        const tr1::shared_ptr<const PackageID> id;
        const std::string string_value;
        mutable tr1::shared_ptr<const URISpecTree::ConstItem> value;

        Implementation(const tr1::shared_ptr<const PackageID> & i, const std::string & v) :
            id(i),
            string_value(v)
        {
        }
    };
}

EURIKey::EURIKey(const tr1::shared_ptr<const PackageID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataSpecTreeKey<URISpecTree>(r, h, t),
    PrivateImplementationPattern<EURIKey>(new Implementation<EURIKey>(id, v)),
    _imp(PrivateImplementationPattern<EURIKey>::_imp.get())
{
}

EURIKey::~EURIKey()
{
}

const tr1::shared_ptr<const URISpecTree::ConstItem>
EURIKey::value() const
{
    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = PortageDepParser::parse_uri(_imp->string_value, *_imp->id->eapi());
    return _imp->value;
}

namespace paludis
{
    template <>
    struct Implementation<ERestrictKey>
    {
        const tr1::shared_ptr<const PackageID> id;
        const std::string string_value;
        mutable tr1::shared_ptr<const RestrictSpecTree::ConstItem> value;

        Implementation(const tr1::shared_ptr<const PackageID> & i, const std::string & v) :
            id(i),
            string_value(v)
        {
        }
    };
}

ERestrictKey::ERestrictKey(const tr1::shared_ptr<const PackageID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataSpecTreeKey<RestrictSpecTree>(r, h, t),
    PrivateImplementationPattern<ERestrictKey>(new Implementation<ERestrictKey>(id, v)),
    _imp(PrivateImplementationPattern<ERestrictKey>::_imp.get())
{
}

ERestrictKey::~ERestrictKey()
{
}

const tr1::shared_ptr<const RestrictSpecTree::ConstItem>
ERestrictKey::value() const
{
    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = PortageDepParser::parse_restrict(_imp->string_value, *_imp->id->eapi());
    return _imp->value;
}

namespace paludis
{
    template <>
    struct Implementation<EProvideKey>
    {
        const tr1::shared_ptr<const PackageID> id;
        const std::string string_value;
        mutable tr1::shared_ptr<const ProvideSpecTree::ConstItem> value;

        Implementation(const tr1::shared_ptr<const PackageID> & i, const std::string & v) :
            id(i),
            string_value(v)
        {
        }
    };
}

EProvideKey::EProvideKey(const tr1::shared_ptr<const PackageID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataSpecTreeKey<ProvideSpecTree>(r, h, t),
    PrivateImplementationPattern<EProvideKey>(new Implementation<EProvideKey>(id, v)),
    _imp(PrivateImplementationPattern<EProvideKey>::_imp.get())
{
}

EProvideKey::~EProvideKey()
{
}

const tr1::shared_ptr<const ProvideSpecTree::ConstItem>
EProvideKey::value() const
{
    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = PortageDepParser::parse_provide(_imp->string_value, *_imp->id->eapi());
    return _imp->value;
}

namespace paludis
{
    template <>
    struct Implementation<EIUseKey>
    {
        const tr1::shared_ptr<const PackageID> id;
        const std::string string_value;
        mutable tr1::shared_ptr<IUseFlagCollection> value;

        Implementation(const tr1::shared_ptr<const PackageID> & i, const std::string & v) :
            id(i),
            string_value(v)
        {
        }
    };
}

EIUseKey::EIUseKey(const tr1::shared_ptr<const PackageID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataCollectionKey<IUseFlagCollection>(r, h, t),
    PrivateImplementationPattern<EIUseKey>(new Implementation<EIUseKey>(id, v)),
    _imp(PrivateImplementationPattern<EIUseKey>::_imp.get())
{
}

EIUseKey::~EIUseKey()
{
}

const tr1::shared_ptr<const IUseFlagCollection>
EIUseKey::value() const
{
    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value.reset(new IUseFlagCollection::Concrete);
    std::list<std::string> tokens;
    WhitespaceTokeniser::get_instance()->tokenise(_imp->string_value, std::back_inserter(tokens));
    for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
            t != t_end ; ++t)
        _imp->value->insert(IUseFlag(*t, _imp->id->eapi()->supported->iuse_flag_parse_mode));

    return _imp->value;
}

namespace paludis
{
    template <>
    struct Implementation<EKeywordsKey>
    {
        const tr1::shared_ptr<const PackageID> id;
        const std::string string_value;
        mutable tr1::shared_ptr<KeywordNameCollection> value;

        Implementation(const tr1::shared_ptr<const PackageID> & i, const std::string & v) :
            id(i),
            string_value(v)
        {
        }
    };
}

EKeywordsKey::EKeywordsKey(const tr1::shared_ptr<const PackageID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataCollectionKey<KeywordNameCollection>(r, h, t),
    PrivateImplementationPattern<EKeywordsKey>(new Implementation<EKeywordsKey>(id, v)),
    _imp(PrivateImplementationPattern<EKeywordsKey>::_imp.get())
{
}

EKeywordsKey::~EKeywordsKey()
{
}

const tr1::shared_ptr<const KeywordNameCollection>
EKeywordsKey::value() const
{
    if (_imp->value)
        return _imp->value;

    _imp->value.reset(new KeywordNameCollection::Concrete);
    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    WhitespaceTokeniser::get_instance()->tokenise(_imp->string_value, create_inserter<KeywordName>(_imp->value->inserter()));
    return _imp->value;
}

namespace paludis
{
    template <>
    struct Implementation<EUseKey>
    {
        const tr1::shared_ptr<const PackageID> id;
        const std::string string_value;
        mutable tr1::shared_ptr<UseFlagNameCollection> value;

        Implementation(const tr1::shared_ptr<const PackageID> & i, const std::string & v) :
            id(i),
            string_value(v)
        {
        }
    };
}

EUseKey::EUseKey(const tr1::shared_ptr<const PackageID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataCollectionKey<UseFlagNameCollection>(r, h, t),
    PrivateImplementationPattern<EUseKey>(new Implementation<EUseKey>(id, v)),
    _imp(PrivateImplementationPattern<EUseKey>::_imp.get())
{
}

EUseKey::~EUseKey()
{
}

const tr1::shared_ptr<const UseFlagNameCollection>
EUseKey::value() const
{
    if (_imp->value)
        return _imp->value;

    _imp->value.reset(new UseFlagNameCollection::Concrete);
    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    std::list<std::string> tokens;
    WhitespaceTokeniser::get_instance()->tokenise(_imp->string_value, std::back_inserter(tokens));
    for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
            t != t_end ; ++t)
        if ('-' != t->at(0))
            _imp->value->insert(UseFlagName(*t));
    return _imp->value;
}

namespace paludis
{
    template <>
    struct Implementation<EInheritedKey>
    {
        const tr1::shared_ptr<const PackageID> id;
        const std::string string_value;
        mutable tr1::shared_ptr<InheritedCollection> value;

        Implementation(const tr1::shared_ptr<const PackageID> & i, const std::string & v) :
            id(i),
            string_value(v)
        {
        }
    };
}

EInheritedKey::EInheritedKey(const tr1::shared_ptr<const PackageID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataCollectionKey<InheritedCollection>(r, h, t),
    PrivateImplementationPattern<EInheritedKey>(new Implementation<EInheritedKey>(id, v)),
    _imp(PrivateImplementationPattern<EInheritedKey>::_imp.get())
{
}

EInheritedKey::~EInheritedKey()
{
}

const tr1::shared_ptr<const InheritedCollection>
EInheritedKey::value() const
{
    if (_imp->value)
        return _imp->value;

    _imp->value.reset(new InheritedCollection::Concrete);
    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    WhitespaceTokeniser::get_instance()->tokenise(_imp->string_value, _imp->value->inserter());
    return _imp->value;
}

