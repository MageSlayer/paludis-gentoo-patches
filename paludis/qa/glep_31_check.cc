/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <fstream>
#include <paludis/qa/glep_31_check.hh>

using namespace paludis;
using namespace paludis::qa;

void
Glep31Check::check_utf8(std::istream & f, CheckResult & r)
{
    struct BadChar
    {
    };

    int c, line_number(1);
    try
    {
        while (std::istream::traits_type::eof() != ((c = f.get())))
        {
            if (c < 0)
                throw BadChar();

            else if (c > 0x7e && (((c & 0xe0) == 0xc0)))
            {
                if (std::istream::traits_type::eof() == ((c = f.get())))
                    throw BadChar();

                if ((c & 0xc0) != 0x80)
                    throw BadChar();
            }
            else if (c > 0x7e && (((c & 0xf0) == 0xe0)))
            {
                if (std::istream::traits_type::eof() == ((c = f.get())))
                    throw BadChar();

                if ((c & 0xc0) != 0x80)
                    throw BadChar();

                if (std::istream::traits_type::eof() == ((c = f.get())))
                    throw BadChar();

                if ((c & 0xc0) != 0x80)
                    throw BadChar();
            }
            else if (c > 0x7e && (((c & 0xf8) == 0xf0)))
            {
                if (std::istream::traits_type::eof() == ((c = f.get())))
                    throw BadChar();

                if ((c & 0xc0) != 0x80)
                    throw BadChar();

                if (std::istream::traits_type::eof() == ((c = f.get())))
                    throw BadChar();

                if ((c & 0xc0) != 0x80)
                    throw BadChar();

                if (std::istream::traits_type::eof() == ((c = f.get())))
                    throw BadChar();

                if ((c & 0xc0) != 0x80)
                    throw BadChar();
            }
            else if ('\n' == c)
                ++line_number;
            else if (c <= 0x7e)
                ;
            else
                throw BadChar();
        }
    }
    catch (const BadChar &)
    {
        r << Message(qal_major, "Bad character on line " + stringify(line_number));
    }
}

Glep31Check::Glep31Check()
{
}

CheckResult
Glep31Check::operator() (const FSEntry & f) const
{
    CheckResult result(f, identifier());

    if (! f.is_regular_file())
        result << Message(qal_skip, "Not a regular file");
    else
    {
        std::ifstream ff(stringify(f).c_str());
        if (! ff)
            result << Message(qal_major, "Can't read file");
        else
            check_utf8(ff, result);
    }

    return result;
}

const std::string &
Glep31Check::identifier()
{
    static const std::string id("glep 31");
    return id;
}

