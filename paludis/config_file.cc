/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/config_file.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/tokeniser.hh>

#include <fstream>
#include <istream>
#include <list>
#include <map>

/** \file
 * Implementation for config_file.hh classes.
 *
 * \ingroup grpconfigfile
 */

using namespace paludis;

ConfigFileError::ConfigFileError(const std::string & our_message) throw () :
    ConfigurationError("Config file error: " + our_message)
{
}

ConfigFile::ConfigFile(std::istream * const stream) :
    _stream(stream),
    _has_lines(false),
    _destroy_stream(false)
{
}

ConfigFile::ConfigFile(const std::string & our_filename) try :
    _stream(_make_stream(our_filename)),
    _has_lines(false),
    _filename(our_filename),
    _destroy_stream(true)
{
}
catch (...)
{
    _destroy_stream = false;
    throw;
}

ConfigFile::ConfigFile(const FSEntry & our_filename) try :
    _stream(_make_stream(stringify(our_filename))),
    _has_lines(false),
    _filename(stringify(our_filename)),
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
    done_reading_lines();
}

void
ConfigFile::done_reading_lines() const
{
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

namespace paludis
{
    /**
     * Implementation data for LineConfigFile.
     *
     * \ingroup grplineconfigfile
     */
    template<>
    struct Implementation<LineConfigFile>
    {
        mutable std::list<std::string> lines;
    };
}

LineConfigFile::LineConfigFile(std::istream * const s) :
    ConfigFile(s),
    PrivateImplementationPattern<LineConfigFile>(new Implementation<LineConfigFile>)
{
    need_lines();
}

LineConfigFile::LineConfigFile(const std::string & our_filename) :
    ConfigFile(our_filename),
    PrivateImplementationPattern<LineConfigFile>(new Implementation<LineConfigFile>)
{
    need_lines();
}

LineConfigFile::LineConfigFile(const FSEntry & our_filename) :
    ConfigFile(our_filename),
    PrivateImplementationPattern<LineConfigFile>(new Implementation<LineConfigFile>)
{
    need_lines();
}

LineConfigFile::~LineConfigFile()
{
}

void
LineConfigFile::accept_line(const std::string & s) const
{
    _imp->lines.push_back(s);
}

LineConfigFile::Iterator
LineConfigFile::begin() const
{
    return Iterator(_imp->lines.begin());
}

LineConfigFile::Iterator
LineConfigFile::end() const
{
    return Iterator(_imp->lines.end());
}

KeyValueConfigFileError::KeyValueConfigFileError(const std::string & msg,
        const std::string & filename) throw () :
    ConfigurationError("Key/Value config file error" +
            (filename.empty() ? ": " : " in file '" + filename + "': ") + msg)
{
}

namespace paludis
{
    /**
     * Implementation data for KeyValueConfigFile.
     *
     * \ingroup grpkvconfigfile
     */
    template <>
    struct Implementation<KeyValueConfigFile>
    {
        mutable std::map<std::string, std::string> entries;
        mutable std::string accum;
        mutable std::string accum_key;
    };
}

KeyValueConfigFile::KeyValueConfigFile(std::istream * const s) :
    ConfigFile(s),
    PrivateImplementationPattern<KeyValueConfigFile>(new Implementation<KeyValueConfigFile>)
{
    need_lines();
}

KeyValueConfigFile::KeyValueConfigFile(const std::string & our_filename) :
    ConfigFile(our_filename),
    PrivateImplementationPattern<KeyValueConfigFile>(new Implementation<KeyValueConfigFile>)
{
    need_lines();
}

KeyValueConfigFile::KeyValueConfigFile(const FSEntry & our_filename) :
    ConfigFile(our_filename),
    PrivateImplementationPattern<KeyValueConfigFile>(new Implementation<KeyValueConfigFile>)
{
    need_lines();
}

KeyValueConfigFile::KeyValueConfigFile(std::istream * const s,
        std::tr1::shared_ptr<const AssociativeCollection<std::string, std::string> > m) :
    ConfigFile(s),
    PrivateImplementationPattern<KeyValueConfigFile>(new Implementation<KeyValueConfigFile>)
{
    _imp->entries.insert(m->begin(), m->end());
    need_lines();
}

KeyValueConfigFile::KeyValueConfigFile(const std::string & our_filename,
        std::tr1::shared_ptr<const AssociativeCollection<std::string, std::string> > m) :
    ConfigFile(our_filename),
    PrivateImplementationPattern<KeyValueConfigFile>(new Implementation<KeyValueConfigFile>)
{
    _imp->entries.insert(m->begin(), m->end());
    need_lines();
}

KeyValueConfigFile::KeyValueConfigFile(const FSEntry & our_filename,
        std::tr1::shared_ptr<const AssociativeCollection<std::string, std::string> > m):
    ConfigFile(our_filename),
    PrivateImplementationPattern<KeyValueConfigFile>(new Implementation<KeyValueConfigFile>)
{
    _imp->entries.insert(m->begin(), m->end());
    need_lines();
}

KeyValueConfigFile::~KeyValueConfigFile()
{
}

void
KeyValueConfigFile::accept_line(const std::string & line) const
{
    if (! _imp->accum.empty())
    {
        std::string value(line);
        normalise_line(value);

        if (value.empty())
            return;

        _imp->accum += " ";
        _imp->accum += value;

        if (value.at(value.length() - 1) == _imp->accum.at(0))
        {
            _imp->entries[_imp->accum_key] = replace_variables(strip_quotes(_imp->accum));
            _imp->accum.clear();
            _imp->accum_key.clear();
        }
    }
    else
    {
        std::string::size_type p(line.find('='));
        if (std::string::npos == p)
            _imp->entries[line] = "";
        else
        {
            std::string key(line.substr(0, p)), value(line.substr(p + 1));
            normalise_line(key);
            normalise_line(value);
            if (quotes_are_balanced(value))
                _imp->entries[key] = replace_variables(strip_quotes(value));
            else
            {
                Log::get_instance()->message(ll_warning, lc_context, "Line continuations should "
                        "be indicated with a backslash");
                _imp->accum = value;
                _imp->accum_key = key;
            }
        }
    }
}

void
KeyValueConfigFile::done_reading_lines() const
{
    if (! _imp->accum.empty())
        throw KeyValueConfigFileError("Unterminated multiline quoted string");
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

bool
KeyValueConfigFile::quotes_are_balanced(const std::string & s) const
{
    if (s.empty())
        return true;

    if (std::string::npos != std::string("'\"").find(s[0]))
    {
        if (s.length() < 2)
            return false;
        if (s[s.length() - 1] != s[0])
            return false;
        return true;
    }
    else
        return true;
}

KeyValueConfigFile::Iterator
KeyValueConfigFile::begin() const
{
    return Iterator(_imp->entries.begin());
}

KeyValueConfigFile::Iterator
KeyValueConfigFile::end() const
{
    return Iterator(_imp->entries.end());
}

std::string
KeyValueConfigFile::get(const std::string & key) const
{
    return _imp->entries[key];
}

