/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2006 Danny van Dyk
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

#include <paludis/util/config_file.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/util/simple_parser.hh>
#include <paludis/util/log.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/system.hh>

#include <istream>
#include <list>
#include <map>

using namespace paludis;

#include <paludis/util/config_file-se.cc>

typedef std::list<std::string> LineConfigFileLines;
typedef std::map<std::string, std::string> KeyValueConfigFileValues;

namespace paludis
{
    template <>
    struct WrappedForwardIteratorTraits<LineConfigFile::ConstIteratorTag>
    {
        typedef LineConfigFileLines::const_iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<KeyValueConfigFile::ConstIteratorTag>
    {
        typedef KeyValueConfigFileValues::const_iterator UnderlyingIterator;
    };
}

ConfigFileError::ConfigFileError(const std::string & filename, const std::string & m) noexcept :
    ConfigurationError(filename.empty() ? m : "In file '" + filename + "': " + m)
{
}

ConfigFileError::ConfigFileError(const std::string & m) noexcept :
    ConfigurationError(m)
{
}

namespace paludis
{
    template <>
    struct Imp<ConfigFile::Source>
    {
        std::string filename;
        std::shared_ptr<const std::string> text;

        Imp(const FSPath & f) :
            filename(stringify(f))
        {
            try
            {
                SafeIFStream ff(f);
                std::shared_ptr<std::string> t(std::make_shared<std::string>());
                std::copy((std::istreambuf_iterator<char>(ff)), std::istreambuf_iterator<char>(), std::back_inserter(*t));
                text = t;
            }
            catch (const SafeIFStreamError & e)
            {
                throw ConfigFileError(filename, "Error reading file: '" + e.message() + "' (" + e.what() + ")");
            }
        }

        Imp(const std::string & s) :
            text(std::make_shared<std::string>(s))
        {
        }

        Imp(std::istream & ff)
        {
            std::shared_ptr<std::string> t(std::make_shared<std::string>());
            std::copy((std::istreambuf_iterator<char>(ff)), std::istreambuf_iterator<char>(), std::back_inserter(*t));
            text = t;
        }

        Imp(const std::string & f, const std::shared_ptr<const std::string> & t) :
            filename(f),
            text(t)
        {
        }
    };
}

ConfigFile::Source::Source(const FSPath & f) :
    _imp(f)
{
}

ConfigFile::Source::Source(const std::string & f) :
    _imp(f)
{
}

ConfigFile::Source::Source(std::istream & f) :
    _imp(f)
{
}

ConfigFile::Source::Source(const ConfigFile::Source & f) :
    _imp(f._imp->filename, f._imp->text)
{
}

const ConfigFile::Source &
ConfigFile::Source::operator= (const ConfigFile::Source & other)
{
    if (this != &other)
    {
        _imp->filename = other._imp->filename;
        _imp->text = other._imp->text;
    }
    return *this;
}

ConfigFile::Source::~Source() = default;

const std::string &
ConfigFile::Source::text() const
{
    return *_imp->text;
}

const std::string &
ConfigFile::Source::filename() const
{
    return _imp->filename;
}

ConfigFile::~ConfigFile() = default;

namespace paludis
{
    template <>
    struct Imp<LineConfigFile>
    {
        const LineConfigFileOptions options;
        LineConfigFileLines lines;

        Imp(const LineConfigFileOptions & o) :
            options(o)
        {
        }
    };
}

namespace
{
    void parse_after_continuation(const ConfigFile::Source & sr, SimpleParser & parser, const bool recognise_comments)
    {
        if (parser.eof())
            throw ConfigFileError(sr.filename(), "EOF after continuation near line " + stringify(parser.current_line_number()));
        else if (recognise_comments && parser.lookahead(*simple_parser::any_of(" \t") & simple_parser::exact("#")))
            throw ConfigFileError(sr.filename(),
                    "Comment not allowed immediately after after continuation near line " + stringify(parser.current_line_number()));
    }
}

LineConfigFile::LineConfigFile(const Source & sr, const LineConfigFileOptions & o) :
    _imp(o)
{
    Context context("When parsing line-based configuration file '" + (sr.filename().empty() ? "?" : sr.filename()) + "':");

    SimpleParser parser(sr.text());
    while (! parser.eof())
    {
        /* is it a comment? */
        if (! _imp->options[lcfo_disallow_comments])
        {
            if (parser.consume(*simple_parser::any_of(" \t") & simple_parser::exact("#") &
                        *simple_parser::any_except("\n")))
            {
                /* expect newline, but handle eof without final newline */
                if (! parser.consume(simple_parser::exact("\n")))
                {
                    if (parser.eof())
                    {
                        Log::get_instance()->message("line_config_file.no_trailing_newline", ll_debug, lc_context)
                            << "No newline at end of file";
                        break;
                    }
                    else
                        throw ConfigFileError(sr.filename(),
                                "Something is very strange at line '" + stringify(parser.current_line_number()) + "'");
                }
                continue;
            }
        }

        if (! _imp->options[lcfo_preserve_whitespace])
            if (! parser.consume(*simple_parser::any_of(" \t")))
                throw InternalError(PALUDIS_HERE, "failed to consume a zero width match");

        if (parser.eof())
        {
            Log::get_instance()->message("line_config_file.no_trailing_newline", ll_debug, lc_context)
                << "No newline at end of file";
            break;
        }

        /* is it a blank line? */
        if (! _imp->options[lcfo_no_skip_blank_lines])
        {
            if (parser.consume(simple_parser::exact("\n")))
                continue;
        }

        /* normal line, or lines with continuation */
        std::string line;
        std::string word;
        std::string space;
        bool need_single_space_unless_eol(false);
        while (true)
        {
            if (parser.eof())
            {
                Log::get_instance()->message("line_config_file.no_trailing_newline", ll_debug, lc_context)
                    << "No newline at end of file";
                break;
            }
            else if (parser.consume(+simple_parser::any_of(" \t") >> space))
            {
                if (_imp->options[lcfo_preserve_whitespace])
                    line.append(space);
                else if (! line.empty())
                    need_single_space_unless_eol = true;
            }
            else if (parser.consume(simple_parser::exact("\n") >> space))
                break;
            else if ((! _imp->options[lcfo_disallow_continuations]) && parser.consume(simple_parser::exact("\\\n")))
            {
                parse_after_continuation(sr, parser, ! _imp->options[lcfo_disallow_comments]);
            }
            else if (parser.consume(simple_parser::exact("\\") >> word))
            {
                if (need_single_space_unless_eol)
                {
                    need_single_space_unless_eol = false;
                    line.append(" ");
                }
                line.append(word);
            }
            else if ((! line.empty()) && (_imp->options[lcfo_allow_inline_comments]) && parser.consume(simple_parser::exact("#") &
                        *simple_parser::any_except("\n")))
            {
                if (! parser.consume(simple_parser::exact("\n")))
                    if (! parser.eof())
                        throw ConfigFileError(sr.filename(),
                                "Something is very strange at line '" + stringify(parser.current_line_number()) + "'");
                break;
            }
            else if (parser.consume(simple_parser::exact("#") >> word))
            {
                if (need_single_space_unless_eol)
                {
                    need_single_space_unless_eol = false;
                    line.append(" ");
                }
                line.append(word);
            }
            else if (parser.consume(+simple_parser::any_except(" \t\n\\#") >> word))
            {
                if (need_single_space_unless_eol)
                {
                    need_single_space_unless_eol = false;
                    line.append(" ");
                }
                line.append(word);
            }
            else
                throw ConfigFileError(sr.filename(), "Unparsable text in line " + stringify(parser.current_line_number()));
        }
        _imp->lines.push_back(line);
    }
}

LineConfigFile::~LineConfigFile() = default;

LineConfigFile::ConstIterator
LineConfigFile::begin() const
{
    return ConstIterator(_imp->lines.begin());
}

LineConfigFile::ConstIterator
LineConfigFile::end() const
{
    return ConstIterator(_imp->lines.end());
}

namespace paludis
{
    template <>
    struct Imp<KeyValueConfigFile>
    {
        const KeyValueConfigFileOptions options;
        const KeyValueConfigFile::DefaultFunction default_function;
        const KeyValueConfigFile::TransformationFunction transformation_function;

        KeyValueConfigFileValues values;

        std::string active_key_prefix;

        Imp(
                const KeyValueConfigFileOptions & o,
                const KeyValueConfigFile::DefaultFunction & d,
                const KeyValueConfigFile::TransformationFunction & i) :
            options(o),
            default_function(d),
            transformation_function(i)
        {
        }
    };
}

namespace
{
    bool parse_value(const KeyValueConfigFile & k, const ConfigFile::Source & sr, SimpleParser & parser, std::string & result)
        PALUDIS_ATTRIBUTE((warn_unused_result));
    bool parse_single_quoted_value(const KeyValueConfigFile & k, const ConfigFile::Source & sr, SimpleParser & parser, std::string & result)
        PALUDIS_ATTRIBUTE((warn_unused_result));
    bool parse_double_quoted_value(const KeyValueConfigFile & k, const ConfigFile::Source & sr, SimpleParser & parser, std::string & result)
        PALUDIS_ATTRIBUTE((warn_unused_result));
    bool parse_unquoted_value(const KeyValueConfigFile & k, const ConfigFile::Source & sr, SimpleParser & parser, std::string & result)
        PALUDIS_ATTRIBUTE((warn_unused_result));
    bool parse_variable(const KeyValueConfigFile & k, const ConfigFile::Source & sr, SimpleParser & parser, std::string & var, bool &)
        PALUDIS_ATTRIBUTE((warn_unused_result));

    bool parse_value(const KeyValueConfigFile & k, const ConfigFile::Source & sr, SimpleParser & parser, std::string & result)
    {
        if ((k.options()[kvcfo_allow_inline_comments] && parser.lookahead(simple_parser::exact("#"))))
            return true;

        if ((! k.options()[kvcfo_disallow_single_quoted_strings]) && parser.consume(simple_parser::exact("'")))
            return parse_single_quoted_value(k, sr, parser, result);

        if ((! k.options()[kvcfo_disallow_double_quoted_strings]) && parser.consume(simple_parser::exact("\"")))
            return parse_double_quoted_value(k, sr, parser, result);

        if (! k.options()[kvcfo_disallow_unquoted_values])
            return parse_unquoted_value(k, sr, parser, result);

        return false;
    }

    bool parse_single_quoted_value(const KeyValueConfigFile & k, const ConfigFile::Source & sr, SimpleParser & parser, std::string & result)
    {
        while (true)
        {
            if (parser.eof())
                throw ConfigFileError(sr.filename(), "Unterminated single quote at line " + stringify(parser.current_line_number()));

            std::string s;

            if ((! k.options()[kvcfo_disallow_continuations]) && parser.consume(simple_parser::exact("\\\n")))
            {
                parse_after_continuation(sr, parser, ! k.options()[kvcfo_disallow_comments]);
                continue;
            }
            else if ((! k.options()[kvcfo_ignore_single_quotes_inside_strings]) && parser.consume(simple_parser::exact("'")))
                break;
            else if ((k.options()[kvcfo_ignore_single_quotes_inside_strings]) && parser.lookahead(simple_parser::exact("'\n"))
                    && parser.consume(simple_parser::exact("'")))
                break;
            else if ((k.options()[kvcfo_ignore_single_quotes_inside_strings]) && parser.lookahead(simple_parser::exact("'"))
                    && ! parser.lookahead(simple_parser::exact("'") & simple_parser::any_except(""))
                    && parser.consume(simple_parser::exact("'")))
                break;
            else if (parser.consume((simple_parser::any_except("") & *simple_parser::any_except("\\'")) >> s))
                result.append(s);
            else
                throw ConfigFileError(sr.filename(), "Can't parse single quoted string at line " + stringify(parser.current_line_number()));
        }

        return true;
    }

    bool parse_double_quoted_value(const KeyValueConfigFile & k, const ConfigFile::Source & sr, SimpleParser & parser, std::string & result)
    {
        while (true)
        {
            if (parser.eof())
                throw ConfigFileError(sr.filename(), "Unterminated double quote at line " + stringify(parser.current_line_number()));

            std::string s;

            if ((! k.options()[kvcfo_disallow_continuations]) && parser.consume(simple_parser::exact("\\\n")))
            {
                parse_after_continuation(sr, parser, ! k.options()[kvcfo_disallow_comments]);
                continue;
            }
            else if (parser.consume(simple_parser::exact("\\t")))
                result.append("\t");
            else if (parser.consume(simple_parser::exact("\\n")))
                result.append("\n");
            else if (parser.consume(simple_parser::exact("\\e")))
                result.append("\033");
            else if (parser.consume(simple_parser::exact("\\a")))
                result.append("\007");
            else if (parser.consume(simple_parser::exact("\\") & simple_parser::any_except("") >> s))
                result.append(s);
            else if ((! k.options()[kvcfo_disallow_variables]) && parser.consume(simple_parser::exact("$")))
            {
                std::string var;
                bool is_env;
                if (! parse_variable(k, sr, parser, var, is_env))
                    throw ConfigFileError(sr.filename(), "Bad variable at line " + stringify(parser.current_line_number()));
                if (is_env)
                    result.append(getenv_with_default(var, ""));
                else
                    result.append(k.get(var));
            }
            else if (parser.consume(simple_parser::exact("\"")))
                break;
            else if (parser.consume((simple_parser::any_except("") & *simple_parser::any_except("\\\"$\t\n")) >> s))
                result.append(s);
            else
                throw ConfigFileError(sr.filename(), "Can't parse double quoted string at line " + stringify(parser.current_line_number()));
        }

        return true;
    }

    bool parse_variable(const KeyValueConfigFile & k, const ConfigFile::Source &, SimpleParser & parser, std::string & var, bool & is_env)
    {
        const std::string var_name_chars(
                "abcdefghijklmnopqrstuvwxyz"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "0123456789_"
                );

        if (k.options()[kvcfo_allow_env])
        {
            is_env = true;

            if (parser.consume(simple_parser::exact("{ENV{") & +simple_parser::any_of(var_name_chars) >> var & simple_parser::exact("}}")))
                return true;
            if (parser.consume(simple_parser::exact("ENV{") & +simple_parser::any_of(var_name_chars) >> var & simple_parser::exact("}")))
                return true;
        }

        is_env = false;

        if (parser.consume(simple_parser::exact("{") & +simple_parser::any_of(var_name_chars) >> var & simple_parser::exact("}")))
            return true;

        if (parser.consume(+simple_parser::any_of(var_name_chars) >> var))
            return true;

        return false;
    }

    bool parse_unquoted_value(const KeyValueConfigFile & k, const ConfigFile::Source & sr, SimpleParser & parser, std::string & result)
    {
        bool need_single_space_unless_eol(false);
        while (true)
        {
            std::string w;
            if (parser.eof() || parser.lookahead(simple_parser::exact("\n")))
                break;
            else if (parser.consume(+simple_parser::any_of(" \t") >> w))
            {
                if (k.options()[kvcfo_disallow_space_inside_unquoted_values])
                {
                    if (k.options()[kvcfo_allow_multiple_assigns_per_line])
                        break;
                    else
                        throw ConfigFileError(sr.filename(), "Not allowed space inside unquoted values at line "
                                + stringify(parser.current_line_number()));
                }
                else if (k.options()[kvcfo_preserve_whitespace])
                    result.append(w);
                else
                    need_single_space_unless_eol = true;
            }
            else if ((k.options()[kvcfo_allow_inline_comments]) && parser.consume(simple_parser::exact("#")))
            {
                if (! parser.consume(*simple_parser::any_except("\n")))
                    throw InternalError(PALUDIS_HERE, "failed to consume a zero width match");
                break;
            }
            else if ((! k.options()[kvcfo_disallow_variables]) && parser.consume(simple_parser::exact("$")))
            {
                if (need_single_space_unless_eol)
                {
                    result.append(" ");
                    need_single_space_unless_eol = false;
                }

                std::string var;
                bool is_env;
                if (! parse_variable(k, sr, parser, var, is_env))
                    throw ConfigFileError(sr.filename(), "Bad variable at line " + stringify(parser.current_line_number()));
                if (is_env)
                    result.append(getenv_with_default(var, ""));
                else
                    result.append(k.get(var));
            }
            else if ((! k.options()[kvcfo_disallow_continuations]) && parser.consume(simple_parser::exact("\\\n")))
            {
                parse_after_continuation(sr, parser, ! k.options()[kvcfo_disallow_comments]);
            }
            else if (parser.consume(simple_parser::exact("\\") & simple_parser::any_except("") >> w))
            {
                if (need_single_space_unless_eol)
                {
                    result.append(" ");
                    need_single_space_unless_eol = false;
                }

                if (w == "n")
                    result.append("\n");
                else if (w == "t")
                    result.append("\t");
                else if (w == "e")
                    result.append("\033");
                else if (w == "a")
                    result.append("\007");
                else
                    result.append(w);
            }
            else if (parser.consume((simple_parser::any_except("") & *simple_parser::any_except("\\\"$#\n\t ")) >> w))
            {
                if (need_single_space_unless_eol)
                {
                    result.append(" ");
                    need_single_space_unless_eol = false;
                }

                result.append(w);
            }
            else
                throw ConfigFileError(sr.filename(), "Can't parse unquoted string at line " + stringify(parser.current_line_number()));
        }

        return true;
    }

    std::string default_from_kv(const KeyValueConfigFile & f, const KeyValueConfigFile &, const std::string & s)
    {
        return f.get(s);
    }
}

KeyValueConfigFile::KeyValueConfigFile(
        const Source & sr,
        const KeyValueConfigFileOptions & o,
        const KeyValueConfigFile::DefaultFunction & f,
        const KeyValueConfigFile::TransformationFunction & i) :
    _imp(o, f, i)
{
    Context context("When parsing key=value-based configuration file '" + (sr.filename().empty() ? "?" : sr.filename()) + "':");

    SimpleParser parser(sr.text());
    while (! parser.eof())
    {
        /* is it a comment? */
        if (! _imp->options[kvcfo_disallow_comments])
        {
            if (parser.consume(*simple_parser::any_of(" \t") & simple_parser::exact("#") &
                        *simple_parser::any_except("\n")))
            {
                /* expect newline, but handle eof without final newline */
                if (! parser.consume(simple_parser::exact("\n")))
                {
                    if (parser.eof())
                    {
                        Log::get_instance()->message("key_value_config_file.no_trailing_newline", ll_debug, lc_context)
                            << "No newline at end of file";
                        break;
                    }
                    else
                        throw ConfigFileError(sr.filename(),
                                "Something is very strange at line '" + stringify(parser.current_line_number()) + "'");
                }
                continue;
            }
        }

        if (! parser.consume(*simple_parser::any_of(" \t")))
            throw InternalError(PALUDIS_HERE, "failed to consume a zero width match");

        if (parser.eof())
        {
            Log::get_instance()->message("key_value_config_file.no_trailing_newline", ll_debug, lc_context)
                << "No newline at end of file";
            break;
        }

        /* is it a blank line? */
        if (parser.consume(simple_parser::exact("\n")))
            continue;

        /* is it a comment? */
        if ((! _imp->options[kvcfo_disallow_comments]) && parser.consume(simple_parser::exact("#") &
                    *simple_parser::any_except("\n")))
        {
            if (! parser.consume(simple_parser::exact("\n")))
            {
                if (parser.eof())
                    Log::get_instance()->message("key_value_config_file.no_trailing_newline", ll_debug, lc_context)
                        << "No newline at end of file";
                else
                    throw ConfigFileError(sr.filename(),
                            "Something is very strange at line '" + stringify(parser.current_line_number()) + "'");
            }
            continue;
        }

        /* is it a source command? */
        if ((! _imp->options[kvcfo_disallow_source]) && parser.consume(simple_parser::exact("source") &
                    +simple_parser::any_of(" \t")))
        {
            std::string filename;
            if (! parse_value(*this, sr, parser, filename))
                throw ConfigFileError(sr.filename(), "Unparsable 'source' command in line " + stringify(parser.current_line_number()));

            if (filename.empty())
                throw ConfigFileError(sr.filename(), "Empty filename for 'source' command in line " + stringify(parser.current_line_number()));

            if (! parser.consume(*simple_parser::any_of(" \t")))
                throw InternalError(PALUDIS_HERE, "failed to consume a zero width match");

            if (_imp->options[kvcfo_allow_inline_comments] && parser.consume(simple_parser::exact("#") &
                        *simple_parser::any_except("\n")))
            {
                /* skippity skippity */
            }

            if (! parser.consume(simple_parser::exact("\n")))
            {
                if (parser.eof())
                    Log::get_instance()->message("key_value_config_file.no_trailing_newline", ll_debug, lc_context)
                        << "No newline at end of file";
                else
                    throw ConfigFileError(sr.filename(), "Expected newline after source command in line " + stringify(parser.current_line_number()));
            }

            Context local_context("When following 'source '" + filename + "' statement:");
            KeyValueConfigFile kv(FSPath(filename), o,
                    std::bind(&default_from_kv, std::cref(*this), std::placeholders::_1, std::placeholders::_2), i);
            for (KeyValueConfigFile::ConstIterator k(kv.begin()), k_end(kv.end()) ;
                    k != k_end ; ++k)
                _imp->values[k->first] = k->second;


            continue;
        }

        /* is it a section? */
        if (_imp->options[kvcfo_allow_sections] && parser.consume(simple_parser::exact("[")))
        {
            std::string sec_t;
            std::string sec_s;
            if (! parser.consume(+simple_parser::any_except(" \t\n$#\"'=\\]") >> sec_t))
                throw ConfigFileError(sr.filename(), "Expected section name on line " + stringify(parser.current_line_number()));

            if (! parser.consume(*simple_parser::any_of(" \t")))
                throw InternalError(PALUDIS_HERE, "failed to consume a zero width match");

            if (! parser.consume(simple_parser::exact("]")))
            {
                if (! parser.consume(+simple_parser::any_except(" \t\n$#\"\\]") >> sec_s))
                    throw ConfigFileError(sr.filename(), "Expected section name value on line "
                            + stringify(parser.current_line_number()));
                if (! parser.consume(*simple_parser::any_of(" \t")))
                    throw InternalError(PALUDIS_HERE, "failed to consume a zero width match");
                if (! parser.consume(simple_parser::exact("]")))
                    throw ConfigFileError(sr.filename(), "Expected ] on line "
                            + stringify(parser.current_line_number()));
            }

            if (! parser.consume(*simple_parser::any_of(" \t")))
                throw InternalError(PALUDIS_HERE, "failed to consume a zero width match");
            if (! parser.consume(*simple_parser::exact("\n")))
            {
                if (parser.eof())
                    Log::get_instance()->message("key_value_config_file.no_trailing_newline", ll_debug, lc_context)
                        << "No newline at end of file";
                else
                    throw ConfigFileError(sr.filename(), "Expected newline after ']' at line "
                            + stringify(parser.current_line_number()) + "'");
            }

            if (sec_s.empty())
                _imp->active_key_prefix = sec_t + "/";
            else
                _imp->active_key_prefix = sec_t + "/" + sec_s + "/";

            continue;
        }

        /* ignore export, if appropriate */
        if (_imp->options[kvcfo_ignore_export] && parser.consume(simple_parser::exact("export") &
                    +simple_parser::any_of(" \t")))
        {
        }

        /* is it superman? */
        std::string key;
        std::string value;

        if (! parser.consume(+simple_parser::any_except(" \t\n$#\"'=\\?") >> key))
            throw ConfigFileError(sr.filename(), "Couldn't find a key in line " + stringify(parser.current_line_number()));

        while (! parser.eof())
        {
            if (! _imp->options[kvcfo_disallow_space_around_equals])
                if (! parser.consume(*simple_parser::any_of(" \t")))
                    throw InternalError(PALUDIS_HERE, "failed to consume a zero width match");

            if ((! _imp->options[kvcfo_disallow_continuations]) && parser.consume(simple_parser::exact("\\\n")))
            {
                parse_after_continuation(sr, parser, ! _imp->options[kvcfo_disallow_comments]);
            }
            else
                break;
        }

        bool question_assign(false);
        if (parser.consume(simple_parser::exact("?=")))
            question_assign = true;
        else if (! parser.consume(simple_parser::exact("=")))
            throw ConfigFileError(sr.filename(), "Expected an = at line " + stringify(parser.current_line_number()));

        if (question_assign && ! _imp->options[kvcfo_allow_fancy_assigns])
            throw ConfigFileError(sr.filename(), "Not allowed to use ?= on line " + stringify(parser.current_line_number()));

        while (! parser.eof())
        {
            if (parser.consume(+simple_parser::any_of(" \t")))
                if (_imp->options[kvcfo_disallow_space_around_equals])
                    throw ConfigFileError(sr.filename(), "Space not allowed after = at line " + stringify(parser.current_line_number()));

            if ((! _imp->options[kvcfo_disallow_continuations]) && parser.consume(simple_parser::exact("\\\n")))
            {
                parse_after_continuation(sr, parser, ! _imp->options[kvcfo_disallow_comments]);
            }
            else
                break;
        }

        if (! parse_value(*this, sr, parser, value))
            throw ConfigFileError(sr.filename(), "Couldn't find a value at line " + stringify(parser.current_line_number()));

        while (! parser.eof())
        {
            std::string s;
            if (parser.consume(+simple_parser::any_of(" \t") >> s))
                if (_imp->options[kvcfo_preserve_whitespace])
                    value += s;

            if ((_imp->options[kvcfo_allow_inline_comments]) && parser.consume(
                        simple_parser::exact("#") & *simple_parser::any_except("\n")))
            {
                if (! parser.consume(simple_parser::exact("\n")))
                {
                    if (parser.eof())
                        Log::get_instance()->message("key_value_config_file.no_trailing_newline", ll_debug, lc_context)
                            << "No newline at end of file";
                    else
                        throw ConfigFileError(sr.filename(),
                                "Something is very strange at line '" + stringify(parser.current_line_number()) + "'");
                }
                break;
            }

            if ((! _imp->options[kvcfo_disallow_continuations]) && parser.consume(simple_parser::exact("\\\n")))
            {
                parse_after_continuation(sr, parser, ! _imp->options[kvcfo_disallow_comments]);
            }
            else
                break;
        }

        key = _imp->active_key_prefix + key;

        bool want(true);
        if (question_assign && ! get(key).empty())
            want = false;

        if (want)
        {
            std::string new_value(transformation_function()(*this, key, get(key), value));
            _imp->values[key] = new_value;
        }
    }

    _imp->active_key_prefix = "";
}

KeyValueConfigFile::~KeyValueConfigFile() = default;

KeyValueConfigFile::ConstIterator
KeyValueConfigFile::begin() const
{
    return ConstIterator(_imp->values.begin());
}

KeyValueConfigFile::ConstIterator
KeyValueConfigFile::end() const
{
    return ConstIterator(_imp->values.end());
}

std::string
KeyValueConfigFile::get(const std::string & s) const
{
    std::map<std::string, std::string>::const_iterator f(_imp->values.find(_imp->active_key_prefix + s));
    if (_imp->values.end() == f)
        f = _imp->values.find(s);

    if (_imp->values.end() == f)
        return _imp->default_function(*this, s);
    else
        return f->second;
}

std::string
KeyValueConfigFile::no_defaults(const KeyValueConfigFile &, const std::string &)
{
    return "";
}

std::string
KeyValueConfigFile::no_transformation(const KeyValueConfigFile &, const std::string &, const std::string &, const std::string & n)
{
    return n;
}

const KeyValueConfigFileOptions &
KeyValueConfigFile::options() const
{
    return _imp->options;
}

const KeyValueConfigFile::DefaultFunction &
KeyValueConfigFile::default_function() const
{
    return _imp->default_function;
}

const KeyValueConfigFile::TransformationFunction &
KeyValueConfigFile::transformation_function() const
{
    return _imp->transformation_function;
}

namespace paludis
{
    template class Pimp<ConfigFile::Source>;
    template class Pimp<LineConfigFile>;
    template class Pimp<KeyValueConfigFile>;

    template class WrappedForwardIterator<LineConfigFile::ConstIteratorTag, const std::string>;
    template class WrappedForwardIterator<KeyValueConfigFile::ConstIteratorTag, const std::pair<const std::string, std::string> >;
}
