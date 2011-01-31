/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/e_choice_value.hh>
#include <paludis/repositories/e/use_desc.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/singleton-impl.hh>
#include <unordered_map>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    class EChoiceValue :
        public ChoiceValue
    {
        private:
            const EChoiceValueParams _params;

        public:
            EChoiceValue(const EChoiceValueParams &);

            const UnprefixedChoiceName unprefixed_name() const;
            const ChoiceNameWithPrefix name_with_prefix() const;
            bool enabled() const;
            bool enabled_by_default() const;
            bool locked() const;
            const std::string description() const;
            bool explicitly_listed() const;
            const std::string parameter() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::shared_ptr<const PermittedChoiceValueParameterValues> permitted_parameter_values() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

EChoiceValue::EChoiceValue(const EChoiceValueParams & p) :
    _params(p)
{
}

const UnprefixedChoiceName
EChoiceValue::unprefixed_name() const
{
    return _params.unprefixed_choice_name();
}

const ChoiceNameWithPrefix
EChoiceValue::name_with_prefix() const
{
    return _params.choice_name_with_prefix();
}

const std::string
EChoiceValue::description() const
{
    return _params.description();
}

bool
EChoiceValue::enabled() const
{
    return _params.enabled();
}

bool
EChoiceValue::enabled_by_default() const
{
    return _params.enabled_by_default();
}

bool
EChoiceValue::locked() const
{
    return _params.locked();
}

bool
EChoiceValue::explicitly_listed() const
{
    return _params.explicitly_listed();
}

const std::string
EChoiceValue::parameter() const
{
    return "";
}

const std::shared_ptr<const PermittedChoiceValueParameterValues>
EChoiceValue::permitted_parameter_values() const
{
    return make_null_shared_ptr();
}

namespace
{
    struct EChoiceValueParamsHash
    {
        Hash<ChoiceNameWithPrefix> choice_name_with_prefix_hash;
        Hash<ChoicePrefixName> choice_prefix_name_hash;
        Hash<std::string> description_hash;
        Hash<bool> enabled_hash;
        Hash<bool> enabled_by_default_hash;
        Hash<bool> explicitly_listed_hash;
        Hash<bool> locked_hash;
        Hash<UnprefixedChoiceName> unprefixed_choice_name_hash;

        std::size_t operator() (const EChoiceValueParams & p) const
        {
            return 0
                ^ choice_name_with_prefix_hash(p.choice_name_with_prefix())
                ^ choice_prefix_name_hash(p.choice_prefix_name())
                ^ description_hash(p.description())
                ^ enabled_hash(p.enabled())
                ^ enabled_by_default_hash(p.enabled_by_default())
                ^ explicitly_listed_hash(p.explicitly_listed())
                ^ locked_hash(p.locked())
                ^ unprefixed_choice_name_hash(p.unprefixed_choice_name())
                ;
        }
    };

    struct EChoiceValueParamsCompare
    {
        bool operator() (const EChoiceValueParams & a, const EChoiceValueParams & b) const
        {
            return true
                && (a.choice_name_with_prefix() == b.choice_name_with_prefix())
                && (a.choice_prefix_name() == b.choice_prefix_name())
                && (a.description() == b.description())
                && (a.enabled() == b.enabled())
                && (a.enabled_by_default() == b.enabled_by_default())
                && (a.explicitly_listed() == b.explicitly_listed())
                && (a.locked() == b.locked())
                && (a.unprefixed_choice_name() == b.unprefixed_choice_name())
                ;
        }
    };

    typedef std::unordered_map<EChoiceValueParams, std::shared_ptr<const EChoiceValue>, EChoiceValueParamsHash, EChoiceValueParamsCompare> Store;
}

namespace paludis
{
    template <>
    struct Imp<EChoiceValueStore>
    {
        mutable Mutex mutex;
        mutable Store store;
    };
}

EChoiceValueStore::EChoiceValueStore() :
    _imp()
{
}

EChoiceValueStore::~EChoiceValueStore() = default;

const std::shared_ptr<const ChoiceValue>
EChoiceValueStore::fetch(const EChoiceValueParams & p) const
{
    Lock lock(_imp->mutex);
    auto i(_imp->store.find(p));
    if (i == _imp->store.end())
        i = _imp->store.insert(std::make_pair(p, std::make_shared<EChoiceValue>(p))).first;

    return i->second;
}

template class Pimp<erepository::EChoiceValueStore>;
template class Singleton<erepository::EChoiceValueStore>;

