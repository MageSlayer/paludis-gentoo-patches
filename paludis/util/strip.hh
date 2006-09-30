/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_STRIP_HH
#define PALUDIS_GUARD_PALUDIS_STRIP_HH 1

#include <functional>
#include <string>
#include <paludis/util/attributes.hh>

/** \file
 * Strip functions and adapters.
 *
 * \ingroup grpstrippers
 */

namespace paludis
{
    /**
     * Return a string equal to s minus any leading characters that are
     * contained in prefix.
     *
     * \ingroup grpstrippers
     */
    std::string strip_leading_string(const std::string & s, const std::string & prefix) PALUDIS_VISIBLE;

    /**
     * Return a string equal to s, minus the string remove if remove occurs at
     * the start of s.
     *
     * \ingroup grpstrippers
     */
    std::string strip_leading(const std::string & s, const std::string & remove) PALUDIS_VISIBLE;

    /**
     * Return a string equal to s minus any trailing characters that are
     * contained in suffix.
     *
     * \ingroup grpstrippers
     */
    std::string strip_trailing_string(const std::string & s, const std::string & suffix) PALUDIS_VISIBLE;

    /**
     * Return a string equal to s, minus the string remove if remove occurs at
     * the end of s.
     *
     * \ingroup grpstrippers
     */
    std::string strip_trailing(const std::string & s, const std::string & remove) PALUDIS_VISIBLE;

    /**
     * Adapt one of the strip_ functions for use as a std::unary_function by
     * binding a value to the second parameter (avoids the reference to const
     * issue with std::bind2nd).
     *
     * \ingroup grpstrippers
     */
    template <std::string (* f_)(const std::string &, const std::string &)>
    class StripAdapter :
        public std::unary_function<std::string, const std::string>
    {
        private:
            const std::string _second;

        public:
            ///\name Basic operations
            ///\{

            StripAdapter(const std::string & second) :
                _second(second)
            {
            }

            ///\}

            /**
             * Operation.
             */
            std::string operator() (const std::string & first) const
            {
                return (*f_)(first, _second);
            }
    };

    /**
     * Adapt strip_leading_string to a functor by binding its second argument.
     *
     * \ingroup grpstrippers
     */
    typedef StripAdapter<&strip_leading_string> StripLeadingString;

    /**
     * Adapt strip_leading to a functor by binding its second argument.
     *
     * \ingroup grpstrippers
     */
    typedef StripAdapter<&strip_leading> StripLeading;

    /**
     * Adapt strip_trailing_string to a functor by binding its second argument.
     *
     * \ingroup grpstrippers
     */
    typedef StripAdapter<&strip_trailing_string> StripTrailingString;

    /**
     * Adapt strip_trailing to a functor by binding its second argument.
     *
     * \ingroup grpstrippers
     */
    typedef StripAdapter<&strip_trailing> StripTrailing;
}

#endif
