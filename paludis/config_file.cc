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

ConfigFile::ConfigFile(const FSEntry & filename) try :
    _stream(_make_stream(stringify(filename))),
    _has_lines(false),
    _filename(stringify(filename)),
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

namespace paludis
{
    /**
     * Implementation data for LineConfigFile.
     *
     * \ingroup grplineconfigfile
     */
    template<>
    struct Implementation<LineConfigFile> :
        InternalCounted<Implementation<LineConfigFile> >
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

LineConfigFile::LineConfigFile(const std::string & filename) :
    ConfigFile(filename),
    PrivateImplementationPattern<LineConfigFile>(new Implementation<LineConfigFile>)
{
    need_lines();
}

LineConfigFile::LineConfigFile(const FSEntry & filename) :
    ConfigFile(filename),
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
    struct Implementation<KeyValueConfigFile> :
        InternalCounted<Implementation<KeyValueConfigFile> >
    {
        mutable std::map<std::string, std::string> entries;
    };
}

KeyValueConfigFile::KeyValueConfigFile(std::istream * const s) :
    ConfigFile(s),
    PrivateImplementationPattern<KeyValueConfigFile>(new Implementation<KeyValueConfigFile>)
{
    need_lines();
}

KeyValueConfigFile::KeyValueConfigFile(const std::string & filename) :
    ConfigFile(filename),
    PrivateImplementationPattern<KeyValueConfigFile>(new Implementation<KeyValueConfigFile>)
{
    need_lines();
}

KeyValueConfigFile::KeyValueConfigFile(const FSEntry & filename) :
    ConfigFile(filename),
    PrivateImplementationPattern<KeyValueConfigFile>(new Implementation<KeyValueConfigFile>)
{
    need_lines();
}

KeyValueConfigFile::KeyValueConfigFile(std::istream * const s,
        AssociativeCollection<std::string, std::string>::ConstPointer m) :
    ConfigFile(s),
    PrivateImplementationPattern<KeyValueConfigFile>(new Implementation<KeyValueConfigFile>)
{
    _imp->entries.insert(m->begin(), m->end());
    need_lines();
}

KeyValueConfigFile::KeyValueConfigFile(const std::string & filename,
        AssociativeCollection<std::string, std::string>::ConstPointer m) :
    ConfigFile(filename),
    PrivateImplementationPattern<KeyValueConfigFile>(new Implementation<KeyValueConfigFile>)
{
    _imp->entries.insert(m->begin(), m->end());
    need_lines();
}

KeyValueConfigFile::KeyValueConfigFile(const FSEntry & filename,
        AssociativeCollection<std::string, std::string>::ConstPointer m) :
    ConfigFile(filename),
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
    std::string::size_type p(line.find('='));
    if (std::string::npos == p)
        _imp->entries[line] = "";
    else
    {
        std::string key(line.substr(0, p)), value(line.substr(p + 1));
        normalise_line(key);
        normalise_line(value);
        _imp->entries[key] = replace_variables(strip_quotes(value));
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

AdvisoryFileError::AdvisoryFileError(const std::string & msg,
        const std::string & filename) throw () :
    ConfigurationError("Advisory file error" +
            (filename.empty() ? ": " : "in file '" + filename + "': ") + msg)
{
}

namespace paludis
{
    /**
     * Implementation data for AdvisoryFile.
     *
     * \ingroup grpadvisoryconfigfile
     */
    template<>
    struct Implementation<AdvisoryFile> :
        InternalCounted<Implementation<AdvisoryFile> >
    {
        mutable std::map<std::string, std::string> entries;
        mutable std::list<std::string> affected;
        mutable std::list<std::string> unaffected;
        mutable bool end_of_header;

        Implementation() :
            end_of_header(false)
        {
        }
    };
}

AdvisoryFile::AdvisoryFile(std::istream * const s) :
    ConfigFile(s),
    PrivateImplementationPattern<AdvisoryFile>(new Implementation<AdvisoryFile>)
{
    need_lines();
    sanitise();
}

AdvisoryFile::AdvisoryFile(const std::string & filename) :
    ConfigFile(filename),
    PrivateImplementationPattern<AdvisoryFile>(new Implementation<AdvisoryFile>)
{
    need_lines();
    sanitise();
}

AdvisoryFile::AdvisoryFile(const FSEntry & filename) :
    ConfigFile(filename),
    PrivateImplementationPattern<AdvisoryFile>(new Implementation<AdvisoryFile>)
{
    need_lines();
    sanitise();
}

#if 0
AdvisoryFile::AdvisoryFile(std::istream * const s,
        const std::map<std::string, std::string> & m) :
    ConfigFile(s),
    PrivateImplementationPattern<AdvisoryFile>(new Implementation<AdvisoryFile>)
{
    _imp->entries.insert(m.begin(), m.end());
    need_lines();
    sanitise();
}

AdvisoryFile::AdvisoryFile(const std::string & filename,
        const std::map<std::string, std::string> & m) :
    ConfigFile(filename),
    PrivateImplementationPattern<AdvisoryFile>(new Implementation<AdvisoryFile>)
{
    _imp->entries.insert(m.begin(), m.end());
    need_lines();
    sanitise();
}

AdvisoryFile::AdvisoryFile(const FSEntry & filename,
        const std::map<std::string, std::string> & m) :
    ConfigFile(filename),
    PrivateImplementationPattern<AdvisoryFile>(new Implementation<AdvisoryFile>)
{
    _imp->entries.insert(m.begin(), m.end());
    need_lines();
    sanitise();
}
#endif

AdvisoryFile::~AdvisoryFile()
{
}

void
AdvisoryFile::accept_line(const std::string & line) const
{
    std::string::size_type p(line.find(':'));

    if ((std::string::npos == p) || (_imp->end_of_header))
    {
        _imp->entries["Description"] += line + "\n";
        _imp->end_of_header = true;
    }
    else
    {
        std::string key(line.substr(0, p)), value(line.substr(p + 1));
        normalise_line(key);
        normalise_line(value);
        if ((key == "Affected") || (key == "Bug-Id") || (key == "CVE") || (key == "Reference")
            || (key == "Restart") || (key == "Unaffected"))
        {
            if (key == "Affected")
                _imp->affected.push_back(value);
            else if (key == "Unaffected")
                _imp->unaffected.push_back(value);
            else
            {
                if (! _imp->entries[key].empty())
                    value = "\n" + value;
                _imp->entries[key] += value;
            }
        }
        else
        {
            if (_imp->entries[key].empty())
                _imp->entries[key] = value;
            else
                throw AdvisoryFileError("When adding value for key '" + key + "': Duplicate key found.");
        }
    }
}

void
AdvisoryFile::sanitise()
{
    if (_imp->entries["Id"].empty())
        throw AdvisoryFileError("Missing mandatory key: 'Id'.");

    if (_imp->entries["Title"].empty())
            throw AdvisoryFileError("Missing mandatory key: 'Title'.");

    if (_imp->entries["Access"].empty())
            throw AdvisoryFileError("Missing mandatory key: 'Access'.");

    if (_imp->entries["Last-Modified"].empty())
            throw AdvisoryFileError("Missing mandatory key: 'Last-Modified'.");

    if (_imp->entries["Revision"].empty())
            throw AdvisoryFileError("Missing mandatory key: 'Revision'.");

    if (_imp->entries["Severity"].empty())
            throw AdvisoryFileError("Missing mandatory key: 'Severity'.");

    if (_imp->entries["Spec-Version"].empty())
            throw AdvisoryFileError("Missing mandatory key: 'Spec-Version'.");
}

AdvisoryFile::EntriesIterator
AdvisoryFile::begin() const
{
    return EntriesIterator(_imp->entries.begin());
}

AdvisoryFile::EntriesIterator
AdvisoryFile::end() const
{
    return EntriesIterator(_imp->entries.end());
}

AdvisoryFile::LineIterator
AdvisoryFile::begin_affected() const
{
    return LineIterator(_imp->affected.begin());
}

AdvisoryFile::LineIterator
AdvisoryFile::end_affected() const
{
    return LineIterator(_imp->affected.end());
}

AdvisoryFile::LineIterator
AdvisoryFile::begin_unaffected() const
{
    return LineIterator(_imp->unaffected.begin());
}

AdvisoryFile::LineIterator
AdvisoryFile::end_unaffected() const
{
    return LineIterator(_imp->unaffected.end());
}

std::string
AdvisoryFile::get(const std::string & key) const
{
    return _imp->entries[key];
}

std::string
KeyValueConfigFile::get(const std::string & key) const
{
    return _imp->entries[key];
}

