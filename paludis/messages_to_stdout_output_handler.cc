/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include <paludis/messages_to_stdout_output_handler.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/discard_output_stream.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/map.hh>
#include <paludis/util/stringify.hh>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<MessagesToStdoutOutputManager>
    {
        DiscardOutputStream output_stream;
        const std::tr1::shared_ptr<OutputManager> child;
        const OutputManagerFactory::ReplaceVarsFunc replace_vars_func;
        const std::string f_debug;
        const std::string f_info;
        const std::string f_warn;
        const std::string f_error;
        const std::string f_log;

        Implementation(
                const std::tr1::shared_ptr<OutputManager> & c,
                const OutputManagerFactory::ReplaceVarsFunc & r,
                const std::string & f_d,
                const std::string & f_i,
                const std::string & f_w,
                const std::string & f_e,
                const std::string & f_l) :
            child(c),
            replace_vars_func(r),
            f_debug(f_d),
            f_info(f_i),
            f_warn(f_w),
            f_error(f_e),
            f_log(f_l)
        {
        }
    };
}

MessagesToStdoutOutputManager::MessagesToStdoutOutputManager(
        const std::tr1::shared_ptr<OutputManager> & s,
        const OutputManagerFactory::ReplaceVarsFunc & replace_vars_func,
        const std::string & f_debug,
        const std::string & f_info,
        const std::string & f_warn,
        const std::string & f_error,
        const std::string & f_log) :
    PrivateImplementationPattern<MessagesToStdoutOutputManager>(new Implementation<MessagesToStdoutOutputManager>(s, replace_vars_func,
                f_debug, f_info, f_warn, f_error, f_log))
{
}

MessagesToStdoutOutputManager::~MessagesToStdoutOutputManager()
{
}

std::ostream &
MessagesToStdoutOutputManager::stdout_stream()
{
    return _imp->output_stream;
}

std::ostream &
MessagesToStdoutOutputManager::stderr_stream()
{
    return _imp->output_stream;
}

void
MessagesToStdoutOutputManager::succeeded()
{
    _imp->child->succeeded();
}

void
MessagesToStdoutOutputManager::message(const MessageType t, const std::string & s)
{
    std::string msg;
    std::tr1::shared_ptr<Map<std::string, std::string> > x(new Map<std::string, std::string>);
    x->insert("message", s);

    do
    {
        switch (t)
        {
            case mt_debug:
                msg = _imp->replace_vars_func(_imp->f_debug, x);
                continue;

            case mt_info:
                msg = _imp->replace_vars_func(_imp->f_info, x);
                continue;

            case mt_warn:
                msg = _imp->replace_vars_func(_imp->f_warn, x);
                continue;

            case mt_error:
                msg = _imp->replace_vars_func(_imp->f_error, x);
                continue;

            case mt_log:
                msg = _imp->replace_vars_func(_imp->f_log, x);
                continue;

            case last_mt:
                break;
        }

        throw InternalError(PALUDIS_HERE, "bad MessageType");
    }
    while (false);

    if (! msg.empty())
        _imp->child->stdout_stream() << msg << std::endl;
}

const std::tr1::shared_ptr<const Set<std::string> >
MessagesToStdoutOutputManager::factory_managers()
{
    std::tr1::shared_ptr<Set<std::string> > result(new Set<std::string>);
    result->insert("messages_to_stdout");
    return result;
}

const std::tr1::shared_ptr<OutputManager>
MessagesToStdoutOutputManager::factory_create(
        const OutputManagerFactory::KeyFunction & key_func,
        const OutputManagerFactory::CreateChildFunction & create_child,
        const OutputManagerFactory::ReplaceVarsFunc & replace_vars_func)
{
    std::string child(key_func("child"));
    if (child.empty())
        throw ConfigurationError("No child specified for MessagesToStdoutOutputManager");

    std::string f_d(key_func("format_debug")), f_i(key_func("format_info")), f_w(key_func("format_warn")),
        f_e(key_func("format_error")), f_l(key_func("format_log"));

    return make_shared_ptr(new MessagesToStdoutOutputManager(create_child(child), replace_vars_func,
                f_d, f_i, f_w, f_e, f_l));
}

template class PrivateImplementationPattern<MessagesToStdoutOutputManager>;

