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
#include <paludis/util/join.hh>

#include <fstream>
#include <istream>
#include <list>
#include <set>
#include <map>

/** \file
 * Implementation for config_file.hh classes.
 *
 * \ingroup grpconfigfile
 */

using namespace paludis;

ConfigFileError::ConfigFileError(const std::string & f, const std::string & m) throw () :
    ConfigurationError("Configuration file error: " + (f.empty() ? m : f + ": " + m))
{
}

ConfigFileError::ConfigFileError(const std::string & m) throw () :
    ConfigurationError("Configuration file error: " + m)
{
}

ConfigFile::ConfigFile(const Source &)
{
}

ConfigFile::~ConfigFile()
{
}

namespace paludis
{
    template<>
    struct Implementation<ConfigFile::Source>
    {
        std::tr1::shared_ptr<std::istream> stream_to_delete;
        std::istream & stream;
        std::string filename;

        Implementation(std::istream & s) :
            stream(s)
        {
        }

        Implementation(const FSEntry & f) :
            stream_to_delete(new std::ifstream(stringify(f).c_str())),
            stream(*stream_to_delete),
            filename(stringify(f))
        {
        }

        Implementation(const std::string & s) :
            stream_to_delete(new std::ifstream(s.c_str())),
            stream(*stream_to_delete),
            filename(s)
        {
        }

        Implementation(std::tr1::shared_ptr<std::istream> s, std::istream & t, const std::string & f) :
            stream_to_delete(s),
            stream(t),
            filename(f)
        {
        }
    };
}

ConfigFile::Source::Source(const FSEntry & f) :
    PrivateImplementationPattern<ConfigFile::Source>(new Implementation<ConfigFile::Source>(f))
{
}

ConfigFile::Source::Source(const std::string & s) :
    PrivateImplementationPattern<ConfigFile::Source>(new Implementation<ConfigFile::Source>(s))
{
}

ConfigFile::Source::Source(std::istream & s) :
    PrivateImplementationPattern<ConfigFile::Source>(new Implementation<ConfigFile::Source>(s))
{
}

ConfigFile::Source::Source(const Source & s) :
    PrivateImplementationPattern<ConfigFile::Source>(new Implementation<ConfigFile::Source>(
                s._imp->stream_to_delete, s._imp->stream, s._imp->filename))
{
}

const ConfigFile::Source &
ConfigFile::Source::operator= (const Source & s)
{
    if (&s != this)
        _imp.reset(new Implementation<ConfigFile::Source>(s._imp->stream_to_delete, s._imp->stream, s._imp->filename));
    return *this;
}

ConfigFile::Source::~Source()
{
}

std::istream &
ConfigFile::Source::stream() const
{
    return _imp->stream;
}

std::string
ConfigFile::Source::filename() const
{
    return _imp->filename;
}

namespace paludis
{
    template<>
    struct Implementation<LineConfigFile>
    {
        std::list<std::string> lines;
    };
}

LineConfigFile::LineConfigFile(const Source & s) :
    ConfigFile(s),
    PrivateImplementationPattern<LineConfigFile>(new Implementation<LineConfigFile>)
{
    Context context("When parsing line configuration file" + (s.filename().empty() ? ":" :
                "'" + s.filename() + "':"));

    if (! s.stream())
        throw ConfigFileError(s.filename(), "Cannot read input");

    std::string line;
    while (std::getline(s.stream(), line))
    {
        if (line.empty())
            continue;

        while ('\\' == line.at(line.length() - 1))
        {
            std::string next_line;
            if (! std::getline(s.stream(), next_line))
                throw ConfigFileError(s.filename(), "Line continuation at end of input");

            if (next_line.empty())
                throw ConfigFileError(s.filename(), "Line continuation followed by empty line");

            next_line = strip_leading(strip_trailing(line, " \t\r\n"), " \t\r\n");
            if ((! line.empty()) && ('#' == line.at(0)))
                throw ConfigFileError(s.filename(), "Line continuation followed by comment");

            line.append(next_line);
        }

        line = strip_leading(strip_trailing(line, " \t\r\n"), " \t\r\n");
        if (line.empty())
            continue;

        if ('#' == line.at(0))
            continue;

        _imp->lines.push_back(line);
    }
}

LineConfigFile::~LineConfigFile()
{
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

namespace paludis
{
    template<>
    struct Implementation<KeyValueConfigFile::Defaults>
    {
        std::tr1::shared_ptr<const KeyValueConfigFile> kv;
        std::tr1::shared_ptr<const AssociativeCollection<std::string, std::string> > a;
        std::string (* f)(const std::string &, const std::string &);

        Implementation(std::tr1::shared_ptr<const KeyValueConfigFile> kvv,
                std::tr1::shared_ptr<const AssociativeCollection<std::string, std::string> > av,
                std::string (* fv)(const std::string &, const std::string &)) :
            kv(kvv),
            a(av),
            f(fv)
        {
        }
    };
}

template<>
KeyValueConfigFile::Defaults::Defaults(std::tr1::shared_ptr<const KeyValueConfigFile> v) :
    PrivateImplementationPattern<KeyValueConfigFile::Defaults>(new Implementation<KeyValueConfigFile::Defaults>(v,
                std::tr1::shared_ptr<const AssociativeCollection<std::string, std::string> >(), 0))
{
}

template<>
KeyValueConfigFile::Defaults::Defaults(std::tr1::shared_ptr<const AssociativeCollection<std::string, std::string> > a) :
    PrivateImplementationPattern<KeyValueConfigFile::Defaults>(new Implementation<KeyValueConfigFile::Defaults>(
                std::tr1::shared_ptr<const KeyValueConfigFile>(), a, 0))
{
}

template<>
KeyValueConfigFile::Defaults::Defaults(std::tr1::shared_ptr<KeyValueConfigFile> v) :
    PrivateImplementationPattern<KeyValueConfigFile::Defaults>(new Implementation<KeyValueConfigFile::Defaults>(v,
                std::tr1::shared_ptr<const AssociativeCollection<std::string, std::string> >(), 0))
{
}

template<>
KeyValueConfigFile::Defaults::Defaults(std::tr1::shared_ptr<AssociativeCollection<std::string, std::string> > a) :
    PrivateImplementationPattern<KeyValueConfigFile::Defaults>(new Implementation<KeyValueConfigFile::Defaults>(
                std::tr1::shared_ptr<const KeyValueConfigFile>(), a, 0))
{
}

KeyValueConfigFile::Defaults::Defaults(std::string (* f) (const std::string &, const std::string &)) :
    PrivateImplementationPattern<KeyValueConfigFile::Defaults>(new Implementation<KeyValueConfigFile::Defaults>(
                std::tr1::shared_ptr<const KeyValueConfigFile>(),
                std::tr1::shared_ptr<const AssociativeCollection<std::string, std::string> >(), f))
{
}

KeyValueConfigFile::Defaults::Defaults() :
    PrivateImplementationPattern<KeyValueConfigFile::Defaults>(new Implementation<KeyValueConfigFile::Defaults>(
                std::tr1::shared_ptr<const KeyValueConfigFile>(),
                std::tr1::shared_ptr<const AssociativeCollection<std::string, std::string> >(), 0))
{
}

KeyValueConfigFile::Defaults::Defaults(const Defaults & v) :
    PrivateImplementationPattern<KeyValueConfigFile::Defaults>(new Implementation<KeyValueConfigFile::Defaults>(
                v._imp->kv, v._imp->a, v._imp->f))
{
}

const KeyValueConfigFile::Defaults &
KeyValueConfigFile::Defaults::operator= (const Defaults & v)
{
    if (this != &v)
        _imp.reset(new Implementation<KeyValueConfigFile::Defaults>(v._imp->kv, v._imp->a, v._imp->f));
    return *this;
}

KeyValueConfigFile::Defaults::~Defaults()
{
}

std::string
KeyValueConfigFile::Defaults::get(const std::string & k) const
{
    if (_imp->kv)
        return _imp->kv->get(k);
    else if (_imp->a)
    {
        AssociativeCollection<std::string, std::string>::Iterator x(_imp->a->find(k));
        if (x == _imp->a->end())
            return "";
        else
            return x->second;
    }
    else if (_imp->f)
        return (_imp->f)(k, "");
    else
        return "";
}

namespace
{
    static std::map<std::string, std::string> empty_map;
}

KeyValueConfigFile::Defaults::Iterator
KeyValueConfigFile::Defaults::begin() const
{
    if (_imp->kv)
        return Iterator(_imp->kv->begin());
    else if (_imp->a)
        return Iterator(_imp->a->begin());
    else
        return Iterator(empty_map.begin());
}

KeyValueConfigFile::Defaults::Iterator
KeyValueConfigFile::Defaults::end() const
{
    if (_imp->kv)
        return Iterator(_imp->kv->end());
    else if (_imp->a)
        return Iterator(_imp->a->end());
    else
        return Iterator(empty_map.end());
}

namespace paludis
{
    template<>
    struct Implementation<KeyValueConfigFile>
    {
        KeyValueConfigFile::Defaults defaults;
        std::map<std::string, std::string> keys;
        std::string filename;
        bool (* is_incremental) (const std::string &, const KeyValueConfigFile &);

        Implementation(const KeyValueConfigFile::Defaults & d, bool (* i) (const std::string &, const KeyValueConfigFile &)) :
            defaults(d),
            is_incremental(i)
        {
        }
    };
}

namespace
{
    void next_line(std::istreambuf_iterator<char> & c, const std::istreambuf_iterator<char> & c_end)
    {
        for ( ; c != c_end ; ++c)
            if (*c == '\n')
                break;
    }

    std::string grab_key(std::istreambuf_iterator<char> & c, const std::istreambuf_iterator<char> & c_end)
    {
        std::string result;

        while (c != c_end)
        {
            if (*c == '\n' || *c == '\r' || *c == ' ' || *c == '\t' || *c == '=' || *c == '$' || *c == '\\'
                    || *c == '"' || *c == '\'')
                break;
            else
                result.append(stringify(*c++));
        }

        return result;
    }

    std::string grab_dollar(std::istreambuf_iterator<char> & c, const std::istreambuf_iterator<char> & c_end,
            const KeyValueConfigFile & d, const std::string & f)
    {
        std::string result;

        if (*c == '{')
        {
            ++c;
            while (c != c_end)
            {
                if (*c == '}')
                {
                    ++c;
                    if (result.empty())
                        throw ConfigFileError(f, "Bad empty variable name");
                    return d.get(result);
                }
                else if (*c == '\\')
                {
                    ++c;
                    if (c == c_end)
                        break;
                    if (*c == '\n')
                        ++c;
                    else
                        throw ConfigFileError(f, "Bad \\escape inside ${variable}");
                }
                else
                    result.append(stringify(*c++));
            }

            throw ConfigFileError(f, "Unterminated ${variable}");
        }
        else
        {
            while ((*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z')
                    || (*c >= '0' && *c <= '9') || (*c == '_'))
            {
                result.append(stringify(*c++));
                if (c == c_end)
                    break;
            }

            if (result.empty())
                throw ConfigFileError(f, "Bad empty variable name");
            return d.get(result);
        }
    }

    std::string grab_squoted(std::istreambuf_iterator<char> & c, const std::istreambuf_iterator<char> & c_end,
            const KeyValueConfigFile &, const std::string & f)
    {
        std::string result;

        while (c != c_end)
        {
            if (*c == '\\')
            {
                if (++c == c_end)
                    throw ConfigFileError(f, "Unterminated 'quoted string ending in a backslash");

                if (*c == '\n')
                {
                    result.append("\\ ");
                    c++;
                }
                else
                    result.append("\\" + stringify(*c++));
            }
            else if (*c == '\'')
            {
                ++c;
                return result;
            }
            else
                result.append(stringify(*c++));
        }

        throw ConfigFileError(f, "Unterminated 'quoted string");
    }

    std::string grab_dquoted(std::istreambuf_iterator<char> & c, const std::istreambuf_iterator<char> & c_end,
            const KeyValueConfigFile & d, const std::string & f)
    {
        std::string result;

        while (c != c_end)
        {
            if (*c == '\\')
            {
                if (++c == c_end)
                    throw ConfigFileError(f, "Unterminated \"quoted string ending in a backslash");

                if (*c == '\n')
                    ++c;
                else if (*c == 't')
                {
                    result.append("\t");
                    ++c;
                }
                else if (*c == 'n')
                {
                    result.append("\n");
                    ++c;
                }
                else
                    result.append(stringify(*c++));
            }
            else if (*c == '$')
                result.append(grab_dollar(++c, c_end, d, f));
            else if (*c == '"')
            {
                ++c;
                return result;
            }
            else
                result.append(stringify(*c++));
        }

        throw ConfigFileError(f, "Unterminated \"quoted string");
    }

    std::string grab_value(std::istreambuf_iterator<char> & c, const std::istreambuf_iterator<char> & c_end,
            const KeyValueConfigFile & d, const std::string & f)
    {
        std::string result;

        while (c != c_end)
        {
            if (*c == '"')
                result.append(grab_dquoted(++c, c_end, d, f));
            else if (*c == '\'')
                result.append(grab_squoted(++c, c_end, d, f));
            else if (*c == '\\')
            {
                if (++c == c_end)
                    throw ConfigFileError(f, "Backslash at end of input");

                if (*c == '\n')
                {
                    if (++c == c_end)
                        throw ConfigFileError(f, "Backslash at end of input");
                    else if (*c == '#')
                        throw ConfigFileError(f, "Line continuation followed by comment");
                }
                else
                    result.append(stringify(*c++));
            }
            else if (*c == '$')
                result.append(grab_dollar(++c, c_end, d, f));
            else if (*c == '\n')
            {
                ++c;
                break;
            }
            else if (*c == '#')
            {
                ++c;
                break;
            }
            else
                result.append(stringify(*c++));
        }

        return result;
    }
}

KeyValueConfigFile::KeyValueConfigFile(const Source & s, const Defaults & d,
        bool (* i) (const std::string &, const KeyValueConfigFile &)) :
    ConfigFile(s),
    PrivateImplementationPattern<KeyValueConfigFile>(new Implementation<KeyValueConfigFile>(d, i))
{
    Context context("When parsing key/value configuration file" + (s.filename().empty() ? ":" :
                "'" + s.filename() + "':"));

    std::copy(d.begin(), d.end(), std::inserter(_imp->keys, _imp->keys.begin()));

    if (! s.stream())
        throw ConfigFileError(s.filename(), "Cannot read input");

    std::istreambuf_iterator<char> c(s.stream()), c_end;

    while (c != c_end)
    {
        if (*c == '#')
            next_line(c, c_end);
        else if (*c == '\t' || *c == '\n' || *c == '\r' || *c == ' ')
            ++c;
        else
        {
            std::string key(grab_key(c, c_end));
            if (key.empty())
                throw ConfigFileError(s.filename(), "Syntax error: invalid identifier");

            if (c == c_end)
                throw ConfigFileError(s.filename(), "Syntax error: trailing token '" + key + "' at end of input");

            if (*c != '=')
            {
                while (*c == '\t' || *c == ' ')
                    if (++c == c_end)
                        throw ConfigFileError(s.filename(), "Unknown command or broken variable '" + key + "' at end of input");

                if (*c != '=')
                {
                    if (key == "source")
                        throw ConfigFileError(s.filename(), "'source' not yet supported");
                    else
                        throw ConfigFileError(s.filename(), "Unknown command or broken variable '" + key + "', trailing text '"
                                + std::string(c, c_end) + "'");
                }
            }
            if (++c == c_end)
                throw ConfigFileError(s.filename(), "= at end of input");

            while (*c == '\t' || *c == ' ')
                if (++c == c_end)
                    throw ConfigFileError(s.filename(), "= at end of input");

            std::string value(grab_value(c, c_end, *this, s.filename()));

            if (_imp->is_incremental && (*_imp->is_incremental)(key, *this))
            {
                std::list<std::string> values;
                std::set<std::string> new_values;
                WhitespaceTokeniser::get_instance()->tokenise(get(key), std::back_inserter(values));
                WhitespaceTokeniser::get_instance()->tokenise(value, std::back_inserter(values));
                for (std::list<std::string>::const_iterator v(values.begin()), v_end(values.end()) ;
                        v != v_end ; ++v)
                    if (v->empty())
                        continue;
                    else if ("-*" == *v)
                        new_values.clear();
                    else if ('-' == v->at(0))
                        new_values.erase(v->substr(1));
                    else
                        new_values.insert(*v);

                _imp->keys.erase(key);
                _imp->keys.insert(std::make_pair(key, join(new_values.begin(), new_values.end(), " ")));
            }
            else
            {
                _imp->keys.erase(key);
                _imp->keys.insert(std::make_pair(key, value));
            }
        }
    }
}

KeyValueConfigFile::~KeyValueConfigFile()
{
}

KeyValueConfigFile::Iterator
KeyValueConfigFile::begin() const
{
    return Iterator(_imp->keys.begin());
}

KeyValueConfigFile::Iterator
KeyValueConfigFile::end() const
{
    return Iterator(_imp->keys.end());
}

std::string
KeyValueConfigFile::get(const std::string & s) const
{
    std::map<std::string, std::string>::const_iterator i(_imp->keys.find(s));
    if (_imp->keys.end() == i)
        return _imp->defaults.get(s);
    else
        return i->second;
}

