/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/command_output_manager.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/set.hh>
#include <paludis/util/map.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/process.hh>
#include <paludis/util/pipe.hh>

using namespace paludis;

CommandOutputManagerError::CommandOutputManagerError(const std::string & s) throw () :
    Exception(s)
{
}

namespace paludis
{
    template <>
    struct Imp<CommandOutputManager>
    {
        std::string start_command;
        std::string end_command;
        std::string stdout_command;
        std::string stderr_command;
        std::string succeeded_command;
        std::string nothing_more_to_come_command;

        std::unique_ptr<Process> stdout_process;
        std::unique_ptr<RunningProcessHandle> stdout_process_handle;
        std::unique_ptr<Pipe> stdout_pipe;
        std::unique_ptr<SafeOFStream> stdout_stream;

        std::unique_ptr<Process> stderr_process;
        std::unique_ptr<RunningProcessHandle> stderr_process_handle;
        std::unique_ptr<Pipe> stderr_pipe;
        std::unique_ptr<SafeOFStream> stderr_stream;

        bool already_nothing_more_to_come;
        bool ignore_succeeded;

        Imp(
                const std::string & s,
                const std::string & e,
                const std::string & so,
                const std::string & se,
                const std::string & su,
                const std::string & n
                ) :
            start_command(s),
            end_command(e),
            stdout_command(so),
            stderr_command(se),
            succeeded_command(su),
            nothing_more_to_come_command(n),
            already_nothing_more_to_come(false),
            ignore_succeeded(false)
        {
        }
    };
}

CommandOutputManager::CommandOutputManager(const std::string & s, const std::string & e,
        const std::string & so, const std::string & se, const std::string & su, const std::string & n) :
    _imp(s, e, so, se, su, n)
{
    if (! _imp->start_command.empty())
    {
        Process process(ProcessCommand(_imp->start_command));
        if (0 != process.run().wait())
            throw CommandOutputManagerError("start command returned non-zero");
    }

    _imp->stdout_pipe.reset(new Pipe(true));
    _imp->stdout_process.reset(new Process(ProcessCommand(_imp->stdout_command)));
    _imp->stdout_process->set_stdin_fd(_imp->stdout_pipe->read_fd());
    _imp->stdout_process_handle.reset(new RunningProcessHandle(_imp->stdout_process->run()));

    if (0 != ::close(_imp->stdout_pipe->read_fd()))
        throw CommandOutputManagerError("close stdout_pipe read_fd failed");
    _imp->stdout_pipe->clear_read_fd();
    _imp->stdout_stream.reset(new SafeOFStream(_imp->stdout_pipe->write_fd(), false));

    _imp->stderr_pipe.reset(new Pipe(true));
    _imp->stderr_process.reset(new Process(ProcessCommand(_imp->stderr_command)));
    _imp->stderr_process->set_stdin_fd(_imp->stderr_pipe->read_fd());
    _imp->stderr_process_handle.reset(new RunningProcessHandle(_imp->stderr_process->run()));

    if (0 != ::close(_imp->stderr_pipe->read_fd()))
        throw CommandOutputManagerError("close stderr_pipe read_fd failed");
    _imp->stderr_pipe->clear_read_fd();
    _imp->stderr_stream.reset(new SafeOFStream(_imp->stderr_pipe->write_fd(), false));
}

CommandOutputManager::~CommandOutputManager()
{
    nothing_more_to_come();

    if (_imp->stdout_process)
    {
        _imp->stdout_stream.reset();

        if (0 != ::close(_imp->stdout_pipe->write_fd()))
            throw CommandOutputManagerError("close stdout_pipe write_fd failed");
        _imp->stdout_pipe->clear_write_fd();

        if (0 != _imp->stdout_process_handle->wait())
            Log::get_instance()->message("command_output_manager.stdout_process.non_zero", ll_warning, lc_context)
                << "Command output manager stdout process returned non-zero";

        _imp->stdout_process_handle.reset();
        _imp->stdout_process.reset();
        _imp->stdout_stream.reset();
        _imp->stdout_pipe.reset();
    }

    if (_imp->stderr_process)
    {
        _imp->stderr_stream.reset();

        if (0 != ::close(_imp->stderr_pipe->write_fd()))
            throw CommandOutputManagerError("close stderr_pipe write_fd failed");
        _imp->stderr_pipe->clear_write_fd();

        if (0 != _imp->stderr_process_handle->wait())
            Log::get_instance()->message("command_output_manager.stderr_process.non_zero", ll_warning, lc_context)
                << "Command output manager stderr process returned non-zero";

        _imp->stderr_process_handle.reset();
        _imp->stderr_process.reset();
        _imp->stderr_stream.reset();
        _imp->stderr_pipe.reset();
    }

    if (! _imp->end_command.empty())
    {
        Process process(ProcessCommand(_imp->end_command));
        if (0 != process.run().wait())
            throw CommandOutputManagerError("end command returned non-zero");
    }
}

std::ostream &
CommandOutputManager::stdout_stream()
{
    return *_imp->stdout_stream;
}

std::ostream &
CommandOutputManager::stderr_stream()
{
    return *_imp->stderr_stream;
}

void
CommandOutputManager::message(const MessageType, const std::string &)
{
}

void
CommandOutputManager::succeeded()
{
    if ((! _imp->ignore_succeeded) && (! _imp->succeeded_command.empty()))
    {
        Process process(ProcessCommand(_imp->succeeded_command));
        if (0 != process.run().wait())
            throw CommandOutputManagerError("succeeded command returned non-zero");
    }
}

void
CommandOutputManager::ignore_succeeded()
{
    _imp->ignore_succeeded = true;
}

void
CommandOutputManager::flush()
{
}

bool
CommandOutputManager::want_to_flush() const
{
    return false;
}

void
CommandOutputManager::nothing_more_to_come()
{
    if (_imp->already_nothing_more_to_come)
        return;
    _imp->already_nothing_more_to_come = true;

    if (! _imp->nothing_more_to_come_command.empty())
    {
        stdout_stream() << std::flush;
        stderr_stream() << std::flush;

        Process process(ProcessCommand(_imp->nothing_more_to_come_command));
        if (0 != process.run().wait())
            throw CommandOutputManagerError("nothing more to come command returned non-zero");
    }
}

const std::shared_ptr<const Set<std::string> >
CommandOutputManager::factory_managers()
{
    std::shared_ptr<Set<std::string> > result(std::make_shared<Set<std::string>>());
    result->insert("command");
    return result;
}

const std::shared_ptr<OutputManager>
CommandOutputManager::factory_create(
        const OutputManagerFactory::KeyFunction & key_func,
        const OutputManagerFactory::CreateChildFunction &,
        const OutputManagerFactory::ReplaceVarsFunc & replace_vars_func)
{
    std::string start_command_s(key_func("start_command"));
    std::string end_command_s(key_func("end_command"));
    std::string stdout_command_s(key_func("stdout_command"));
    std::string stderr_command_s(key_func("stderr_command"));
    std::string succeeded_command_s(key_func("succeeded_command"));
    std::string nothing_more_to_come_command_s(key_func("nothing_more_to_come_command"));

    if (stdout_command_s.empty())
        throw ConfigurationError("Key 'stdout_command' not specified when creating a command output manager");
    stdout_command_s = replace_vars_func(stdout_command_s, std::make_shared<Map<std::string, std::string>>());

    if (stderr_command_s.empty())
        throw ConfigurationError("Key 'stderr_command' not specified when creating a command output manager");
    stderr_command_s = replace_vars_func(stderr_command_s, std::make_shared<Map<std::string, std::string>>());

    start_command_s = replace_vars_func(start_command_s, std::make_shared<Map<std::string, std::string>>());
    end_command_s = replace_vars_func(end_command_s, std::make_shared<Map<std::string, std::string>>());
    succeeded_command_s = replace_vars_func(succeeded_command_s, std::make_shared<Map<std::string, std::string>>());
    nothing_more_to_come_command_s = replace_vars_func(nothing_more_to_come_command_s, std::make_shared<Map<std::string, std::string>>());

    return std::make_shared<CommandOutputManager>(
            start_command_s, end_command_s, stdout_command_s, stderr_command_s,
            succeeded_command_s, nothing_more_to_come_command_s);
}

template class Pimp<CommandOutputManager>;

