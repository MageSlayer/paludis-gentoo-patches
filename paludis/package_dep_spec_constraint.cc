/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#include <paludis/package_dep_spec_constraint.hh>
#include <paludis/version_spec.hh>
#include <paludis/version_operator.hh>
#include <paludis/contents.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_id.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>

#include <paludis/util/pool-impl.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/join.hh>

#include <istream>
#include <ostream>
#include <algorithm>

using namespace paludis;

#include <paludis/package_dep_spec_constraint-se.cc>

PackageDepSpecConstraint::~PackageDepSpecConstraint() = default;

NameConstraint::NameConstraint(const QualifiedPackageName & n) :
    _name(n)
{
}

NameConstraint::~NameConstraint() = default;

const QualifiedPackageName
NameConstraint::name() const
{
    return _name;
}

template class Pool<NameConstraint>;
template class Singleton<Pool<NameConstraint> >;
template const std::shared_ptr<const NameConstraint> Pool<NameConstraint>::create(
        const QualifiedPackageName &) const;

CategoryNamePartConstraint::CategoryNamePartConstraint(const CategoryNamePart & n) :
    _name_part(n)
{
}

CategoryNamePartConstraint::~CategoryNamePartConstraint() = default;

const CategoryNamePart
CategoryNamePartConstraint::name_part() const
{
    return _name_part;
}

template class Pool<CategoryNamePartConstraint>;
template class Singleton<Pool<CategoryNamePartConstraint> >;
template const std::shared_ptr<const CategoryNamePartConstraint> Pool<CategoryNamePartConstraint>::create(
        const CategoryNamePart &) const;

PackageNamePartConstraint::PackageNamePartConstraint(const PackageNamePart & n) :
    _name_part(n)
{
}

PackageNamePartConstraint::~PackageNamePartConstraint() = default;

const PackageNamePart
PackageNamePartConstraint::name_part() const
{
    return _name_part;
}

template class Pool<PackageNamePartConstraint>;
template class Singleton<Pool<PackageNamePartConstraint> >;
template const std::shared_ptr<const PackageNamePartConstraint> Pool<PackageNamePartConstraint>::create(
        const PackageNamePart &) const;

namespace paludis
{
    template <>
    struct Imp<VersionConstraint>
    {
        VersionSpec spec;
        VersionOperator op;
        VersionConstraintCombiner combiner;

        Imp(const VersionSpec & s, const VersionOperator & o, const VersionConstraintCombiner c) :
            spec(s),
            op(o),
            combiner(c)
        {
        }
    };
}

VersionConstraint::VersionConstraint(const VersionSpec & s, const VersionOperator & o, const VersionConstraintCombiner c) :
    _imp(s, o, c)
{
}

VersionConstraint::~VersionConstraint() = default;

const VersionSpec
VersionConstraint::version_spec() const
{
    return _imp->spec;
}

const VersionOperator
VersionConstraint::version_operator() const
{
    return _imp->op;
}

VersionConstraintCombiner
VersionConstraint::combiner() const
{
    return _imp->combiner;
}

template class Sequence<std::shared_ptr<const VersionConstraint> >;
template class WrappedForwardIterator<Sequence<std::shared_ptr<const VersionConstraint> >::ConstIteratorTag, const std::shared_ptr<const VersionConstraint> >;
template class Pimp<VersionConstraint>;

InRepositoryConstraint::InRepositoryConstraint(const RepositoryName & n) :
    _name(n)
{
}

InRepositoryConstraint::~InRepositoryConstraint() = default;

const RepositoryName
InRepositoryConstraint::name() const
{
    return _name;
}

template class Pool<InRepositoryConstraint>;
template class Singleton<Pool<InRepositoryConstraint> >;
template const std::shared_ptr<const InRepositoryConstraint> Pool<InRepositoryConstraint>::create(
        const RepositoryName &) const;

FromRepositoryConstraint::FromRepositoryConstraint(const RepositoryName & n) :
    _name(n)
{
}

FromRepositoryConstraint::~FromRepositoryConstraint() = default;

const RepositoryName
FromRepositoryConstraint::name() const
{
    return _name;
}

template class Pool<FromRepositoryConstraint>;
template class Singleton<Pool<FromRepositoryConstraint> >;
template const std::shared_ptr<const FromRepositoryConstraint> Pool<FromRepositoryConstraint>::create(
        const RepositoryName &) const;

InstalledAtPathConstraint::InstalledAtPathConstraint(const FSPath & n) :
    _path(n)
{
}

InstalledAtPathConstraint::~InstalledAtPathConstraint() = default;

const FSPath
InstalledAtPathConstraint::path() const
{
    return _path;
}

template class Pool<InstalledAtPathConstraint>;
template class Singleton<Pool<InstalledAtPathConstraint> >;
template const std::shared_ptr<const InstalledAtPathConstraint> Pool<InstalledAtPathConstraint>::create(
        const FSPath &) const;

InstallableToPathConstraint::InstallableToPathConstraint(const FSPath & n, const bool i) :
    _path(n),
    _include_masked(i)
{
}

InstallableToPathConstraint::~InstallableToPathConstraint() = default;

const FSPath
InstallableToPathConstraint::path() const
{
    return _path;
}

bool
InstallableToPathConstraint::include_masked() const
{
    return _include_masked;
}

template class Pool<InstallableToPathConstraint>;
template class Singleton<Pool<InstallableToPathConstraint> >;
template const std::shared_ptr<const InstallableToPathConstraint> Pool<InstallableToPathConstraint>::create(
        const FSPath &, const bool &) const;

InstallableToRepositoryConstraint::InstallableToRepositoryConstraint(const RepositoryName & n, const bool i) :
    _name(n),
    _include_masked(i)
{
}

InstallableToRepositoryConstraint::~InstallableToRepositoryConstraint() = default;

const RepositoryName
InstallableToRepositoryConstraint::name() const
{
    return _name;
}

bool
InstallableToRepositoryConstraint::include_masked() const
{
    return _include_masked;
}

template class Pool<InstallableToRepositoryConstraint>;
template class Singleton<Pool<InstallableToRepositoryConstraint> >;
template const std::shared_ptr<const InstallableToRepositoryConstraint> Pool<InstallableToRepositoryConstraint>::create(
        const RepositoryName &, const bool &) const;

ExactSlotConstraint::ExactSlotConstraint(const SlotName & n, const bool i) :
    _name(n),
    _locked(i)
{
}

ExactSlotConstraint::~ExactSlotConstraint() = default;

const SlotName
ExactSlotConstraint::name() const
{
    return _name;
}

bool
ExactSlotConstraint::locked() const
{
    return _locked;
}

template class Pool<ExactSlotConstraint>;
template class Singleton<Pool<ExactSlotConstraint> >;
template const std::shared_ptr<const ExactSlotConstraint> Pool<ExactSlotConstraint>::create(const SlotName &, const bool &) const;

AnySlotConstraint::AnySlotConstraint(const bool i) :
    _locking(i)
{
}

AnySlotConstraint::~AnySlotConstraint() = default;

bool
AnySlotConstraint::locking() const
{
    return _locking;
}

template class Pool<AnySlotConstraint>;
template class Singleton<Pool<AnySlotConstraint> >;
template const std::shared_ptr<const AnySlotConstraint> Pool<AnySlotConstraint>::create(const bool &) const;

KeyConstraint::KeyConstraint(const KeyConstraintKeyType t, const std::string & k, const KeyConstraintOperation o, const std::string & p) :
    _key_type(t),
    _key(k),
    _operation(o),
    _pattern(p)
{
}

KeyConstraint::~KeyConstraint() = default;

KeyConstraintKeyType
KeyConstraint::key_type() const
{
    return _key_type;
}

const std::string
KeyConstraint::key() const
{
    return _key;
}

KeyConstraintOperation
KeyConstraint::operation() const
{
    return _operation;
}

const std::string
KeyConstraint::pattern() const
{
    return _pattern;
}

namespace
{
    std::string stringify_contents_entry(const ContentsEntry & e)
    {
        return stringify(e.location_key()->value());
    }

    struct StringifyEqual
    {
        const std::string pattern;

        StringifyEqual(const std::string & p) :
            pattern(p)
        {
        }

        template <typename T_>
        bool operator() (const T_ & t) const
        {
            return stringify(t) == pattern;
        }

        bool operator() (const ContentsEntry & e) const
        {
            return stringify_contents_entry(e) == pattern;
        }
    };

    struct SpecTreeSearcher
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> id;
        const std::string pattern;

        SpecTreeSearcher(const Environment * const e, const std::shared_ptr<const PackageID> & i, const std::string & p) :
            env(e),
            id(i),
            pattern(p)
        {
        }

        bool visit(const GenericSpecTree::NodeType<AllDepSpec>::Type & n) const
        {
            return indirect_iterator(n.end()) != std::find_if(indirect_iterator(n.begin()), indirect_iterator(n.end()),
                    accept_visitor_returning<bool>(*this));
        }

        bool visit(const GenericSpecTree::NodeType<AnyDepSpec>::Type & n) const
        {
            return indirect_iterator(n.end()) != std::find_if(indirect_iterator(n.begin()), indirect_iterator(n.end()),
                    accept_visitor_returning<bool>(*this));
        }

        bool visit(const GenericSpecTree::NodeType<ExactlyOneDepSpec>::Type & n) const
        {
            return indirect_iterator(n.end()) != std::find_if(indirect_iterator(n.begin()), indirect_iterator(n.end()),
                    accept_visitor_returning<bool>(*this));
        }

        bool visit(const GenericSpecTree::NodeType<ConditionalDepSpec>::Type & n) const
        {
            if (n.spec()->condition_met(env, id))
                return indirect_iterator(n.end()) != std::find_if(indirect_iterator(n.begin()), indirect_iterator(n.end()),
                        accept_visitor_returning<bool>(*this));
            else
                return false;
        }

        bool visit(const GenericSpecTree::NodeType<NamedSetDepSpec>::Type & n) const
        {
            return stringify(*n.spec()) == pattern;
        }

        bool visit(const GenericSpecTree::NodeType<PlainTextDepSpec>::Type & n) const
        {
            return stringify(*n.spec()) == pattern;
        }

        bool visit(const GenericSpecTree::NodeType<PackageDepSpec>::Type & n) const
        {
            return stringify(*n.spec()) == pattern;
        }

        bool visit(const GenericSpecTree::NodeType<BlockDepSpec>::Type & n) const
        {
            return stringify(*n.spec()) == pattern;
        }

        bool visit(const GenericSpecTree::NodeType<LicenseDepSpec>::Type & n) const
        {
            return stringify(*n.spec()) == pattern;
        }

        bool visit(const GenericSpecTree::NodeType<SimpleURIDepSpec>::Type & n) const
        {
            return stringify(*n.spec()) == pattern;
        }

        bool visit(const GenericSpecTree::NodeType<FetchableURIDepSpec>::Type & n) const
        {
            return stringify(*n.spec()) == pattern;
        }

        bool visit(const GenericSpecTree::NodeType<DependenciesLabelsDepSpec>::Type & n) const
        {
            return indirect_iterator(n.spec()->end()) != std::find_if(indirect_iterator(n.spec()->begin()),
                    indirect_iterator(n.spec()->end()), StringifyEqual(pattern));
        }

        bool visit(const GenericSpecTree::NodeType<URILabelsDepSpec>::Type & n) const
        {
            return indirect_iterator(n.spec()->end()) != std::find_if(indirect_iterator(n.spec()->begin()),
                    indirect_iterator(n.spec()->end()), StringifyEqual(pattern));
        }

        bool visit(const GenericSpecTree::NodeType<PlainTextLabelDepSpec>::Type & n) const
        {
            return stringify(*n.spec()) == pattern;
        }
    };

    struct KeyComparator
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> id;
        const std::string pattern;
        const KeyConstraintOperation op;

        KeyComparator(const Environment * const e, const std::shared_ptr<const PackageID> & i,
                const std::string & p, const KeyConstraintOperation o) :
            env(e),
            id(i),
            pattern(p),
            op(o)
        {
        }

        bool visit(const MetadataSectionKey &) const
        {
            return false;
        }

        bool visit(const MetadataTimeKey & k) const
        {
            switch (op)
            {
                case kco_equals:
                    return pattern == stringify(k.value().seconds());
                case kco_less_than:
                    return k.value().seconds() < destringify<time_t>(pattern);
                case kco_greater_than:
                    return k.value().seconds() > destringify<time_t>(pattern);
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataValueKey<std::string> & k) const
        {
            return pattern == stringify(k.value());
        }

        bool visit(const MetadataValueKey<SlotName> & k) const
        {
            return pattern == stringify(k.value());
        }

        bool visit(const MetadataValueKey<FSPath> & k) const
        {
            return pattern == stringify(k.value());
        }

        bool visit(const MetadataValueKey<bool> & k) const
        {
            return pattern == stringify(k.value());
        }

        bool visit(const MetadataValueKey<long> & k) const
        {
            switch (op)
            {
                case kco_equals:
                    return pattern == stringify(k.value());
                case kco_less_than:
                    return k.value() < destringify<long>(pattern);
                case kco_greater_than:
                    return k.value() > destringify<long>(pattern);
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataValueKey<std::shared_ptr<const Choices> > &) const
        {
            return false;
        }

        bool visit(const MetadataValueKey<std::shared_ptr<const Contents> > & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return pattern == join(indirect_iterator(s.value()->begin()), indirect_iterator(s.value()->end()), " ",
                            stringify_contents_entry);
                case kco_less_than:
                    return indirect_iterator(s.value()->end()) != std::find_if(
                            indirect_iterator(s.value()->begin()),
                            indirect_iterator(s.value()->end()),
                            StringifyEqual(pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataValueKey<std::shared_ptr<const PackageID> > & k) const
        {
            return pattern == stringify(*k.value());
        }

        bool visit(const MetadataSpecTreeKey<DependencySpecTree> & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return false;
                case kco_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<SetSpecTree> & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return false;
                case kco_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<PlainTextSpecTree> & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return false;
                case kco_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<RequiredUseSpecTree> & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return false;
                case kco_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<ProvideSpecTree> & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return false;
                case kco_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<SimpleURISpecTree> & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return false;
                case kco_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<FetchableURISpecTree> & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return false;
                case kco_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<LicenseSpecTree> & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return false;
                case kco_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<FSPathSequence> & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return pattern == join(s.value()->begin(), s.value()->end(), " ");
                case kco_less_than:
                    return s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            StringifyEqual(pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<PackageIDSequence> & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return pattern == join(indirect_iterator(s.value()->begin()), indirect_iterator(s.value()->end()), " ");
                case kco_less_than:
                    return indirect_iterator(s.value()->end()) != std::find_if(
                            indirect_iterator(s.value()->begin()),
                            indirect_iterator(s.value()->end()),
                            StringifyEqual(pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<Sequence<std::string> > & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return pattern == join(s.value()->begin(), s.value()->end(), " ");
                case kco_less_than:
                    return s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            StringifyEqual(pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<Set<std::string> > & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return pattern == join(s.value()->begin(), s.value()->end(), " ");
                case kco_less_than:
                    return s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            StringifyEqual(pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<Map<std::string, std::string> > &) const
        {
            return false;
        }

        bool visit(const MetadataCollectionKey<KeywordNameSet> & s) const
        {
            switch (op)
            {
                case kco_equals:
                    return pattern == join(s.value()->begin(), s.value()->end(), " ");
                case kco_less_than:
                    return s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            StringifyEqual(pattern));

                case kco_greater_than:
                case kco_question:
                case last_kco:
                    break;
            }

            return false;
        }
    };
}

bool
KeyConstraint::matches(
        const Environment * const env,
        const std::shared_ptr<const PackageID> & id) const
{
    const MetadataKey * k(0);

    switch (key_type())
    {
        case kckt_repo_role:
            {
                auto repo(env->fetch_repository(id->repository_name()));
                if (key() == "format")
                    k = repo->format_key().get();
                else if (key() == "location")
                    k = repo->location_key().get();
                else if (key() == "installed_root")
                    k = repo->installed_root_key().get();
                else if (key() == "accept_keywords")
                    k = repo->accept_keywords_key().get();
                else if (key() == "sync_host")
                    k = repo->sync_host_key().get();
            }
            break;

        case kckt_id_role:
            {
                if (key() == "behaviours")
                    k = id->behaviours_key().get();
                else if (key() == "build_dependencies")
                    k = id->build_dependencies_key().get();
                else if (key() == "choices")
                    k = id->choices_key().get();
                else if (key() == "contained_in")
                    k = id->contained_in_key().get();
                else if (key() == "contains")
                    k = id->contains_key().get();
                else if (key() == "contents")
                    k = id->contents_key().get();
                else if (key() == "dependencies")
                    k = id->dependencies_key().get();
                else if (key() == "fetches")
                    k = id->fetches_key().get();
                else if (key() == "from_repositories")
                    k = id->from_repositories_key().get();
                else if (key() == "fs_location")
                    k = id->fs_location_key().get();
                else if (key() == "homepage")
                    k = id->homepage_key().get();
                else if (key() == "installed_time")
                    k = id->installed_time_key().get();
                else if (key() == "keywords")
                    k = id->keywords_key().get();
                else if (key() == "long_description")
                    k = id->long_description_key().get();
                else if (key() == "post_dependencies")
                    k = id->post_dependencies_key().get();
                else if (key() == "provide")
                    k = id->provide_key().get();
                else if (key() == "run_dependencies")
                    k = id->run_dependencies_key().get();
                else if (key() == "short_description")
                    k = id->short_description_key().get();
                else if (key() == "slot")
                    k = id->slot_key().get();
                else if (key() == "suggested_dependencies")
                    k = id->suggested_dependencies_key().get();
                else if (key() == "virtual_for")
                    k = id->virtual_for_key().get();
            }
            break;

        case kckt_repo:
            {
                auto repo(env->fetch_repository(id->repository_name()));
                Repository::MetadataConstIterator m(repo->find_metadata(key()));
                if (m != repo->end_metadata())
                    k = m->get();
            }
            break;

        case kckt_id:
            {
                PackageID::MetadataConstIterator m(id->find_metadata(key()));
                if (m != id->end_metadata())
                    k = m->get();
            }
            break;

        case last_kckt:
            break;
    }

    if (! k)
        return false;

    if (operation() == kco_question)
        return true;
    else
    {
        KeyComparator c(env, id, pattern(), operation());
        return k->accept_returning<bool>(c);
    }
}

const std::string
KeyConstraint::as_raw_string() const
{
    std::stringstream s;
    s << "[.";

    switch (key_type())
    {
        case kckt_id:                    break;
        case kckt_id_role:   s << "$";   break;
        case kckt_repo:      s << "::"; break;
        case kckt_repo_role: s << "::$"; break;
        case last_kckt:
            break;
    }

    s << key();

    switch (operation())
    {
        case kco_equals:         s << "=" << pattern(); break;
        case kco_less_than:      s << "<" << pattern(); break;
        case kco_greater_than:   s << ">" << pattern(); break;
        case kco_question:       s << "?";              break;

        case last_kco:
            throw InternalError(PALUDIS_HERE, "Bad KeyConstraintOperation");
    }

    s << "]";

    return s.str();
}

template class Pool<KeyConstraint>;
template class Singleton<Pool<KeyConstraint> >;
template const std::shared_ptr<const KeyConstraint> Pool<KeyConstraint>::create(
        const KeyConstraintKeyType &, const std::string &, const KeyConstraintOperation &, const std::string &) const;
template class Sequence<std::shared_ptr<const KeyConstraint> >;
template class WrappedForwardIterator<Sequence<std::shared_ptr<const KeyConstraint> >::ConstIteratorTag, const std::shared_ptr<const KeyConstraint> >;

ChoiceConstraint::ChoiceConstraint() = default;

template class Sequence<std::shared_ptr<const ChoiceConstraint> >;
template class WrappedForwardIterator<Sequence<std::shared_ptr<const ChoiceConstraint> >::ConstIteratorTag, const std::shared_ptr<const ChoiceConstraint> >;

