/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include <paludis/config_file.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <fstream>

using namespace paludis;

ConfigFileError::ConfigFileError(const std::string & message) throw () :
    ConfigurationError("Config file error: " + message)
{
}

ConfigFile::ConfigFile(std::istream * const stream) :
    _stream(stream),
    _has_lines(false),
    _destroy_stream(false)
{
}

ConfigFile::ConfigFile(const std::string & filename) try :
    _stream(_make_stream(filename)),
    _has_lines(false),
    _filename(filename),
    _destroy_stream(true)
{
}
catch (...)
{
    _destroy_stream = false;
    throw;
}

ConfigFile::~ConfigFile()
{
    if (_stream && _destroy_stream)
        delete _stream;
}

std::istream *
ConfigFile::_make_stream(const std::string & filename)
{
    Context context("When creating the filestream for a ConfigFile from file '" + filename + "':");

    std::ifstream * result(new std::ifstream(filename.c_str()));
    if (! *result)
    {
        delete result;
        throw ConfigFileError("Could not open '" + filename + "'");
    }

    return result;
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
        Context c("When handling line " + stringify(++line_number) +
                (_filename.empty() ? std::string(":") : " in file '" + _filename + "':"));
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

