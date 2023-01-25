/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011, 2012, 2013 Ciaran McCreesh
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

#include <paludis/repositories/e/e_slot_key.hh>
#include <paludis/repositories/e/eapi.hh>

#include <paludis/util/singleton-impl.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/make_named_values.hh>

#include <paludis/name.hh>
#include <paludis/slot.hh>

#include <tuple>
#include <unordered_map>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    struct ESlotKey :
        MetadataValueKey<Slot>
    {
        const Slot slot_value;
        const std::shared_ptr<const EAPIMetadataVariable> variable;
        const MetadataKeyType key_type;

        ESlotKey(const Slot & v, const std::shared_ptr<const EAPIMetadataVariable> & m, const MetadataKeyType t) :
            slot_value(v),
            variable(m),
            key_type(t)
        {
        }

        ~ESlotKey() override = default;

        const Slot parse_value() const override
        {
            return slot_value;
        }

        const std::string raw_name() const override
        {
            return variable->name();
        }

        const std::string human_name() const override
        {
            return variable->description();
        }

        MetadataKeyType type() const override
        {
            return key_type;
        }
    };

    typedef std::tuple<std::shared_ptr<const EAPIMetadataVariable>, std::string, MetadataKeyType> ESlotKeyStoreIndex;

    struct ESlotKeyStoreHash
    {
        std::size_t operator() (const ESlotKeyStoreIndex & p) const
        {
            return
                Hash<std::string>()(std::get<0>(p)->description()) ^
                std::get<0>(p)->flat_list_index() ^
                Hash<std::string>()(std::get<0>(p)->name()) ^
                Hash<std::string>()(std::get<1>(p)) ^
                static_cast<int>(std::get<2>(p));
        }
    };
}

namespace paludis
{
    template <>
    struct Imp<ESlotKeyStore>
    {
        mutable std::mutex mutex;
        mutable std::unordered_map<ESlotKeyStoreIndex, std::shared_ptr<const ESlotKey>, ESlotKeyStoreHash> store;
    };
}

ESlotKeyStore::ESlotKeyStore() :
    _imp()
{
}

ESlotKeyStore::~ESlotKeyStore() = default;

const std::shared_ptr<const MetadataValueKey<Slot> >
ESlotKeyStore::fetch(
        const EAPI & eapi,
        const std::shared_ptr<const EAPIMetadataVariable> & v,
        const std::string & ss,
        const MetadataKeyType mkt) const
{
    std::string s(ss);
    std::string t(ss);
    if (eapi.supported()->ebuild_options()->has_subslots())
    {
        auto p(s.find('/'));
        if (std::string::npos != p)
        {
            s = ss.substr(0, p);
            t = ss.substr(p + 1);
        }
    }

    std::unique_lock<std::mutex> lock(_imp->mutex);

    ESlotKeyStoreIndex x(v, ss, mkt);
    auto i(_imp->store.find(x));
    if (i == _imp->store.end())
        i = _imp->store.insert(std::make_pair(x, std::make_shared<const ESlotKey>(make_named_values<Slot>(
                            n::match_values() = std::make_pair(SlotName(s), SlotName(t)),
                            n::parallel_value() = s,
                            n::raw_value() = ss), v, mkt))).first;
    return i->second;
}

