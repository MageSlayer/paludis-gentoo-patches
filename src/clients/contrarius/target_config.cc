/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Danny van Dyk <kugelfang@gentoo.org>
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

#include <paludis/util/fs_entry.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/config_file.hh>
#include <list>
#include <string>
#include <vector>

#include "command_line.hh"
#include "target_config.hh"

using namespace paludis;

template class InstantiationPolicy<TargetConfig, instantiation_method::SingletonTag>;

void
TargetConfig::_parse_defaults()
{
    Context c("While parsing default configuration:");

    LineConfigFile defaults(FSEntry(DATADIR "/paludis/contrarius/default_config.txt"), LineConfigFileOptions());
    for (LineConfigFile::Iterator l(defaults.begin()), l_end(defaults.end()) ;
            l != l_end ; ++l)
    {
        Context c2("While parsing line '" + *l + "'");
        std::vector<std::string> tokens;
        WhitespaceTokeniser::get_instance()->tokenise(*l, std::back_inserter(tokens));
        SpecEntryList * list(&_binutils_list);

        if ((("aux" == tokens[1]) || "headers" == tokens[1]) && (2 == std::distance(tokens.begin(), tokens.end())))
            tokens.push_back("");
        else if (3 > std::distance(tokens.begin(), tokens.end()))
            throw TargetConfigError("Illegal number of tokens encountered");

        if ("binutils" == tokens[1])
            list = &_binutils_list;
        else if ("gcc" == tokens[1])
            list = &_gcc_list;
        else if ("headers" == tokens[1])
            list = &_headers_list;
        else if ("libc" == tokens[1])
            list = &_libc_list;
        else if ("aux" == tokens[1])
            list = &_aux_list;
        else
            throw TargetConfigError("Invalid key '" + tokens[1] + "' encountered");

        std::string entry(tokens[2]);
        for (std::vector<std::string>::const_iterator t(tokens.begin() + 3), t_end(tokens.end()) ;
                t != t_end ; ++t)
            entry += " " + *t;

        list->push_back(std::make_pair(tokens[0], entry));
    }
}

std::string
TargetConfig::_find_match(SpecEntryList & list)
{
    Tokeniser<delim_kind::AnyOfTag, delim_mode::DelimiterTag> tokeniser("-");
    std::vector<std::string> tokens;

    for (SpecEntryList::const_iterator i(list.begin()), i_end(list.end()) ;
            i != i_end ; ++i)
    {
        tokens.clear();
        tokeniser.tokenise(i->first, std::back_inserter(tokens));

        for (unsigned index(0) ; index < 4 ; ++index)
        {
            std::string & token(tokens[index]);

            std::string t;
            if (0 == index)
                t = stringify(_target.architecture);
            else if (1 == index)
                t = stringify(_target.manufacturer);
            else if (2 == index)
                t = stringify(_target.kernel);
            else if (3 == index)
                t = stringify(_target.userland);

            if ((t.empty()) && ("*" == token))
                continue;

            if (t.length() < token.length())
                break;

            if (0 != token.compare(0, token.length() - 1, t, 0, token.length() - 1))
                break;

            if (('*' == token[token.length() - 1]) || (t[token.length() - 1] == token[token.length() - 1]))
            {
                if (3 == index)
                    return i->second;
                else
                    continue;
            }
            else
                break;
        }
    }

    throw TargetConfigError("Unknown CTARGET '" + stringify(_target) + "'");
}

TargetConfig::TargetConfig() :
    _target(CommandLine::get_instance()->a_target.argument())
{
    _parse_defaults();

    _binutils = "cross-" + stringify(_target) + "/" + _find_match(_binutils_list);
     Log::get_instance()->message(ll_debug, lc_no_context, "Using configuration:\n"
            "  binutils: " + _binutils + "\n");

    _gcc = "cross-" + stringify(_target) + "/" + _find_match(_gcc_list);
    Log::get_instance()->message(ll_debug, lc_no_context, "Using configuration:\n"
            "  gcc:      " + _gcc + "\n");

    _headers = _find_match(_headers_list);
    if (! _headers.empty())
        _headers = "cross-" + stringify(_target) + "/" + _find_match(_headers_list);
    Log::get_instance()->message(ll_debug, lc_no_context, "Using configuration:\n"
            "  headers:  " + (_headers.empty() ? "[none]" : _headers) + "\n");

    _libc = "cross-" + stringify(_target) + "/" + _find_match(_libc_list);
    Log::get_instance()->message(ll_debug, lc_no_context, "Using configuration:\n"
            "  libc:     " + _libc + "\n");

    _aux = _find_match(_aux_list);
    Log::get_instance()->message(ll_debug, lc_no_context, "Using configuration:\n"
            "  aux:      " + _aux + "\n");
}

