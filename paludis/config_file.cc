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

#include "config_file.hh"
#include "config_file_error.hh"
#include "strip.hh"
#include "exception.hh"
#include "stringify.hh"

using namespace paludis;

ConfigFile::ConfigFile(std::istream * const stream) :
    _stream(stream),
    _has_lines(false)
{
}

void
ConfigFile::need_lines() const
{
    if (_has_lines)
        return;

    std::string line;
    unsigned line_number(0);
    while (std::getline(*_stream, line))
    {
        Context c("When handling line " + stringify(++line_number) + ":");
        normalise_line(line);
        if (skip_line(line))
            continue;
        accept_line(line);
    }
    if (! _stream->eof())
        throw ConfigFileError("Error reading from file");

    _has_lines = true;
}

void
ConfigFile::normalise_line(std::string & s) const
{
    s = strip_leading(strip_trailing(s, " \t\n"), " \t\n");
}

bool
ConfigFile::skip_line(const std::string & s) const
{
    return (s.empty() || '#' == s.at(0));
}

