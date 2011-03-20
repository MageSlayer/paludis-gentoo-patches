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

#include <paludis/util/process.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/thread.hh>
#include <paludis/util/pipe.hh>
#include <paludis/util/pty.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/log.hh>
#include <paludis/util/system.hh>
#include <paludis/util/env_var_names.hh>

#include <iostream>
#include <functional>
#include <algorithm>
#include <vector>
#include <map>
#include <cstdlib>
#include <cstring>

#include <errno.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <fcntl.h>
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
        std::string args_string;

        Imp(std::vector<std::string> && i, const std::string & s) :
            args(i),
            args_string(s)
        {
        }
    };
}

ProcessCommand::ProcessCommand(const std::initializer_list<std::string> & i) :
    _imp(std::vector<std::string>(i), "")
{
}

ProcessCommand::ProcessCommand(const std::string & s) :
    _imp(std::vector<std::string>(), s)
{
}

ProcessCommand::ProcessCommand(ProcessCommand && other) :
    _imp(std::move(other._imp->args), other._imp->args_string)
{
}

ProcessCommand::~ProcessCommand() = default;

void
ProcessCommand::prepend_args(const std::initializer_list<std::string> & l)
{
    _imp->args.insert(_imp->args.begin(), l);
}

void
ProcessCommand::append_args(const std::initializer_list<std::string> & l)
{
    _imp->args.insert(_imp->args.end(), l);
}

void
ProcessCommand::exec()
{
    if (! _imp->args_string.empty())
    {
        std::string s;
        for (auto v_begin(_imp->args.begin()), v(v_begin), v_end(_imp->args.end()) ;
                v != v_end ; ++v)
        {
            if (v != v_begin)
                s.append(" ");
            s.append(*v);
        }

        if (! s.empty())
            s.append(" ");
        s.append(_imp->args_string);

        execl("/bin/sh", "sh", "-c", s.c_str(), static_cast<char *>(0));

        throw ProcessError("execl failed");
    }
    else
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

        std::unique_ptr<SafeOFStream> own_capture_stdout;
        std::unique_ptr<SafeOFStream> own_capture_stderr;

        std::string prefix_stdout;
        std::ostream * capture_stdout;
        std::unique_ptr<Channel> capture_stdout_pipe;
        std::string prefix_stdout_buffer;

        std::string prefix_stderr;
        std::ostream * capture_stderr;
        std::unique_ptr<Channel> capture_stderr_pipe;
        std::string prefix_stderr_buffer;

        std::ostream * capture_output_to_fd;
        std::unique_ptr<Pipe> capture_output_to_fd_pipe;

        std::istream * send_input_to_fd;
        std::unique_ptr<Pipe> send_input_to_fd_pipe;

        ProcessPipeCommandFunction pipe_command_handler;
        std::unique_ptr<Pipe> pipe_command_handler_command_pipe;
        std::unique_ptr<Pipe> pipe_command_handler_response_pipe;
        std::string pipe_command_handler_buffer;

        bool as_main_process;

        /* must be last, so the thread gets join()ed before its FDs vanish */
        std::unique_ptr<Thread> thread;

        RunningProcessThread() :
            ctl_pipe(true),
            capture_stdout(0),
            capture_stderr(0),
            capture_output_to_fd(0),
            send_input_to_fd(0),
            as_main_process(false)
        {
        }

        void thread_func();

        void start();
    };
}

void
RunningProcessThread::thread_func()
{
    bool prefix_stdout_buffer_has_newline(false), prefix_stderr_buffer_has_newline(false), want_to_finish(true);
    std::string input_stream_pending;

    if (as_main_process && send_input_to_fd)
        want_to_finish = false;

    bool done(false);
    while (! done)
    {
        fd_set read_fds, write_fds;
        int max_fd(0);
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);

        if (want_to_finish)
        {
            FD_SET(ctl_pipe.read_fd(), &read_fds);
            max_fd = std::max(max_fd, ctl_pipe.read_fd());
        }

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

        if (send_input_to_fd)
        {
            FD_SET(send_input_to_fd_pipe->write_fd(), &write_fds);
            max_fd = std::max(max_fd, send_input_to_fd_pipe->write_fd());
        }

        if (pipe_command_handler)
        {
            FD_SET(pipe_command_handler_command_pipe->read_fd(), &read_fds);
            max_fd = std::max(max_fd, pipe_command_handler_command_pipe->read_fd());
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
            {
                if (prefix_stdout.empty())
                    capture_stdout->write(buf, n);
                else
                {
                    std::string s(buf, n);
                    prefix_stdout_buffer.append(s);
                    if (std::string::npos != s.find('\n'))
                        prefix_stdout_buffer_has_newline = true;
                }
            }
            done_anything = true;
        }

        if (capture_stderr_pipe && FD_ISSET(capture_stderr_pipe->read_fd(), &read_fds))
        {
            int n(::read(capture_stderr_pipe->read_fd(), &buf, sizeof(buf)));
            if (-1 == n)
                throw ProcessError("read() capture_stderr_pipe read_fd failed");
            else if (0 != n)
            {
                if (prefix_stderr.empty())
                    capture_stderr->write(buf, n);
                else
                {
                    std::string s(buf, n);
                    prefix_stderr_buffer.append(s);
                    if (std::string::npos != s.find('\n'))
                        prefix_stderr_buffer_has_newline = true;
                }
            }
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

        if (send_input_to_fd && FD_ISSET(send_input_to_fd_pipe->write_fd(), &write_fds))
        {
            while ((! input_stream_pending.empty()) || send_input_to_fd->good())
            {
                if (input_stream_pending.empty() && send_input_to_fd->good())
                {
                    send_input_to_fd->read(buf, sizeof(buf));
                    input_stream_pending.assign(buf, send_input_to_fd->gcount());
                }

                int w(::write(send_input_to_fd_pipe->write_fd(), input_stream_pending.data(),
                            input_stream_pending.length()));

                if (0 == w || (-1 == w && (errno == EAGAIN || errno == EWOULDBLOCK)))
                    break;
                else if (-1 == w)
                    throw ProcessError("write() send_input_to_fd_pipe write_fd failed");
                else
                    input_stream_pending.erase(0, w);
            }

            if (input_stream_pending.empty() && ! send_input_to_fd->good())
            {
                if (0 != ::close(send_input_to_fd_pipe->write_fd()))
                    throw ProcessError("close() send_input_to_fd_pipe write_fd failed");
                send_input_to_fd_pipe->clear_write_fd();
                send_input_to_fd = 0;
                want_to_finish = true;
            }

            done_anything = true;
        }

        if (pipe_command_handler && FD_ISSET(pipe_command_handler_command_pipe->read_fd(), &read_fds))
        {
            int n(::read(pipe_command_handler_command_pipe->read_fd(), &buf, sizeof(buf)));
            if (-1 == n)
                throw ProcessError("read() pipe_command_handler_command_pipe read_fd failed");
            else if (0 != n)
                pipe_command_handler_buffer.append(buf, n);
            done_anything = true;
        }

        while (! pipe_command_handler_buffer.empty())
        {
            std::string::size_type n_p(pipe_command_handler_buffer.find('\0'));
            if (std::string::npos == n_p)
                break;

            std::string op(pipe_command_handler_buffer.substr(0, n_p));
            pipe_command_handler_buffer.erase(0, n_p + 1);

            std::string response(pipe_command_handler(op));

            ssize_t n(0);
            while (! response.empty())
            {
                n = write(pipe_command_handler_response_pipe->write_fd(), response.c_str(), response.length());
                if (-1 == n)
                    throw ProcessError("write() pipe_command_handler_response_pipe write_fd failed");
                else
                    response.erase(0, n);
            }

            char c(0);
            n = write(pipe_command_handler_response_pipe->write_fd(), &c, 1);
            if (1 != n)
                throw ProcessError("write() pipe_command_handler_response_pipe write_fd failed");
        }

        if (prefix_stdout_buffer_has_newline)
        {
            while (true)
            {
                std::string::size_type p(prefix_stdout_buffer.find('\n'));
                if (std::string::npos == p)
                    break;

                capture_stdout->write(prefix_stdout.data(), prefix_stdout.length());
                capture_stdout->write(prefix_stdout_buffer.data(), p + 1);
                prefix_stdout_buffer.erase(0, p + 1);
            }

            prefix_stdout_buffer_has_newline = false;
        }

        if (prefix_stderr_buffer_has_newline)
        {
            while (true)
            {
                std::string::size_type p(prefix_stderr_buffer.find('\n'));
                if (std::string::npos == p)
                    break;

                capture_stderr->write(prefix_stderr.data(), prefix_stderr.length());
                capture_stderr->write(prefix_stderr_buffer.data(), p + 1);
                prefix_stderr_buffer.erase(0, p + 1);
            }

            prefix_stderr_buffer_has_newline = false;
        }

        if (done_anything)
            continue;

        /* don't do this until nothing else has anything to do */
        if (FD_ISSET(ctl_pipe.read_fd(), &read_fds))
        {
            /* haxx: flush our buffers first */
            if (! prefix_stdout_buffer.empty())
            {
                prefix_stdout_buffer.append("\n");
                prefix_stdout_buffer_has_newline = true;
                continue;
            }

            if (! prefix_stderr_buffer.empty())
            {
                prefix_stderr_buffer.append("\n");
                prefix_stderr_buffer_has_newline = true;
                continue;
            }

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
        std::istream * send_input_to_fd_stream;
        int send_input_to_fd_fd;
        std::string send_input_to_fd_env_var;
        ProcessPipeCommandFunction pipe_command_handler;
        std::string pipe_command_handler_env_var;
        int set_stdin_fd;

        std::map<std::string, std::string> setenvs;
        bool clearenv;
        std::string chdir;
        uid_t setuid;
        gid_t setgid;
        std::ostream * echo_command_to;

        std::string prefix_stdout;
        std::string prefix_stderr;

        bool as_main_process;

        Imp(ProcessCommand && c) :
            command(std::move(c)),
            need_thread(false),
            use_ptys(false),
            capture_stdout(0),
            capture_stderr(0),
            capture_output_to_fd_stream(0),
            capture_output_to_fd_fd(-1),
            send_input_to_fd_stream(0),
            send_input_to_fd_fd(-1),
            set_stdin_fd(-1),
            clearenv(false),
            setuid(getuid()),
            setgid(getgid()),
            echo_command_to(0),
            as_main_process(false)
        {
        }
    };
}

Process::Process(ProcessCommand && c) :
    _imp(std::move(c))
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
        thread->as_main_process = _imp->as_main_process;

        if (! _imp->prefix_stdout.empty())
        {
            thread->prefix_stdout = _imp->prefix_stdout;
            if (! _imp->capture_stdout)
            {
                thread->own_capture_stdout.reset(new SafeOFStream(STDOUT_FILENO, false));
                _imp->capture_stdout = thread->own_capture_stdout.get();
            }
        }

        if (! _imp->prefix_stderr.empty())
        {
            thread->prefix_stderr = _imp->prefix_stderr;
            if (! _imp->capture_stderr)
            {
                thread->own_capture_stderr.reset(new SafeOFStream(STDERR_FILENO, false));
                _imp->capture_stderr = thread->own_capture_stderr.get();
            }
        }

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

        if (_imp->send_input_to_fd_stream)
        {
            thread->send_input_to_fd = _imp->send_input_to_fd_stream;
            thread->send_input_to_fd_pipe.reset(new Pipe(true));

            int arg(::fcntl(thread->send_input_to_fd_pipe->write_fd(), F_GETFL, NULL));
            if (-1 == arg)
                throw ProcessError("fcntl(F_GETFL) failed");
            arg |= O_NONBLOCK;
            if (-1 == ::fcntl(thread->send_input_to_fd_pipe->write_fd(), F_SETFL, arg))
                throw ProcessError("fcntl(F_SETFL) failed");
        }

        if (_imp->pipe_command_handler)
        {
            thread->pipe_command_handler = _imp->pipe_command_handler;
            thread->pipe_command_handler_command_pipe.reset(new Pipe(true));
            thread->pipe_command_handler_response_pipe.reset(new Pipe(true));
        }
    }

    /* Temporarily disable SIGINT and SIGTERM to this thread */
    sigset_t intandterm;
    sigemptyset(&intandterm);
    sigaddset(&intandterm, SIGINT);
    sigaddset(&intandterm, SIGTERM);
    if (0 != pthread_sigmask(SIG_BLOCK, &intandterm, 0))
        throw ProcessError("pthread_sigmask failed");

    pid_t child(fork());
    if (-1 == child)
        throw ProcessError("fork() failed: " + stringify(::strerror(errno)));
    else if ((0 == child) ^ _imp->as_main_process)
    {
        try
        {
            if (_imp->as_main_process)
            {
                int status;
                if (-1 == ::waitpid(child, &status, 0))
                    throw ProcessError("waitpid() returned -1");
            }

            /* clear any SIGINT or SIGTERM handlers we inherit, and unblock signals */
            struct sigaction act;
            sigemptyset(&act.sa_mask);
            act.sa_handler = SIG_DFL;
            act.sa_flags = 0;
            sigaction(SIGINT,  &act, 0);
            sigaction(SIGTERM, &act, 0);
            if (0 != pthread_sigmask(SIG_UNBLOCK, &intandterm, 0))
                throw ProcessError("pthread_sigmask failed");

            if (_imp->clearenv)
            {
                std::map<std::string, std::string> setenvs;
                for (const char * const * it(environ) ; 0 != *it ; ++it)
                {
                    std::string var(*it);
                    if (std::string::npos != var.find('=') &&
                            ("PALUDIS_" == var.substr(0, 8) ||
                             "PATH=" == var.substr(0, 5) ||
                             "HOME=" == var.substr(0, 5) ||
                             "LD_LIBRARY_PATH=" == var.substr(0, 16)))
                        setenvs.insert(std::make_pair(var.substr(0, var.find('=')), var.substr(var.find('=') + 1)));
                }
                ::clearenv();

                for (std::map<std::string, std::string>::const_iterator it(setenvs.begin()),
                        it_end(setenvs.end()) ; it_end != it ; ++it)
                    ::setenv(it->first.c_str(), it->second.c_str(), 1);
            }

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

            if (thread && thread->send_input_to_fd_pipe)
            {
                int fd(_imp->send_input_to_fd_fd);
                if (-1 == fd)
                    fd = ::dup(thread->send_input_to_fd_pipe->read_fd());
                else
                    fd = ::dup2(thread->send_input_to_fd_pipe->read_fd(), fd);

                if (-1 == fd)
                    throw ProcessError("dup failed");
                else if (! _imp->send_input_to_fd_env_var.empty())
                    ::setenv(_imp->send_input_to_fd_env_var.c_str(), stringify(fd).c_str(), 1);
            }

            if (thread && thread->pipe_command_handler)
            {
                int read_fd(::dup(thread->pipe_command_handler_response_pipe->read_fd()));
                if (-1 == read_fd)
                    throw ProcessError("dup failed");

                int write_fd(::dup(thread->pipe_command_handler_command_pipe->write_fd()));
                if (-1 == write_fd)
                    throw ProcessError("dup failed");

                if (! _imp->pipe_command_handler_env_var.empty())
                {
                    ::setenv((_imp->pipe_command_handler_env_var + "_READ_FD").c_str(), stringify(read_fd).c_str(), 1);
                    ::setenv((_imp->pipe_command_handler_env_var + "_WRITE_FD").c_str(), stringify(write_fd).c_str(), 1);
                }
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
        if (_imp->as_main_process)
        {
            /* Ignore CLD. POSIX may or may not say that if we do this, our child will
             * not become a zombie. */
            struct sigaction act;
            sigemptyset(&act.sa_mask);
            act.sa_handler = SIG_IGN;
            act.sa_flags = 0;
            sigaction(SIGCLD, &act, 0);

            pid_t p(fork());
            if (-1 == p)
                throw ProcessError("fork() failed: " + stringify(::strerror(errno)));
            else if (0 != p)
                _exit(0);
        }

        /* Restore SIGINT and SIGTERM handling */
        if (0 != pthread_sigmask(SIG_UNBLOCK, &intandterm, 0))
            throw ProcessError("pthread_sigmask failed");

        if (thread)
            thread->start();
        return RunningProcessHandle(_imp->as_main_process ? 0 : child, std::move(thread));
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
Process::send_input_to_fd(std::istream & s, int fd_or_minus_one, const std::string & env_var_with_fd)
{
    _imp->need_thread = true;
    _imp->send_input_to_fd_stream = &s;
    _imp->send_input_to_fd_fd = fd_or_minus_one;
    _imp->send_input_to_fd_env_var = env_var_with_fd;
    return *this;
}

Process &
Process::set_stdin_fd(int f)
{
    _imp->set_stdin_fd = f;
    return *this;
}

Process &
Process::pipe_command_handler(const std::string & v, const ProcessPipeCommandFunction & f)
{
    _imp->need_thread = true;
    _imp->pipe_command_handler = f;
    _imp->pipe_command_handler_env_var = v;
    return *this;
}

Process &
Process::setenv(const std::string & a, const std::string & b)
{
    _imp->setenvs.insert(std::make_pair(a, b)).first->second = b;
    return *this;
}

Process &
Process::clearenv()
{
    _imp->clearenv = true;
    return *this;
}

Process &
Process::chdir(const FSPath & f)
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

Process &
Process::prefix_stdout(const std::string & s)
{
    _imp->need_thread = true;
    _imp->prefix_stdout = s;
    return *this;
}

Process &
Process::prefix_stderr(const std::string & s)
{
    _imp->need_thread = true;
    _imp->prefix_stderr = s;
    return *this;
}

Process &
Process::as_main_process()
{
    _imp->as_main_process = true;
    return *this;
}

namespace
{
    bool check_cmd(const std::string & s)
    {
        bool result(0 == Process(ProcessCommand({ "sh", "-c", s + " --version >/dev/null 2>/dev/null" })).run().wait());
        if (! result)
            Log::get_instance()->message("util.system.boxless", ll_warning, lc_context) <<
                "I don't seem to be able to use " + s;
        return result;
    }
}

Process &
Process::sandbox()
{
    static bool can_use_sandbox(check_cmd("sandbox"));

    if (can_use_sandbox)
    {
        if (! getenv_with_default(env_vars::do_nothing_sandboxy, "").empty())
            Log::get_instance()->message("util.system.nothing_sandboxy", ll_debug, lc_no_context)
                << "PALUDIS_DO_NOTHING_SANDBOXY is set, not using sandbox";
        else if (! getenv_with_default("SANDBOX_ACTIVE", "").empty())
            Log::get_instance()->message("util.system.sandbox_in_sandbox", ll_warning, lc_no_context)
                << "Already inside sandbox, not spawning another sandbox instance";
        else
        {
            _imp->command.prepend_args({ "sandbox" });
            if (getenv_with_default("BASH_ENV", "").empty())
                setenv("BASH_ENV", "/dev/null");
        }
    }

    return *this;
}

Process &
Process::sydbox()
{
    static bool can_use_sydbox(check_cmd("sydbox"));

    if (can_use_sydbox)
    {
        if (! getenv_with_default(env_vars::do_nothing_sandboxy, "").empty())
            Log::get_instance()->message("util.system.nothing_sandboxy", ll_debug, lc_no_context)
                << "PALUDIS_DO_NOTHING_SANDBOXY is set, not using sydbox";
        else if (! getenv_with_default("SYDBOX_ACTIVE", "").empty())
            Log::get_instance()->message("util.system.sandbox_in_sandbox", ll_warning, lc_no_context)
                << "Already inside sydbox, not spawning another sydbox instance";
        else
            _imp->command.prepend_args({ "sydbox", "--profile", "paludis", "--" });
    }

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
    _imp(p, std::move(t))
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
    _imp(other._imp->pid, std::move(other._imp->thread))
{
    _imp->pid = -1;
}

int
RunningProcessHandle::wait()
{
    bool actually_wait(0 != _imp->pid);

    if (-1 == _imp->pid)
        throw ProcessError("Already wait()ed");

    int status(0);
    if (actually_wait)
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

    if (actually_wait)
        return (WIFSIGNALED(status) ? WTERMSIG(status) + 128 : WEXITSTATUS(status));
    else
        return status;
}

