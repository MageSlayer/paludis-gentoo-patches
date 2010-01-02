/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/format_messages_output_manager.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/discard_output_stream.hh>
#include <paludis/util/set.hh>
#include <paludis/util/map.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/stringify.hh>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<FormatMessagesOutputManager>
    {
        DiscardOutputStream stream;
        const std::tr1::shared_ptr<OutputManager> child;
        const std::string format_debug;
        const std::string format_info;
        const std::string format_warn;
        const std::string format_error;
        const std::string format_log;

        const FormatMessagesOutputManagerFormatFunction format_func;

        Implementation(
                const std::tr1::shared_ptr<OutputManager> & c,
                const std::string & d,
                const std::string & i,
                const std::string & w,
                const std::string & e,
                const std::string & l,
                const FormatMessagesOutputManagerFormatFunction & f
                ) :
            child(c),
            format_debug(d),
            format_info(i),
            format_warn(w),
            format_error(e),
            format_log(l),
            format_func(f)
        {
        }
    };
}

FormatMessagesOutputManager::FormatMessagesOutputManager(
        const std::tr1::shared_ptr<OutputManager> & child,
        const std::string & format_debug,
        const std::string & format_info,
        const std::string & format_warn,
        const std::string & format_error,
        const std::string & format_log,
        const FormatMessagesOutputManagerFormatFunction & f) :
    PrivateImplementationPattern<FormatMessagesOutputManager>(new Implementation<FormatMessagesOutputManager>(
                child, format_debug, format_info, format_warn, format_error, format_log, f))
{
}

FormatMessagesOutputManager::~FormatMessagesOutputManager()
{
}

std::ostream &
FormatMessagesOutputManager::stdout_stream()
{
    return _imp->stream;
}

std::ostream &
FormatMessagesOutputManager::stderr_stream()
{
    return _imp->stream;
}

void
FormatMessagesOutputManager::message(const MessageType t, const std::string & s)
{
    std::string f;
    switch (t)
    {
        case mt_info:
            f = _imp->format_info;
            break;
        case mt_debug:
            f = _imp->format_debug;
            break;
        case mt_warn:
            f = _imp->format_warn;
            break;
        case mt_error:
            f = _imp->format_error;
            break;
        case mt_log:
            f = _imp->format_log;
            break;

        default:
            break;
    }

    if (f.empty())
        return;

    f = _imp->format_func(f, s);
    _imp->child->stdout_stream() << f << std::flush;
}

void
FormatMessagesOutputManager::succeeded()
{
    _imp->child->succeeded();
}

void
FormatMessagesOutputManager::flush()
{
    _imp->child->flush();
}

void
FormatMessagesOutputManager::nothing_more_to_come()
{
    _imp->child->nothing_more_to_come();
}

const std::tr1::shared_ptr<const Set<std::string> >
FormatMessagesOutputManager::factory_managers()
{
    std::tr1::shared_ptr<Set<std::string> > result(new Set<std::string>);
    result->insert("format_messages");
    return result;
}

namespace
{
    std::string format_message(
            const OutputManagerFactory::ReplaceVarsFunc r,
            const std::string & f,
            const std::string & s)
    {
        std::tr1::shared_ptr<Map<std::string, std::string> > m(new Map<std::string, std::string>);
        m->insert("message", s);
        return r(f, m);
    }
}

const std::tr1::shared_ptr<OutputManager>
FormatMessagesOutputManager::factory_create(
        const OutputManagerFactory::KeyFunction & key_func,
        const OutputManagerFactory::CreateChildFunction & create_child_function,
        const OutputManagerFactory::ReplaceVarsFunc & replace_vars_func)
{
    std::string child_s(key_func("child")),
        format_debug_s(key_func("format_debug")),
        format_info_s(key_func("format_info")),
        format_warn_s(key_func("format_warn")),
        format_error_s(key_func("format_error")),
        format_log_s(key_func("format_log"));

    std::tr1::shared_ptr<OutputManager> child(create_child_function(child_s));

    FormatMessagesOutputManagerFormatFunction format_func(std::tr1::bind(
                &format_message, replace_vars_func, std::tr1::placeholders::_1, std::tr1::placeholders::_2));

    return make_shared_ptr(new FormatMessagesOutputManager(
                child, format_debug_s, format_info_s, format_warn_s, format_error_s, format_log_s, format_func));
}

template class PrivateImplementationPattern<FormatMessagesOutputManager>;

