/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011, 2012 Ciaran McCreesh
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

#include <paludis/util/persona.hh>
#include <paludis/util/process.hh>
#include <paludis/util/pimp-impl.hh>
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
#include <thread>
#include <mutex>

#include <errno.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/ioctl.h>

using namespace paludis;

ProcessError::ProcessError(const std::string & s) noexcept :
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

        std::map<std::string, std::string> setenvs;
        bool clearenv;
        std::string chdir;
        uid_t setuid;
        gid_t setgid;

        std::vector<std::string> env_storage;
        std::vector<const char*> env_ptrs;

        std::string args_storage;
        std::vector<const char*> argv_ptrs;

        gid_t groups[NGROUPS_MAX];
        int group_count;

        Imp(std::vector<std::string> && i, const std::string & s) :
            args(i),
            args_string(s),
            clearenv(false),
            setuid(getuid()),
            setgid(getgid())
        {
        }

        void clear_alloc()
        {
            env_storage.clear();
            env_ptrs.clear();
            args_storage.clear();
            argv_ptrs.clear();
        }
    };
}

namespace
{
    class ExecError
    {
        public:
            enum Type
            {
                NO_ERROR = 0,

                ERROR_READING_FAILED,
                ERROR_SENDING_FAILED,
                CHDIR_FAILED,
                SETGID_FAILED,
                SETGROUPS_FAILED,
                SETUID_FAILED,
                EXECVE_FAILED,
                EXECVPE_FAILED,
                DUP2_FAILED,
                FCNTL_FAILED,
                WAITPID_FAILED,
                PTHREAD_SIGMASK_FAILED
            };

            ExecError(Type t, int e) :
                type(t),
                errsv(e)
            {}

            ExecError(int err_fd)
            {
                ExecError tmp;
                ssize_t r = read_all(err_fd, static_cast<void*>(&tmp), sizeof(ExecError));

                if (r < 0)
                {
                    type = ERROR_READING_FAILED;
                    errsv = errno;
                }
                else if (0 == r)
                {
                    type = NO_ERROR;
                    errsv = 0;
                }
                else if (sizeof(ExecError) != r)
                {
                    type = ERROR_SENDING_FAILED;
                    errsv = 0;
                }
                else
                {
                    type = tmp.type;
                    errsv = tmp.errsv;
                }
            }

            void send(int err_fd) const
            {
                /* Just a small AS-safe function to get the error code to the
                 * parent process. */
                write_all(err_fd, static_cast<const void*>(this), sizeof(ExecError));
            }

            operator std::string() const
            {
                std::string msg;

                switch (type)
                {
                    case NO_ERROR:
                        return "the exec call in the child process succeeded";
                    case ERROR_READING_FAILED:
                        msg = "read() on the error fd failed in the parent";
                        break;
                    case ERROR_SENDING_FAILED:
                        return "the child process did not send a complete error message";
                    case CHDIR_FAILED:
                        msg = "chdir() failed in the child process";
                        break;
                    case SETGID_FAILED:
                        msg = "setgid() failed in the child process";
                        break;
                    case SETGROUPS_FAILED:
                        msg = "setgroups() failed in the child process";
                        break;
                    case SETUID_FAILED:
                        msg = "setuid() failed in the child process";
                        break;
                    case EXECVE_FAILED:
                        msg = "execve() failed in the child process";
                        break;
                    case EXECVPE_FAILED:
                        msg = "execvpe() failed in the child process";
                        break;
                    case DUP2_FAILED:
                        msg = "dup2() failed in the child process";
                        break;
                    case FCNTL_FAILED:
                        msg = "fcntl() failed in the child process";
                        break;
                    case WAITPID_FAILED:
                        msg = "waitpid() failed in the child process";
                        break;
                    case PTHREAD_SIGMASK_FAILED:
                        return "pthread_sigmask() failed in the child process";
                    default:
                        return "unknown err.type received on the error pipeline";
                }
                msg += ": " + std::string(std::strerror(errsv));
                return msg;
            }

            operator bool() const
            {
                return type != NO_ERROR;
            }
        private:
            Type type;
            int errsv;

            ExecError() :
                type(NO_ERROR),
                errsv(0)
            {}

            static int write_all(int fd, const void *buf_, size_t count)
            {
                const char *buf = static_cast<const char*>(buf_);
                const char *cur = buf;
                const char *end = buf + count;

                while(cur != end)
                {
                    ssize_t r = write(fd, cur, end-cur);

                    if (r < 0)
                    {
                        if(errno == EINTR)
                            continue;
                        else
                            return -1;
                    }

                    cur += r;
                }

                return 0;
            }

            static ssize_t read_all(int fd, void *buf_, size_t count)
            {
                char *buf = static_cast<char*>(buf_);
                char *cur = buf;
                char *end = buf + count;

                ssize_t r = -1;
                while(cur != end && r != 0)
                {
                    r = read(fd, cur, end-cur);

                    if (r < 0)
                    {
                        if(errno == EINTR)
                            continue;
                        else
                            return -1;
                    }

                    cur += r;
                }

                return cur-buf;
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

ProcessCommand &
ProcessCommand::setenv(const std::string & a, const std::string & b)
{
    _imp->setenvs.insert(std::make_pair(a, b)).first->second = b;
    _imp->clear_alloc();
    return *this;
}

ProcessCommand &
ProcessCommand::clearenv()
{
    _imp->clearenv = true;
    _imp->clear_alloc();
    return *this;
}

ProcessCommand &
ProcessCommand::chdir(const FSPath & f)
{
    _imp->chdir = stringify(f.realpath_if_exists());
    return *this;
}

ProcessCommand &
ProcessCommand::setuid_setgid(uid_t u, gid_t g)
{
    _imp->setuid = u;
    _imp->setgid = g;
    return *this;
}

void
ProcessCommand::prepend_args(const std::initializer_list<std::string> & l)
{
    _imp->clear_alloc();
    _imp->args.insert(_imp->args.begin(), l);
}

void
ProcessCommand::append_args(const std::initializer_list<std::string> & l)
{
    _imp->clear_alloc();
    _imp->args.insert(_imp->args.end(), l);
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

void
ProcessCommand::exec_prepare()
{

    if (! _imp->env_ptrs.empty())
        return;

    _imp->clear_alloc();

    std::map<std::string, std::string> env_map = _imp->setenvs;
    for(const char * const * it(environ) ; nullptr != *it ; ++it)
    {
        std::string var(*it);
        size_t delim = var.find('=');

        std::string name = var.substr(0, delim);
        std::string val  = delim == std::string::npos || delim+1 > var.size() ? "" : var.substr(delim+1);

        if (!_imp->clearenv ||
            "PALUDIS_" == name.substr(0, 8) ||
            "PATH" == name ||
            "HOME" == name ||
            "LD_LIBRARY_PATH" == name)
            env_map.insert(std::make_pair(name, val));
    }

    for (std::map<std::string, std::string>::const_iterator it(env_map.begin()),
            it_end(env_map.end()) ; it_end != it ; ++it)
        _imp->env_storage.push_back(it->first + "=" + it->second);

    for(const std::string & env : _imp->env_storage)
        _imp->env_ptrs.push_back(env.c_str());

    _imp->env_ptrs.push_back(static_cast<char*>(nullptr));

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

        _imp->args_storage = s;
        _imp->argv_ptrs = { "sh", "-c", _imp->args_storage.c_str() };
    }
    else
    {
        if (_imp->args.size() < 1)
            throw ProcessError("No command specified");

        for (auto v_begin(_imp->args.begin()), v(v_begin), v_end(_imp->args.end()) ;
                v != v_end ; ++v)
            _imp->argv_ptrs.push_back(v->c_str());
    }
    _imp->argv_ptrs.push_back(static_cast<char*>(nullptr));

    _imp->group_count = NGROUPS_MAX;
    struct passwd pwd;
    struct passwd *result;
    std::vector<char> buffer;

    if (0 != getpwuid_r_s(_imp->setuid, buffer, pwd, result) || result == nullptr)
        throw ProcessError("getpwuid_r() failed");

    if (getgrouplist(pwd.pw_name, _imp->setgid, _imp->groups, &_imp->group_count) < 0)
        throw ProcessError("getgrouplist() failed");
}

namespace
{
    PALUDIS_ATTRIBUTE((noreturn))
    void
    send_error(int err_fd, const ExecError & e)
    {
        if (-1 == err_fd)
        {
            throw ProcessError(e);
        }
        else
        {
            e.send(err_fd);
            _exit(1);
        }
    }
}

void
ProcessCommand::exec(int err_fd)
{
    if (! _imp->chdir.empty())
        if (-1 == ::chdir(_imp->chdir.c_str()))
            send_error(err_fd, ExecError(ExecError::CHDIR_FAILED, errno));

    if (_imp->setuid != getuid() || _imp->setgid != getgid())
    {
        if (0 != ::setgid(_imp->setgid))
            send_error(err_fd, ExecError(ExecError::SETGID_FAILED, errno));

        if (0 != ::setgroups(_imp->group_count, _imp->groups))
            send_error(err_fd, ExecError(ExecError::SETGROUPS_FAILED, errno));

        if (0 != ::setuid(_imp->setuid))
            send_error(err_fd, ExecError(ExecError::SETUID_FAILED, errno));
    }

    if (! _imp->args_string.empty())
    {
        if (execve("/bin/sh", const_cast<char *const *>(_imp->argv_ptrs.data()), const_cast<char *const *>(_imp->env_ptrs.data())) < 0)
            send_error(err_fd, ExecError(ExecError::EXECVE_FAILED, errno));
    }
    else
    {
        if (execvpe(_imp->argv_ptrs[0], const_cast<char *const *>(_imp->argv_ptrs.data()), const_cast<char *const *>(_imp->env_ptrs.data())) < 0)
            send_error(err_fd, ExecError(ExecError::EXECVPE_FAILED, errno));
    }

    _exit(1);
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

        bool extra_newlines_if_any_output_exists;

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
        std::thread thread;

        RunningProcessThread() :
            ctl_pipe(true),
            capture_stdout(nullptr),
            capture_stderr(nullptr),
            capture_output_to_fd(nullptr),
            send_input_to_fd(nullptr),
            as_main_process(false)
        {
        }

        ~RunningProcessThread()
        {
            if (thread.joinable())
                thread.join();
        }

        void thread_func();

        void start();
    };
}

void
RunningProcessThread::thread_func()
{
    bool prefix_stdout_buffer_has_newline(false);
    bool prefix_stderr_buffer_has_newline(false);
    bool want_to_finish(true);
    bool done_extra_newlines_stdout(false);
    bool done_extra_newlines_stderr(false);
    std::string input_stream_pending;

    if (as_main_process && send_input_to_fd)
        want_to_finish = false;

    bool done(false);
    while (! done)
    {
        fd_set read_fds;
        fd_set write_fds;
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

        int retval(::pselect(max_fd + 1, &read_fds, &write_fds, nullptr, nullptr, nullptr));
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
                send_input_to_fd = nullptr;
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

                if (extra_newlines_if_any_output_exists)
                {
                    if (! done_extra_newlines_stdout)
                        capture_stdout->write("\n", 1);
                    done_extra_newlines_stdout = true;
                }

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

                if (extra_newlines_if_any_output_exists)
                {
                    if (! done_extra_newlines_stderr)
                        capture_stderr->write("\n", 1);
                    done_extra_newlines_stderr = true;
                }

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

    if (extra_newlines_if_any_output_exists)
    {
        if (done_extra_newlines_stdout)
            capture_stdout->write("\n", 1);
        if (done_extra_newlines_stderr)
            capture_stderr->write("\n", 1);
    }
}

void
RunningProcessThread::start()
{
    thread = std::thread(std::bind(&RunningProcessThread::thread_func, this));
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

        std::ostream * echo_command_to;

        std::string prefix_stdout;
        std::string prefix_stderr;
        bool extra_newlines_if_any_output_exists;

        bool as_main_process;

        Imp(ProcessCommand && c) :
            command(std::move(c)),
            need_thread(false),
            use_ptys(false),
            capture_stdout(nullptr),
            capture_stderr(nullptr),
            capture_output_to_fd_stream(nullptr),
            capture_output_to_fd_fd(-1),
            send_input_to_fd_stream(nullptr),
            send_input_to_fd_fd(-1),
            set_stdin_fd(-1),
            echo_command_to(nullptr),
            extra_newlines_if_any_output_exists(false),
            as_main_process(false)
        {
        }
    };
}

Process::Process(ProcessCommand && c) :
    _imp(std::move(c))
{
}

Process::~Process() = default;

namespace
{
    pid_t
    waitpid_noeintr(pid_t pid, int *wstatus, int options)
    {
        while (true)
        {
            pid_t ret = ::waitpid(pid, wstatus, options);
            if (ret == -1 && errno == EINTR) // waitpid was interrupted by a signal, retry
                continue;
            return ret;
        }
    }
}

RunningProcessHandle
Process::run()
{
    if (_imp->echo_command_to)
        _imp->command.echo_command_to(*_imp->echo_command_to);

    bool set_tty_size_envvars(false);
    unsigned short columns(80);
    unsigned short lines(24);

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
                _imp->capture_stdout = *thread->own_capture_stdout;
            }
        }

        if (! _imp->prefix_stderr.empty())
        {
            thread->prefix_stderr = _imp->prefix_stderr;
            if (! _imp->capture_stderr)
            {
                thread->own_capture_stderr.reset(new SafeOFStream(STDERR_FILENO, false));
                _imp->capture_stderr = *thread->own_capture_stderr;
            }
        }

        thread->extra_newlines_if_any_output_exists = _imp->extra_newlines_if_any_output_exists;

        if ((_imp->capture_stdout || _imp->capture_stderr) && _imp->use_ptys)
        {
            int tty(::open("/dev/tty", O_RDONLY | O_CLOEXEC));
            if (-1 == tty)
                Log::get_instance()->message("util.system.gwinsz.open_failed", ll_debug, lc_context)
                    << "open(/dev/tty) failed: " + std::string(std::strerror(errno));
            else
            {
                struct winsize ws;
                if (-1 == ::ioctl(tty, TIOCGWINSZ, &ws))
                    Log::get_instance()->message("util.system.gwinsz.ioctl_failed", ll_warning, lc_context)
                        << "ioctl(TIOCGWINSZ) failed: " + std::string(std::strerror(errno));
                else if (0 == ws.ws_col || 0 == ws.ws_row)
                {
                    static std::once_flag once;
                    std::call_once(once, [&] () {
                            Log::get_instance()->message("util.system.gwinsz.dodgy", ll_warning, lc_context)
                                << "Got zero for terminal columns and/or lines (" << ws.ws_col << "x" << ws.ws_row << "), ignoring";
                        });
                }
                else
                {
                    columns = ws.ws_col;
                    lines = ws.ws_row;
                }
                if (-1 == ::close(tty))
                    throw InternalError(PALUDIS_HERE, "close(/dev/tty) failed: " + std::string(std::strerror(errno)));
            }
            Log::get_instance()->message("util.system.gwinsz", ll_debug, lc_context)
                << "Terminal size is " << columns << "x" << lines;
            set_tty_size_envvars = true;
        }

        if (_imp->capture_stdout)
        {
            thread->capture_stdout = _imp->capture_stdout;
            if (_imp->use_ptys)
                thread->capture_stdout_pipe.reset(new Pty(true, columns, lines));
            else
                thread->capture_stdout_pipe.reset(new Pipe(true));
        }

        if (_imp->capture_stderr)
        {
            thread->capture_stderr = _imp->capture_stderr;
            if (_imp->use_ptys)
                thread->capture_stderr_pipe.reset(new Pty(true, columns, lines));
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

    int capture_output_to_fd_env_fd = -1;
    if (thread && thread->capture_output_to_fd_pipe && ! _imp->capture_output_to_fd_env_var.empty())
    {
        const int src_fd(thread->capture_output_to_fd_pipe->write_fd());
        const int tgt_fd(_imp->capture_output_to_fd_fd);

        if (-1 == tgt_fd)
            capture_output_to_fd_env_fd = src_fd;
        else
            capture_output_to_fd_env_fd = tgt_fd;
    }

    int send_input_to_fd_env_fd = -1;
    if (thread && thread->send_input_to_fd_pipe && ! _imp->send_input_to_fd_env_var.empty())
    {
        const int src_fd(thread->send_input_to_fd_pipe->read_fd());
        const int tgt_fd(_imp->send_input_to_fd_fd);

        if (-1 == tgt_fd)
            send_input_to_fd_env_fd = src_fd;
        else
            send_input_to_fd_env_fd = tgt_fd;
    }

    int pipe_command_handler_response_pipe_env_fd = -1;
    int pipe_command_handler_command_pipe_env_fd = -1;
    if (thread && thread->pipe_command_handler && ! _imp->pipe_command_handler_env_var.empty())
    {
        pipe_command_handler_response_pipe_env_fd = thread->pipe_command_handler_response_pipe->read_fd();
        pipe_command_handler_command_pipe_env_fd = thread->pipe_command_handler_command_pipe->write_fd();
    }

    if (-1 != capture_output_to_fd_env_fd)
        _imp->command.setenv(_imp->capture_output_to_fd_env_var, stringify(capture_output_to_fd_env_fd));
    if (-1 != send_input_to_fd_env_fd)
        _imp->command.setenv(_imp->send_input_to_fd_env_var, stringify(send_input_to_fd_env_fd));
    if (-1 != pipe_command_handler_response_pipe_env_fd)
        _imp->command.setenv(_imp->pipe_command_handler_env_var + "_READ_FD", stringify(pipe_command_handler_response_pipe_env_fd));
    if (-1 != pipe_command_handler_command_pipe_env_fd)
        _imp->command.setenv(_imp->pipe_command_handler_env_var + "_WRITE_FD", stringify(pipe_command_handler_command_pipe_env_fd));

    if (set_tty_size_envvars)
    {
        _imp->command.setenv("COLUMNS", stringify(columns));
        _imp->command.setenv("LINES", stringify(lines));
    }

    /* Prepare exec so we don't allocate after fork */
    _imp->command.exec_prepare();

    /* This pipe is used for error handling. It will be open until the
     * child process either fails to exec, in which case the error is sent
     * to the parent through it, or the child succeeds to exec, in which
     * case the pipe is closed through CLOEXEC and nothing is ever written
     * to it. The parent process will throw an appropriate exception if it
     * receives an error from the child. */
    Pipe error_pipe(true);
    const int err_fd = error_pipe.write_fd();

    /* Temporarily disable SIGINT and SIGTERM to this thread */
    sigset_t intandterm;
    sigemptyset(&intandterm);
    sigaddset(&intandterm, SIGINT);
    sigaddset(&intandterm, SIGTERM);
    if (0 != pthread_sigmask(SIG_BLOCK, &intandterm, nullptr))
        throw ProcessError("pthread_sigmask failed");

    pid_t child(fork());
    if (-1 == child)
        throw ProcessError("fork() failed: " + stringify(::strerror(errno)));
    else if ((0 == child) ^ _imp->as_main_process)
    {
        if (_imp->as_main_process)
        {
            int status;
            if (-1 == ::waitpid(child, &status, 0))
            {
                ExecError(ExecError::WAITPID_FAILED, errno).send(err_fd);
                _exit(1);
            }
        }

        /* clear any SIGINT or SIGTERM handlers we inherit, and unblock signals */
        struct sigaction act;
        sigemptyset(&act.sa_mask);
        act.sa_handler = SIG_DFL;
        act.sa_flags = 0;
        sigaction(SIGINT,  &act, nullptr);
        sigaction(SIGTERM, &act, nullptr);
        if (0 != pthread_sigmask(SIG_UNBLOCK, &intandterm, nullptr))
        {
            ExecError(ExecError::PTHREAD_SIGMASK_FAILED, 0).send(err_fd);
            _exit(1);
        }

        if (thread && thread->capture_stdout_pipe)
        {
            if (-1 == ::dup2(thread->capture_stdout_pipe->write_fd(), STDOUT_FILENO))
            {
                ExecError(ExecError::DUP2_FAILED, errno).send(err_fd);
                _exit(1);
            }
        }

        if (thread && thread->capture_stderr_pipe)
        {
            if (-1 == ::dup2(thread->capture_stderr_pipe->write_fd(), STDERR_FILENO))
            {
                ExecError(ExecError::DUP2_FAILED, errno).send(err_fd);
                _exit(1);
            }
        }

        if (thread && thread->capture_output_to_fd_pipe)
        {
            const int src_fd(thread->capture_output_to_fd_pipe->write_fd());
            const int tgt_fd(_imp->capture_output_to_fd_fd);

            if (-1 == tgt_fd)
            {
                int flags = ::fcntl(src_fd, F_GETFD);
                if (-1 == flags || -1 == ::fcntl(src_fd, F_SETFD, flags & ~FD_CLOEXEC))
                {
                    ExecError(ExecError::FCNTL_FAILED, errno).send(err_fd);
                    _exit(1);
                }
            }
            else
            {
                if (-1 == ::dup2(src_fd, tgt_fd))
                {
                    ExecError(ExecError::DUP2_FAILED, errno).send(err_fd);
                    _exit(1);
                }
            }
        }

        if (thread && thread->send_input_to_fd_pipe)
        {
            const int src_fd(thread->send_input_to_fd_pipe->read_fd());
            const int tgt_fd(_imp->send_input_to_fd_fd);

            if (-1 == tgt_fd)
            {
                int flags = ::fcntl(src_fd, F_GETFD);
                if (-1 == flags || -1 == ::fcntl(src_fd, F_SETFD, flags & ~FD_CLOEXEC))
                {
                    ExecError(ExecError::FCNTL_FAILED, errno).send(err_fd);
                    _exit(1);
                }
            }
            else
            {
                if (-1 == ::dup2(src_fd, tgt_fd))
                {
                    ExecError(ExecError::DUP2_FAILED, errno).send(err_fd);
                    _exit(1);
                }
            }
        }

        if (thread && thread->pipe_command_handler)
        {
            int read_fd = thread->pipe_command_handler_response_pipe->read_fd();
            int read_flags = ::fcntl(read_fd, F_GETFD);
            if (-1 == read_flags || -1 == ::fcntl(read_fd, F_SETFD, read_flags & ~FD_CLOEXEC))
            {
                ExecError(ExecError::FCNTL_FAILED, errno).send(err_fd);
                _exit(1);
            }

            int write_fd = thread->pipe_command_handler_command_pipe->write_fd();
            int write_flags = ::fcntl(write_fd, F_GETFD);
            if (-1 == write_flags || -1 == ::fcntl(write_fd, F_SETFD, write_flags & ~FD_CLOEXEC))
            {
                ExecError(ExecError::FCNTL_FAILED, errno).send(err_fd);
                _exit(1);
            }
        }

        if (-1 != _imp->set_stdin_fd)
        {
            if (-1 == ::dup2(_imp->set_stdin_fd, STDIN_FILENO))
            {
                ExecError(ExecError::DUP2_FAILED, errno).send(err_fd);
                _exit(1);
            }
        }

        _imp->command.exec(err_fd);
    }
    else
    {
        if (_imp->as_main_process)
        {
            /* Ignore CHLD. POSIX may or may not say that if we do this, our child will
             * not become a zombie. */
            struct sigaction act;
            sigemptyset(&act.sa_mask);
            act.sa_handler = SIG_IGN;
            act.sa_flags = 0;
            sigaction(SIGCHLD, &act, nullptr);

            pid_t p(fork());
            if (-1 == p)
                throw ProcessError("fork() failed: " + stringify(::strerror(errno)));
            else if (0 != p)
                _exit(0);
        }

        /* Restore SIGINT and SIGTERM handling */
        if (0 != pthread_sigmask(SIG_UNBLOCK, &intandterm, nullptr))
            throw ProcessError("pthread_sigmask failed");

        /* Close our end of the error pipe so we actually get EOF if the
         * child closes its end. */
        close(error_pipe.write_fd());
        error_pipe.clear_write_fd();

        ExecError error(error_pipe.read_fd());
        if (error)
        {
            if (-1 == ::waitpid_noeintr(child, nullptr, 0))
                throw ProcessError("waitpid() returned -1");

            throw ProcessError(error);
        }

        if (thread)
            thread->start();
        return RunningProcessHandle(_imp->as_main_process ? 0 : child, std::move(thread));
    }
}

Process &
Process::setenv(const std::string & name, const std::string & val)
{
    _imp->command.setenv(name, val);
    return *this;
}

Process &
Process::clearenv()
{
    _imp->command.clearenv();
    return *this;
}

Process &
Process::chdir(const FSPath & path)
{
    _imp->command.chdir(path);
    return *this;
}

Process &
Process::setuid_setgid(uid_t uid, gid_t gid)
{
    _imp->command.setuid_setgid(uid, gid);
    return *this;
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
Process::use_ptys()
{
    _imp->use_ptys = true;
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
Process::extra_newlines_if_any_output_exists()
{
    _imp->extra_newlines_if_any_output_exists = true;
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
                _imp->command.setenv("BASH_ENV", "/dev/null");
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

RunningProcessHandle::~RunningProcessHandle() noexcept(false)
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
        if (-1 == ::waitpid_noeintr(_imp->pid, &status, 0))
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

