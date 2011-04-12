/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/name.hh>

#include <tuple>
#include <unordered_map>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    struct ESlotKey :
        MetadataValueKey<SlotName>
    {
        const SlotName slot_value;
        const std::shared_ptr<const EAPIMetadataVariable> variable;
        const MetadataKeyType key_type;

        ESlotKey(const SlotName & v, const std::shared_ptr<const EAPIMetadataVariable> & m, const MetadataKeyType t) :
            slot_value(v),
            variable(m),
            key_type(t)
        {
        }

        ~ESlotKey()
        {
        }

        virtual const SlotName parse_value() const
        {
            return slot_value;
        }

        virtual const std::string raw_name() const
        {
            return variable->name();
        }

        virtual const std::string human_name() const
        {
            return variable->description();
        }

        virtual MetadataKeyType type() const
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
        mutable Mutex mutex;
        mutable std::unordered_map<ESlotKeyStoreIndex, std::shared_ptr<const ESlotKey>, ESlotKeyStoreHash> store;
    };
}

ESlotKeyStore::ESlotKeyStore() :
    _imp()
{
}

ESlotKeyStore::~ESlotKeyStore() = default;

const std::shared_ptr<const MetadataValueKey<SlotName> >
ESlotKeyStore::fetch(
        const std::shared_ptr<const EAPIMetadataVariable> & v,
        const std::string & s,
        const MetadataKeyType t) const
{
    Lock lock(_imp->mutex);

    ESlotKeyStoreIndex x(v, s, t);
    auto i(_imp->store.find(x));
    if (i == _imp->store.end())
        i = _imp->store.insert(std::make_pair(x, std::make_shared<const ESlotKey>(SlotName(s), v, t))).first;
    return i->second;
}

