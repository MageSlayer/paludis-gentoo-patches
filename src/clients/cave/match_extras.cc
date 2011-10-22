/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include "match_extras.hh"
#include <paludis/args/do_help.hh>
#include <pcrecpp.h>

using namespace paludis;

extern "C" bool
cave_match_extras_match_regex(const std::string & text, const std::string & pattern_str, bool case_sensitive)
{
    const pcrecpp::RE pattern(pattern_str, pcrecpp::RE_Options().set_caseless(!case_sensitive));
    if (! pattern.error().empty())
        throw args::DoHelp("Pattern '" + pattern_str + "' error: " + pattern.error());

    return pattern.PartialMatch(text);
}

