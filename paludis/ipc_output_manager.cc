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

#include <paludis/ipc_output_manager.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/set.hh>
#include <paludis/util/map.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/log.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/join.hh>
#include <paludis/util/thread.hh>
#include <paludis/util/pipe.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/options.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/create_output_manager_info.hh>
#include <paludis/serialise.hh>
#include <paludis/environment.hh>
#include <functional>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<IPCOutputManager>
    {
        std::shared_ptr<SafeOFStream> stdout_stream;
        std::shared_ptr<SafeOFStream> stderr_stream;

        std::shared_ptr<SafeIFStream> pipe_command_read_stream;
        std::shared_ptr<SafeOFStream> pipe_command_write_stream;

        Implementation(int r, int w) :
            pipe_command_write_stream(new SafeOFStream(w))
        {
            *pipe_command_write_stream << "PING 1 GOAT" << '\0' << std::flush;

            pipe_command_read_stream.reset(new SafeIFStream(r));
            std::string response;
            if (! std::getline(*pipe_command_read_stream, response, '\0'))
                throw InternalError(PALUDIS_HERE, "couldn't get a pipe command response");
            if (response != "OPONG GOAT")
                throw InternalError(PALUDIS_HERE, "got response '" + response + "'");
        }
    };
}

IPCOutputManager::IPCOutputManager(const int r, const int w, const CreateOutputManagerInfo & i) :
    PrivateImplementationPattern<IPCOutputManager>(new Implementation<IPCOutputManager>(r, w))
{
    std::stringstream ser_stream;
    Serialiser ser(ser_stream);
    i.serialise(ser);
    *_imp->pipe_command_write_stream << "CREATE 1 " << ser_stream.str() << '\0' << std::flush;

    std::string response;
    if (! std::getline(*_imp->pipe_command_read_stream, response, '\0'))
        throw InternalError(PALUDIS_HERE, "couldn't get a pipe command response");

    std::vector<std::string> tokens;
    tokenise_whitespace(response, std::back_inserter(tokens));

    if (tokens.size() != 4 || tokens[0] != "O" || tokens[1] != "1")
        throw InternalError(PALUDIS_HERE, "got response '" + response + "'");

    int stdout_fd(destringify<int>(tokens[2])), stderr_fd(destringify<int>(tokens[3]));
    _imp->stdout_stream.reset(new SafeOFStream(stdout_fd));
    _imp->stderr_stream.reset(new SafeOFStream(stderr_fd));

    if (0 != ::fcntl(stdout_fd, F_SETFD, FD_CLOEXEC))
        throw InternalError(PALUDIS_HERE, "fcntl failed");
    if (0 != ::fcntl(stderr_fd, F_SETFD, FD_CLOEXEC))
        throw InternalError(PALUDIS_HERE, "fcntl failed");
}

IPCOutputManager::~IPCOutputManager()
{
    *_imp->pipe_command_write_stream << "FINISHED 1" << '\0' << std::flush;

    std::string response;
    if (! std::getline(*_imp->pipe_command_read_stream, response, '\0'))
        throw InternalError(PALUDIS_HERE, "couldn't get a pipe command response");
    if (response != "O")
        throw InternalError(PALUDIS_HERE, "got response '" + response + "'");
}

std::ostream &
IPCOutputManager::stdout_stream()
{
    return *_imp->stdout_stream;
}

std::ostream &
IPCOutputManager::stderr_stream()
{
    return *_imp->stderr_stream;
}

void
IPCOutputManager::message(const MessageType t, const std::string & s)
{
    *_imp->pipe_command_write_stream << "MESSAGE 1 " << t << " " << s << '\0' << std::flush;

    std::string response;
    if (! std::getline(*_imp->pipe_command_read_stream, response, '\0'))
        throw InternalError(PALUDIS_HERE, "couldn't get a pipe command response");
    if (response != "O")
        throw InternalError(PALUDIS_HERE, "got response '" + response + "'");
}

void
IPCOutputManager::succeeded()
{
    *_imp->pipe_command_write_stream << "SUCCEEDED 1" << '\0' << std::flush;

    std::string response;
    if (! std::getline(*_imp->pipe_command_read_stream, response, '\0'))
        throw InternalError(PALUDIS_HERE, "couldn't get a pipe command response");
    if (response != "O")
        throw InternalError(PALUDIS_HERE, "got response '" + response + "'");
}

void
IPCOutputManager::flush()
{
}

bool
IPCOutputManager::want_to_flush() const
{
    return false;
}

void
IPCOutputManager::nothing_more_to_come()
{
    *_imp->pipe_command_write_stream << "NOTHING_MORE_TO_COME 1" << '\0' << std::flush;

    std::string response;
    if (! std::getline(*_imp->pipe_command_read_stream, response, '\0'))
        throw InternalError(PALUDIS_HERE, "couldn't get a pipe command response");
    if (response != "O")
        throw InternalError(PALUDIS_HERE, "got response '" + response + "'");
}

namespace paludis
{
    template <>
    struct Implementation<IPCInputManager>
    {
        const Environment * const env;
        const std::function<void (const std::shared_ptr<OutputManager> &)> on_create;

        mutable Mutex mutex;
        std::shared_ptr<OutputManager> output_manager;

        Pipe stdout_pipe, stderr_pipe, finished_pipe;

        std::shared_ptr<Thread> copy_thread;

        Implementation(const Environment * const e,
                const std::function<void (const std::shared_ptr<OutputManager> &)> & c) :
            env(e),
            on_create(c)
        {
            if (0 != ::fcntl(finished_pipe.read_fd(), F_SETFD, FD_CLOEXEC))
                throw InternalError(PALUDIS_HERE, "fcntl failed");
            if (0 != ::fcntl(finished_pipe.write_fd(), F_SETFD, FD_CLOEXEC))
                throw InternalError(PALUDIS_HERE, "fcntl failed");
            if (0 != ::fcntl(stdout_pipe.read_fd(), F_SETFD, FD_CLOEXEC))
                throw InternalError(PALUDIS_HERE, "fcntl failed");
            if (0 != ::fcntl(stderr_pipe.read_fd(), F_SETFD, FD_CLOEXEC))
                throw InternalError(PALUDIS_HERE, "fcntl failed");
        }
    };
}

IPCInputManager::IPCInputManager(const Environment * const e,
        const std::function<void (const std::shared_ptr<OutputManager> &)> & c) :
    PrivateImplementationPattern<IPCInputManager>(new Implementation<IPCInputManager>(e, c))
{
}

IPCInputManager::~IPCInputManager()
{
    char c('x');
    if (1 != write(_imp->finished_pipe.write_fd(), &c, 1))
        throw InternalError(PALUDIS_HERE, "write failed");
}

const std::function<std::string (const std::string &)>
IPCInputManager::pipe_command_handler()
{
    return std::bind(&IPCInputManager::_pipe_command_handler, this, std::placeholders::_1);
}

std::string
IPCInputManager::_pipe_command_handler(const std::string & s)
{
    std::vector<std::string> tokens;
    tokenise_whitespace(s, std::back_inserter(tokens));

    if (tokens.empty())
        throw InternalError(PALUDIS_HERE, "tokens is empty");

    if (tokens[0] == "PING")
    {
        if (tokens.size() != 3 || tokens[1] != "1")
            return "Ebad PING subcommand";
        return "OPONG " + tokens[2];
    }
    else if (tokens[0] == "CREATE")
    {
        if (tokens.size() != 3 || tokens[1] != "1")
            return "Ebad CREATE subcommand";

        std::stringstream stream(tokens[2]);
        Deserialiser deserialiser(_imp->env, stream);
        Deserialisation deserialisation("CreateOutputManagerInfo", deserialiser);
        const std::shared_ptr<CreateOutputManagerInfo> i(CreateOutputManagerInfo::deserialise(deserialisation));

        {
            Lock lock(_imp->mutex);
            if (_imp->output_manager)
                return "Ealready constructed";

            _imp->output_manager = _imp->env->create_output_manager(*i);
            if (_imp->on_create)
                _imp->on_create(_imp->output_manager);
        }

        _imp->copy_thread.reset(new Thread(std::bind(&IPCInputManager::_copy_thread, this)));

        return "O 1 " + stringify(_imp->stdout_pipe.write_fd()) + " " + stringify(_imp->stderr_pipe.write_fd());
    }
    else if (tokens[0] == "MESSAGE")
    {
        if (tokens.size() < 3 || tokens[1] != "1")
            return "Ebad MESSAGE subcommand";

        MessageType t(destringify<MessageType>(tokens[2]));
        std::string m(join(next(tokens.begin(), 3), tokens.end(), " "));
        _imp->output_manager->message(t, m);

        return "O";
    }
    else if (tokens[0] == "SUCCEEDED")
    {
        if (tokens.size() != 2 || tokens[1] != "1")
            return "Ebad SUCCEEDED subcommand";
        _imp->output_manager->succeeded();

        return "O";
    }
    else if (tokens[0] == "NOTHING_MORE_TO_COME")
    {
        if (tokens.size() != 2 || tokens[1] != "1")
            return "Ebad NOTHING_MORE_TO_COME subcommand";
        _imp->output_manager->nothing_more_to_come();

        return "O";
    }
    else if (tokens[0] == "FINISHED")
    {
        if (tokens.size() != 2 || tokens[1] != "1")
            return "Ebad FINISHED subcommand";

        char c('x');
        if (1 != write(_imp->finished_pipe.write_fd(), &c, 1))
            throw InternalError(PALUDIS_HERE, "write failed");

        return "O";
    }
    else
        return "Eunknown pipe command";
}

void
IPCInputManager::_copy_thread()
{
    bool done(false);
    while (! done)
    {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        int max_fd(0);

        FD_SET(_imp->stdout_pipe.read_fd(), &read_fds);
        max_fd = std::max(max_fd, _imp->stdout_pipe.read_fd());

        FD_SET(_imp->stderr_pipe.read_fd(), &read_fds);
        max_fd = std::max(max_fd, _imp->stderr_pipe.read_fd());

        FD_SET(_imp->finished_pipe.read_fd(), &read_fds);
        max_fd = std::max(max_fd, _imp->finished_pipe.read_fd());

        int retval(pselect(max_fd + 1, &read_fds, 0, 0, 0, 0));
        if (-1 == retval)
            throw InternalError(PALUDIS_HERE, "pselect failed: " + stringify(strerror(errno)));

        bool got_output(false);
        char buf[1024];

        if (FD_ISSET(_imp->stdout_pipe.read_fd(), &read_fds))
        {
            got_output = true;
            int r;
            if (((r = read(_imp->stdout_pipe.read_fd(), buf, 1024))) > 0)
                _imp->output_manager->stdout_stream() <<  std::string(buf, r);
        }

        if (FD_ISSET(_imp->stderr_pipe.read_fd(), &read_fds))
        {
            got_output = true;
            int r;
            if (((r = read(_imp->stderr_pipe.read_fd(), buf, 1024))) > 0)
                _imp->output_manager->stderr_stream() <<  std::string(buf, r);
        }

        if (got_output)
            continue;

        if (FD_ISSET(_imp->finished_pipe.read_fd(), &read_fds))
        {
            int r;
            if (((r = read(_imp->finished_pipe.read_fd(), buf, 1024))) > 0)
                done = true;
        }
    }
}

const std::shared_ptr<OutputManager>
IPCInputManager::underlying_output_manager_if_constructed() const
{
    Lock lock(_imp->mutex);
    return _imp->output_manager;
}

namespace paludis
{
    template <>
    struct Implementation<OutputManagerFromIPC>
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> id;
        const OutputExclusivity exclusivity;
        const ClientOutputFeatures client_output_features;

        int read_fd, write_fd;

        std::shared_ptr<OutputManager> result;

        Implementation(const Environment * const e, const std::shared_ptr<const PackageID> & i,
                const OutputExclusivity x, const ClientOutputFeatures & c) :
            env(e),
            id(i),
            exclusivity(x),
            client_output_features(c),
            read_fd(destringify<int>(getenv_with_default("PALUDIS_IPC_READ_FD", "-1"))),
            write_fd(destringify<int>(getenv_with_default("PALUDIS_IPC_WRITE_FD", "-1")))
        {
            if (-1 == read_fd || -1 == write_fd)
                throw InternalError(PALUDIS_HERE, "no pipe command handler available");
            unsetenv("PALUDIS_IPC_READ_FD");
            unsetenv("PALUDIS_IPC_WRITE_FD");

            if (0 != ::fcntl(read_fd, F_SETFD, FD_CLOEXEC))
                throw InternalError(PALUDIS_HERE, "fcntl failed");
            if (0 != ::fcntl(write_fd, F_SETFD, FD_CLOEXEC))
                throw InternalError(PALUDIS_HERE, "fcntl failed");
        }
    };
}

OutputManagerFromIPC::OutputManagerFromIPC(const Environment * const e,
        const std::shared_ptr<const PackageID> & i,
        const OutputExclusivity x,
        const ClientOutputFeatures & c) :
    PrivateImplementationPattern<OutputManagerFromIPC>(new Implementation<OutputManagerFromIPC>(e, i, x, c))
{
}

OutputManagerFromIPC::~OutputManagerFromIPC()
{
}

const std::shared_ptr<OutputManager>
OutputManagerFromIPC::operator() (const Action & a)
{
    if (! _imp->result)
    {
        CreateOutputManagerForPackageIDActionInfo info(_imp->id, a, _imp->exclusivity, _imp->client_output_features);
        _imp->result.reset(new IPCOutputManager(_imp->read_fd, _imp->write_fd, info));
    }
    return _imp->result;
}

const std::shared_ptr<OutputManager>
OutputManagerFromIPC::output_manager_if_constructed()
{
    return _imp->result;
}

void
OutputManagerFromIPC::construct_standard_if_unconstructed()
{
    if (! _imp->result)
    {
        Log::get_instance()->message("output_manager_from_ipc.constructed_standard", ll_warning, lc_context)
            << "No output manager available, creating a standard output manager. This is probably a bug.";
        _imp->result.reset(new StandardOutputManager);
    }
}

template class PrivateImplementationPattern<IPCOutputManager>;
template class PrivateImplementationPattern<IPCInputManager>;
template class PrivateImplementationPattern<OutputManagerFromIPC>;

