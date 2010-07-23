/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/args/args_handler.hh>
#include <paludis/args/args_dumper.hh>
#include <paludis/args/args_visitor.hh>
#include <paludis/args/bad_argument.hh>
#include <paludis/util/system.hh>
#include <paludis/util/join.hh>
#include <paludis/util/accept_visitor.hh>

#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/sequence-impl.hh>

#include <memory>
#include <algorithm>
#include <sstream>
#include <list>
#include <map>

using namespace paludis;
using namespace paludis::args;

namespace paludis
{
    /**
     * Imp data for ArgsHandler.
     *
     * \ingroup grplibpaludisargs
     */
    template<>
    struct Imp<ArgsHandler>
    {
        std::list<ArgsSection *> sections;
        std::list<std::string> parameters;
        std::list<std::string> usage_lines;
        std::list<std::pair<std::string, std::string> > environment_lines;
        std::list<std::pair<std::string, std::string> > example_lines;
        std::list<std::string> notes;
        std::list<std::string> descriptions;
        std::list<std::pair<std::string, int> > see_alsos;
        std::shared_ptr<Sequence<std::string> > separate_after_dashes_args;

        std::map<std::string, ArgsOption *> longopts;
        std::map<char, ArgsOption *> shortopts;

        std::shared_ptr<ArgsSection> main_options_section;

        Imp() :
            separate_after_dashes_args(std::make_shared<Sequence<std::string>>())
        {
        }
    };

    template <>
    struct WrappedForwardIteratorTraits<ArgsHandler::ArgsSectionsConstIteratorTag>
    {
        typedef IndirectIterator<std::list<ArgsSection *>::const_iterator> UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<ArgsHandler::ParametersConstIteratorTag>
    {
        typedef std::list<std::string>::const_iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<ArgsHandler::NotesIteratorTag>
    {
        typedef std::list<std::string>::const_iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<ArgsHandler::DescriptionLineConstIteratorTag>
    {
        typedef std::list<std::string>::const_iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<ArgsHandler::ExamplesConstIteratorTag>
    {
        typedef std::list<std::pair<std::string, std::string> >::const_iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<ArgsHandler::EnvironmentLineConstIteratorTag>
    {
        typedef std::list<std::pair<std::string, std::string> >::const_iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<ArgsHandler::UsageLineConstIteratorTag>
    {
        typedef std::list<std::string>::const_iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<ArgsHandler::ArgsIteratorTag>
    {
        typedef std::list<std::string>::iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<ArgsHandler::SeeAlsoConstIteratorTag>
    {
        typedef std::list<std::pair<std::string, int> >::const_iterator UnderlyingIterator;
    };
}

ArgsHandler::ArgsHandler() :
    Pimp<ArgsHandler>()
{
}

ArgsHandler::~ArgsHandler()
{
}

void
ArgsHandler::add_usage_line(const std::string & l)
{
    _imp->usage_lines.push_back(l);
}

void
ArgsHandler::add_environment_variable(const std::string & e, const std::string & desc)
{
    _imp->environment_lines.push_back(std::make_pair(e, desc));
}

void
ArgsHandler::add_example(const std::string & e, const std::string & desc)
{
    _imp->example_lines.push_back(std::make_pair(e, desc));
}

void
ArgsHandler::add_note(const std::string & e)
{
    _imp->notes.push_back(e);
}

void
ArgsHandler::add_see_also(const std::string & e, const int s)
{
    _imp->see_alsos.push_back(std::make_pair(e, s));
}

void
ArgsHandler::add(ArgsSection * const s)
{
    _imp->sections.push_back(s);
}

void
ArgsHandler::run(
        const std::shared_ptr<const Sequence<std::string> > & argseq,
        const std::string & client,
        const std::string & env_var,
        const std::string & env_prefix,
        const ArgsHandlerOptions & options)
{
    std::list<std::string> args;
    std::string env_options;

    setenv("PALUDIS_CLIENT", client.c_str(), 1);

    if (! env_var.empty())
        env_options = paludis::getenv_with_default(env_var, "");

    std::istringstream iss(env_options);
    std::string option;
    while (iss.good())
    {
        iss >> option;
        if (!option.empty())
            args.push_back(option);
    }

    args.insert(args.end(), argseq->begin(), argseq->end());

    ArgsIterator argit(args.begin()), arge(args.end());

    for ( ; argit != arge; ++argit )
    {
        std::string arg = *argit;

        if (arg == "--")
        {
            ++argit;

            if (options[aho_separate_after_dashes])
                for ( ; argit != arge ; ++argit)
                    _imp->separate_after_dashes_args->push_back(*argit);

            break;
        }
        else if (0 == arg.compare(0, 2, "--"))
        {
            arg.erase(0, 2);
            std::map<std::string, ArgsOption *>::iterator it = _imp->longopts.find(arg);
            if (it == _imp->longopts.end())
            {
                if (0 != arg.compare(0, 3, "no-"))
                    throw BadArgument("--" + arg);
                arg.erase(0, 3);
                it = _imp->longopts.find(arg);
                if (it == _imp->longopts.end())
                    throw BadArgument("--no-" + arg);

                std::string remaining_chars;
                ArgsVisitor visitor(&argit, arge, env_prefix, remaining_chars, true);
                it->second->accept(visitor);
            }
            else
            {
                std::string remaining_chars;
                ArgsVisitor visitor(&argit, arge, env_prefix, remaining_chars, false);
                it->second->accept(visitor);
            }
        }
        else if (arg[0] == '-')
        {
            arg.erase(0, 1);
            for (std::string::iterator c = arg.begin(); c != arg.end(); ++c)
            {
                bool maybe_second_char_used(false);
                std::string remaining_chars;
                if (arg.length() >= 2 && c == arg.begin())
                {
                    maybe_second_char_used = true;
                    remaining_chars = arg.substr(1);
                }

                std::map<char, ArgsOption *>::iterator it = _imp->shortopts.find(*c);
                if (it == _imp->shortopts.end())
                {
                    throw BadArgument(std::string("-") + *c);
                }

                ArgsVisitor visitor(&argit, arge, env_prefix, remaining_chars, false);
                it->second->accept(visitor);

                if (maybe_second_char_used && remaining_chars.empty())
                    break;
            }
        }
        else
        {
            if (options[aho_stop_on_first_parameter])
                break;
            else
                _imp->parameters.push_back(arg);
        }
    }

    _imp->parameters.insert(_imp->parameters.end(), argit, ArgsIterator(args.end()));

    if (! env_prefix.empty())
        setenv((env_prefix + "_PARAMS").c_str(), join(_imp->parameters.begin(),
                    _imp->parameters.end(), " ").c_str(), 1);

    post_run();
}

void
ArgsHandler::run(
        const int argc,
        const char * const * const argv,
        const std::string & client,
        const std::string & env_var,
        const std::string & env_prefix,
        const ArgsHandlerOptions & options)
{
    std::shared_ptr<Sequence<std::string> > s(std::make_shared<Sequence<std::string>>());
    std::copy(&argv[1], &argv[argc], create_inserter<std::string>(s->back_inserter()));
    run(s, client, env_var, env_prefix, options);
}

void
ArgsHandler::post_run()
{
}

void
ArgsHandler::dump_to_stream(std::ostream & s) const
{
    ArgsDumper dump(s);
    for (ArgsSectionsConstIterator a(begin_args_sections()), a_end(end_args_sections()) ;
            a != a_end ; ++a)
    {
        for (ArgsSection::GroupsConstIterator g(a->begin()), g_end(a->end()) ;
                g != g_end ; ++g)
        {
            s << g->name() << ":" << std::endl;

            std::for_each(indirect_iterator(g->begin()), indirect_iterator(g->end()), accept_visitor(dump));

            s << std::endl;
        }
    }
}

ArgsHandler::ParametersConstIterator
ArgsHandler::begin_parameters() const
{
    return ParametersConstIterator(_imp->parameters.begin());
}

ArgsHandler::ParametersConstIterator
ArgsHandler::end_parameters() const
{
    return ParametersConstIterator(_imp->parameters.end());
}

bool
ArgsHandler::empty() const
{
    return _imp->parameters.empty();
}

std::ostream &
paludis::args::operator<< (std::ostream & s, const ArgsHandler & h)
{
    h.dump_to_stream(s);
    return s;
}

void
ArgsHandler::add_option(ArgsOption * const opt, const std::string & long_name,
        const char short_name)
{
    if (! _imp->longopts.insert(std::make_pair(long_name, opt)).second)
        throw InternalError(PALUDIS_HERE, "duplicate long name '" + long_name + "'");

    _imp->longopts[long_name] = opt;
    if (short_name != '\0')
        if (! _imp->shortopts.insert(std::make_pair(short_name, opt)).second)
            throw InternalError(PALUDIS_HERE, "duplicate short name '" + stringify(short_name) + "'");
}

void
ArgsHandler::remove_option(const std::string & long_name, const char short_name)
{
    _imp->longopts.erase(long_name);
    if (short_name != '\0')
        _imp->shortopts.erase(short_name);
}

ArgsHandler::UsageLineConstIterator
ArgsHandler::begin_usage_lines() const
{
    return UsageLineConstIterator(_imp->usage_lines.begin());
}

ArgsHandler::UsageLineConstIterator
ArgsHandler::end_usage_lines() const
{
    return UsageLineConstIterator(_imp->usage_lines.end());
}

ArgsHandler::EnvironmentLineConstIterator
ArgsHandler::begin_environment_lines() const
{
    return EnvironmentLineConstIterator(_imp->environment_lines.begin());
}

ArgsHandler::EnvironmentLineConstIterator
ArgsHandler::end_environment_lines() const
{
    return EnvironmentLineConstIterator(_imp->environment_lines.end());
}

ArgsHandler::ExamplesConstIterator
ArgsHandler::begin_examples() const
{
    return ExamplesConstIterator(_imp->example_lines.begin());
}

ArgsHandler::ExamplesConstIterator
ArgsHandler::end_examples() const
{
    return ExamplesConstIterator(_imp->example_lines.end());
}

ArgsHandler::NotesIterator
ArgsHandler::begin_notes() const
{
    return NotesIterator(_imp->notes.begin());
}

ArgsHandler::NotesIterator
ArgsHandler::end_notes() const
{
    return NotesIterator(_imp->notes.end());
}

ArgsHandler::SeeAlsoConstIterator
ArgsHandler::begin_see_alsos() const
{
    return SeeAlsoConstIterator(_imp->see_alsos.begin());
}

ArgsHandler::SeeAlsoConstIterator
ArgsHandler::end_see_alsos() const
{
    return SeeAlsoConstIterator(_imp->see_alsos.end());
}

ArgsHandler::ArgsSectionsConstIterator
ArgsHandler::begin_args_sections() const
{
    return ArgsSectionsConstIterator(indirect_iterator(_imp->sections.begin()));
}

ArgsHandler::ArgsSectionsConstIterator
ArgsHandler::end_args_sections() const
{
    return ArgsSectionsConstIterator(indirect_iterator(_imp->sections.end()));
}

ArgsSection *
ArgsHandler::main_options_section()
{
    if (! _imp->main_options_section)
        _imp->main_options_section = std::make_shared<ArgsSection>(this, "Options");
    return _imp->main_options_section.get();
}

ArgsHandler::DescriptionLineConstIterator
ArgsHandler::begin_description_lines() const
{
    return DescriptionLineConstIterator(_imp->descriptions.begin());
}

ArgsHandler::DescriptionLineConstIterator
ArgsHandler::end_description_lines() const
{
    return DescriptionLineConstIterator(_imp->descriptions.end());
}

void
ArgsHandler::add_description_line(const std::string & e)
{
    _imp->descriptions.push_back(e);
}

const std::shared_ptr<const Sequence<std::string> >
ArgsHandler::separate_after_dashes_args() const
{
    return _imp->separate_after_dashes_args;
}

template class WrappedForwardIterator<ArgsHandler::ParametersConstIteratorTag, const std::string>;
template class WrappedForwardIterator<ArgsHandler::UsageLineConstIteratorTag, const std::string>;
template class WrappedForwardIterator<ArgsHandler::EnvironmentLineConstIteratorTag,
         const std::pair<std::string, std::string> >;
template class WrappedForwardIterator<ArgsHandler::ExamplesConstIteratorTag,
         const std::pair<std::string, std::string> >;
template class WrappedForwardIterator<ArgsHandler::ArgsSectionsConstIteratorTag, const ArgsSection>;
template class WrappedForwardIterator<ArgsHandler::NotesIteratorTag, const std::string>;
template class WrappedForwardIterator<ArgsHandler::DescriptionLineConstIteratorTag, const std::string>;
template class WrappedForwardIterator<ArgsHandler::ArgsIteratorTag, std::string>;
template class WrappedForwardIterator<ArgsHandler::SeeAlsoConstIteratorTag, const std::pair<std::string, int> >;

