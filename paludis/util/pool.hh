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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_POOL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_POOL_HH 1

#include <paludis/util/pool-fwd.hh>
#include <paludis/util/singleton.hh>
#include <paludis/util/pimp.hh>
#include <memory>

namespace paludis
{
    class PoolKeysHasher;
    class PoolKeysComparator;

    template <typename T_>
    class PALUDIS_VISIBLE Pool :
        public Singleton<Pool<T_> >
    {
        friend class Singleton<Pool<T_> >;

        private:
            Pimp<Pool<T_> > _imp;

            Pool();
            ~Pool();

            template <typename... Args_>
            const std::shared_ptr<const T_> really_create(const Args_ & ...) const PALUDIS_ATTRIBUTE((warn_unused_result));

        public:
            template <typename... Args_>
            const std::shared_ptr<const T_> create(const Args_ & ...) const PALUDIS_ATTRIBUTE((warn_unused_result));

            // can't explicitly instantiate variadics

            template <typename T1_>
            const std::shared_ptr<const T_> create(const T1_ &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            template <typename T1_, typename T2_>
            const std::shared_ptr<const T_> create(const T1_ &, const T2_ &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            template <typename T1_, typename T2_, typename T3_>
            const std::shared_ptr<const T_> create(const T1_ &, const T2_ &, const T3_ &) const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE PoolKey
    {
        private:
            int _tc;

        protected:
            explicit PoolKey(int tc);

            virtual bool same_value(const PoolKey &) const = 0;

        public:
            virtual ~PoolKey() = 0;

            virtual std::size_t hash() const = 0;

            bool same_type_and_value(const PoolKey &) const;
    };

    template <typename T_>
    class PALUDIS_VISIBLE ConcretePoolKey :
        public PoolKey
    {
        friend class PoolKeysHasher;
        friend class PoolKeysComparator;

        private:
            T_ _value;

        public:
            explicit ConcretePoolKey(const T_ &);

            virtual ~ConcretePoolKey();

            virtual std::size_t hash() const;

            bool same_value(const PoolKey &) const;
    };

    class PALUDIS_VISIBLE PoolKeys
    {
        friend class PoolKeysHasher;
        friend class PoolKeysComparator;

        private:
            Pimp<PoolKeys> _imp;

            void add_one(const std::shared_ptr<const PoolKey> &);
            void add();

        public:
            PoolKeys();
            PoolKeys(const PoolKeys &);
            ~PoolKeys();

            template <typename Arg_, typename... Args_>
            void add(const Arg_ &, const Args_ & ...);
    };

    class PALUDIS_VISIBLE PoolKeysHasher
    {
        public:
            std::size_t operator() (const PoolKeys &) const PALUDIS_VISIBLE;
    };

    class PALUDIS_VISIBLE PoolKeysComparator
    {
        public:
            bool operator() (const PoolKeys &, const PoolKeys &) const PALUDIS_VISIBLE;
    };

    class PALUDIS_VISIBLE PoolKeyTypeCodes
    {
        private:
            static int next();

        public:
            template <typename T_>
            static int get();
    };

    extern template class Pimp<PoolKeys>;
}

#endif
