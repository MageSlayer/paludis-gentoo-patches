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

#include <paludis/package_dep_spec_requirement.hh>
#include <paludis/version_spec.hh>
#include <paludis/version_operator.hh>
#include <paludis/contents.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_id.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/mask.hh>

#include <paludis/util/pool-impl.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>
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

#include <paludis/package_dep_spec_requirement-se.cc>

PackageDepSpecRequirement::~PackageDepSpecRequirement() = default;

template class Sequence<std::shared_ptr<const PackageDepSpecRequirement> >;
template class WrappedForwardIterator<Sequence<std::shared_ptr<const PackageDepSpecRequirement> >::ConstIteratorTag, const std::shared_ptr<const PackageDepSpecRequirement> >;
template class WrappedOutputIterator<Sequence<std::shared_ptr<const PackageDepSpecRequirement> >::InserterTag, std::shared_ptr<const PackageDepSpecRequirement> >;

NameRequirement::NameRequirement(const QualifiedPackageName & n) :
    _name(n)
{
}

NameRequirement::~NameRequirement() = default;

const QualifiedPackageName
NameRequirement::name() const
{
    return _name;
}

template class Pool<NameRequirement>;
template class Singleton<Pool<NameRequirement> >;
template const std::shared_ptr<const NameRequirement> Pool<NameRequirement>::create(
        const QualifiedPackageName &) const;

CategoryNamePartRequirement::CategoryNamePartRequirement(const CategoryNamePart & n) :
    _name_part(n)
{
}

CategoryNamePartRequirement::~CategoryNamePartRequirement() = default;

const CategoryNamePart
CategoryNamePartRequirement::name_part() const
{
    return _name_part;
}

template class Pool<CategoryNamePartRequirement>;
template class Singleton<Pool<CategoryNamePartRequirement> >;
template const std::shared_ptr<const CategoryNamePartRequirement> Pool<CategoryNamePartRequirement>::create(
        const CategoryNamePart &) const;

PackageNamePartRequirement::PackageNamePartRequirement(const PackageNamePart & n) :
    _name_part(n)
{
}

PackageNamePartRequirement::~PackageNamePartRequirement() = default;

const PackageNamePart
PackageNamePartRequirement::name_part() const
{
    return _name_part;
}

template class Pool<PackageNamePartRequirement>;
template class Singleton<Pool<PackageNamePartRequirement> >;
template const std::shared_ptr<const PackageNamePartRequirement> Pool<PackageNamePartRequirement>::create(
        const PackageNamePart &) const;

namespace paludis
{
    template <>
    struct Imp<VersionRequirement>
    {
        VersionSpec spec;
        VersionOperator op;
        VersionRequirementCombiner combiner;

        Imp(const VersionSpec & s, const VersionOperator & o, const VersionRequirementCombiner c) :
            spec(s),
            op(o),
            combiner(c)
        {
        }
    };
}

VersionRequirement::VersionRequirement(const VersionSpec & s, const VersionOperator & o, const VersionRequirementCombiner c) :
    _imp(s, o, c)
{
}

VersionRequirement::~VersionRequirement() = default;

const VersionSpec
VersionRequirement::version_spec() const
{
    return _imp->spec;
}

const VersionOperator
VersionRequirement::version_operator() const
{
    return _imp->op;
}

VersionRequirementCombiner
VersionRequirement::combiner() const
{
    return _imp->combiner;
}

template class Sequence<std::shared_ptr<const VersionRequirement> >;
template class WrappedForwardIterator<Sequence<std::shared_ptr<const VersionRequirement> >::ConstIteratorTag, const std::shared_ptr<const VersionRequirement> >;
template class WrappedOutputIterator<Sequence<std::shared_ptr<const VersionRequirement> >::InserterTag, std::shared_ptr<const VersionRequirement> >;
template class Pimp<VersionRequirement>;

InRepositoryRequirement::InRepositoryRequirement(const RepositoryName & n) :
    _name(n)
{
}

InRepositoryRequirement::~InRepositoryRequirement() = default;

const RepositoryName
InRepositoryRequirement::name() const
{
    return _name;
}

template class Pool<InRepositoryRequirement>;
template class Singleton<Pool<InRepositoryRequirement> >;
template const std::shared_ptr<const InRepositoryRequirement> Pool<InRepositoryRequirement>::create(
        const RepositoryName &) const;

FromRepositoryRequirement::FromRepositoryRequirement(const RepositoryName & n) :
    _name(n)
{
}

FromRepositoryRequirement::~FromRepositoryRequirement() = default;

const RepositoryName
FromRepositoryRequirement::name() const
{
    return _name;
}

template class Pool<FromRepositoryRequirement>;
template class Singleton<Pool<FromRepositoryRequirement> >;
template const std::shared_ptr<const FromRepositoryRequirement> Pool<FromRepositoryRequirement>::create(
        const RepositoryName &) const;

InstalledAtPathRequirement::InstalledAtPathRequirement(const FSPath & n) :
    _path(n)
{
}

InstalledAtPathRequirement::~InstalledAtPathRequirement() = default;

const FSPath
InstalledAtPathRequirement::path() const
{
    return _path;
}

template class Pool<InstalledAtPathRequirement>;
template class Singleton<Pool<InstalledAtPathRequirement> >;
template const std::shared_ptr<const InstalledAtPathRequirement> Pool<InstalledAtPathRequirement>::create(
        const FSPath &) const;

InstallableToPathRequirement::InstallableToPathRequirement(const FSPath & n, const bool i) :
    _path(n),
    _include_masked(i)
{
}

InstallableToPathRequirement::~InstallableToPathRequirement() = default;

const FSPath
InstallableToPathRequirement::path() const
{
    return _path;
}

bool
InstallableToPathRequirement::include_masked() const
{
    return _include_masked;
}

template class Pool<InstallableToPathRequirement>;
template class Singleton<Pool<InstallableToPathRequirement> >;
template const std::shared_ptr<const InstallableToPathRequirement> Pool<InstallableToPathRequirement>::create(
        const FSPath &, const bool &) const;

InstallableToRepositoryRequirement::InstallableToRepositoryRequirement(const RepositoryName & n, const bool i) :
    _name(n),
    _include_masked(i)
{
}

InstallableToRepositoryRequirement::~InstallableToRepositoryRequirement() = default;

const RepositoryName
InstallableToRepositoryRequirement::name() const
{
    return _name;
}

bool
InstallableToRepositoryRequirement::include_masked() const
{
    return _include_masked;
}

template class Pool<InstallableToRepositoryRequirement>;
template class Singleton<Pool<InstallableToRepositoryRequirement> >;
template const std::shared_ptr<const InstallableToRepositoryRequirement> Pool<InstallableToRepositoryRequirement>::create(
        const RepositoryName &, const bool &) const;

ExactSlotRequirement::ExactSlotRequirement(const SlotName & n, const bool i) :
    _name(n),
    _locked(i)
{
}

ExactSlotRequirement::~ExactSlotRequirement() = default;

const SlotName
ExactSlotRequirement::name() const
{
    return _name;
}

bool
ExactSlotRequirement::locked() const
{
    return _locked;
}

template class Pool<ExactSlotRequirement>;
template class Singleton<Pool<ExactSlotRequirement> >;
template const std::shared_ptr<const ExactSlotRequirement> Pool<ExactSlotRequirement>::create(const SlotName &, const bool &) const;

AnySlotRequirement::AnySlotRequirement(const bool i) :
    _locking(i)
{
}

AnySlotRequirement::~AnySlotRequirement() = default;

bool
AnySlotRequirement::locking() const
{
    return _locking;
}

template class Pool<AnySlotRequirement>;
template class Singleton<Pool<AnySlotRequirement> >;
template const std::shared_ptr<const AnySlotRequirement> Pool<AnySlotRequirement>::create(const bool &) const;

KeyRequirement::KeyRequirement(const KeyRequirementKeyType t, const std::string & k, const KeyRequirementOperation o, const std::string & p) :
    _key_type(t),
    _key(k),
    _operation(o),
    _pattern(p)
{
}

KeyRequirement::~KeyRequirement() = default;

KeyRequirementKeyType
KeyRequirement::key_type() const
{
    return _key_type;
}

const std::string
KeyRequirement::key() const
{
    return _key;
}

KeyRequirementOperation
KeyRequirement::operation() const
{
    return _operation;
}

const std::string
KeyRequirement::pattern() const
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
        const KeyRequirementOperation op;

        KeyComparator(const Environment * const e, const std::shared_ptr<const PackageID> & i,
                const std::string & p, const KeyRequirementOperation o) :
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
                case kro_equals:
                    return pattern == stringify(k.value().seconds());
                case kro_tilde:
                    return std::string::npos != stringify(k.value().seconds()).find(pattern);
                case kro_less_than:
                    return k.value().seconds() < destringify<time_t>(pattern);
                case kro_greater_than:
                    return k.value().seconds() > destringify<time_t>(pattern);
                case kro_question:
                case last_kro:
                    break;
            }

            return false;
        }

        bool visit(const MetadataValueKey<std::string> & k) const
        {
            switch (op)
            {
                case kro_equals:
                    return pattern == stringify(k.value());
                case kro_tilde:
                    return std::string::npos != stringify(k.value()).find(pattern);
                case kro_less_than:
                case kro_greater_than:
                case kro_question:
                case last_kro:
                    break;
            }

            return false;
        }

        bool visit(const MetadataValueKey<SlotName> & k) const
        {
            switch (op)
            {
                case kro_equals:
                    return pattern == stringify(k.value());
                case kro_tilde:
                    return std::string::npos != stringify(k.value()).find(pattern);
                case kro_less_than:
                case kro_greater_than:
                case kro_question:
                case last_kro:
                    break;
            }

            return false;
        }

        bool visit(const MetadataValueKey<FSPath> & k) const
        {
            switch (op)
            {
                case kro_equals:
                    return pattern == stringify(k.value());
                case kro_tilde:
                    return std::string::npos != stringify(k.value()).find(pattern);
                case kro_less_than:
                case kro_greater_than:
                case kro_question:
                case last_kro:
                    break;
            }

            return false;
        }

        bool visit(const MetadataValueKey<bool> & k) const
        {
            switch (op)
            {
                case kro_equals:
                    return pattern == stringify(k.value());
                case kro_tilde:
                    return std::string::npos != stringify(k.value()).find(pattern);
                case kro_less_than:
                case kro_greater_than:
                case kro_question:
                case last_kro:
                    break;
            }

            return false;
        }

        bool visit(const MetadataValueKey<long> & k) const
        {
            switch (op)
            {
                case kro_equals:
                    return pattern == stringify(k.value());
                case kro_tilde:
                    return std::string::npos != stringify(k.value()).find(pattern);
                case kro_less_than:
                    return k.value() < destringify<long>(pattern);
                case kro_greater_than:
                    return k.value() > destringify<long>(pattern);
                case kro_question:
                case last_kro:
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
                case kro_equals:
                    return pattern == join(indirect_iterator(s.value()->begin()), indirect_iterator(s.value()->end()), " ",
                            stringify_contents_entry);
                case kro_less_than:
                    return indirect_iterator(s.value()->end()) != std::find_if(
                            indirect_iterator(s.value()->begin()),
                            indirect_iterator(s.value()->end()),
                            StringifyEqual(pattern));

                case kro_greater_than:
                case kro_question:
                case kro_tilde:
                case last_kro:
                    break;
            }

            return false;
        }

        bool visit(const MetadataValueKey<std::shared_ptr<const PackageID> > & k) const
        {
            switch (op)
            {
                case kro_equals:
                    return pattern == stringify(*k.value());
                case kro_tilde:
                    return std::string::npos != stringify(*k.value()).find(pattern);
                case kro_less_than:
                case kro_greater_than:
                case kro_question:
                case last_kro:
                    break;
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<DependencySpecTree> & s) const
        {
            switch (op)
            {
                case kro_equals:
                    return false;
                case kro_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kro_tilde:
                case kro_greater_than:
                case kro_question:
                case last_kro:
                    break;
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<SetSpecTree> & s) const
        {
            switch (op)
            {
                case kro_equals:
                    return false;
                case kro_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kro_tilde:
                case kro_greater_than:
                case kro_question:
                case last_kro:
                    break;
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<PlainTextSpecTree> & s) const
        {
            switch (op)
            {
                case kro_equals:
                    return false;
                case kro_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kro_tilde:
                case kro_greater_than:
                case kro_question:
                case last_kro:
                    break;
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<RequiredUseSpecTree> & s) const
        {
            switch (op)
            {
                case kro_equals:
                    return false;
                case kro_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kro_tilde:
                case kro_greater_than:
                case kro_question:
                case last_kro:
                    break;
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<ProvideSpecTree> & s) const
        {
            switch (op)
            {
                case kro_equals:
                    return false;
                case kro_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kro_tilde:
                case kro_greater_than:
                case kro_question:
                case last_kro:
                    break;
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<SimpleURISpecTree> & s) const
        {
            switch (op)
            {
                case kro_equals:
                    return false;
                case kro_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kro_tilde:
                case kro_greater_than:
                case kro_question:
                case last_kro:
                    break;
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<FetchableURISpecTree> & s) const
        {
            switch (op)
            {
                case kro_equals:
                    return false;
                case kro_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kro_tilde:
                case kro_greater_than:
                case kro_question:
                case last_kro:
                    break;
            }

            return false;
        }

        bool visit(const MetadataSpecTreeKey<LicenseSpecTree> & s) const
        {
            switch (op)
            {
                case kro_equals:
                    return false;
                case kro_less_than:
                    return s.value()->top()->accept_returning<bool>(SpecTreeSearcher(env, id, pattern));

                case kro_tilde:
                case kro_greater_than:
                case kro_question:
                case last_kro:
                    break;
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<FSPathSequence> & s) const
        {
            switch (op)
            {
                case kro_equals:
                    return pattern == join(s.value()->begin(), s.value()->end(), " ");
                case kro_less_than:
                    return s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            StringifyEqual(pattern));

                case kro_tilde:
                case kro_greater_than:
                case kro_question:
                case last_kro:
                    break;
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<PackageIDSequence> & s) const
        {
            switch (op)
            {
                case kro_equals:
                    return pattern == join(indirect_iterator(s.value()->begin()), indirect_iterator(s.value()->end()), " ");
                case kro_less_than:
                    return indirect_iterator(s.value()->end()) != std::find_if(
                            indirect_iterator(s.value()->begin()),
                            indirect_iterator(s.value()->end()),
                            StringifyEqual(pattern));

                case kro_tilde:
                case kro_greater_than:
                case kro_question:
                case last_kro:
                    break;
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<Sequence<std::string> > & s) const
        {
            switch (op)
            {
                case kro_equals:
                    return pattern == join(s.value()->begin(), s.value()->end(), " ");
                case kro_less_than:
                    return s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            StringifyEqual(pattern));

                case kro_tilde:
                case kro_greater_than:
                case kro_question:
                case last_kro:
                    break;
            }

            return false;
        }

        bool visit(const MetadataCollectionKey<Set<std::string> > & s) const
        {
            switch (op)
            {
                case kro_equals:
                    return pattern == join(s.value()->begin(), s.value()->end(), " ");
                case kro_less_than:
                    return s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            StringifyEqual(pattern));

                case kro_tilde:
                case kro_greater_than:
                case kro_question:
                case last_kro:
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
                case kro_equals:
                    return pattern == join(s.value()->begin(), s.value()->end(), " ");
                case kro_less_than:
                    return s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            StringifyEqual(pattern));

                case kro_tilde:
                case kro_greater_than:
                case kro_question:
                case last_kro:
                    break;
            }

            return false;
        }
    };

    struct AssociatedKeyFinder
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> id;

        const MetadataKey * const visit(const UserMask &) const
        {
            return 0;
        }

        const MetadataKey * const visit(const UnacceptedMask & m) const
        {
            auto k(id->find_metadata(m.unaccepted_key_name()));
            if (k != id->end_metadata())
                return &**k;
            else
                return 0;
        }

        const MetadataKey * const visit(const RepositoryMask &) const
        {
            return 0;
        }

        const MetadataKey * const visit(const UnsupportedMask &) const
        {
            return 0;
        }

        const MetadataKey * const visit(const AssociationMask &) const
        {
            return 0;
        }
    };

    struct MaskChecker
    {
        const std::string key;

        bool visit(const UserMask & m) const
        {
            return key == "*" || key == "user" || m.token() == key;
        }

        bool visit(const UnacceptedMask & m) const
        {
            return key == "*" || key == "unaccepted" || m.unaccepted_key_name() == key;
        }

        bool visit(const RepositoryMask & m) const
        {
            return key == "*" || key == "repository" || m.token() == key;
        }

        bool visit(const UnsupportedMask &) const
        {
            return key == "*" || key == "unsupported";
        }

        bool visit(const AssociationMask &) const
        {
            return key == "*" || key == "association";
        }
    };
}

bool
KeyRequirement::matches(
        const Environment * const env,
        const std::shared_ptr<const PackageID> & id) const
{
    const MetadataKey * k(0);
    const Mask * m(0);

    switch (key_type())
    {
        case krkt_repo_role:
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

        case krkt_id_role:
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

        case krkt_repo:
            {
                auto repo(env->fetch_repository(id->repository_name()));
                Repository::MetadataConstIterator mm(repo->find_metadata(key()));
                if (mm != repo->end_metadata())
                    k = mm->get();
            }
            break;

        case krkt_id:
            {
                PackageID::MetadataConstIterator mm(id->find_metadata(key()));
                if (mm != id->end_metadata())
                    k = mm->get();
            }
            break;

        case krkt_id_mask:
            {
                for (auto mm(id->begin_masks()), mm_end(id->end_masks()) ;
                        mm != mm_end ; ++mm)
                    if ((*mm)->accept_returning<bool>(MaskChecker{key()}))
                    {
                        m = &**mm;
                        break;
                    }
            }
            break;

        case last_krkt:
            break;
    }

    if ((! k) && (! m))
        return false;

    if (operation() == kro_question)
        return true;
    else
    {
        if (m)
            k = m->accept_returning<const MetadataKey *>(AssociatedKeyFinder{env, id});

        if (k)
        {
            KeyComparator c(env, id, pattern(), operation());
            return k->accept_returning<bool>(c);
        }

        return false;
    }
}

const std::string
KeyRequirement::as_raw_string() const
{
    std::stringstream s;
    s << "[.";

    switch (key_type())
    {
        case krkt_id:                    break;
        case krkt_id_role:   s << "$";   break;
        case krkt_id_mask:   s << "(";   break;
        case krkt_repo:      s << "::";  break;
        case krkt_repo_role: s << "::$"; break;
        case last_krkt:
            break;
    }

    s << key();

    switch (operation())
    {
        case kro_equals:         s << "=" << pattern(); break;
        case kro_tilde:          s << "~" << pattern(); break;
        case kro_less_than:      s << "<" << pattern(); break;
        case kro_greater_than:   s << ">" << pattern(); break;
        case kro_question:       s << "?";              break;

        case last_kro:
            throw InternalError(PALUDIS_HERE, "Bad KeyRequirementOperation");
    }

    switch (key_type())
    {
        case krkt_id:                    break;
        case krkt_id_role:               break;
        case krkt_id_mask:   s << ")";   break;
        case krkt_repo:                  break;
        case krkt_repo_role:             break;
        case last_krkt:
            break;
    }

    s << "]";

    return s.str();
}

template class Pool<KeyRequirement>;
template class Singleton<Pool<KeyRequirement> >;
template const std::shared_ptr<const KeyRequirement> Pool<KeyRequirement>::create(
        const KeyRequirementKeyType &, const std::string &, const KeyRequirementOperation &, const std::string &) const;
template class Sequence<std::shared_ptr<const KeyRequirement> >;
template class WrappedOutputIterator<Sequence<std::shared_ptr<const KeyRequirement> >::InserterTag, std::shared_ptr<const KeyRequirement> >;
template class WrappedForwardIterator<Sequence<std::shared_ptr<const KeyRequirement> >::ConstIteratorTag, const std::shared_ptr<const KeyRequirement> >;

ChoiceRequirement::ChoiceRequirement() = default;

template class Sequence<std::shared_ptr<const ChoiceRequirement> >;
template class WrappedForwardIterator<Sequence<std::shared_ptr<const ChoiceRequirement> >::ConstIteratorTag, const std::shared_ptr<const ChoiceRequirement> >;
template class WrappedOutputIterator<Sequence<std::shared_ptr<const ChoiceRequirement> >::InserterTag, std::shared_ptr<const ChoiceRequirement> >;

