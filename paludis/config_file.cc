/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
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
#include <paludis/config_file.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>

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

    std::string line, accum;
    unsigned line_number(0);
    while (std::getline(*_stream, line))
    {
        Context c("When handling line " + stringify(++line_number) +
                (_filename.empty() ? std::string(":") : " in file '" + _filename + "':"));
        normalise_line(line);

        if (line.empty() || skip_line(line))
        {
            if (!accum.empty())
                throw ConfigFileError("Line-continuation followed by a blank line or comment is invalid.");

            continue;
        }
        if ('\\' == line.at(line.length() - 1))
        {
            line.erase(line.length() - 1);
            accum += line;
            continue;
        }

        accept_line(accum + line);
        accum.clear();
    }
    if (! _stream->eof())
        throw ConfigFileError("Error reading from file");
    if (! accum.empty())
        throw ConfigFileError("Line-continuation needs a continuation.");

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

LineConfigFile::LineConfigFile(std::istream * const s) :
    ConfigFile(s)
{
    need_lines();
}

LineConfigFile::LineConfigFile(const std::string & filename) :
    ConfigFile(filename)
{
    need_lines();
}

void
LineConfigFile::accept_line(const std::string & s) const
{
    _lines.push_back(s);
}

KeyValueConfigFileError::KeyValueConfigFileError(const std::string & msg,
        const std::string & filename) throw () :
    ConfigurationError("Key/Value config file error" +
            (filename.empty() ? ": " : "in file '" + filename + "': ") + msg)
{
}

KeyValueConfigFile::KeyValueConfigFile(std::istream * const s) :
    ConfigFile(s)
{
    need_lines();
}

KeyValueConfigFile::KeyValueConfigFile(const std::string & filename) :
    ConfigFile(filename)
{
    need_lines();
}

KeyValueConfigFile::KeyValueConfigFile(std::istream * const s,
        const std::map<std::string, std::string> & m) :
    ConfigFile(s),
    _entries(m.begin(), m.end())
{
    need_lines();
}

KeyValueConfigFile::KeyValueConfigFile(const std::string & filename,
        const std::map<std::string, std::string> & m) :
    ConfigFile(filename),
    _entries(m.begin(), m.end())
{
    need_lines();
}

KeyValueConfigFile::~KeyValueConfigFile()
{
}

void
KeyValueConfigFile::accept_line(const std::string & line) const
{
    std::string::size_type p(line.find('='));
    if (std::string::npos == p)
        _entries[line] = "";
    else
    {
        std::string key(line.substr(0, p)), value(line.substr(p + 1));
        normalise_line(key);
        normalise_line(value);
        _entries[key] = replace_variables(strip_quotes(value));
    }
}

std::string
KeyValueConfigFile::replace_variables(const std::string & s) const
{
    std::string r;
    std::string::size_type p(0), old_p(0);

    while (p < s.length())
    {
        old_p = p;

        if ('\\' == s[p])
        {
            if (++p >= s.length())
                throw KeyValueConfigFileError("Backslash not followed by a character", filename());
            r += s[p++];
        }
        else if ('$' != s[p])
            r += s[p++];
        else
        {
            std::string name;
            if (++p >= s.length())
                throw KeyValueConfigFileError("Dollar not followed by a character", filename());

            if ('{' == s[p])
            {
                std::string::size_type q;
                if (std::string::npos == ((q = s.find("}", p))))
                    throw KeyValueConfigFileError("Closing } not found", filename());

                name = s.substr(p + 1, q - p - 1);
                p = q + 1;
            }
            else
            {
                std::string::size_type q;
                if (std::string::npos == ((q = s.find_first_not_of(
                                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                    "abcdefghijklmnopqrstuvwxyz"
                                    "_0123456789", p))))
                    q = s.length();

                name = s.substr(p, q - p);
                p = q;
            }

            if (name.empty())
                throw KeyValueConfigFileError("Empty variable name", filename());
            r += get(name);
        }

        if (p <= old_p)
            throw InternalError(PALUDIS_HERE, "Infinite loop");
    }

    return r;
}

std::string
KeyValueConfigFile::strip_quotes(const std::string & s) const
{
    if (s.empty())
        return s;
    if (std::string::npos != std::string("'\"").find(s[0]))
    {
        if (s.length() < 2)
            throw KeyValueConfigFileError("Unterminated quote", filename());
        if (s[s.length() - 1] != s[0])
            throw KeyValueConfigFileError("Mismatched quote", filename());
        return s.substr(1, s.length() - 2);
    }
    else
        return s;
}

AdvisoryFileError::AdvisoryFileError(const std::string & msg,
        const std::string & filename) throw () :
    ConfigurationError("Advisory file error" +
            (filename.empty() ? ": " : "in file '" + filename + "': ") + msg)
{
}

AdvisoryFile::AdvisoryFile(std::istream * const s) :
    ConfigFile(s),
    _end_of_header(false)
{
    need_lines();
    sanitise();
}

AdvisoryFile::AdvisoryFile(const std::string & filename) :
    ConfigFile(filename),
    _end_of_header(false)
{
    need_lines();
    sanitise();
}

AdvisoryFile::AdvisoryFile(std::istream * const s,
        const std::map<std::string, std::string> & m) :
    ConfigFile(s),
    _entries(m.begin(), m.end()),
    _end_of_header(false)
{
    _end_of_header = false;
    need_lines();
    sanitise();
}

AdvisoryFile::AdvisoryFile(const std::string & filename,
        const std::map<std::string, std::string> & m) :
    ConfigFile(filename),
    _entries(m.begin(), m.end()),
    _end_of_header(false)
{
    need_lines();
    sanitise();
}

AdvisoryFile::~AdvisoryFile()
{
}

void
AdvisoryFile::accept_line(const std::string & line) const
{
    std::string::size_type p(line.find(':'));

    if ((std::string::npos == p) || (_end_of_header))
    {
        _entries["Description"] += line + "\n";
        _end_of_header = true;
    }
    else
    {
        std::string key(line.substr(0, p)), value(line.substr(p + 1));
        normalise_line(key);
        normalise_line(value);
        if ((key == "Affected") || (key == "Unaffected") || (key == "Bug-Id") || (key == "Url")
            || (key == "Reviewed-By"))
        {
            if (!_entries[key].empty())
                value = "\n" + value;
            _entries[key] += value;
        }
        else
        {
            if (_entries[key].empty())
                _entries[key] = value;
            else
                throw ConfigFileError("When adding value for key '" + key + "': Duplicate key found.");
        }
    }
}

void
AdvisoryFile::sanitise()
{
    if (_entries["Id"].empty())
        throw AdvisoryFileError("Missing mandatory key: 'Id'.");

    if (_entries["Title"].empty())
            throw AdvisoryFileError("Missing mandatory key: 'Title'.");

    if (_entries["Commited-By"].empty())
            throw AdvisoryFileError("Missing mandatory key: 'Commited-By'.");

    if (_entries["Reviewed-By"].empty())
            throw AdvisoryFileError("Missing mandatory key: 'Reviewed-by'.");

    if ((_entries["Affected"].size() + _entries["Unaffected"].size()) == 0)
            throw AdvisoryFileError("Missing either 'Affected' or 'Unaffected' key.");
}
