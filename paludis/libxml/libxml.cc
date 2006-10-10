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

#include <paludis/libxml/libxml.hh>
#include <list>
#include <paludis/util/join.hh>
#include <paludis/util/tokeniser.hh>

std::string
paludis::retarded_libxml_string_to_string(const xmlChar * s)
{
    if (s)
        return std::string(reinterpret_cast<const char *>(s));
    else
        return "";
}

std::string
paludis::normalise(const std::string & s)
{
    std::list<std::string> words;
    WhitespaceTokeniser::get_instance()->tokenise(s, std::back_inserter(words));
    return join(words.begin(), words.end(), " ");
}

