/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
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

#include <string>
#include <functional>

namespace paludis
{
    std::string strip_leading_string(const std::string & s, const std::string & prefix);

    std::string strip_leading(const std::string & s, const std::string & remove);

    std::string strip_trailing_string(const std::string & s, const std::string & suffix);

    std::string strip_trailing(const std::string & s, const std::string & remove);

    template <std::string (* f_)(const std::string &, const std::string &)>
    class StripAdapter :
        public std::unary_function<std::string, const std::string>
    {
        private:
            const std::string _second;

        public:
            StripAdapter(const std::string & second) :
                _second(second)
            {
            }

            std::string operator() (const std::string & first) const
            {
                return (*f_)(first, _second);
            }
    };

    typedef StripAdapter<&strip_leading_string> StripLeadingString;
    typedef StripAdapter<&strip_leading> StripLeading;
    typedef StripAdapter<&strip_trailing_string> StripTrailingString;
    typedef StripAdapter<&strip_trailing> StripTrailing;
}

#endif
