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

#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/repositories/fake/fake_repository_base.hh>
#include <paludis/eapi.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/visitor-impl.hh>

#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

using namespace paludis;

namespace paludis
{
    template <>
    template <typename C_>
    struct Implementation<FakeMetadataCollectionKey<C_> >
    {
        tr1::shared_ptr<C_> collection;
    };
}

template <typename C_>
FakeMetadataCollectionKey<C_>::FakeMetadataCollectionKey(
        const std::string & r, const std::string & h, const MetadataKeyType t) :
    MetadataCollectionKey<C_>(r, h, t),
    PrivateImplementationPattern<FakeMetadataCollectionKey<C_> >(new Implementation<FakeMetadataCollectionKey<C_> >),
    _imp(PrivateImplementationPattern<FakeMetadataCollectionKey<C_> >::_imp.get())
{
}

template <typename C_>
FakeMetadataCollectionKey<C_>::~FakeMetadataCollectionKey()
{
}

template <typename C_>
const tr1::shared_ptr<const C_>
FakeMetadataCollectionKey<C_>::value() const
{
    return _imp->collection;
}

FakeMetadataKeywordCollectionKey::FakeMetadataKeywordCollectionKey(const std::string & r,
        const std::string & h, const std::string & v, const MetadataKeyType t) :
    FakeMetadataCollectionKey<KeywordNameCollection>(r, h, t)
{
    set_from_string(v);
}

void
FakeMetadataKeywordCollectionKey::set_from_string(const std::string & s)
{
    _imp->collection.reset(new KeywordNameCollection::Concrete);
    WhitespaceTokeniser::get_instance()->tokenise(s, create_inserter<KeywordName>(_imp->collection->inserter()));
}

FakeMetadataIUseCollectionKey::FakeMetadataIUseCollectionKey(const std::string & r,
        const std::string & h, const std::string & v, const IUseFlagParseMode m, const MetadataKeyType t) :
    FakeMetadataCollectionKey<IUseFlagCollection>(r, h, t)
{
    set_from_string(v, m);
}

void
FakeMetadataIUseCollectionKey::set_from_string(const std::string & s, const IUseFlagParseMode m)
{
    _imp->collection.reset(new IUseFlagCollection::Concrete);
    std::list<std::string> tokens;
    WhitespaceTokeniser::get_instance()->tokenise(s, std::back_inserter(tokens));
    for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
            t != t_end ; ++t)
        _imp->collection->insert(IUseFlag(*t, m));
}

namespace paludis
{
    template <>
    template <typename C_>
    struct Implementation<FakeMetadataSpecTreeKey<C_> >
    {
        tr1::shared_ptr<const typename C_::ConstItem> value;
        const tr1::function<const tr1::shared_ptr<const typename C_::ConstItem> (const std::string &)> func;

        Implementation(const tr1::function<const tr1::shared_ptr<const typename C_::ConstItem> (const std::string &)> & f) :
            func(f)
        {
        }
    };
}

template <typename C_>
FakeMetadataSpecTreeKey<C_>::FakeMetadataSpecTreeKey(const std::string & r, const std::string & h, const std::string & v,
        const tr1::function<const tr1::shared_ptr<const typename C_::ConstItem> (const std::string &)> & f, const MetadataKeyType t) :
    MetadataSpecTreeKey<C_>(r, h, t),
    PrivateImplementationPattern<FakeMetadataSpecTreeKey<C_> >(new Implementation<FakeMetadataSpecTreeKey<C_> >(f)),
    _imp(PrivateImplementationPattern<FakeMetadataSpecTreeKey<C_> >::_imp.get())
{
    set_from_string(v);
}

template <typename C_>
FakeMetadataSpecTreeKey<C_>::~FakeMetadataSpecTreeKey()
{
}

template <typename C_>
void
FakeMetadataSpecTreeKey<C_>::set_from_string(const std::string & s)
{
    _imp->value = _imp->func(s);
}

template <typename C_>
const tr1::shared_ptr<const typename C_::ConstItem>
FakeMetadataSpecTreeKey<C_>::value() const
{
    return _imp->value;
}

namespace paludis
{
    template <>
    struct Implementation<FakeMetadataPackageIDKey>
    {
        tr1::shared_ptr<const PackageID> value;
    };
}

FakeMetadataPackageIDKey::FakeMetadataPackageIDKey(const std::string & r, const std::string & h,
        const tr1::shared_ptr<const PackageID> & v, const MetadataKeyType t) :
    MetadataPackageIDKey(r, h, t),
    PrivateImplementationPattern<FakeMetadataPackageIDKey>(new Implementation<FakeMetadataPackageIDKey>),
    _imp(PrivateImplementationPattern<FakeMetadataPackageIDKey>::_imp.get())
{
    _imp->value = v;
}

FakeMetadataPackageIDKey::~FakeMetadataPackageIDKey()
{
}

const tr1::shared_ptr<const PackageID>
FakeMetadataPackageIDKey::value() const
{
    return _imp->value;
}

namespace paludis
{
    using namespace tr1::placeholders;

    template <>
    struct Implementation<FakePackageID>
    {
        const tr1::shared_ptr<const FakeRepositoryBase> repository;
        const QualifiedPackageName name;
        const VersionSpec version;
        SlotName slot;
        tr1::shared_ptr<const EAPI> eapi;

        tr1::shared_ptr<FakeMetadataPackageIDKey> package_id;
        tr1::shared_ptr<FakeMetadataPackageIDKey> virtual_for;
        tr1::shared_ptr<FakeMetadataKeywordCollectionKey> keywords;
        tr1::shared_ptr<FakeMetadataIUseCollectionKey> iuse;
        tr1::shared_ptr<FakeMetadataSpecTreeKey<LicenseSpecTree> > license;
        tr1::shared_ptr<FakeMetadataSpecTreeKey<ProvideSpecTree> > provide;
        tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> > build_dependencies;
        tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> > run_dependencies;
        tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> > post_dependencies;
        tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> > suggested_dependencies;
        tr1::shared_ptr<FakeMetadataSpecTreeKey<RestrictSpecTree> > restrictions;
        tr1::shared_ptr<FakeMetadataSpecTreeKey<URISpecTree> > src_uri;
        tr1::shared_ptr<FakeMetadataSpecTreeKey<URISpecTree> > bin_uri;
        tr1::shared_ptr<FakeMetadataSpecTreeKey<URISpecTree> > homepage;

        Implementation(const tr1::shared_ptr<const FakeRepositoryBase> & r,
                const QualifiedPackageName & q, const VersionSpec & v) :
            repository(r),
            name(q),
            version(v),
            slot("0"),
            eapi(EAPIData::get_instance()->eapi_from_string("0")),
            keywords(new FakeMetadataKeywordCollectionKey("KEYWORDS", "Keywords", "test", mkt_normal)),
            iuse(new FakeMetadataIUseCollectionKey("IUSE", "Used USE flags", "", iuse_pm_permissive, mkt_normal)),
            license(new FakeMetadataSpecTreeKey<LicenseSpecTree>("LICENSE", "Licenses",
                        "", tr1::bind(&PortageDepParser::parse_license, _1, tr1::cref(*eapi)), mkt_normal)),
            provide(new FakeMetadataSpecTreeKey<ProvideSpecTree>("PROVIDE", "Provided packages",
                        "", tr1::bind(&PortageDepParser::parse_provide, _1, tr1::cref(*eapi)), mkt_normal)),
            build_dependencies(new FakeMetadataSpecTreeKey<DependencySpecTree>("DEPEND", "Build dependencies",
                        "", tr1::bind(&PortageDepParser::parse_depend, _1, tr1::cref(*eapi)), mkt_dependencies)),
            run_dependencies(new FakeMetadataSpecTreeKey<DependencySpecTree>("RDEPEND", "Run dependencies",
                        "", tr1::bind(&PortageDepParser::parse_depend, _1, tr1::cref(*eapi)), mkt_dependencies)),
            post_dependencies(new FakeMetadataSpecTreeKey<DependencySpecTree>("PDEPEND", "Post dependencies",
                        "", tr1::bind(&PortageDepParser::parse_depend, _1, tr1::cref(*eapi)), mkt_dependencies)),
            suggested_dependencies(new FakeMetadataSpecTreeKey<DependencySpecTree>("SDEPEND", "Suggested dependencies",
                        "", tr1::bind(&PortageDepParser::parse_depend, _1, tr1::cref(*eapi)), mkt_dependencies))
        {
        }
    };
}

FakePackageID::FakePackageID(const tr1::shared_ptr<const FakeRepositoryBase> & r,
        const QualifiedPackageName & q, const VersionSpec & v) :
    PrivateImplementationPattern<FakePackageID>(new Implementation<FakePackageID>(r, q, v)),
    _imp(PrivateImplementationPattern<FakePackageID>::_imp.get())
{
    add_key(_imp->keywords);
    add_key(_imp->iuse);
    add_key(_imp->license);
    add_key(_imp->provide);
    add_key(_imp->build_dependencies);
    add_key(_imp->run_dependencies);
    add_key(_imp->post_dependencies);
    add_key(_imp->suggested_dependencies);
}

FakePackageID::~FakePackageID()
{
}

const std::string
FakePackageID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            return stringify(_imp->name) + "-" + stringify(_imp->version) + ":" + stringify(slot()) + "::" + stringify(_imp->repository->name());

        case idcf_version:
            return stringify(_imp->version);

        case idcf_no_version:
            return stringify(_imp->name) + ":" + stringify(slot()) + "::" + stringify(_imp->repository->name());

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

const QualifiedPackageName
FakePackageID::name() const
{
    return _imp->name;
}

const VersionSpec
FakePackageID::version() const
{
    return _imp->version;
}

const SlotName
FakePackageID::slot() const
{
    return _imp->slot;
}

const tr1::shared_ptr<const Repository>
FakePackageID::repository() const
{
    return _imp->repository;
}

const tr1::shared_ptr<const EAPI>
FakePackageID::eapi() const
{
    return _imp->eapi;
}

const tr1::shared_ptr<const MetadataPackageIDKey>
FakePackageID::virtual_for_key() const
{
    return _imp->virtual_for;
}

const tr1::shared_ptr<const MetadataCollectionKey<KeywordNameCollection> >
FakePackageID::keywords_key() const
{
    return _imp->keywords;
}

const tr1::shared_ptr<const MetadataCollectionKey<IUseFlagCollection> >
FakePackageID::iuse_key() const
{
    return _imp->iuse;
}

const tr1::shared_ptr<const MetadataCollectionKey<InheritedCollection> >
FakePackageID::inherited_key() const
{
    return tr1::shared_ptr<const MetadataCollectionKey<InheritedCollection> >();
}

const tr1::shared_ptr<const MetadataCollectionKey<UseFlagNameCollection> >
FakePackageID::use_key() const
{
    return tr1::shared_ptr<const MetadataCollectionKey<UseFlagNameCollection> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> >
FakePackageID::license_key() const
{
    return _imp->license;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
FakePackageID::provide_key() const
{
    return _imp->provide;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::build_dependencies_key() const
{
    return _imp->build_dependencies;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::run_dependencies_key() const
{
    return _imp->run_dependencies;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::post_dependencies_key() const
{
    return _imp->post_dependencies;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::suggested_dependencies_key() const
{
    return _imp->suggested_dependencies;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<RestrictSpecTree> >
FakePackageID::restrict_key() const
{
    return _imp->restrictions;
}

const tr1::shared_ptr<FakeMetadataKeywordCollectionKey>
FakePackageID::keywords_key()
{
    return _imp->keywords;
}

const tr1::shared_ptr<FakeMetadataIUseCollectionKey>
FakePackageID::iuse_key()
{
    return _imp->iuse;
}

const tr1::shared_ptr<FakeMetadataSpecTreeKey<ProvideSpecTree> >
FakePackageID::provide_key()
{
    return _imp->provide;
}

const tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::build_dependencies_key()
{
    return _imp->build_dependencies;
}

const tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::run_dependencies_key()
{
    return _imp->run_dependencies;
}

const tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::post_dependencies_key()
{
    return _imp->post_dependencies;
}

const tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::suggested_dependencies_key()
{
    return _imp->suggested_dependencies;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> >
FakePackageID::src_uri_key() const
{
    return _imp->src_uri;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> >
FakePackageID::homepage_key() const
{
    return _imp->homepage;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> >
FakePackageID::bin_uri_key() const
{
    return _imp->bin_uri;
}

const tr1::shared_ptr<const MetadataStringKey>
FakePackageID::short_description_key() const
{
    return tr1::shared_ptr<const MetadataStringKey>();
}

const tr1::shared_ptr<const MetadataStringKey>
FakePackageID::long_description_key() const
{
    return tr1::shared_ptr<const MetadataStringKey>();
}

void
FakePackageID::set_slot(const SlotName & s)
{
    _imp->slot = s;
}

bool
FakePackageID::arbitrary_less_than_comparison(const PackageID & other) const
{
    return slot().data() < other.slot().data();
}

void
FakePackageID::need_keys_added() const
{
}

std::size_t
FakePackageID::extra_hash_value() const
{
    return CRCHash<SlotName>()(slot());
}

template class FakeMetadataSpecTreeKey<LicenseSpecTree>;
template class FakeMetadataSpecTreeKey<ProvideSpecTree>;
template class FakeMetadataSpecTreeKey<DependencySpecTree>;
template class FakeMetadataSpecTreeKey<RestrictSpecTree>;
template class FakeMetadataSpecTreeKey<URISpecTree>;

template class FakeMetadataCollectionKey<KeywordNameCollection>;
template class FakeMetadataCollectionKey<IUseFlagCollection>;

