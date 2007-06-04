/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_HASHED_CONTAINERS_HH
#define PALUDIS_GUARD_PALUDIS_HASHED_CONTAINERS_HH 1

/** \file
 * Declarations for the MakeHashedMap and MakeHashedSet classes, and related
 * utilities.
 *
 * \ingroup grphashedcontainers
 */

#include <paludis/name.hh>
#include <paludis/util/validated.hh>
#include <paludis/version_spec.hh>

#ifdef PALUDIS_HASH_IS_STD_TR1_UNORDERED
#  include <tr1/unordered_set>
#  include <tr1/unordered_map>
#elif defined(PALUDIS_HASH_IS_GNU_CXX_HASH)
#  include <ext/hash_set>
#  include <ext/hash_map>
#elif defined(PALUDIS_HASH_IS_STD_HASH)
#  include <hash_set>
#  include <hash_map>
#else
#  include <set>
#  include <map>
#endif

#include <limits>
#include <string>
#include <functional>

namespace paludis
{
    /**
     * Hash function base template.
     *
     * \ingroup grphashedcontainers
     */
    template <typename T_>
    struct CRCHash;

    /**
     * Make a hashed map of some kind from Key_ to Value_.
     *
     * \ingroup grphashedcontainers
     */
    template <typename Key_, typename Value_>
    struct MakeHashedMap
    {
#ifdef PALUDIS_HASH_IS_STD_TR1_UNORDERED
        /// Our map type.
        typedef std::tr1::unordered_map<Key_, Value_, CRCHash<Key_> > Type;

#elif defined(PALUDIS_HASH_IS_GNU_CXX_HASH)
        /// Our map type.
        typedef __gnu_cxx::hash_map<Key_, Value_, CRCHash<Key_> > Type;

#elif defined(PALUDIS_HASH_IS_STD_HASH)
        /// Our map type.
        typedef std::hash_map<Key_, Value_, CRCHash<Key_> > Type;

#else
        /// Our map type.
        typedef std::map<Key_, Value_> Type;
#endif
    };

    /**
     * Make a hashed map of some kind from Key_ to Value_.
     *
     * \ingroup grphashedcontainers
     */
    template <typename Key_, typename Value_>
    struct MakeHashedMultiMap
    {
#ifdef PALUDIS_HASH_IS_STD_TR1_UNORDERED
        /// Our map type.
        typedef std::tr1::unordered_multimap<Key_, Value_, CRCHash<Key_> > Type;

#elif defined(PALUDIS_HASH_IS_GNU_CXX_HASH)
        /// Our map type.
        typedef __gnu_cxx::hash_multimap<Key_, Value_, CRCHash<Key_> > Type;

#elif defined(PALUDIS_HASH_IS_STD_HASH)
        /// Our map type.
        typedef std::hash_multimap<Key_, Value_, CRCHash<Key_> > Type;

#else
        /// Our map type.
        typedef std::multimap<Key_, Value_> Type;
#endif
    };

    /**
     * Make a hashed set of some kind of Key_.
     *
     * \ingroup grphashedcontainers
     */
    template <typename Key_>
    struct MakeHashedSet
    {
#ifdef PALUDIS_HASH_IS_STD_TR1_UNORDERED
        /// Our set type.
        typedef std::tr1::unordered_set<Key_, CRCHash<Key_> > Type;

#elif defined(PALUDIS_HASH_IS_GNU_CXX_HASH)
        /// Our set type.
        typedef __gnu_cxx::hash_set<Key_, CRCHash<Key_> > Type;

#elif defined(PALUDIS_HASH_IS_STD_HASH)
        /// Our set type.
        typedef std::hash_set<Key_, CRCHash<Key_> > Type;

#else
        /// Our set type.
        typedef std::set<Key_> Type;
#endif
    };

    /**
     * Make a hashed set of some kind of Key_.
     *
     * \ingroup grphashedcontainers
     */
    template <typename Key_>
    struct MakeHashedMultiSet
    {
#ifdef PALUDIS_HASH_IS_STD_TR1_UNORDERED
        /// Our set type.
        typedef std::tr1::unordered_multiset<Key_, CRCHash<Key_> > Type;

#elif defined(PALUDIS_HASH_IS_GNU_CXX_HASH)
        /// Our set type.
        typedef __gnu_cxx::hash_multiset<Key_, CRCHash<Key_> > Type;

#elif defined(PALUDIS_HASH_IS_STD_HASH)
        /// Our set type.
        typedef std::hash_multiset<Key_, CRCHash<Key_> > Type;

#else
        /// Our set type.
        typedef std::multiset<Key_> Type;
#endif
    };

#if defined(PALUDIS_HASH_IS_STD_TR1_UNORDERED) || defined(PALUDIS_HASH_IS_GNU_CXX_HASH) || defined(PALUDIS_HASH_IS_STD_HASH)
    namespace hashed_containers_internals
    {
        /**
         * Base definitions for our CRCHash.
         *
         * \ingroup grphashedcontainers
         */
        struct CRCHashBase
        {
            /// Shift value.
            static const std::size_t h_shift = std::numeric_limits<std::size_t>::digits - 5;

            /// Mask value.
            static const std::size_t h_mask = static_cast<std::size_t>(0x1f) << h_shift;
        };
    }

    /**
     * Hash, for QualifiedPackageName.
     *
     * \ingroup grphashedcontainers
     */
    template <>
    class PALUDIS_VISIBLE CRCHash<QualifiedPackageName> :
        public std::unary_function<QualifiedPackageName, std::size_t>,
        protected hashed_containers_internals::CRCHashBase
    {
        public:
            /// Hash function.
            std::size_t operator() (const QualifiedPackageName & val) const;

#if (! defined(PALUDIS_HASH_IS_STD_TR1_UNORDERED)) && (! defined(PALUDIS_HASH_IS_GNU_CXX_HASH))
            enum
            {
                min_buckets = 32,
                bucket_size = 4
            };

            bool operator() (const QualifiedPackageName & i1, const QualifiedPackageName & i2) const
            {
                return i1 < i2;
            }
#endif
    };

    /**
     * Hash, for a validated string type.
     *
     * \ingroup grphashedcontainers
     */
    template <typename Validated_>
    class CRCHash<Validated<std::string, Validated_> > :
        public std::unary_function<Validated<std::string, Validated_>, std::size_t>,
        protected hashed_containers_internals::CRCHashBase
    {
        public:
            /// Hash function.
            std::size_t operator() (const Validated<std::string, Validated_> & val) const;

#if (! defined(PALUDIS_HASH_IS_STD_TR1_UNORDERED)) && (! defined(PALUDIS_HASH_IS_GNU_CXX_HASH))
            enum
            {
                min_buckets = 32,
                bucket_size = 4
            };

            bool operator() (const Validated<std::string, Validated_> i1,
                    const Validated<std::string, Validated_> & i2) const
            {
                return i1 < i2;
            }
#endif
    };


    /**
     * Hash, for a string.
     *
     * \ingroup grphashedcontainers
     */
    template<>
    class PALUDIS_VISIBLE CRCHash<std::string> :
        public std::unary_function<std::string, std::size_t>,
        protected hashed_containers_internals::CRCHashBase
    {
        public:
            /// Hash function.
            std::size_t operator() (const std::string & val) const;

#if (! defined(PALUDIS_HASH_IS_STD_TR1_UNORDERED)) && (! defined(PALUDIS_HASH_IS_GNU_CXX_HASH))
            enum
            {
                min_buckets = 32,
                bucket_size = 4
            };

            bool operator() (const std::string & i1, const std::string & i2) const
            {
                return i1 < i2;
            }
#endif
    };

    /**
     * Hash, for a QualifiedPackageName + VersionSpec pair.
     *
     * \ingroup grphashedcontainers
     */
    template <>
    class PALUDIS_VISIBLE CRCHash<std::pair<QualifiedPackageName, VersionSpec> > :
        public std::unary_function<std::pair<QualifiedPackageName, VersionSpec>, std::size_t>,
        protected hashed_containers_internals::CRCHashBase
    {
        public:
            /// Hash function.
            std::size_t operator() (const std::pair<QualifiedPackageName, VersionSpec> & val) const;

#if (! defined(PALUDIS_HASH_IS_STD_TR1_UNORDERED)) && (! defined(PALUDIS_HASH_IS_GNU_CXX_HASH))
            enum
            {
                min_buckets = 32,
                bucket_size = 4
            };

            bool operator() (const std::pair<QualifiedPackageName, VersionSpec> & i1,
                    const std::pair<QualifiedPackageName, VersionSpec> & i2) const
            {
                return i1 < i2;
            }
#endif
    };

    template <typename Validated_>
    std::size_t
    CRCHash<Validated<std::string, Validated_> >::operator() (const Validated<std::string, Validated_> & val) const
    {
        const std::string & s1(val.data());
        std::size_t h(0);

        for (std::string::size_type t(0) ; t < s1.length() ; ++t)
        {
            std::size_t hh(h & h_mask);
            h <<= 5;
            h ^= (hh >> h_shift);
            h ^= s1[t];
        }

        return h;
    }

#endif

}

#endif
