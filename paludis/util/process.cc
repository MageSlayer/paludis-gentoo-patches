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

#include <paludis/util/process.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/thread.hh>
#include <paludis/util/pipe.hh>

#include <iostream>
#include <functional>
#include <algorithm>
#include <vector>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>

using namespace paludis;

ProcessError::ProcessError(const std::string & s) throw () :
    Exception(s)
{
}

namespace paludis
{
    template <>
    struct Imp<ProcessCommand>
    {
        std::vector<std::string> args;

        Imp(std::vector<std::string> && i) :
            args(i)
        {
        }
    };
}

ProcessCommand::ProcessCommand(const std::initializer_list<std::string> & i) :
    Pimp<ProcessCommand>(std::vector<std::string>(i))
{
}

ProcessCommand::ProcessCommand(ProcessCommand && other) :
    Pimp<ProcessCommand>(std::move(other._imp->args))
{
}

ProcessCommand::~ProcessCommand() = default;

void
ProcessCommand::exec()
{
    if (_imp->args.size() < 1)
        throw ProcessError("No command specified");

    /* no need to worry about free()ing this lot, since if our execvp fails we
     * call _exit() shortly afterwards */

    char ** argv(new char * [_imp->args.size() + 1]);
    argv[_imp->args.size()] = 0;
    for (auto v_begin(_imp->args.begin()), v(v_begin), v_end(_imp->args.end()) ;
            v != v_end ; ++v)
    {
        argv[v - v_begin] = new char [v->length() + 1];
        argv[v - v_begin][v->length()] = '\0';
        std::copy(v->begin(), v->end(), argv[v - v_begin]);
    }

    execvp(_imp->args[0].c_str(), argv);

    throw ProcessError("execvp failed");
}

namespace paludis
{
    struct RunningProcessThread
    {
        Pipe ctl_pipe;

        std::ostream * capture_stdout;
        std::unique_ptr<Pipe> capture_stdout_pipe;

        std::ostream * capture_stderr;
        std::unique_ptr<Pipe> capture_stderr_pipe;

        /* must be last, so the thread gets join()ed before its FDs vanish */
        std::unique_ptr<Thread> thread;

        RunningProcessThread() :
            ctl_pipe(true),
            capture_stdout(0),
            capture_stderr(0)
        {
        }

        void thread_func();

        void start();
    };
}

void
RunningProcessThread::thread_func()
{
    bool done(false);
    while (! done)
    {
        fd_set read_fds, write_fds;
        int max_fd(0);
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);

        FD_SET(ctl_pipe.read_fd(), &read_fds);
        max_fd = std::max(max_fd, ctl_pipe.read_fd());

        if (capture_stdout_pipe)
        {
            FD_SET(capture_stdout_pipe->read_fd(), &read_fds);
            max_fd = std::max(max_fd, capture_stdout_pipe->read_fd());
        }

        if (capture_stderr_pipe)
        {
            FD_SET(capture_stderr_pipe->read_fd(), &read_fds);
            max_fd = std::max(max_fd, capture_stderr_pipe->read_fd());
        }

        int retval(::pselect(max_fd + 1, &read_fds, &write_fds, 0, 0, 0));
        if (-1 == retval)
            throw ProcessError("pselect() failed");

        bool done_anything(false);
        char buf[4096];

        if (capture_stdout_pipe && FD_ISSET(capture_stdout_pipe->read_fd(), &read_fds))
        {
            int n(::read(capture_stdout_pipe->read_fd(), &buf, sizeof(buf)));
            if (-1 == n)
                throw ProcessError("read() capture_stdout_pipe read_fd failed");
            else if (0 != n)
                capture_stdout->write(buf, n);
            done_anything = true;
        }

        if (capture_stderr_pipe && FD_ISSET(capture_stderr_pipe->read_fd(), &read_fds))
        {
            int n(::read(capture_stderr_pipe->read_fd(), &buf, sizeof(buf)));
            if (-1 == n)
                throw ProcessError("read() capture_stderr_pipe read_fd failed");
            else if (0 != n)
                capture_stderr->write(buf, n);
            done_anything = true;
        }

        if (done_anything)
            continue;

        /* don't do this until nothing else has anything to do */
        if (FD_ISSET(ctl_pipe.read_fd(), &read_fds))
        {
            char c('?');
            if (1 != ::read(ctl_pipe.read_fd(), &c, 1))
                throw ProcessError("read() on our ctl pipe failed");
            else if (c != 'x')
                throw ProcessError("read() on our ctl pipe gave '" + std::string(1, c) + "' not 'x'");
            done = true;
        }
    }
}

void
RunningProcessThread::start()
{
    thread.reset(new Thread(std::bind(&RunningProcessThread::thread_func, this)));
}

namespace paludis
{
    template <>
    struct Imp<Process>
    {
        ProcessCommand command;

        bool need_thread;
        std::ostream * capture_stdout;
        std::ostream * capture_stderr;

        Imp(ProcessCommand && c) :
            command(std::move(c)),
            need_thread(false),
            capture_stdout(0),
            capture_stderr(0)
        {
        }
    };
}

Process::Process(ProcessCommand && c) :
    Pimp<Process>(c)
{
}

Process::~Process()
{
}

RunningProcessHandle
Process::run()
{
    std::unique_ptr<RunningProcessThread> thread;
    if (_imp->need_thread)
    {
        thread.reset(new RunningProcessThread{});

        if (_imp->capture_stdout)
        {
            thread->capture_stdout = _imp->capture_stdout;
            thread->capture_stdout_pipe.reset(new Pipe(true));
        }

        if (_imp->capture_stderr)
        {
            thread->capture_stderr = _imp->capture_stderr;
            thread->capture_stderr_pipe.reset(new Pipe(true));
        }
    }

    pid_t child(fork());
    if (-1 == child)
        throw ProcessError("fork() failed");
    else if (0 == child)
    {
        try
        {
            if (thread && thread->capture_stdout_pipe)
            {
                if (-1 == ::dup2(thread->capture_stdout_pipe->write_fd(), STDOUT_FILENO))
                    throw ProcessError("dup2() failed");
            }

            if (thread && thread->capture_stderr_pipe)
            {
                if (-1 == ::dup2(thread->capture_stderr_pipe->write_fd(), STDERR_FILENO))
                    throw ProcessError("dup2() failed");
            }

            _imp->command.exec();
        }
        catch (const ProcessError & e)
        {
            std::cerr << "Eek! child thread got error '" << e.message() << "'" << std::endl;
        }
        _exit(1);
    }
    else
    {
        if (thread)
            thread->start();
        return RunningProcessHandle(child, thread);
    }
}

Process &
Process::capture_stdout(std::ostream & s)
{
    _imp->need_thread = true;
    _imp->capture_stdout = &s;
    return *this;
}

Process &
Process::capture_stderr(std::ostream & s)
{
    _imp->need_thread = true;
    _imp->capture_stderr = &s;
    return *this;
}

namespace paludis
{
    template <>
    struct Imp<RunningProcessHandle>
    {
        pid_t pid;
        std::unique_ptr<RunningProcessThread> thread;

        Imp(pid_t p, std::unique_ptr<RunningProcessThread> && t) :
            pid(p),
            thread(std::move(t))
        {
        }
    };
}

RunningProcessHandle::RunningProcessHandle(pid_t p, std::unique_ptr<RunningProcessThread> && t) :
    Pimp<RunningProcessHandle>(p, t)
{
}

RunningProcessHandle::~RunningProcessHandle()
{
    if (-1 != _imp->pid)
    {
        int dummy PALUDIS_ATTRIBUTE((unused)) (wait());
        throw ProcessError("Didn't wait()");
    }
}

RunningProcessHandle::RunningProcessHandle(RunningProcessHandle && other) :
    Pimp<RunningProcessHandle>(other._imp->pid, other._imp->thread)
{
    _imp->pid = -1;
}

int
RunningProcessHandle::wait()
{
    if (-1 == _imp->pid)
        throw ProcessError("Already wait()ed");

    int status(0);
    if (-1 == ::waitpid(_imp->pid, &status, 0))
        throw ProcessError("waitpid() returned -1");
    _imp->pid = -1;

    if (_imp->thread)
    {
        char c('x');
        if (1 != ::write(_imp->thread->ctl_pipe.write_fd(), &c, 1))
            throw ProcessError("write() on our ctl pipe failed");

        _imp->thread.reset();
    }

    return (WIFSIGNALED(status) ? WTERMSIG(status) + 128 : WEXITSTATUS(status));
}

