/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2010, 2011 Ciaran McCreesh
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

#include "text_matcher.hh"
#include <paludis/util/pimp-impl.hh>
#include <string.h>

using namespace paludis;
using namespace inquisitio;

namespace paludis
{
    template<>
    struct Imp<TextMatcher>
    {
        std::string pattern;

        Imp(const std::string & s) :
            pattern(s)
        {
        }
    };
}

TextMatcher::TextMatcher(const std::string & s) :
    _imp(s)
{
}

bool
TextMatcher::operator() (const std::string & s) const
{
    return 0 != strcasestr(s.c_str(), _imp->pattern.c_str());
}

TextMatcher::~TextMatcher()
{
}


