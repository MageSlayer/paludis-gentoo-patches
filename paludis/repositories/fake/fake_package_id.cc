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
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/name.hh>
#include <paludis/action.hh>
#include <paludis/environment.hh>
#include <paludis/version_spec.hh>
#include <paludis/dep_spec.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/save.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/make_shared_ptr.hh>

#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

#include <list>
#include <sstream>

using namespace paludis;

namespace paludis
{
    template <>
    template <typename C_>
    struct Implementation<FakeMetadataSetKey<C_> >
    {
        tr1::shared_ptr<C_> collection;
    };
}

template <typename C_>
FakeMetadataSetKey<C_>::FakeMetadataSetKey(
        const std::string & r, const std::string & h, const MetadataKeyType t) :
    MetadataSetKey<C_>(r, h, t),
    PrivateImplementationPattern<FakeMetadataSetKey<C_> >(new Implementation<FakeMetadataSetKey<C_> >),
    _imp(PrivateImplementationPattern<FakeMetadataSetKey<C_> >::_imp.get())
{
}

template <typename C_>
FakeMetadataSetKey<C_>::~FakeMetadataSetKey()
{
}

template <typename C_>
const tr1::shared_ptr<const C_>
FakeMetadataSetKey<C_>::value() const
{
    return _imp->collection;
}

FakeMetadataKeywordSetKey::FakeMetadataKeywordSetKey(const std::string & r,
        const std::string & h, const std::string & v, const MetadataKeyType t) :
    FakeMetadataSetKey<KeywordNameSet>(r, h, t)
{
    set_from_string(v);
}

void
FakeMetadataKeywordSetKey::set_from_string(const std::string & s)
{
    _imp->collection.reset(new KeywordNameSet);
    WhitespaceTokeniser::get_instance()->tokenise(s, create_inserter<KeywordName>(_imp->collection->inserter()));
}

FakeMetadataIUseSetKey::FakeMetadataIUseSetKey(const std::string & r,
        const std::string & h, const std::string & v, const IUseFlagParseMode m, const MetadataKeyType t) :
    FakeMetadataSetKey<IUseFlagSet>(r, h, t)
{
    set_from_string(v, m);
}

void
FakeMetadataIUseSetKey::set_from_string(const std::string & s, const IUseFlagParseMode m)
{
    _imp->collection.reset(new IUseFlagSet);
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
        std::string string_value;
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
    _imp->string_value = s;
    _imp->value = _imp->func(s);
}

template <typename C_>
const tr1::shared_ptr<const typename C_::ConstItem>
FakeMetadataSpecTreeKey<C_>::value() const
{
    return _imp->value;
}

template <typename C_>
std::string
FakeMetadataSpecTreeKey<C_>::pretty_print() const
{
    return _imp->string_value;
}

template <typename C_>
std::string
FakeMetadataSpecTreeKey<C_>::pretty_print_flat() const
{
    return _imp->string_value;
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
    template <>
    struct Implementation<FakeUnacceptedMask>
    {
        const char key;
        const std::string description;
        const tr1::shared_ptr<const MetadataKey> unaccepted_key;

        Implementation(const char k, const std::string & d, const tr1::shared_ptr<const MetadataKey> & u) :
            key(k),
            description(d),
            unaccepted_key(u)
        {
        }
    };
}

FakeUnacceptedMask::FakeUnacceptedMask(const char c, const std::string & s, const tr1::shared_ptr<const MetadataKey> & k) :
    PrivateImplementationPattern<FakeUnacceptedMask>(new Implementation<FakeUnacceptedMask>(c, s, k))
{
}

FakeUnacceptedMask::~FakeUnacceptedMask()
{
}

const char
FakeUnacceptedMask::key() const
{
    return _imp->key;
}

const std::string
FakeUnacceptedMask::description() const
{
    return _imp->description;
}

const tr1::shared_ptr<const MetadataKey>
FakeUnacceptedMask::unaccepted_key() const
{
    return _imp->unaccepted_key;
}

namespace paludis
{
    using namespace tr1::placeholders;

    template <>
    struct Implementation<FakePackageID>
    {
        mutable Mutex mutex;

        const Environment * const env;
        const tr1::shared_ptr<const FakeRepositoryBase> repository;
        const QualifiedPackageName name;
        const VersionSpec version;
        SlotName slot;

        tr1::shared_ptr<FakeMetadataPackageIDKey> package_id;
        tr1::shared_ptr<FakeMetadataPackageIDKey> virtual_for;
        tr1::shared_ptr<FakeMetadataKeywordSetKey> keywords;
        tr1::shared_ptr<FakeMetadataIUseSetKey> iuse;
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

        mutable bool has_masks;

        Implementation(const Environment * const e, const tr1::shared_ptr<const FakeRepositoryBase> & r,
                const QualifiedPackageName & q, const VersionSpec & v) :
            env(e),
            repository(r),
            name(q),
            version(v),
            slot("0"),
            keywords(new FakeMetadataKeywordSetKey("KEYWORDS", "Keywords", "test", mkt_normal)),
            iuse(new FakeMetadataIUseSetKey("IUSE", "Used USE flags", "", iuse_pm_permissive, mkt_normal)),
            license(new FakeMetadataSpecTreeKey<LicenseSpecTree>("LICENSE", "Licenses",
                        "", tr1::bind(&erepository::DepParser::parse_license, _1,
                            *erepository::EAPIData::get_instance()->eapi_from_string("0")), mkt_normal)),
            provide(new FakeMetadataSpecTreeKey<ProvideSpecTree>("PROVIDE", "Provided packages",
                        "", tr1::bind(&erepository::DepParser::parse_provide, _1,
                            *erepository::EAPIData::get_instance()->eapi_from_string("0")), mkt_normal)),
            build_dependencies(new FakeMetadataSpecTreeKey<DependencySpecTree>("DEPEND", "Build dependencies",
                        "", tr1::bind(&erepository::DepParser::parse_depend, _1,
                            *erepository::EAPIData::get_instance()->eapi_from_string("0")), mkt_dependencies)),
            run_dependencies(new FakeMetadataSpecTreeKey<DependencySpecTree>("RDEPEND", "Run dependencies",
                        "", tr1::bind(&erepository::DepParser::parse_depend, _1,
                            *erepository::EAPIData::get_instance()->eapi_from_string("0")), mkt_dependencies)),
            post_dependencies(new FakeMetadataSpecTreeKey<DependencySpecTree>("PDEPEND", "Post dependencies",
                        "", tr1::bind(&erepository::DepParser::parse_depend, _1,
                            *erepository::EAPIData::get_instance()->eapi_from_string("0")), mkt_dependencies)),
            suggested_dependencies(new FakeMetadataSpecTreeKey<DependencySpecTree>("SDEPEND", "Suggested dependencies",
                        "", tr1::bind(&erepository::DepParser::parse_depend, _1,
                            *erepository::EAPIData::get_instance()->eapi_from_string("0")), mkt_dependencies)),
            has_masks(false)
        {
        }
    };
}

FakePackageID::FakePackageID(const Environment * const e, const tr1::shared_ptr<const FakeRepositoryBase> & r,
        const QualifiedPackageName & q, const VersionSpec & v) :
    PrivateImplementationPattern<FakePackageID>(new Implementation<FakePackageID>(e, r, q, v)),
    _imp(PrivateImplementationPattern<FakePackageID>::_imp.get())
{
    add_metadata_key(_imp->keywords);
    add_metadata_key(_imp->iuse);
    add_metadata_key(_imp->license);
    add_metadata_key(_imp->provide);
    add_metadata_key(_imp->build_dependencies);
    add_metadata_key(_imp->run_dependencies);
    add_metadata_key(_imp->post_dependencies);
    add_metadata_key(_imp->suggested_dependencies);
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

const tr1::shared_ptr<const MetadataPackageIDKey>
FakePackageID::virtual_for_key() const
{
    return _imp->virtual_for;
}

const tr1::shared_ptr<const MetadataSetKey<KeywordNameSet> >
FakePackageID::keywords_key() const
{
    return _imp->keywords;
}

const tr1::shared_ptr<const MetadataSetKey<IUseFlagSet> >
FakePackageID::iuse_key() const
{
    return _imp->iuse;
}

const tr1::shared_ptr<const MetadataSetKey<InheritedSet> >
FakePackageID::inherited_key() const
{
    return tr1::shared_ptr<const MetadataSetKey<InheritedSet> >();
}

const tr1::shared_ptr<const MetadataSetKey<UseFlagNameSet> >
FakePackageID::use_key() const
{
    return tr1::shared_ptr<const MetadataSetKey<UseFlagNameSet> >();
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

const tr1::shared_ptr<FakeMetadataKeywordSetKey>
FakePackageID::keywords_key()
{
    return _imp->keywords;
}

const tr1::shared_ptr<FakeMetadataIUseSetKey>
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

const tr1::shared_ptr<const MetadataContentsKey>
FakePackageID::contents_key() const
{
    return tr1::shared_ptr<const MetadataContentsKey>();
}

const tr1::shared_ptr<const MetadataTimeKey>
FakePackageID::installed_time_key() const
{
    return tr1::shared_ptr<const MetadataTimeKey>();
}

const tr1::shared_ptr<const MetadataStringKey>
FakePackageID::source_origin_key() const
{
    return tr1::shared_ptr<const MetadataStringKey>();
}

const tr1::shared_ptr<const MetadataStringKey>
FakePackageID::binary_origin_key() const
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

bool
FakePackageID::supports_action(const SupportsActionTestBase & b) const
{
    return repository()->some_ids_might_support_action(b);
}

namespace
{
    struct LicenceChecker :
        ConstVisitor<LicenseSpecTree>,
        ConstVisitor<LicenseSpecTree>::VisitConstSequence<LicenceChecker, AllDepSpec>
    {
        using ConstVisitor<LicenseSpecTree>::VisitConstSequence<LicenceChecker, AllDepSpec>::visit_sequence;

        bool ok;
        const Environment * const env;
        bool  (Environment::* const func) (const std::string &, const PackageID &) const;
        const PackageID * const id;

        LicenceChecker(const Environment * const e,
                bool (Environment::* const f) (const std::string &, const PackageID &) const,
                const PackageID * const d) :
            ok(true),
            env(e),
            func(f),
            id(d)
        {
        }

        void visit_sequence(const AnyDepSpec &,
                LicenseSpecTree::ConstSequenceIterator begin,
                LicenseSpecTree::ConstSequenceIterator end)
        {
            bool local_ok(false);

            if (begin == end)
                local_ok = true;
            else
            {
                for ( ; begin != end ; ++begin)
                {
                    Save<bool> save_ok(&ok, true);
                    begin->accept(*this);
                    local_ok |= ok;
                }
            }

            ok &= local_ok;
        }

        void visit_sequence(const UseDepSpec & spec,
                LicenseSpecTree::ConstSequenceIterator begin,
                LicenseSpecTree::ConstSequenceIterator end)
        {
            if (env->query_use(spec.flag(), *id))
                std::for_each(begin, end, accept_visitor(*this));
        }

        void visit_leaf(const PlainTextDepSpec & spec)
        {
            if (! (env->*func)(spec.text(), *id))
                ok = false;
        }
    };
}

void
FakePackageID::need_masks_added() const
{
    Lock l(_imp->mutex);

    if (_imp->has_masks)
        return;

    _imp->has_masks = true;

    Context context("When generating masks for ID '" + canonical_form(idcf_full) + "':");

    if (keywords_key())
        if (! _imp->env->accept_keywords(keywords_key()->value(), *this))
            add_mask(make_shared_ptr(new FakeUnacceptedMask('K', "keywords", keywords_key())));

    if (license_key())
    {
        LicenceChecker c(_imp->env, &Environment::accept_license, this);
        license_key()->value()->accept(c);
        if (! c.ok)
            add_mask(make_shared_ptr(new FakeUnacceptedMask('L', "license", license_key())));
    }

    if (! _imp->env->unmasked_by_user(*this))
    {
        tr1::shared_ptr<const Mask> user_mask(_imp->env->mask_for_user(*this));
        if (user_mask)
            add_mask(user_mask);
    }

    tr1::shared_ptr<const Mask> breaks_mask(_imp->env->mask_for_breakage(*this));
    if (breaks_mask)
        add_mask(breaks_mask);
}

namespace
{
    struct PerformAction :
        ConstVisitor<ActionVisitorTypes>
    {
        const PackageID * const id;

        PerformAction(const PackageID * const i) :
            id(i)
        {
        }

        void visit(const InstallAction & a)
        {
            SupportsActionTest<InstallAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw UnsupportedActionError(*id, a);
        }

        void visit(const InstalledAction & a)
        {
            SupportsActionTest<InstalledAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw UnsupportedActionError(*id, a);
        }

        void visit(const UninstallAction & a)
        {
            SupportsActionTest<UninstallAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw UnsupportedActionError(*id, a);
        }

        void visit(const ConfigAction & a)
        {
            SupportsActionTest<ConfigAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw UnsupportedActionError(*id, a);
        }

        void visit(const PretendAction & a)
        {
            SupportsActionTest<PretendAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw UnsupportedActionError(*id, a);
        }
    };
}

void
FakePackageID::perform_action(Action & a) const
{
    PerformAction b(this);
    a.accept(b);
}

bool
FakePackageID::breaks_portage() const
{
    return (version().has_try_part() || version().has_scm_part());
}

template class FakeMetadataSpecTreeKey<LicenseSpecTree>;
template class FakeMetadataSpecTreeKey<ProvideSpecTree>;
template class FakeMetadataSpecTreeKey<DependencySpecTree>;
template class FakeMetadataSpecTreeKey<RestrictSpecTree>;
template class FakeMetadataSpecTreeKey<URISpecTree>;

template class FakeMetadataSetKey<KeywordNameSet>;
template class FakeMetadataSetKey<IUseFlagSet>;

