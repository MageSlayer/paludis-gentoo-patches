/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include <paludis/util/extract_host_from_url.hh>
#include <paludis/util/simple_parser.hh>

using namespace paludis;

std::string
paludis::extract_host_from_url(const std::string & h)
{
    std::string result;
    SimpleParser parser(h);

    if (! parser.consume(+simple_parser::any_except(":")))
        return "";
    if (! parser.consume(+simple_parser::exact("://")))
        return "";

    if (parser.consume(+simple_parser::any_except("/") >> result))
        return result;
    else if (parser.consume(+simple_parser::any_except("") >> result))
        return result;
    else
        return result;
}

