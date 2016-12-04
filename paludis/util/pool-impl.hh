/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011, 2012 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_POOL_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_POOL_IMPL_HH 1

#include <paludis/util/pool.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/stringify.hh>
#include <unordered_map>
#include <memory>
#include <mutex>

namespace paludis
{
    template <typename Arg_, typename... Args_>
    void
    PoolKeys::add(const Arg_ & arg, const Args_ & ... args)
    {
        add_one(std::make_shared<ConcretePoolKey<Arg_> >(arg));
        add(args...);
    }

    template <typename T_>
    struct PoolReuseScore
    {
        static const std::string message(const int size, const int reused)
        {
            return "Size " + stringify(size) + " reused " + stringify(reused) + " for " + __PRETTY_FUNCTION__;
        }
    };

    template <typename T_>
    struct Imp<Pool<T_> >
    {
        mutable std::mutex mutex;
        mutable std::unordered_map<PoolKeys, std::shared_ptr<const T_>, PoolKeysHasher, PoolKeysComparator> store;
        mutable unsigned reused;

        Imp() :
            reused(0)
        {
        }
    };

    template <typename T_>
    Pool<T_>::Pool() :
        _imp()
    {
    }

    template <typename T_>
    Pool<T_>::~Pool() = default;

    template <typename T_>
    struct PreventConversion
    {
        const T_ & ref;

        PreventConversion(const T_ & r) :
            ref(r)
        {
        }

        operator const T_ & ()
        {
            return ref;
        }
    };

    template <typename T_>
    template <typename... Args_>
    const std::shared_ptr<const T_>
    Pool<T_>::really_create(const Args_ & ... args) const
    {
        PoolKeys keys;
        keys.add(args...);

        std::unique_lock<std::mutex> lock(_imp->mutex);
        auto i(_imp->store.find(keys));
        if (i == _imp->store.end())
        {
            /* std::make_shared<> doesn't play well with private ctors */
            i = _imp->store.insert(std::make_pair(keys, std::shared_ptr<const T_>(new T_(PreventConversion<Args_>(args)...)))).first;
        }
        else
            ++_imp->reused;

        return i->second;
    }

    template <typename T_>
    template <typename... Args_>
    const std::shared_ptr<const T_>
    Pool<T_>::create(const Args_ & ... args) const
    {
        return really_create(args...);
    }

    template <typename T_>
    template <typename T1_>
    const std::shared_ptr<const T_>
    Pool<T_>::create(const T1_ & a1) const
    {
        return really_create(a1);
    }

    template <typename T_>
    template <typename T1_, typename T2_>
    const std::shared_ptr<const T_>
    Pool<T_>::create(const T1_ & a1, const T2_ & a2) const
    {
        return really_create(a1, a2);
    }

    template <typename T_>
    template <typename T1_, typename T2_, typename T3_>
    const std::shared_ptr<const T_>
    Pool<T_>::create(const T1_ & a1, const T2_ & a2, const T3_ & a3) const
    {
        return really_create(a1, a2, a3);
    }

    template <typename T_>
    template <typename T1_, typename T2_, typename T3_, typename T4_>
    const std::shared_ptr<const T_>
    Pool<T_>::create(const T1_ & a1, const T2_ & a2, const T3_ & a3, const T4_ & a4) const
    {
        return really_create(a1, a2, a3, a4);
    }

    template <typename T_>
    ConcretePoolKey<T_>::ConcretePoolKey(const T_ & t) :
        PoolKey(PoolKeyTypeCodes::get<T_>()),
        _value(t)
    {
    }

    template <typename T_>
    ConcretePoolKey<T_>::~ConcretePoolKey() = default;

    template <typename T_>
    std::size_t
    ConcretePoolKey<T_>::hash() const
    {
        return Hash<T_>()(_value);
    }

    template <typename T_>
    bool
    ConcretePoolKey<T_>::same_value(const PoolKey & other) const
    {
        return _value == static_cast<const ConcretePoolKey<T_> &>(other)._value;
    }

    template <typename T_>
    int
    PoolKeyTypeCodes::get()
    {
        static int result(next());
        return result;
    }
}

#endif
