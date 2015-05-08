/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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

#include "format_string.hh"
#include "format_user_config.hh"
#include <paludis/util/map-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/destringify.hh>
#include <vector>
#include <list>

using namespace paludis;
using namespace cave;

FormatStringError::FormatStringError(const std::string & f, const std::string & m) noexcept :
    Exception("Bad format string '" + f + "': " + m)
{
}

std::string
paludis::cave::format_string(
        const std::string & f,
        const std::shared_ptr<Map<char, std::string> > & m
        )
{
    std::string result;
    std::list<bool> condition_stack;
    condition_stack.push_front(true);

    for (std::string::size_type p(0), p_end(f.length()) ; p != p_end ; ++p)
    {
        switch (f.at(p))
        {
            case '\\':
                if (++p == p_end)
                    throw FormatStringError(f, "backslash at end of input");
                switch (f.at(p))
                {
                    case 'n':
                        if (*condition_stack.begin())
                            result.append("\n");
                        break;

                    case 't':
                        if (*condition_stack.begin())
                            result.append("\t");
                        break;

                    case 'a':
                        if (*condition_stack.begin())
                            result.append("\a");
                        break;

                    case 'e':
                        if (*condition_stack.begin())
                            result.append("\033");
                        break;

                    default:
                        if (*condition_stack.begin())
                            result.append(stringify(f.at(p)));
                        break;
                }
                break;

            case '%':
                if (++p == p_end)
                    throw FormatStringError(f, "percent at end of input");
                if (f.at(p) == '%')
                {
                    if (*condition_stack.begin())
                        result.append("%");
                }
                else if (f.at(p) == '{')
                {
                    std::string::size_type pp(f.find('}', p));
                    if (std::string::npos == p)
                        throw FormatStringError(f, "no closing brace found");
                    std::string ff(f.substr(p + 1, pp - p - 1));
                    std::vector<std::string> tokens;
                    tokenise_whitespace(ff, std::back_inserter(tokens));

                    if (0 == tokens.size())
                        throw FormatStringError(f, "no command inside {}");

                    if ("column" == tokens.at(0))
                    {
                        if (tokens.size() != 2)
                            throw FormatStringError(f, "{column} takes one parameter");

                        int c(destringify<unsigned>(tokens.at(1))), l(0);
                        for (int q(0), q_end(result.length()) ; q != q_end ; ++q)
                            if (27 == result[q])
                            {
                                for ( ; q != q_end ; ++q)
                                    if ('m' == result[q])
                                        break;
                            }
                            else if ('\n' == result[q])
                                l = 0;
                            else
                                ++l;

                        if (*condition_stack.begin())
                            result.append(std::string(std::max(1, c - l), ' '));
                    }
                    else if ("endif" == tokens.at(0))
                    {
                        if (tokens.size() != 1)
                            throw FormatStringError(f, "{endif} takes no parameters");
                        if (condition_stack.size() <= 1)
                            throw FormatStringError(f, "{endif} has no matching {if}");
                        condition_stack.pop_front();
                    }
                    else if ("else" == tokens.at(0))
                    {
                        if (tokens.size() != 1)
                            throw FormatStringError(f, "{else} takes no parameters");
                        *condition_stack.begin() = ! *condition_stack.begin();
                    }
                    else if ("if" == tokens.at(0))
                    {
                        if (tokens.size() != 2)
                            throw FormatStringError(f, "{if} takes one parameter");
                        if (tokens.at(1).length() != 1)
                            throw FormatStringError(f, "{if} parameter should be a single character");
                        if (m->end() == m->find(tokens.at(1).at(0)))
                            throw FormatStringError(f, "{if} parameter '" + tokens.at(1) + "' not a variable");
                        condition_stack.push_front(! m->find(tokens.at(1).at(0))->second.empty());
                    }
                    else if ("colour" == tokens.at(0))
                    {
                        if (tokens.size() != 2)
                            throw FormatStringError(f, "{colour} takes one parameter");

                        if (*condition_stack.begin())
                            result.append(FormatUserConfigFile::get_instance()->fetch(tokens.at(1), 0, ""));
                    }
                    else
                        throw FormatStringError(f, "unknown command '" + tokens.at(0) + "' inside {}");

                    p = pp;
                }
                else if (m->end() != m->find(f.at(p)))
                {
                    if (*condition_stack.begin())
                        result.append(stringify(m->find(f.at(p))->second));
                }
                else
                    throw FormatStringError(f, "unrecognised format item '%" + stringify(f.at(p)) + "'");
                break;

            default:
                if (*condition_stack.begin())
                    result.append(stringify(f.at(p)));
                break;
        }
    }

    if (condition_stack.size() != 1)
        throw FormatStringError(f, "{if} has no matching {endif}");

    return result;
}

namespace paludis
{
    template class Map<char, std::string>;
}

