/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011, 2013 Ciaran McCreesh
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
#include <paludis/literal_metadata_key.hh>
#include <paludis/dep_spec.hh>
#include <paludis/choice.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/always_enabled_dependency_label.hh>
#include <paludis/pretty_printer.hh>
#include <paludis/call_pretty_printer.hh>
#include <paludis/slot.hh>

#include <paludis/util/stringify.hh>
#include <paludis/util/pimp-impl.hh>
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
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/join.hh>

#include <map>
#include <list>
#include <sstream>
#include <algorithm>

using namespace paludis;
using namespace paludis::fakerepository;

namespace paludis
{
    template <typename C_>
    struct Imp<FakeMetadataCollectionKey<C_> >
    {
        std::shared_ptr<C_> collection;
        const std::shared_ptr<const PackageID> id;
        const Environment * const env;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Imp(const std::shared_ptr<const PackageID> & i, const Environment * const e,
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
        const std::string & r, const std::string & h, const MetadataKeyType t, const std::shared_ptr<const PackageID> & i,
        const Environment * const e) :
    _imp(i, e, r, h, t)
{
}

template <typename C_>
FakeMetadataCollectionKey<C_>::~FakeMetadataCollectionKey()
{
}

template <typename C_>
const std::shared_ptr<const C_>
FakeMetadataCollectionKey<C_>::parse_value() const
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

template <typename C_>
const std::string
FakeMetadataCollectionKey<C_>::pretty_print_value(
        const PrettyPrinter & pretty_printer,
        const PrettyPrintOptions &) const
{
    return join(_imp->collection->begin(), _imp->collection->end(), " ", CallPrettyPrinter(pretty_printer));
}

FakeMetadataKeywordSetKey::FakeMetadataKeywordSetKey(const std::string & r,
        const std::string & h, const std::string & v, const MetadataKeyType t,
        const std::shared_ptr<const PackageID> & i, const Environment * const e) :
    FakeMetadataCollectionKey<KeywordNameSet>(r, h, t, i, e)
{
    set_from_string(v);
}

void
FakeMetadataKeywordSetKey::set_from_string(const std::string & s)
{
    _imp->collection = std::make_shared<KeywordNameSet>();
    tokenise_whitespace(s, create_inserter<KeywordName>(_imp->collection->inserter()));
}

namespace paludis
{
    template <typename C_>
    struct Imp<FakeMetadataSpecTreeKey<C_> >
    {
        std::shared_ptr<const C_> value;
        std::string string_value;
        const std::function<const std::shared_ptr<const C_> (const std::string &)> func;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Imp(const std::function<const std::shared_ptr<const C_> (const std::string &)> & f,
                const std::string & r, const std::string & h, const MetadataKeyType t) :
            func(f),
            raw_name(r),
            human_name(h),
            type(t)
        {
        }
    };

    template <>
    struct Imp<FakeMetadataSpecTreeKey<FetchableURISpecTree> >
    {
        std::shared_ptr<const FetchableURISpecTree> value;
        std::string string_value;
        const std::function<const std::shared_ptr<const FetchableURISpecTree> (const std::string &)> func;
        std::shared_ptr<const URILabel> initial_label;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Imp(const std::function<const std::shared_ptr<const FetchableURISpecTree> (const std::string &)> & f,
                const std::string & r, const std::string & h, const MetadataKeyType t) :
            func(f),
            initial_label(std::make_shared<URIListedThenMirrorsLabel>("listed-then-mirrors")),
            raw_name(r),
            human_name(h),
            type(t)
        {
        }
    };

    template <>
    struct Imp<FakeMetadataSpecTreeKey<DependencySpecTree> >
    {
        std::shared_ptr<const DependencySpecTree> value;
        std::string string_value;
        const std::function<const std::shared_ptr<const DependencySpecTree> (const std::string &)> func;
        std::shared_ptr<const DependenciesLabelSequence> labels;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Imp(const std::function<const std::shared_ptr<const DependencySpecTree> (const std::string &)> & f,
                const std::shared_ptr<const DependenciesLabelSequence> & s,
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
        const std::function<const std::shared_ptr<const C_> (const std::string &)> & f, const MetadataKeyType t) :
    _imp(f, r, h, t)
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
const std::shared_ptr<const C_>
FakeMetadataSpecTreeKey<C_>::parse_value() const
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
const std::string
FakeMetadataSpecTreeKey<C_>::pretty_print_value(
        const PrettyPrinter &, const PrettyPrintOptions &) const
{
    return _imp->string_value;
}

FakeMetadataSpecTreeKey<FetchableURISpecTree>::FakeMetadataSpecTreeKey(const std::string & r, const std::string & h, const std::string & v,
        const std::function<const std::shared_ptr<const FetchableURISpecTree> (const std::string &)> & f, const MetadataKeyType t) :
    _imp(f, r, h, t)
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

const std::shared_ptr<const FetchableURISpecTree>
FakeMetadataSpecTreeKey<FetchableURISpecTree>::parse_value() const
{
    return _imp->value;
}

const std::string
FakeMetadataSpecTreeKey<FetchableURISpecTree>::pretty_print_value(const PrettyPrinter &, const PrettyPrintOptions &) const
{
    return _imp->string_value;
}

const std::shared_ptr<const URILabel>
FakeMetadataSpecTreeKey<FetchableURISpecTree>::initial_label() const
{
    return _imp->initial_label;
}

FakeMetadataSpecTreeKey<DependencySpecTree>::FakeMetadataSpecTreeKey(const std::string & r, const std::string & h, const std::string & v,
        const std::function<const std::shared_ptr<const DependencySpecTree> (const std::string &)> & f,
        const std::shared_ptr<const DependenciesLabelSequence> & s, const MetadataKeyType t) :
    _imp(f, s, r, h, t)
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

const std::shared_ptr<const DependencySpecTree>
FakeMetadataSpecTreeKey<DependencySpecTree>::parse_value() const
{
    return _imp->value;
}

const std::string
FakeMetadataSpecTreeKey<DependencySpecTree>::pretty_print_value(const PrettyPrinter &, const PrettyPrintOptions &) const
{
    return _imp->string_value;
}

const std::shared_ptr<const DependenciesLabelSequence>
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
    struct Imp<FakeMetadataChoicesKey>
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> id;
        std::shared_ptr<Choices> value;
        std::map<std::string, std::shared_ptr<Choice> > choices;

        Imp(const Environment * const e, const std::shared_ptr<const PackageID> & i) :
            env(e),
            id(i),
            value(std::make_shared<Choices>())
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
        const std::shared_ptr<const PackageID> id;
        const std::shared_ptr<const Choice> choice;
        const UnprefixedChoiceName value_name;

        FakeChoiceValue(
                const Environment * const e,
                const std::shared_ptr<const PackageID> & i,
                const std::shared_ptr<const Choice> & c,
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
            return ChoiceNameWithPrefix((choice->prefix().value().empty() ? "" : stringify(choice->prefix()) + ":") + stringify(value_name));
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

        virtual ChoiceOrigin origin() const
        {
            return co_explicit;
        }

        virtual const std::string parameter() const PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return env->value_for_choice_parameter(id, choice, value_name);
        }

        virtual const std::shared_ptr<const PermittedChoiceValueParameterValues> permitted_parameter_values() const
        {
            return nullptr;
        }

        virtual bool presumed() const PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return false;
        }
    };
}

FakeMetadataChoicesKey::FakeMetadataChoicesKey(const Environment * const e,
        const std::shared_ptr<const PackageID> & i) :
    _imp(e, i)
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
        std::shared_ptr<Choice> c(std::make_shared<Choice>(make_named_values<ChoiceParams>(
                        n::consider_added_or_changed() = false,
                        n::contains_every_value() = false,
                        n::hidden() = false,
                        n::human_name() = n.empty() ? "default" : n,
                        n::prefix() = ChoicePrefixName(n),
                        n::raw_name() = n.empty() ? "default" : n,
                        n::show_with_no_prefix() = false
                        )));
        _imp->value->add(c);
        _imp->choices.insert(std::make_pair(n, c));
    }

    _imp->choices.find(n)->second->add(std::make_shared<FakeChoiceValue>(_imp->env, _imp->id,
                    _imp->choices.find(n)->second, UnprefixedChoiceName(v)));
}

const std::shared_ptr<const Choices>
FakeMetadataChoicesKey::parse_value() const
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
    struct Imp<FakeUnacceptedMask>
    {
        const char key;
        const std::string description;
        const std::string unaccepted_key_name;

        Imp(const char k, const std::string & d, const std::string & u) :
            key(k),
            description(d),
            unaccepted_key_name(u)
        {
        }
    };
}

FakeUnacceptedMask::FakeUnacceptedMask(const char c, const std::string & s, const std::string & k) :
    _imp(c, s, k)
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

const std::string
FakeUnacceptedMask::unaccepted_key_name() const
{
    return _imp->unaccepted_key_name;
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

namespace
{
    struct FakePackageIDData :
        Singleton<FakePackageIDData>
    {
        std::shared_ptr<DependenciesLabelSequence> build_dependencies_labels;
        std::shared_ptr<DependenciesLabelSequence> run_dependencies_labels;
        std::shared_ptr<DependenciesLabelSequence> post_dependencies_labels;

        FakePackageIDData() :
            build_dependencies_labels(std::make_shared<DependenciesLabelSequence>()),
            run_dependencies_labels(std::make_shared<DependenciesLabelSequence>()),
            post_dependencies_labels(std::make_shared<DependenciesLabelSequence>())
        {
            build_dependencies_labels->push_back(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesBuildLabelTag> >("DEPEND"));
            run_dependencies_labels->push_back(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesRunLabelTag> >("RDEPEND"));
            post_dependencies_labels->push_back(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesPostLabelTag> >("PDEPEND"));
        }
    };
}

namespace paludis
{
    using namespace std::placeholders;

    template <>
    struct Imp<FakePackageID>
    {
        mutable std::recursive_mutex mutex;

        const Environment * const env;
        const RepositoryName repository_name;
        const QualifiedPackageName name;
        const VersionSpec version;

        mutable std::shared_ptr<LiteralMetadataValueKey<Slot> > slot;
        mutable std::shared_ptr<FakeMetadataKeywordSetKey> keywords;
        mutable std::shared_ptr<FakeMetadataSpecTreeKey<LicenseSpecTree> > license;
        mutable std::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> > build_dependencies;
        mutable std::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> > run_dependencies;
        mutable std::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> > post_dependencies;
        mutable std::shared_ptr<FakeMetadataSpecTreeKey<PlainTextSpecTree> > restrictions;
        mutable std::shared_ptr<FakeMetadataSpecTreeKey<FetchableURISpecTree> > src_uri;
        mutable std::shared_ptr<FakeMetadataSpecTreeKey<SimpleURISpecTree> > homepage;
        mutable std::shared_ptr<FakeMetadataChoicesKey> choices;
        mutable std::shared_ptr<LiteralMetadataValueKey<long> > hitchhiker;
        mutable std::shared_ptr<LiteralMetadataStringSetKey> behaviours;

        std::shared_ptr<Set<std::string> > behaviours_set;

        std::shared_ptr<Mask> unsupported_mask;
        mutable bool has_masks;

        Imp(const Environment * const e, const RepositoryName & r,
                const QualifiedPackageName & q, const VersionSpec & v) :
            env(e),
            repository_name(r),
            name(q),
            version(v),
            slot(std::make_shared<LiteralMetadataValueKey<Slot>>("SLOT", "Slot", mkt_internal, make_named_values<Slot>(
                            n::match_values() = std::make_pair(SlotName("0"), SlotName("0")),
                            n::parallel_value() = SlotName("0"),
                            n::raw_value() = "0"))),
            behaviours_set(std::make_shared<Set<std::string>>()),
            has_masks(false)
        {
        }
    };
}

FakePackageID::FakePackageID(const Environment * const e, const RepositoryName & r,
        const QualifiedPackageName & q, const VersionSpec & v) :
    _imp(e, r, q, v)
{
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
            return stringify(_imp->name) + "-" + stringify(_imp->version) + ":" + stringify(_imp->slot->parse_value().parallel_value())
                + "::" + stringify(repository_name());

        case idcf_version:
            return stringify(_imp->version);

        case idcf_no_version:
            return stringify(_imp->name) + ":" + stringify(_imp->slot->parse_value().parallel_value()) + "::" + stringify(repository_name());

        case idcf_no_name:
            return stringify(_imp->version) + ":" + stringify(_imp->slot->parse_value().parallel_value())
                + "::" + stringify(repository_name());

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

PackageDepSpec
FakePackageID::uniquely_identifying_spec() const
{
    return parse_user_package_dep_spec("=" + stringify(name()) + "-" + stringify(version()) +
            (slot_key() ? ":" + stringify(slot_key()->parse_value().parallel_value()) : "") + "::" + stringify(repository_name()),
            _imp->env, { });
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

const RepositoryName
FakePackageID::repository_name() const
{
    return _imp->repository_name;
}

const std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
FakePackageID::keywords_key() const
{
    need_keys_added();
    return _imp->keywords;
}

const std::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> >
FakePackageID::license_key() const
{
    need_keys_added();
    return _imp->license;
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::build_dependencies_key() const
{
    need_keys_added();
    return _imp->build_dependencies;
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::run_dependencies_key() const
{
    need_keys_added();
    return _imp->run_dependencies;
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::post_dependencies_key() const
{
    need_keys_added();
    return _imp->post_dependencies;
}

const std::shared_ptr<FakeMetadataKeywordSetKey>
FakePackageID::keywords_key()
{
    need_keys_added();
    return _imp->keywords;
}

const std::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::build_dependencies_key()
{
    need_keys_added();
    return _imp->build_dependencies;
}

const std::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::run_dependencies_key()
{
    need_keys_added();
    return _imp->run_dependencies;
}

const std::shared_ptr<FakeMetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::post_dependencies_key()
{
    need_keys_added();
    return _imp->post_dependencies;
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
FakePackageID::dependencies_key() const
{
    return nullptr;
}

const std::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
FakePackageID::fetches_key() const
{
    need_keys_added();
    return _imp->src_uri;
}

const std::shared_ptr<FakeMetadataSpecTreeKey<FetchableURISpecTree> >
FakePackageID::fetches_key()
{
    need_keys_added();
    return _imp->src_uri;
}

const std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
FakePackageID::homepage_key() const
{
    need_keys_added();
    return _imp->homepage;
}

const std::shared_ptr<FakeMetadataSpecTreeKey<SimpleURISpecTree> >
FakePackageID::homepage_key()
{
    need_keys_added();
    return _imp->homepage;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
FakePackageID::short_description_key() const
{
    need_keys_added();
    return std::shared_ptr<const MetadataValueKey<std::string> >();
}

const std::shared_ptr<const MetadataValueKey<std::string> >
FakePackageID::long_description_key() const
{
    need_keys_added();
    return std::shared_ptr<const MetadataValueKey<std::string> >();
}

const std::shared_ptr<const MetadataTimeKey>
FakePackageID::installed_time_key() const
{
    need_keys_added();
    return std::shared_ptr<const MetadataTimeKey>();
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
FakePackageID::from_repositories_key() const
{
    need_keys_added();
    return std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >();
}

void
FakePackageID::set_slot(const SlotName & s)
{
    _imp->slot->change_value(make_named_values<Slot>(
                n::match_values() = std::make_pair(s, s),
                n::parallel_value() = s,
                n::raw_value() = stringify(s)));
}

bool
FakePackageID::arbitrary_less_than_comparison(const PackageID & other) const
{
    return stringify(slot_key()->parse_value().raw_value()) < (other.slot_key() ? stringify(other.slot_key()->parse_value().raw_value()) : "");
}

void
FakePackageID::need_keys_added() const
{
    std::unique_lock<std::recursive_mutex> lock(_imp->mutex);

    if (! _imp->build_dependencies)
    {
        using namespace std::placeholders;

        _imp->build_dependencies = std::make_shared<FakeMetadataSpecTreeKey<DependencySpecTree>>("DEPEND", "Build dependencies",
                    "", std::bind(&parse_depend, _1, _imp->env),
                    FakePackageIDData::get_instance()->build_dependencies_labels, mkt_dependencies);

        _imp->run_dependencies = std::make_shared<FakeMetadataSpecTreeKey<DependencySpecTree>>("RDEPEND", "Run dependencies",
                    "", std::bind(&parse_depend, _1, _imp->env),
                    FakePackageIDData::get_instance()->run_dependencies_labels, mkt_dependencies);

        _imp->post_dependencies = std::make_shared<FakeMetadataSpecTreeKey<DependencySpecTree>>("PDEPEND", "Post dependencies",
                    "", std::bind(&parse_depend, _1, _imp->env),
                    FakePackageIDData::get_instance()->post_dependencies_labels, mkt_dependencies);

        _imp->src_uri = std::make_shared<FakeMetadataSpecTreeKey<FetchableURISpecTree>>("SRC_URI", "Source URI",
                    "", std::bind(&parse_fetchable_uri, _1, _imp->env), mkt_normal);

        _imp->homepage = std::make_shared<FakeMetadataSpecTreeKey<SimpleURISpecTree>>("HOMEPAGE", "Homepage",
                    "", std::bind(&parse_simple_uri, _1, _imp->env), mkt_normal);

        _imp->license = std::make_shared<FakeMetadataSpecTreeKey<LicenseSpecTree>>("LICENSE", "License",
                    "", std::bind(&parse_license, _1, _imp->env), mkt_normal);

        _imp->choices = std::make_shared<FakeMetadataChoicesKey>(_imp->env, shared_from_this());

        _imp->behaviours = std::make_shared<LiteralMetadataStringSetKey>("BEHAVIOURS", "Behaviours",
                    mkt_internal, _imp->behaviours_set);

        _imp->hitchhiker = std::make_shared<LiteralMetadataValueKey<long>>("HITCHHIKER", "Hitchhiker",
                    mkt_internal, 42);

        _imp->keywords = std::make_shared<FakeMetadataKeywordSetKey>("KEYWORDS", "Keywords", "test", mkt_normal, shared_from_this(), _imp->env);

        add_metadata_key(_imp->slot);
        add_metadata_key(_imp->build_dependencies);
        add_metadata_key(_imp->run_dependencies);
        add_metadata_key(_imp->post_dependencies);
        add_metadata_key(_imp->src_uri);
        add_metadata_key(_imp->homepage);
        add_metadata_key(_imp->license);
        add_metadata_key(_imp->choices);
        add_metadata_key(_imp->behaviours);
        add_metadata_key(_imp->hitchhiker);
        add_metadata_key(_imp->keywords);
    }
}

std::size_t
FakePackageID::extra_hash_value() const
{
    return Hash<std::string>()(slot_key()->parse_value().raw_value());
}

bool
FakePackageID::supports_action(const SupportsActionTestBase & b) const
{
    auto repo(_imp->env->fetch_repository(repository_name()));
    return repo->some_ids_might_support_action(b);
}

namespace
{
    struct LicenceChecker
    {
        bool ok;
        const Environment * const env;
        bool (Environment::* const func) (const std::string &, const std::shared_ptr<const PackageID> &) const;
        const std::shared_ptr<const PackageID> id;

        LicenceChecker(const Environment * const e,
                bool (Environment::* const f) (const std::string &, const std::shared_ptr<const PackageID> &) const,
                const std::shared_ptr<const PackageID> & i) :
            ok(true),
            env(e),
            func(f),
            id(i)
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
            if (node.spec()->condition_met(env, id))
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const LicenseSpecTree::NodeType<LicenseDepSpec>::Type & node)
        {
            if (! (env->*func)(node.spec()->text(), id))
                ok = false;
        }
    };
}

void
FakePackageID::need_masks_added() const
{
    std::unique_lock<std::recursive_mutex> lock(_imp->mutex);

    if (_imp->has_masks)
        return;

    _imp->has_masks = true;

    Context context("When generating masks for ID '" + canonical_form(idcf_full) + "':");

    if (keywords_key())
        if (! _imp->env->accept_keywords(keywords_key()->parse_value(), shared_from_this()))
            add_mask(std::make_shared<FakeUnacceptedMask>('K', "keywords", keywords_key()->raw_name()));

    if (license_key())
    {
        LicenceChecker c(_imp->env, &Environment::accept_license, shared_from_this());
        license_key()->parse_value()->top()->accept(c);
        if (! c.ok)
            add_mask(std::make_shared<FakeUnacceptedMask>('L', "license", license_key()->raw_name()));
    }

    if (! _imp->env->unmasked_by_user(shared_from_this(), ""))
    {
        std::shared_ptr<const Mask> user_mask(_imp->env->mask_for_user(shared_from_this(), false));
        if (user_mask)
            add_mask(user_mask);
    }
    else
    {
        std::shared_ptr<const Mask> user_mask(_imp->env->mask_for_user(shared_from_this(), true));
        if (user_mask)
            add_overridden_mask(std::make_shared<OverriddenMask>(
                            make_named_values<OverriddenMask>(
                                n::mask() = user_mask,
                                n::override_reason() = mro_overridden_by_user
                                )));
    }

    std::shared_ptr<const Mask> breaks_mask(_imp->env->mask_for_breakage(shared_from_this()));
    if (breaks_mask)
        add_mask(breaks_mask);

    if (_imp->unsupported_mask)
        add_mask(_imp->unsupported_mask);
}

namespace
{
    struct PerformAction
    {
        const Environment * const env;
        const PackageID * const id;

        void visit(const InstallAction & a)
        {
            SupportsActionTest<InstallAction> t;
            auto repo(env->fetch_repository(id->repository_name()));
            if (! repo->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const UninstallAction & a)
        {
            SupportsActionTest<UninstallAction> t;
            auto repo(env->fetch_repository(id->repository_name()));
            if (! repo->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const FetchAction & a)
        {
            SupportsActionTest<FetchAction> t;
            auto repo(env->fetch_repository(id->repository_name()));
            if (! repo->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const ConfigAction & a)
        {
            SupportsActionTest<ConfigAction> t;
            auto repo(env->fetch_repository(id->repository_name()));
            if (! repo->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const InfoAction & a)
        {
            SupportsActionTest<InfoAction> t;
            auto repo(env->fetch_repository(id->repository_name()));
            if (! repo->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const PretendAction & a)
        {
            SupportsActionTest<PretendAction> t;
            auto repo(env->fetch_repository(id->repository_name()));
            if (! repo->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const PretendFetchAction & a)
        {
            SupportsActionTest<PretendFetchAction> t;
            auto repo(env->fetch_repository(id->repository_name()));
            if (! repo->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }
    };
}

void
FakePackageID::perform_action(Action & a) const
{
    PerformAction b{_imp->env, this};
    a.accept(b);
}

std::shared_ptr<const Set<std::string> >
FakePackageID::breaks_portage() const
{
    return std::shared_ptr<const Set<std::string> >();
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
FakePackageID::fs_location_key() const
{
    return std::shared_ptr<const MetadataValueKey<FSPath> >();
}

const std::shared_ptr<const MetadataValueKey<long> >
FakePackageID::size_of_download_required_key() const
{
    return std::shared_ptr<const MetadataValueKey<long> >();
}

const std::shared_ptr<const MetadataValueKey<long> >
FakePackageID::size_of_all_distfiles_key() const
{
    return std::shared_ptr<const MetadataValueKey<long> >();
}

char
FakePackageID::use_expand_separator() const
{
    return '_';
}

const std::string
FakeMetadataKeywordSetKey::pretty_print_value(
        const PrettyPrinter & pretty_printer, const PrettyPrintOptions &) const
{
    return join(_imp->collection->begin(), _imp->collection->end(), " ", CallPrettyPrinter(pretty_printer));
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > >
FakePackageID::choices_key() const
{
    need_keys_added();
    return _imp->choices;
}

const std::shared_ptr<FakeMetadataChoicesKey>
FakePackageID::choices_key()
{
    need_keys_added();
    return _imp->choices;
}

const std::shared_ptr<const MetadataValueKey<Slot> >
FakePackageID::slot_key() const
{
    need_keys_added();
    return _imp->slot;
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
FakePackageID::behaviours_key() const
{
    need_keys_added();
    return _imp->behaviours;
}

const std::shared_ptr<Set<std::string> >
FakePackageID::behaviours_set()
{
    return _imp->behaviours_set;
}

const std::shared_ptr<const Contents>
FakePackageID::contents() const
{
    return nullptr;
}

namespace paludis
{
    template class FakeMetadataSpecTreeKey<LicenseSpecTree>;
    template class FakeMetadataSpecTreeKey<PlainTextSpecTree>;
#ifndef PALUDIS_NO_EXPLICIT_FULLY_SPECIALISED
    template class FakeMetadataSpecTreeKey<FetchableURISpecTree>;
    template class FakeMetadataSpecTreeKey<DependencySpecTree>;
#endif
    template class FakeMetadataSpecTreeKey<SimpleURISpecTree>;

    template class FakeMetadataCollectionKey<KeywordNameSet>;
}

