/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/util/private_implementation_pattern.hh>

namespace paludis
{
    class OptionsStore :
        private PrivateImplementationPattern<OptionsStore>
    {
        public:
            OptionsStore();
            OptionsStore(const OptionsStore &);
            const OptionsStore & operator= (const OptionsStore &);
            ~OptionsStore();

            void add(const unsigned);
            void remove(const unsigned);
            void combine(const OptionsStore &);
            void subtract(const OptionsStore &);

            bool test(const unsigned) const;
            bool any() const;
    };

    template <typename E_>
    class Options
    {
        private:
            OptionsStore _store;

        public:
            Options operator+ (const E_ & e) const
            {
                Options result(*this);
                result._store.add(static_cast<unsigned>(e));
                return result;
            }

            Options & operator+= (const E_ & e)
            {
                _store.add(static_cast<unsigned>(e));
                return *this;
            }

            Options operator- (const E_ & e) const
            {
                Options result(*this);
                result._store.remove(static_cast<unsigned>(e));
                return result;
            }

            Options & operator-= (const E_ & e)
            {
                _store.remove(static_cast<unsigned>(e));
                return *this;
            }

            Options operator| (const Options<E_> & e) const
            {
                Options result(*this);
                result._store.combine(e._store);
                return result;
            }

            Options & operator|= (const Options<E_> & e)
            {
                _store.combine(e._store);
                return *this;
            }

            Options & subtract(const Options<E_> & e)
            {
                _store.subtract(e._store);
                return *this;
            }

            bool operator[] (const E_ & e) const
            {
                return _store.test(static_cast<unsigned>(e));
            }

            bool any() const
            {
                return _store.any();
            }

            bool none() const
            {
                return ! _store.any();
            }
    };
}

#endif
