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
    a_all_versions(&g_candidate_options, "all-versions", 'a', "Search in every version of packages", true),
    a_visible(&g_candidate_options, "visible", 'v', "Search only in visible (not masked) versions of packages", true),
    a_matching(&g_candidate_options, "matching", 'm', "Search only in packages matching the supplied specification")
{
}

SearchCommandLineMatchOptions::SearchCommandLineMatchOptions(args::ArgsHandler * const h) :
    ArgsSection(h, "Match Options"),
    g_pattern_options(this, "Pattern Options", "Alter how patterns are matched."),
    a_type(&g_pattern_options, "type", 't', "Specify which matching algorithm to use",
            args::EnumArg::EnumArgOptions
            ("text",  't', "Match an exact text substring, ignoring case")
            ("exact", 'x', "Match only an entire exact string, ignoring case")
            ("regex", 'r', "Match using ECMAScript regular expressions, ignoring case"),
            "text"
          ),
    a_case_sensitive(&g_pattern_options, "case-sensitive", 'C', "Make matching case sensitive.", true),
    a_and(&g_pattern_options, "and", '&', "If multiple patterns are specified, require that "
            "all patterns match. Default is to succeed if any pattern matches.", true),
    a_not(&g_pattern_options, "not", '!', "Invert the results of pattern matches.", true),

    g_search_key_options(this, "Search Key Options", "Alter the keys used for searching. If "
            "no option in this group is specified, matches are carried out on name and description. Otherwise, "
            "matches are carried out on all of the specified keys."),
    a_key(&g_search_key_options, "key", 'k', "Search the named metadata key (e.g. DESCRIPTION). May be specified "
            "multiple times."),
    a_name(&g_search_key_options, "name", 'n', "Search package names.", true),
    a_description(&g_search_key_options, "description", 'd', "Search package descriptions.", true),

    g_key_handling_options(this, "Key Handling Options", "Alter how key values are interpreted."),
    a_enabled_only(&g_key_handling_options, "enabled-only", 'e', "Only search enabled parts of conditional "
            "dependency spec trees", true)
{
}

SearchCommandLineIndexOptions::SearchCommandLineIndexOptions(args::ArgsHandler * const h) :
    ArgsSection(h, "Index Options"),
    g_index_options(this, "Index Options", "Controls the use of an index. An index may be created using "
            "cave manage-search-index. Note that strange errors or partial results may occur if the index "
            "is not up to date."),
    a_index(&g_index_options, "index", '\0', "Use the specified index file")
{
}

