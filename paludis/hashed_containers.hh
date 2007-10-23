/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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
 * Declarations for various hashed container things.
 *
 * These are more complicated than they should be for two reasons:
 *
 * - There is no standard hashed container in C++. There is one in TR1, but not
 *   all compilers support TR1 yet. Most pre-TR1 standard library
 *   implementations do include hashed containers, but with various different
 *   class names and in different header files. For performance reasons, we need
 *   hashed containers if at all possible.
 *
 * - The current C++ standard doesn't have template typedefs.
 *
 * \ingroup g_data_structures
 *
 * \section Examples
 *
 * - None at this time.
 */

#include <paludis/util/validated.hh>
#include <paludis/util/tr1_type_traits.hh>
#include <paludis/name-fwd.hh>
#include <paludis/version_spec-fwd.hh>
#include <paludis/package_id-fwd.hh>

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
     * \ingroup g_data_structures
     */
    template <typename T_>
    struct CRCHash;

    /**
     * Make a hashed map of some kind from Key_ to Value_.
     *
     * \ingroup g_data_structures
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
     * \ingroup g_data_structures
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
     * \ingroup g_data_structures
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
     * \ingroup g_data_structures
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
         * \ingroup g_data_structures
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
     * \ingroup g_data_structures
     */
    template <>
    class PALUDIS_VISIBLE CRCHash<QualifiedPackageName> :
        public std::unary_function<const QualifiedPackageName, std::size_t>,
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
     * Hash, for PackageID.
     *
     * \ingroup g_data_structures
     */
    template <>
    class PALUDIS_VISIBLE CRCHash<PackageID> :
        public std::unary_function<const PackageID, std::size_t>,
        protected hashed_containers_internals::CRCHashBase
    {
        public:
            /// Hash function.
            std::size_t operator() (const PackageID & val) const;

#if (! defined(PALUDIS_HASH_IS_STD_TR1_UNORDERED)) && (! defined(PALUDIS_HASH_IS_GNU_CXX_HASH))
            enum
            {
                min_buckets = 32,
                bucket_size = 4
            };

            bool operator() (const PackageID & i1, const PackageID & i2) const;
#endif
    };

    /**
     * Hash, for a validated string type.
     *
     * \ingroup g_data_structures
     */
    template <typename Validated_, bool b_>
    class CRCHash<Validated<std::string, Validated_, b_> > :
        public std::unary_function<const Validated<std::string, Validated_, b_>, std::size_t>,
        protected hashed_containers_internals::CRCHashBase
    {
        public:
            /// Hash function.
            std::size_t operator() (const Validated<std::string, Validated_, b_> & val) const;

#if (! defined(PALUDIS_HASH_IS_STD_TR1_UNORDERED)) && (! defined(PALUDIS_HASH_IS_GNU_CXX_HASH))
            enum
            {
                min_buckets = 32,
                bucket_size = 4
            };

            bool operator() (const Validated<std::string, Validated_, b_> i1,
                    const Validated<std::string, Validated_> & i2, b_) const
            {
                return i1.data() < i2.data();
            }
#endif
    };


    /**
     * Hash, for a string.
     *
     * \ingroup g_data_structures
     */
    template<>
    class PALUDIS_VISIBLE CRCHash<std::string> :
        public std::unary_function<const std::string, std::size_t>,
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
     * \ingroup g_data_structures
     */
    template <>
    class PALUDIS_VISIBLE CRCHash<std::pair<QualifiedPackageName, VersionSpec> > :
        public std::unary_function<const std::pair<const QualifiedPackageName, VersionSpec>, std::size_t>,
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

    /**
     * Hash, for a shared pointer.
     *
     * \ingroup g_data_structures
     */
    template <typename T_>
    class PALUDIS_VISIBLE CRCHash<tr1::shared_ptr<T_> > :
        public std::unary_function<tr1::shared_ptr<T_>, std::size_t>,
        protected hashed_containers_internals::CRCHashBase
    {
        public:
            /// Hash function.
            std::size_t operator() (const tr1::shared_ptr<T_> & val) const
            {
                return CRCHash<typename tr1::remove_const<T_>::type>()(*val);
            }

#if (! defined(PALUDIS_HASH_IS_STD_TR1_UNORDERED)) && (! defined(PALUDIS_HASH_IS_GNU_CXX_HASH))
            enum
            {
                min_buckets = 32,
                bucket_size = 4
            };

            bool operator() (const tr1::shared_ptr<T_> & i1, const tr1::shared_ptr<T_> & i2) const
            {
                return CRCHash<typename tr1::remove_const<T_>::type>()(*i1, *i2);
            }
#endif
    };

    template <typename Validated_, bool b_>
    std::size_t
    CRCHash<Validated<std::string, Validated_, b_> >::operator() (const Validated<std::string, Validated_, b_> & val) const
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
