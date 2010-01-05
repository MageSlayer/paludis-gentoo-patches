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

#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/repositories/fake/fake_repository_base.hh>
#include <paludis/repositories/fake/dep_parser.hh>
#include <paludis/name.hh>
#include <paludis/action.hh>
#include <paludis/environment.hh>
#include <paludis/version_spec.hh>
#include <paludis/formatter.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/dep_spec.hh>
#include <paludis/choice.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/set.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/save.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <map>
#include <list>
#include <sstream>

using namespace paludis;
using namespace paludis::fakerepository;

namespace paludis
{
#ifndef PALUDIS_NO_DOUBLE_TEMPLATE
    template <>
#endif
    template <typename C_>
    struct Implementation<FakeMetadataCollectionKey<C_> >
    {
        std::tr1::shared_ptr<C_> collection;
        const PackageID * const id;
        const Environment * const env;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Implementation(const PackageID * const i, const Environment * const e,
                const std::string & r, const std::string & h, const MetadataKeyType t) :
            id(i),
            env(e),
            raw_name(r),
            human_name(h),
            type(t)
        {
        }
    };
}

template <typename C_>
FakeMetadataCollectionKey<C_>::FakeMetadataCollectionKey(
        const std::string & r, const std::string & h, const MetadataKeyType t, const PackageID * const i,
        const Environment * const e) :
    PrivateImplementationPattern<FakeMetadataCollectionKey<C_> >(new Implementation<FakeMetadataCollectionKey<C_> >(i, e, r, h, t)),
    _imp(PrivateImplementationPattern<FakeMetadataCollectionKey<C_> >::_imp)
{
}

template <typename C_>
FakeMetadataCollectionKey<C_>::~FakeMetadataCollectionKey()
{
}

template <typename C_>
const std::tr1::shared_ptr<const C_>
FakeMetadataCollectionKey<C_>::value() const
{
    return this->_imp->collection;
}

template <typename C_>
const std::string
FakeMetadataCollectionKey<C_>::raw_name() const
{
    return this->_imp->raw_name;
}

template <typename C_>
const std::string
FakeMetadataCollectionKey<C_>::human_name() const
{
    return this->_imp->human_name;
}

template <typename C_>
MetadataKeyType
FakeMetadataCollectionKey<C_>::type() const
{
    return this->_imp->type;
}

FakeMetadataKeywordSetKey::FakeMetadataKeywordSetKey(const std::string & r,
        const std::string & h, const std::string & v, const MetadataKeyType t,
        const PackageID * const i, const Environment * const e) :
    FakeMetadataCollectionKey<KeywordNameSet>(r, h, t, i, e)
{
    set_from_string(v);
}

void
FakeMetadataKeywordSetKey::set_from_string(const std::string & s)
{
    _imp->collection.reset(new KeywordNameSet);
    tokenise_whitespace(s, create_inserter<KeywordName>(_imp->collection->inserter()));
}

namespace paludis
{
#ifndef PALUDIS_NO_DOUBLE_TEMPLATE
    template <>
#endif
    template <typename C_>
    struct Implementation<FakeMetadataSpecTreeKey<C_> >
    {
        std::tr1::shared_ptr<const C_> value;
        std::string string_value;
        const std::tr1::function<const std::tr1::shared_ptr<const C_> (const std::string &)> func;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Implementation(const std::tr1::function<const std::tr1::shared_ptr<const C_> (const std::string &)> & f,
                const std::string & r, const std::string & h, const MetadataKeyType t) :
            func(f),
            raw_name(r),
            human_name(h),
            type(t)
        {
        }
    };

    template <>
    struct Implementation<FakeMetadataSpecTreeKey<FetchableURISpecTree> >
    {
        std::tr1::shared_ptr<const FetchableURISpecTree> value;
        std::string string_value;
        const std::tr1::function<const std::tr1::shared_ptr<const FetchableURISpecTree> (const std::string &)> func;
        std::tr1::shared_ptr<const URILabel> initial_label;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Implementation(const std::tr1::function<const std::tr1::shared_ptr<const FetchableURISpecTree> (const std::string &)> & f,
                const std::string & r, const std::string & h, const MetadataKeyType t) :
            func(f),
            initial_label(new URIListedThenMirrorsLabel("listed-then-mirrors")),
            raw_name(r),
            human_name(h),
            type(t)
        {
        }
    };

    template <>
    struct Implementation<FakeMetadataSpecTreeKey<DependencySpecTree> >
    {
        std::tr1::shared_ptr<const DependencySpecTree> value;
        std::string string_value;
        const std::tr1::function<const std::tr1::shared_ptr<const DependencySpecTree> (const std::string &)> func;
        std::tr1::shared_ptr<const DependenciesLabelSequence> labels;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Implementation(const std::tr1::function<const std::tr1::shared_ptr<const DependencySpecTree> (const std::string &)> & f,
                const std::tr1::shared_ptr<const DependenciesLabelSequence> & s,
                const std::string & r, const std::string & h, const MetadataKeyType t) :
            func(f),
            labels(s),
            raw_name(r),
            human_name(h),
            type(t)
        {
        }
    };
}

template <typename C_>
FakeMetadataSpecTreeKey<C_>::FakeMetadataSpecTreeKey(const std::string & r, const std::string & h, const std::string & v,
        const std::tr1::function<const std::tr1::shared_ptr<const C_> (const std::string &)> & f, const MetadataKeyType t) :
    PrivateImplementationPattern<FakeMetadataSpecTreeKey<C_> >(new Implementation<FakeMetadataSpecTreeKey<C_> >(f, r, h, t)),
    _imp(PrivateImplementationPattern<FakeMetadataSpecTreeKey<C_> >::_imp)
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
const std::tr1::shared_ptr<const C_>
FakeMetadataSpecTreeKey<C_>::value() const
{
    return _imp->value;
}

template <typename C_>
const std::string
FakeMetadataSpecTreeKey<C_>::raw_name() const
{
    return this->_imp->raw_name;
}

template <typename C_>
const std::string
FakeMetadataSpecTreeKey<C_>::human_name() const
{
    return this->_imp->human_name;
}

template <typename C_>
MetadataKeyType
FakeMetadataSpecTreeKey<C_>::type() const
{
    return this->_imp->type;
}

template <typename C_>
std::string
FakeMetadataSpecTreeKey<C_>::pretty_print(const typename C_::ItemFormatter &) const
{
    return _imp->string_value;
}

template <typename C_>
std::string
FakeMetadataSpecTreeKey<C_>::pretty_print_flat(const typename C_::ItemFormatter &) const
{
    return _imp->string_value;
}

FakeMetadataSpecTreeKey<FetchableURISpecTree>::FakeMetadataSpecTreeKey(const std::string & r, const std::string & h, const std::string & v,
        const std::tr1::function<const std::tr1::shared_ptr<const FetchableURISpecTree> (const std::string &)> & f, const MetadataKeyType t) :
    PrivateImplementationPattern<FakeMetadataSpecTreeKey<FetchableURISpecTree> >(
            new Implementation<FakeMetadataSpecTreeKey<FetchableURISpecTree> >(f, r, h, t)),
    _imp(PrivateImplementationPattern<FakeMetadataSpecTreeKey<FetchableURISpecTree> >::_imp)
{
    set_from_string(v);
}

FakeMetadataSpecTreeKey<FetchableURISpecTree>::~FakeMetadataSpecTreeKey()
{
}

void
FakeMetadataSpecTreeKey<FetchableURISpecTree>::set_from_string(const std::string & s)
{
    _imp->string_value = s;
    _imp->value = _imp->func(s);
}

const std::string
FakeMetadataSpecTreeKey<FetchableURISpecTree>::raw_name() const
{
    return _imp->raw_name;
}

const std::string
FakeMetadataSpecTreeKey<FetchableURISpecTree>::human_name() const
{
    return _imp->human_name;
}

MetadataKeyType
FakeMetadataSpecTreeKey<FetchableURISpecTree>::type() const
{
    return _imp->type;
}

const std::tr1::shared_ptr<const FetchableURISpecTree>
FakeMetadataSpecTreeKey<FetchableURISpecTree>::value() const
{
    return _imp->value;
}

std::string
FakeMetadataSpecTreeKey<FetchableURISpecTree>::pretty_print(const FetchableURISpecTree::ItemFormatter &) const
{
    return _imp->string_value;
}

std::string
FakeMetadataSpecTreeKey<FetchableURISpecTree>::pretty_print_flat(const FetchableURISpecTree::ItemFormatter &) const
{
    return _imp->string_value;
}

const std::tr1::shared_ptr<const URILabel>
FakeMetadataSpecTreeKey<FetchableURISpecTree>::initial_label() const
{
    return _imp->initial_label;
}

FakeMetadataSpecTreeKey<DependencySpecTree>::FakeMetadataSpecTreeKey(const std::string & r, const std::string & h, const std::string & v,
        const std::tr1::function<const std::tr1::shared_ptr<const DependencySpecTree> (const std::string &)> & f,
        const std::tr1::shared_ptr<const DependenciesLabelSequence> & s, const MetadataKeyType t) :
    PrivateImplementationPattern<FakeMetadataSpecTreeKey<DependencySpecTree> >(
            new Implementation<FakeMetadataSpecTreeKey<DependencySpecTree> >(f, s, r, h, t)),
    _imp(PrivateImplementationPattern<FakeMetadataSpecTreeKey<DependencySpecTree> >::_imp)
{
    set_from_string(v);
}

FakeMetadataSpecTreeKey<DependencySpecTree>::~FakeMetadataSpecTreeKey()
{
}

void
FakeMetadataSpecTreeKey<DependencySpecTree>::set_from_string(const std::string & s)
{
    _imp->string_value = s;
    _imp->value = _imp->func(s);
}

const std::tr1::shared_ptr<const DependencySpecTree>
FakeMetadataSpecTreeKey<DependencySpecTree>::value() const
{
    return _imp->value;
}

std::string
FakeMetadataSpecTreeKey<DependencySpecTree>::pretty_print(const DependencySpecTree::ItemFormatter &) const
{
    return _imp->string_value;
}

std::string
FakeMetadataSpecTreeKey<DependencySpecTree>::pretty_print_flat(const DependencySpecTree::ItemFormatter &) const
{
    return _imp->string_value;
}

const std::tr1::shared_ptr<const DependenciesLabelSequence>
FakeMetadataSpecTreeKey<DependencySpecTree>::initial_labels() const
{
    return _imp->labels;
}

const std::string
FakeMetadataSpecTreeKey<DependencySpecTree>::raw_name() const
{
    return _imp->raw_name;
}

const std::string
FakeMetadataSpecTreeKey<DependencySpecTree>::human_name() const
{
    return _imp->human_name;
}

MetadataKeyType
FakeMetadataSpecTreeKey<DependencySpecTree>::type() const
{
    return _imp->type;
}

namespace paludis
{
    template <>
    struct Implementation<FakeMetadataChoicesKey>
    {
        const Environment * const env;
        const std::tr1::shared_ptr<const PackageID> id;
        std::tr1::shared_ptr<Choices> value;
        std::map<std::string, std::tr1::shared_ptr<Choice> > choices;

        Implementation(const Environment * const e, const std::tr1::shared_ptr<const PackageID> & i) :
            env(e),
            id(i),
            value(new Choices)
        {
        }
    };
}

namespace
{
    struct FakeChoiceValue :
        ChoiceValue
    {
        const Environment * const env;
        const std::tr1::shared_ptr<const PackageID> id;
        const std::tr1::shared_ptr<const Choice> choice;
        const UnprefixedChoiceName value_name;

        FakeChoiceValue(
                const Environment * const e,
                const std::tr1::shared_ptr<const PackageID> & i,
                const std::tr1::shared_ptr<const Choice> & c,
                const UnprefixedChoiceName & n) :
            env(e),
            id(i),
            choice(c),
            value_name(n)
        {
        }

        const UnprefixedChoiceName unprefixed_name() const
        {
            return value_name;
        }

        virtual const ChoiceNameWithPrefix name_with_prefix() const
        {
            return ChoiceNameWithPrefix((choice->prefix().data().empty() ? "" : stringify(choice->prefix()) + "*") + stringify(value_name));
        }

        virtual bool enabled() const
        {
            return env->want_choice_enabled(id, choice, value_name).is_true();
        }

        virtual bool enabled_by_default() const
        {
            return false;
        }

        virtual bool locked() const
        {
            return false;
        }

        virtual const std::string description() const
        {
            return "monkey";
        }

        virtual bool explicitly_listed() const
        {
            return true;
        }

        virtual const std::string parameter() const PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return env->value_for_choice_parameter(id, choice, value_name);
        }
    };
}

FakeMetadataChoicesKey::FakeMetadataChoicesKey(const Environment * const e,
        const std::tr1::shared_ptr<const PackageID> & i) :
    PrivateImplementationPattern<FakeMetadataChoicesKey>(new Implementation<FakeMetadataChoicesKey>(e, i)),
    _imp(PrivateImplementationPattern<FakeMetadataChoicesKey>::_imp)
{
}

FakeMetadataChoicesKey::~FakeMetadataChoicesKey()
{
}

void
FakeMetadataChoicesKey::add(const std::string & n, const std::string & v)
{
    if (_imp->choices.end() == _imp->choices.find(n))
    {
        std::tr1::shared_ptr<Choice> c(new Choice(make_named_values<ChoiceParams>(
                        value_for<n::consider_added_or_changed>(false),
                        value_for<n::contains_every_value>(false),
                        value_for<n::hidden>(false),
                        value_for<n::human_name>(n.empty() ? "default" : n),
                        value_for<n::prefix>(ChoicePrefixName(n)),
                        value_for<n::raw_name>(n.empty() ? "default" : n),
                        value_for<n::show_with_no_prefix>(false)
                        )));
        _imp->value->add(c);
        _imp->choices.insert(std::make_pair(n, c));
    }

    _imp->choices.find(n)->second->add(make_shared_ptr(new FakeChoiceValue(_imp->env, _imp->id,
                    _imp->choices.find(n)->second, UnprefixedChoiceName(v))));
}

const std::tr1::shared_ptr<const Choices>
FakeMetadataChoicesKey::value() const
{
    return _imp->value;
}

const std::string
FakeMetadataChoicesKey::raw_name() const
{
    return "PALUDIS_CHOICES";
}

const std::string
FakeMetadataChoicesKey::human_name() const
{
    return "Choices";
}

MetadataKeyType
FakeMetadataChoicesKey::type() const
{
    return mkt_internal;
}

namespace paludis
{
    template <>
    struct Implementation<FakeUnacceptedMask>
    {
        const char key;
        const std::string description;
        const std::tr1::shared_ptr<const MetadataKey> unaccepted_key;

        Implementation(const char k, const std::string & d, const std::tr1::shared_ptr<const MetadataKey> & u) :
            key(k),
            description(d),
            unaccepted_key(u)
        {
        }
    };
}

FakeUnacceptedMask::FakeUnacceptedMask(const char c, const std::string & s, const std::tr1::shared_ptr<const MetadataKey> & k) :
    PrivateImplementationPattern<FakeUnacceptedMask>(new Implementation<FakeUnacceptedMask>(c, s, k))
{
}

FakeUnacceptedMask::~FakeUnacceptedMask()
{
}

char
FakeUnacceptedMask::key() const
{
    return _imp->key;
}

const std::string
FakeUnacceptedMask::description() const
{
    return _imp->description;
}

const std::tr1::shared_ptr<const MetadataKey>
FakeUnacceptedMask::unaccepted_key() const
{
    return _imp->unaccepted_key;
}

FakeUnsupportedMask::FakeUnsupportedMask()
{
}

FakeUnsupportedMask::~FakeUnsupportedMask()
{
}

char
FakeUnsupportedMask::key() const
{
    return 'E';
}

const std::string
FakeUnsupportedMask::description() const
{
    return "Unsupported";
}

const std::string
FakeUnsupportedMask::explanation() const
{
    return "Marked as unsupported";
}

namespace paludis
{
    using namespace std::tr1::placeholders;

    template <>
    struct Implementation<FakePackageID>
    {
        mutable Mutex mutex;

        const Environment * const env;
        const std::tr1::shared_ptr<const FakeRepositoryBase> repository;
        const QualifiedPackageName name;
        const VersionSpec version;

        mutable std::tr1::shared_ptr<DependenciesLabelSequence> build_dependencies_labels;
        mutable std::tr1::shared_ptr<DependenciesLabelSequence> run_dependencies_labels;
        mutable std::tr1::shared_ptr<DependenciesLabelSequence> post_dependencies_labels;
        mutable std::tr1::shared_ptr<DependenciesLabelSequence> suggested_dependencies_labels;

        std::tr1::shared_ptr<LiteralMetadataValueKey<SlotName> > slot;
        std::tr1::shared_ptr<LiteralMetadataValueKey<std::tr1::shared_ptr<const PackageID> > > package_id;
        std::tr1::shared_ptr<LiteralMetadataValueKey<std::tr1::shared_ptr<const PackageID> > > virtual_for;
        std::tr1::shared_ptr<FakeMetadataKeywordSetKey> keywords;
        std::tr1::shared_ptr<FakeMetadataSpecTreeKey<LicenseSpecTree> > license;
        std::tr1::shared_ptr<FakeMetadataSpecTreeKey<ProvideSpecTree> > provide;
        std::tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> > build_dependencies;
        std::tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> > run_dependencies;
        std::tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> > post_dependencies;
        std::tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> > suggested_dependencies;
        std::tr1::shared_ptr<FakeMetadataSpecTreeKey<PlainTextSpecTree> > restrictions;
        std::tr1::shared_ptr<FakeMetadataSpecTreeKey<FetchableURISpecTree> > src_uri;
        std::tr1::shared_ptr<FakeMetadataSpecTreeKey<SimpleURISpecTree> > homepage;
        std::tr1::shared_ptr<FakeMetadataChoicesKey> choices;

        std::tr1::shared_ptr<Mask> unsupported_mask;
        mutable bool has_masks;

        Implementation(const Environment * const e, const std::tr1::shared_ptr<const FakeRepositoryBase> & r,
                const QualifiedPackageName & q, const VersionSpec & v, const PackageID * const id) :
            env(e),
            repository(r),
            name(q),
            version(v),
            build_dependencies_labels(new DependenciesLabelSequence),
            run_dependencies_labels(new DependenciesLabelSequence),
            post_dependencies_labels(new DependenciesLabelSequence),
            suggested_dependencies_labels(new DependenciesLabelSequence),
            slot(new LiteralMetadataValueKey<SlotName>("SLOT", "Slot", mkt_internal, SlotName("0"))),
            keywords(new FakeMetadataKeywordSetKey("KEYWORDS", "Keywords", "test", mkt_normal, id, env)),
            has_masks(false)
        {
            build_dependencies_labels->push_back(make_shared_ptr(new DependenciesBuildLabel("DEPEND",
                            return_literal_function(true))));
            run_dependencies_labels->push_back(make_shared_ptr(new DependenciesRunLabel("RDEPEND",
                            return_literal_function(true))));
            post_dependencies_labels->push_back(make_shared_ptr(new DependenciesPostLabel("PDEPEND",
                            return_literal_function(true))));
            suggested_dependencies_labels->push_back(make_shared_ptr(new DependenciesSuggestionLabel("SDEPEND",
                            return_literal_function(true))));
        }
    };
}

FakePackageID::FakePackageID(const Environment * const e, const std::tr1::shared_ptr<const FakeRepositoryBase> & r,
        const QualifiedPackageName & q, const VersionSpec & v) :
    PrivateImplementationPattern<FakePackageID>(new Implementation<FakePackageID>(e, r, q, v, this)),
    _imp(PrivateImplementationPattern<FakePackageID>::_imp)
{
    add_metadata_key(_imp->keywords);
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
            return stringify(_imp->name) + "-" + stringify(_imp->version) + ":" + stringify(_imp->slot->value())
                + "::" + stringify(_imp->repository->name());

        case idcf_version:
            return stringify(_imp->version);

        case idcf_no_version:
            return stringify(_imp->name) + ":" + stringify(_imp->slot->value()) + "::" + stringify(_imp->repository->name());

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

PackageDepSpec
FakePackageID::uniquely_identifying_spec() const
{
    return parse_user_package_dep_spec("=" + stringify(name()) + "-" + stringify(version()) +
            (slot_key() ? ":" + stringify(slot_key()->value()) : "") + "::" + stringify(repository()->name()),
            _imp->env, UserPackageDepSpecOptions());
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

const std::tr1::shared_ptr<const Repository>
FakePackageID::repository() const
{
    return _imp->repository;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
FakePackageID::virtual_for_key() const
{
    need_keys_added();
    return _imp->virtual_for;
}

const std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
FakePackageID::keywords_key() const
{
    need_keys_added();
    return _imp->keywords;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> >
FakePackageID::license_key() const
{
    need_keys_added();
    return _imp->license;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
FakePackageID::provide_key() const
{
    need_keys_added();
    return _imp->provide;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::build_dependencies_key() const
{
    need_keys_added();
    return _imp->build_dependencies;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::run_dependencies_key() const
{
    need_keys_added();
    return _imp->run_dependencies;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::post_dependencies_key() const
{
    need_keys_added();
    return _imp->post_dependencies;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::suggested_dependencies_key() const
{
    need_keys_added();
    return _imp->suggested_dependencies;
}

const std::tr1::shared_ptr<FakeMetadataKeywordSetKey>
FakePackageID::keywords_key()
{
    need_keys_added();
    return _imp->keywords;
}

const std::tr1::shared_ptr<FakeMetadataSpecTreeKey<ProvideSpecTree> >
FakePackageID::provide_key()
{
    need_keys_added();
    return _imp->provide;
}

const std::tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::build_dependencies_key()
{
    need_keys_added();
    return _imp->build_dependencies;
}

const std::tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::run_dependencies_key()
{
    need_keys_added();
    return _imp->run_dependencies;
}

const std::tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::post_dependencies_key()
{
    need_keys_added();
    return _imp->post_dependencies;
}

const std::tr1::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::suggested_dependencies_key()
{
    need_keys_added();
    return _imp->suggested_dependencies;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::dependencies_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
FakePackageID::fetches_key() const
{
    need_keys_added();
    return _imp->src_uri;
}

const std::tr1::shared_ptr<FakeMetadataSpecTreeKey<FetchableURISpecTree> >
FakePackageID::fetches_key()
{
    need_keys_added();
    return _imp->src_uri;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
FakePackageID::homepage_key() const
{
    need_keys_added();
    return _imp->homepage;
}

const std::tr1::shared_ptr<FakeMetadataSpecTreeKey<SimpleURISpecTree> >
FakePackageID::homepage_key()
{
    need_keys_added();
    return _imp->homepage;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
FakePackageID::short_description_key() const
{
    need_keys_added();
    return std::tr1::shared_ptr<const MetadataValueKey<std::string> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
FakePackageID::long_description_key() const
{
    need_keys_added();
    return std::tr1::shared_ptr<const MetadataValueKey<std::string> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >
FakePackageID::contents_key() const
{
    need_keys_added();
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >();
}

const std::tr1::shared_ptr<const MetadataTimeKey>
FakePackageID::installed_time_key() const
{
    need_keys_added();
    return std::tr1::shared_ptr<const MetadataTimeKey>();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
FakePackageID::from_repositories_key() const
{
    need_keys_added();
    return std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >();
}

void
FakePackageID::set_slot(const SlotName & s)
{
    _imp->slot->change_value(s);
}

bool
FakePackageID::arbitrary_less_than_comparison(const PackageID & other) const
{
    return slot_key()->value().data() < (other.slot_key() ? stringify(other.slot_key()->value()) : "");
}

void
FakePackageID::need_keys_added() const
{
    Lock l(_imp->mutex);

    if (! _imp->build_dependencies)
    {
        using namespace std::tr1::placeholders;

        _imp->build_dependencies.reset(new FakeMetadataSpecTreeKey<DependencySpecTree>("DEPEND", "Build dependencies",
                    "", std::tr1::bind(&parse_depend, _1, _imp->env,
                        shared_from_this()),
                    _imp->build_dependencies_labels, mkt_dependencies));

        _imp->run_dependencies.reset(new FakeMetadataSpecTreeKey<DependencySpecTree>("RDEPEND", "Run dependencies",
                    "", std::tr1::bind(&parse_depend, _1, _imp->env,
                        shared_from_this()),
                    _imp->run_dependencies_labels, mkt_dependencies)),

        _imp->post_dependencies.reset(new FakeMetadataSpecTreeKey<DependencySpecTree>("PDEPEND", "Post dependencies",
                    "", std::tr1::bind(&parse_depend, _1, _imp->env,
                        shared_from_this()),
                    _imp->post_dependencies_labels, mkt_dependencies)),

        _imp->suggested_dependencies.reset(new FakeMetadataSpecTreeKey<DependencySpecTree>("SDEPEND", "Suggested dependencies",
                    "", std::tr1::bind(&parse_depend, _1, _imp->env,
                        shared_from_this()),
                    _imp->suggested_dependencies_labels, mkt_dependencies)),

        _imp->src_uri.reset(new FakeMetadataSpecTreeKey<FetchableURISpecTree>("SRC_URI", "Source URI",
                    "", std::tr1::bind(&parse_fetchable_uri, _1, _imp->env,
                        shared_from_this()), mkt_normal));

        _imp->homepage.reset(new FakeMetadataSpecTreeKey<SimpleURISpecTree>("HOMEPAGE", "Homepage",
                    "", std::tr1::bind(&parse_simple_uri, _1, _imp->env,
                        shared_from_this()), mkt_normal));

        _imp->license.reset(new FakeMetadataSpecTreeKey<LicenseSpecTree>("LICENSE", "License",
                    "", std::tr1::bind(&parse_license, _1, _imp->env,
                        shared_from_this()), mkt_normal));

        _imp->provide.reset(new FakeMetadataSpecTreeKey<ProvideSpecTree>("PROVIDE", "Provide",
                    "", std::tr1::bind(&parse_provide, _1, _imp->env,
                        shared_from_this()), mkt_normal));

        _imp->choices.reset(new FakeMetadataChoicesKey(_imp->env, shared_from_this()));

        add_metadata_key(_imp->slot);
        add_metadata_key(_imp->build_dependencies);
        add_metadata_key(_imp->run_dependencies);
        add_metadata_key(_imp->post_dependencies);
        add_metadata_key(_imp->suggested_dependencies);
        add_metadata_key(_imp->src_uri);
        add_metadata_key(_imp->homepage);
        add_metadata_key(_imp->provide);
        add_metadata_key(_imp->license);
        add_metadata_key(_imp->choices);
    }
}

std::size_t
FakePackageID::extra_hash_value() const
{
    return Hash<SlotName>()(slot_key()->value());
}

bool
FakePackageID::supports_action(const SupportsActionTestBase & b) const
{
    return repository()->some_ids_might_support_action(b);
}

namespace
{
    struct LicenceChecker
    {
        bool ok;
        const Environment * const env;
        bool (Environment::* const func) (const std::string &, const PackageID &) const;
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

        void visit(const LicenseSpecTree::NodeType<AnyDepSpec>::Type & node)
        {
            bool local_ok(false);

            if (node.begin() == node.end())
                local_ok = true;
            else
            {
                for (LicenseSpecTree::NodeType<AnyDepSpec>::Type::ConstIterator c(node.begin()), c_end(node.end()) ;
                        c != c_end ; ++c)
                {
                    Save<bool> save_ok(&ok, true);
                    (*c)->accept(*this);
                    local_ok |= ok;
                }
            }

            ok &= local_ok;
        }

        void visit(const LicenseSpecTree::NodeType<AllDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const LicenseSpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            if (node.spec()->condition_met())
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const LicenseSpecTree::NodeType<LicenseDepSpec>::Type & node)
        {
            if (! (env->*func)(node.spec()->text(), *id))
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
        license_key()->value()->root()->accept(c);
        if (! c.ok)
            add_mask(make_shared_ptr(new FakeUnacceptedMask('L', "license", license_key())));
    }

    if (! _imp->env->unmasked_by_user(*this))
    {
        std::tr1::shared_ptr<const Mask> user_mask(_imp->env->mask_for_user(*this, false));
        if (user_mask)
            add_mask(user_mask);
    }
    else
    {
        std::tr1::shared_ptr<const Mask> user_mask(_imp->env->mask_for_user(*this, true));
        if (user_mask)
            add_overridden_mask(make_shared_ptr(new OverriddenMask(
                            make_named_values<OverriddenMask>(
                                value_for<n::mask>(user_mask),
                                value_for<n::override_reason>(mro_overridden_by_user)
                                ))));
    }

    std::tr1::shared_ptr<const Mask> breaks_mask(_imp->env->mask_for_breakage(*this));
    if (breaks_mask)
        add_mask(breaks_mask);

    if (_imp->unsupported_mask)
        add_mask(_imp->unsupported_mask);
}

void
FakePackageID::invalidate_masks() const
{
    Lock l(_imp->mutex);

    if (! _imp->has_masks)
        return;

    _imp->has_masks = false;
    PackageID::invalidate_masks();
}

void
FakePackageID::make_unsupported()
{
    invalidate_masks();
    _imp->unsupported_mask.reset(new FakeUnsupportedMask);
}

namespace
{
    struct PerformAction
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
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const UninstallAction & a)
        {
            SupportsActionTest<UninstallAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const FetchAction & a)
        {
            SupportsActionTest<FetchAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const ConfigAction & a)
        {
            SupportsActionTest<ConfigAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const InfoAction & a)
        {
            SupportsActionTest<InfoAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const PretendAction & a)
        {
            SupportsActionTest<PretendAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const PretendFetchAction & a)
        {
            SupportsActionTest<PretendFetchAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }
    };
}

void
FakePackageID::perform_action(Action & a) const
{
    PerformAction b(this);
    a.accept(b);
}

std::tr1::shared_ptr<const Set<std::string> >
FakePackageID::breaks_portage() const
{
    return std::tr1::shared_ptr<const Set<std::string> >();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >
FakePackageID::contains_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
FakePackageID::contained_in_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >();
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
FakePackageID::fs_location_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<long> >
FakePackageID::size_of_download_required_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<long> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<long> >
FakePackageID::size_of_all_distfiles_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<long> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<bool> >
FakePackageID::transient_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<bool> >();
}

char
FakePackageID::use_expand_separator() const
{
    return '_';
}

std::string
FakeMetadataKeywordSetKey::pretty_print_flat(const Formatter<KeywordName> & f) const
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

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > >
FakePackageID::choices_key() const
{
    need_keys_added();
    return _imp->choices;
}

const std::tr1::shared_ptr<FakeMetadataChoicesKey>
FakePackageID::choices_key()
{
    need_keys_added();
    return _imp->choices;
}

const std::tr1::shared_ptr<const MetadataValueKey<SlotName> >
FakePackageID::slot_key() const
{
    need_keys_added();
    return _imp->slot;
}

template class FakeMetadataSpecTreeKey<LicenseSpecTree>;
template class FakeMetadataSpecTreeKey<ProvideSpecTree>;
template class FakeMetadataSpecTreeKey<PlainTextSpecTree>;
#ifndef PALUDIS_NO_EXPLICIT_FULLY_SPECIALISED
template class FakeMetadataSpecTreeKey<FetchableURISpecTree>;
template class FakeMetadataSpecTreeKey<DependencySpecTree>;
template class FakeMetadataSpecTreeKey<DependencySpecTree>;
#endif
template class FakeMetadataSpecTreeKey<SimpleURISpecTree>;

template class FakeMetadataCollectionKey<KeywordNameSet>;

