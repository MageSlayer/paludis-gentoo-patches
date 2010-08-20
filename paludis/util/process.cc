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
#include <paludis/util/pty.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>

#include <iostream>
#include <functional>
#include <algorithm>
#include <vector>
#include <map>
#include <cstdlib>

#include <unistd.h>
#include <grp.h>
#include <pwd.h>
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

void
ProcessCommand::echo_command_to(std::ostream & s)
{
    for (auto v_begin(_imp->args.begin()), v(v_begin), v_end(_imp->args.end()) ;
            v != v_end ; ++v)
    {
        if (v != v_begin)
            s << " ";
        s << *v;
    }

    s << std::endl;
}

namespace paludis
{
    struct RunningProcessThread
    {
        Pipe ctl_pipe;

        std::ostream * capture_stdout;
        std::unique_ptr<Channel> capture_stdout_pipe;

        std::ostream * capture_stderr;
        std::unique_ptr<Channel> capture_stderr_pipe;

        std::ostream * capture_output_to_fd;
        std::unique_ptr<Pipe> capture_output_to_fd_pipe;

        /* must be last, so the thread gets join()ed before its FDs vanish */
        std::unique_ptr<Thread> thread;

        RunningProcessThread() :
            ctl_pipe(true),
            capture_stdout(0),
            capture_stderr(0),
            capture_output_to_fd(0)
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

        if (capture_output_to_fd)
        {
            FD_SET(capture_output_to_fd_pipe->read_fd(), &read_fds);
            max_fd = std::max(max_fd, capture_output_to_fd_pipe->read_fd());
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

        if (capture_output_to_fd_pipe && FD_ISSET(capture_output_to_fd_pipe->read_fd(), &read_fds))
        {
            int n(::read(capture_output_to_fd_pipe->read_fd(), &buf, sizeof(buf)));
            if (-1 == n)
                throw ProcessError("read() capture_output_to_fd_pipe read_fd failed");
            else if (0 != n)
                capture_output_to_fd->write(buf, n);
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
        bool use_ptys;
        std::ostream * capture_stdout;
        std::ostream * capture_stderr;
        std::ostream * capture_output_to_fd_stream;
        int capture_output_to_fd_fd;
        std::string capture_output_to_fd_env_var;
        int set_stdin_fd;

        std::map<std::string, std::string> setenvs;
        std::string chdir;
        uid_t setuid;
        gid_t setgid;
        std::ostream * echo_command_to;

        Imp(ProcessCommand && c) :
            command(std::move(c)),
            need_thread(false),
            use_ptys(false),
            capture_stdout(0),
            capture_stderr(0),
            capture_output_to_fd_stream(0),
            capture_output_to_fd_fd(-1),
            set_stdin_fd(-1),
            setuid(getuid()),
            setgid(getgid()),
            echo_command_to(0)
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
    if (_imp->echo_command_to)
        _imp->command.echo_command_to(*_imp->echo_command_to);

    std::unique_ptr<RunningProcessThread> thread;
    if (_imp->need_thread)
    {
        thread.reset(new RunningProcessThread{});

        if (_imp->capture_stdout)
        {
            thread->capture_stdout = _imp->capture_stdout;
            if (_imp->use_ptys)
                thread->capture_stdout_pipe.reset(new Pty(true));
            else
                thread->capture_stdout_pipe.reset(new Pipe(true));
        }

        if (_imp->capture_stderr)
        {
            thread->capture_stderr = _imp->capture_stderr;
            if (_imp->use_ptys)
                thread->capture_stderr_pipe.reset(new Pty(true));
            else
                thread->capture_stderr_pipe.reset(new Pipe(true));
        }

        if (_imp->capture_output_to_fd_stream)
        {
            thread->capture_output_to_fd = _imp->capture_output_to_fd_stream;
            thread->capture_output_to_fd_pipe.reset(new Pipe(true));
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

            if (thread && thread->capture_output_to_fd_pipe)
            {
                int fd(_imp->capture_output_to_fd_fd);
                if (-1 == fd)
                    fd = ::dup(thread->capture_output_to_fd_pipe->write_fd());
                else
                    fd = ::dup2(thread->capture_output_to_fd_pipe->write_fd(), fd);

                if (-1 == fd)
                    throw ProcessError("dup failed");
                else if (! _imp->capture_output_to_fd_env_var.empty())
                    ::setenv(_imp->capture_output_to_fd_env_var.c_str(), stringify(fd).c_str(), 1);
            }

            if (-1 != _imp->set_stdin_fd)
            {
                if (-1 == ::dup2(_imp->set_stdin_fd, STDIN_FILENO))
                    throw ProcessError("dup2() failed");
            }

            for (auto m(_imp->setenvs.begin()), m_end(_imp->setenvs.end()) ;
                    m != m_end ; ++m)
                ::setenv(m->first.c_str(), m->second.c_str(), 1);

            if (! _imp->chdir.empty())
                if (-1 == ::chdir(_imp->chdir.c_str()))
                    throw ProcessError("chdir() failed");

            if (_imp->setuid != getuid() || _imp->setgid != getgid())
            {
                if (0 != ::setgid(_imp->setgid))
                    throw ProcessError("setgid() failed");

                int buflen(::sysconf(_SC_GETPW_R_SIZE_MAX));
                if (-1 == buflen)
                    buflen = 1 << 16;

                std::unique_ptr<char []> buf(new char[buflen]);
                struct passwd pw;
                struct passwd * pw_result(0);
                if (0 != getpwuid_r(_imp->setuid, &pw, buf.get(), buflen, &pw_result) || 0 == pw_result)
                    throw ProcessError("getpwuid_r() failed");

                if (0 != ::initgroups(pw_result->pw_name, getgid()))
                    throw ProcessError("initgroups() failed");

                if (0 != ::setuid(_imp->setuid))
                    throw ProcessError("setuid() failed");
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

Process &
Process::capture_output_to_fd(std::ostream & s, int fd_or_minus_one, const std::string & env_var_with_fd)
{
    _imp->need_thread = true;
    _imp->capture_output_to_fd_stream = &s;
    _imp->capture_output_to_fd_fd = fd_or_minus_one;
    _imp->capture_output_to_fd_env_var = env_var_with_fd;
    return *this;
}

Process &
Process::set_stdin_fd(int f)
{
    _imp->set_stdin_fd = f;
    return *this;
}

Process &
Process::setenv(const std::string & a, const std::string & b)
{
    _imp->setenvs.insert(std::make_pair(a, b)).first->second = b;
    return *this;
}

Process &
Process::chdir(const FSEntry & f)
{
    _imp->chdir = stringify(f.realpath_if_exists());
    return *this;
}

Process &
Process::use_ptys()
{
    _imp->use_ptys = true;
    return *this;
}

Process &
Process::setuid_setgid(uid_t u, gid_t g)
{
    _imp->setuid = u;
    _imp->setgid = g;
    return *this;
}

Process &
Process::echo_command_to(std::ostream & s)
{
    _imp->echo_command_to = &s;
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

