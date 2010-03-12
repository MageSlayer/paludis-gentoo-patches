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

#include "cmd_search_cmdline.hh"

using namespace paludis;
using namespace cave;

SearchCommandLineCandidateOptions::SearchCommandLineCandidateOptions(args::ArgsHandler * const h) :
    ArgsSection(h, "Search Candidate Options"),
    g_candidate_options(this, "Candidate Options", "Control which packages and versions are selected as "
            "candidates for matching."),
    a_all_versions(&g_candidate_options, "all-versions", 'a', "Search in every version of packages", true)
{
}

