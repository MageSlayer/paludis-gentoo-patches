/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_OPTIONS_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_OPTIONS_HH 1

#include <paludis/util/options-fwd.hh>
#include <paludis/util/attributes.hh>
#include <initializer_list>

/** \file
 * Declarations for the Options<> class.
 *
 * \ingroup g_data_structures
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Used by Options<> for underlying storage.
     *
     * Holds a collection of bits, similar to std::bitset<>, but with no fixed
     * underlying size.
     *
     * \see Options<>
     * \ingroup g_data_structures
     */
    class PALUDIS_VISIBLE OptionsStore
    {
        private:
            unsigned long _bits;

        public:
            ///\name Basic operations
            ///\{

            OptionsStore();
            OptionsStore(const OptionsStore &) = default;
            ~OptionsStore() = default;

            OptionsStore & operator= (const OptionsStore &) = default;

            ///\}

            ///\name Modifications
            ///\{

            /**
             * Set the specified bit.
             */
            void add(const unsigned);

            /**
             * Unset the specified bit.
             */
            void remove(const unsigned);

            /**
             * Set any bit that is set in the parameter.
             */
            void combine(const OptionsStore &);

            /**
             * Unset any bit that is set in the parameter.
             */
            void subtract(const OptionsStore &);

            /**
             * Unset any bit that is not set in the parameter.
             */
            void intersect(const OptionsStore &);

            ///\}

            ///\name Tests
            ///\{

            /**
             * Is a particular bit set?
             */
            bool test(const unsigned) const;

            /**
             * Is any bit set?
             */
            bool any() const;

            /**
             * The highest bit that might be set.
             *
             * \since 0.40.1
             */
            unsigned highest_bit() const;

            ///\}
    };

    /**
     * Holds a series of true/false values mapped on an enum type, like a
     * std::bitset<> without the static size requirement.
     *
     * \ingroup g_data_structures
     */
    template <typename E_>
    class Options
    {
        private:
            OptionsStore _store;

        public:
            Options() = default;
            Options(const Options &) = default;
            Options & operator= (const Options &) = default;

            Options(std::initializer_list<E_> e)
            {
                for (auto i(e.begin()), i_end(e.end()) ;
                        i != i_end ; ++i)
                    _store.add(static_cast<unsigned>(*i));
            }

            /**
             * Return a copy of ourself with the specified bit enabled.
             */
            Options operator+ (const E_ & e) const
            {
                Options result(*this);
                result._store.add(static_cast<unsigned>(e));
                return result;
            }

            /**
             * Enable the specified bit.
             */
            Options & operator+= (const E_ & e)
            {
                _store.add(static_cast<unsigned>(e));
                return *this;
            }

            /**
             * Return a copy of ourself with the specified bit disabled.
             */
            Options operator- (const E_ & e) const
            {
                Options result(*this);
                result._store.remove(static_cast<unsigned>(e));
                return result;
            }

            /**
             * Disable the specified bit.
             */
            Options & operator-= (const E_ & e)
            {
                _store.remove(static_cast<unsigned>(e));
                return *this;
            }

            /**
             * Return a copy of ourself, bitwise 'or'ed with another Options set.
             */
            Options operator| (const Options<E_> & e) const
            {
                Options result(*this);
                result._store.combine(e._store);
                return result;
            }

            /**
             * Enable any bits that are enabled in the parameter.
             */
            Options & operator|= (const Options<E_> & e)
            {
                _store.combine(e._store);
                return *this;
            }

            /**
             * Return a copy of ourself, bitwise 'and'ed with another Options set.
             */
            Options operator& (const Options<E_> & e) const
            {
                Options result(*this);
                result._store.intersect(e._store);
                return result;
            }

            /**
             * Disable any bits that are not enabled in the parameter.
             */
            Options & operator&= (const Options<E_> & e)
            {
                _store.intersect(e._store);
                return *this;
            }

            /**
             * Disable any bits that are enabled in the parameter.
             */
            Options & subtract(const Options<E_> & e)
            {
                _store.subtract(e._store);
                return *this;
            }

            /**
             * Returns whether the specified bit is enabled.
             */
            bool operator[] (const E_ & e) const
            {
                return _store.test(static_cast<unsigned>(e));
            }

            /**
             * Returns whether any bit is enabled.
             */
            bool any() const
            {
                return _store.any();
            }

            /**
             * Returns whether all bits are disabled.
             */
            bool none() const
            {
                return ! _store.any();
            }

            /**
             * Return the value of the highest bit that might be enabled.
             *
             * \since 0.40.1
             */
            E_ highest_bit() const
            {
                return static_cast<E_>(_store.highest_bit());
            }
    };
}

#endif
