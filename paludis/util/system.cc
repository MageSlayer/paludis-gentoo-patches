/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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

#include <cstdlib>
#include <paludis/util/system.hh>
#include <paludis/util/log.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/pipe.hh>
#include <paludis/util/pty.hh>

#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <fcntl.h>
#include <map>
#include <iostream>
#include <cstring>
#include "config.h"

using namespace paludis;

typedef std::map<std::string, std::string> CommandSetenvValues;

namespace paludis
{
    template <>
    struct WrappedForwardIteratorTraits<Command::ConstIteratorTag>
    {
        typedef CommandSetenvValues::const_iterator UnderlyingIterator;
    };
}

namespace
{
    static int stdout_write_fd = -1;
    static int stdout_close_fd = -1;

    static int stderr_write_fd = -1;
    static int stderr_close_fd = -1;

    pid_t get_paludis_pid()
    {
        pid_t result(0);
        std::string str;

        if (((str = getenv_with_default("PALUDIS_PID", ""))).empty())
        {
            result = getpid();
            setenv("PALUDIS_PID", stringify(result).c_str(), 1);
        }
        else
            result = destringify<pid_t>(str);

        return result;
    }


    static pid_t paludis_pid PALUDIS_ATTRIBUTE((used)) = get_paludis_pid();
}

void
paludis::set_run_command_stdout_fds(const int w, const int c)
{
    stdout_write_fd = w;
    stdout_close_fd = c;
}

void
paludis::set_run_command_stderr_fds(const int w, const int c)
{
    stderr_write_fd = w;
    stderr_close_fd = c;
}

GetenvError::GetenvError(const std::string & key) throw () :
    Exception("Environment variable '" + key + "' not set")
{
}

RunCommandError::RunCommandError(const std::string & our_message) throw () :
    Exception(our_message)
{
}

std::string
paludis::getenv_with_default(const std::string & key, const std::string & def)
{
    const char * const e(std::getenv(key.c_str()));
    return e ? e : def;
}

std::string
paludis::getenv_or_error(const std::string & key)
{
    const char * const e(std::getenv(key.c_str()));
    if (! e)
        throw GetenvError(key);
    return e;
}

namespace
{
    /**
     * Fetch the kernel version, for paludis::kernel_version.
     *
     * \ingroup grpsystem
     */
    std::string get_kernel_version()
    {
        struct utsname u;
        if (0 != uname(&u))
            throw InternalError(PALUDIS_HERE, "uname call failed");
        return u.release;
    }

    std::map<std::string, std::string> make_me_a_frickin_map_because_gcc_sucks()
    {
        return std::map<std::string, std::string>();
    }
}

std::string
paludis::kernel_version()
{
    static const std::string result(get_kernel_version());
    return result;
}

namespace paludis
{
    template<>
    struct Implementation<Command>
    {
        std::string command;
        bool clearenv;
        CommandSetenvValues setenv_values;
        std::string chdir;
        bool echo_to_stderr;
        std::tr1::shared_ptr<uid_t> uid;
        std::tr1::shared_ptr<gid_t> gid;
        std::string stdout_prefix;
        std::string stderr_prefix;
        bool prefix_discard_blank_output;
        bool prefix_blank_lines;
        std::tr1::function<std::string (const std::string &)> pipe_command_handler;
        std::ostream * captured_stdout_stream;
        std::ostream * captured_stderr_stream;
        std::istream * input_stream;
        int input_fd;
        std::string input_fd_env_var;
        bool ptys;

        Implementation(const std::string & c, bool cl = false,
                const std::map<std::string, std::string> & s = make_me_a_frickin_map_because_gcc_sucks(),
                const std::string & d = "", bool e = false,
                std::tr1::shared_ptr<uid_t> u = std::tr1::shared_ptr<uid_t>(),
                std::tr1::shared_ptr<gid_t> g = std::tr1::shared_ptr<gid_t>(),
                const std::string & p = "", const std::string & q = "",
                const bool b = false, const bool bb = false,
                const std::tr1::function<std::string (const std::string &)> & h = std::tr1::function<std::string (const std::string &)>(),
                std::ostream * cs = 0,
                std::ostream * ds = 0,
                std::istream * is = 0,
                int isf = -1,
                const std::string & isfe = "",
                const bool ps = false) :
            command(c),
            clearenv(cl),
            setenv_values(s),
            chdir(d),
            echo_to_stderr(e),
            uid(u),
            gid(g),
            stdout_prefix(p),
            stderr_prefix(q),
            prefix_discard_blank_output(b),
            prefix_blank_lines(bb),
            pipe_command_handler(h),
            captured_stdout_stream(cs),
            captured_stderr_stream(ds),
            input_stream(is),
            input_fd(isf),
            input_fd_env_var(isfe),
            ptys(ps)
        {
        }
    };
}

Command::Command(const std::string & s) :
    PrivateImplementationPattern<Command>(new Implementation<Command>(s))
{
}

Command::Command(const char * const s) :
    PrivateImplementationPattern<Command>(new Implementation<Command>(s))
{
}

Command::Command(const Command & other) :
    PrivateImplementationPattern<Command>(new Implementation<Command>(other._imp->command,
                other._imp->clearenv, other._imp->setenv_values, other._imp->chdir, other._imp->echo_to_stderr,
                other._imp->uid, other._imp->gid, other._imp->stdout_prefix, other._imp->stderr_prefix,
                other._imp->prefix_discard_blank_output,
                other._imp->prefix_blank_lines, other._imp->pipe_command_handler, other._imp->captured_stdout_stream,
                other._imp->captured_stderr_stream, other._imp->input_stream, other._imp->input_fd,
                other._imp->input_fd_env_var, other._imp->ptys))
{
}

const Command &
Command::operator= (const Command & other)
{
    if (this != &other)
    {
        _imp.reset(new Implementation<Command>(other._imp->command, other._imp->clearenv, other._imp->setenv_values,
                    other._imp->chdir, other._imp->echo_to_stderr,
                    std::tr1::shared_ptr<uid_t>(),
                    std::tr1::shared_ptr<gid_t>(),
                    other._imp->stdout_prefix,
                    other._imp->stderr_prefix,
                    other._imp->prefix_discard_blank_output,
                    other._imp->prefix_blank_lines,
                    other._imp->pipe_command_handler, other._imp->captured_stdout_stream,
                    other._imp->captured_stderr_stream,
                    other._imp->input_stream,
                    other._imp->input_fd,
                    other._imp->input_fd_env_var,
                    other._imp->ptys));
        if (other.uid() && other.gid())
            with_uid_gid(*other.uid(), *other.gid());
    }

    return *this;
}

Command::~Command()
{
}

Command &
Command::with_chdir(const FSEntry & c)
{
    _imp->chdir = stringify(c);
    return *this;
}

Command &
Command::with_setenv(const std::string & k, const std::string & v)
{
    _imp->setenv_values.insert(std::make_pair(k, v));
    return *this;
}

Command &
Command::with_clearenv()
{
    _imp->clearenv = true;
    return *this;
}

Command &
Command::with_uid_gid(const uid_t u, const gid_t g)
{
    _imp->uid.reset(new uid_t(u));
    _imp->gid.reset(new gid_t(g));
    return *this;
}

Command &
Command::with_captured_stdout_stream(std::ostream * const c)
{
    _imp->captured_stdout_stream = c;
    return *this;
}

Command &
Command::with_captured_stderr_stream(std::ostream * const c)
{
    _imp->captured_stderr_stream = c;
    return *this;
}

Command &
Command::with_input_stream(std::istream * const c, int fd, const std::string & e)
{
    _imp->input_stream = c;
    _imp->input_fd = fd;
    _imp->input_fd_env_var = e;
    return *this;
}

Command &
Command::with_ptys()
{
    _imp->ptys = true;
    return *this;
}

namespace
{
    bool check_cmd(const std::string & s)
    {
        bool result(0 == run_command(Command(s + " --version >/dev/null 2>/dev/null")));
        if (! result)
            Log::get_instance()->message("util.system.boxless", ll_warning, lc_context) <<
                "I don't seem to be able to use " + s;
        return result;
    }
}

Command &
Command::with_sandbox()
{
    static bool can_use_sandbox(check_cmd("sandbox"));

    if (can_use_sandbox)
    {
        if (! getenv_with_default("PALUDIS_DO_NOTHING_SANDBOXY", "").empty())
            Log::get_instance()->message("util.system.nothing_sandboxy", ll_debug, lc_no_context)
                << "PALUDIS_DO_NOTHING_SANDBOXY is set, not using sandbox";
        else if (! getenv_with_default("SANDBOX_ACTIVE", "").empty())
            Log::get_instance()->message("util.system.sandbox_in_sandbox", ll_warning, lc_no_context)
                << "Already inside sandbox, not spawning another sandbox instance";
        else
        {
            _imp->command = "sandbox " + _imp->command;
            if (getenv_with_default("BASH_ENV", "").empty())
                with_setenv("BASH_ENV", "/dev/null");
        }
    }

    return *this;
}

Command &
Command::with_sydbox()
{
    static bool can_use_sydbox(check_cmd("sydbox"));

    if (can_use_sydbox)
    {
        if (! getenv_with_default("PALUDIS_DO_NOTHING_SANDBOXY", "").empty())
            Log::get_instance()->message("util.system.nothing_sandboxy", ll_debug, lc_no_context)
                << "PALUDIS_DO_NOTHING_SANDBOXY is set, not using sydbox";
        else if (! getenv_with_default("SYDBOX_ACTIVE", "").empty())
            Log::get_instance()->message("util.system.sandbox_in_sandbox", ll_warning, lc_no_context)
                << "Already inside sydbox, not spawning another sydbox instance";
        else
            _imp->command = "sydbox --profile paludis -- " + _imp->command;
    }

    return *this;
}

std::tr1::shared_ptr<const uid_t>
Command::uid() const
{
    return _imp->uid;
}

std::tr1::shared_ptr<const gid_t>
Command::gid() const
{
    return _imp->gid;
}

namespace
{
    void wait_handler(int sig)
    {
        std::cerr << "Caught signal " << sig << " in run_command child process" << std::endl;
    }

    std::string build_extras(const Command & cmd)
    {
        std::string extras;

        if (! cmd.chdir().empty())
            extras.append(" [chdir " + cmd.chdir() + "]");

        if (cmd.clearenv())
            extras.append(" [clearenv]");

        for (Command::ConstIterator s(cmd.begin_setenvs()), s_end(cmd.end_setenvs()) ; s != s_end ; ++s)
            extras.append(" [setenv " + s->first + "=" + s->second + "]");

        if (cmd.gid() && *cmd.gid() != getgid())
            extras.append(" [setgid " + stringify(*cmd.gid()) + "]");

        if (cmd.uid() && *cmd.uid() != getuid())
            extras.append(" [setuid " + stringify(*cmd.uid()) + "]");

        return extras;
    }

    void do_chdir(const Command & cmd)
    {
        if (! cmd.chdir().empty())
            if (-1 == chdir(stringify(cmd.chdir()).c_str()))
                throw RunCommandError("chdir failed: " + stringify(strerror(errno)));
    }

    void do_env(const Command & cmd)
    {
        if (cmd.clearenv())
        {
            std::map<std::string, std::string> setenvs;
            for (const char * const * it(environ); 0 != *it; ++it)
            {
                std::string var(*it);
                if (std::string::npos != var.find('=') &&
                    ("PALUDIS_" == var.substr(0, 8) ||
                     "PATH=" == var.substr(0, 5) ||
                     "HOME=" == var.substr(0, 5) ||
                     "LD_LIBRARY_PATH=" == var.substr(0, 16)))
                    setenvs.insert(std::make_pair(var.substr(0, var.find('=')), var.substr(var.find('=') + 1)));
            }
            clearenv();
            for (std::map<std::string, std::string>::const_iterator it(setenvs.begin()),
                     it_end(setenvs.end()); it_end != it; ++it)
                setenv(it->first.c_str(), it->second.c_str(), 1);
        }

        for (Command::ConstIterator s(cmd.begin_setenvs()), s_end(cmd.end_setenvs()) ; s != s_end ; ++s)
            setenv(s->first.c_str(), s->second.c_str(), 1);

        /* This is what happens when people are allowed to play with
         * things they don't understand. Yay Gentoo! */
        setenv("PACKAGE_MANAGER", "paludis", 1);
        setenv("ESELECT_PACKAGE_MANAGER", "paludis", 1);
    }

    void do_uid_gid(const Command & cmd)
    {
        std::string command(cmd.command());

        if (cmd.gid() && *cmd.gid() != getgid())
        {
            gid_t g(*cmd.gid());

            if (0 != ::setgid(*cmd.gid()))
                std::cerr << "setgid(" << *cmd.uid() << ") failed for exec of '" << command << "': "
                    << strerror(errno) << std::endl;
            else if (0 != ::setgroups(1, &g))
                std::cerr << "setgroups failed for exec of '" << command << "': " << strerror(errno) << std::endl;
        }

        if (cmd.uid() && *cmd.uid() != getuid())
            if (0 != ::setuid(*cmd.uid()))
                std::cerr << "setuid(" << *cmd.uid() << ") failed for exec of '" << command << "': "
                    << strerror(errno) << std::endl;
    }
}

int
paludis::run_command(const Command & cmd)
{
    Context context("When running command '" + stringify(cmd.command()) + "':");

    std::string extras(build_extras(cmd));
    std::string command(cmd.command());

    if ((! cmd.stdout_prefix().empty()) || (! cmd.stderr_prefix().empty()))
        command = getenv_with_default("PALUDIS_OUTPUTWRAPPER_DIR", LIBEXECDIR "/paludis/utils") + "/outputwrapper --stdout-prefix '"
            + cmd.stdout_prefix() + "' --stderr-prefix '" + cmd.stderr_prefix() + "' "
            + (cmd.prefix_discard_blank_output() ? " --discard-blank-output " : "")
            + (cmd.prefix_blank_lines() ? " --wrap-blanks " : "")
            + " -- " + command;

    cmd.echo_to_stderr();
    Log::get_instance()->message("util.system.execl", ll_debug, lc_no_context) << "execl /bin/sh -c " << command
        << " " << extras;

    std::tr1::shared_ptr<Pipe> internal_command_reader(new Pipe), pipe_command_reader,
        pipe_command_response, input_stream;
    std::tr1::shared_ptr<Channel> captured_stdout, captured_stderr;
    if (cmd.pipe_command_handler())
    {
        pipe_command_reader.reset(new Pipe);
        pipe_command_response.reset(new Pipe);
    }
    if (cmd.captured_stdout_stream())
        captured_stdout.reset(cmd.ptys() ? static_cast<Channel *>(new Pty) : new Pipe);
    if (cmd.captured_stderr_stream())
        captured_stderr.reset(cmd.ptys() ? static_cast<Channel *>(new Pty) : new Pipe);
    if (cmd.input_stream())
    {
        input_stream.reset(new Pipe);
        int arg(fcntl(input_stream->write_fd(), F_GETFL, NULL));
        if (-1 == arg)
            throw RunCommandError("fcntl F_GETFL failed: " + stringify(strerror(errno)));
        arg |= O_NONBLOCK;
        if (-1 == fcntl(input_stream->write_fd(), F_SETFL, arg))
            throw RunCommandError("fcntl F_SETFL failed: " + stringify(strerror(errno)));
    }


    /* Why do we fork twice, rather than install a SIGCHLD handler that writes to a pipe that
     * our pselect watches? Simple: Unix is retarded. APUE 12.8 says "Each thread has its own signal
     * mask, but the signal disposition is shared by all threads in the process.", which means
     * we have to do crazy things... */

    /* Temporarily disable SIGINT and SIGTERM to this thread, so that we can set up signal
     * handlers. */
    sigset_t intandterm;
    sigemptyset(&intandterm);
    sigaddset(&intandterm, SIGINT);
    sigaddset(&intandterm, SIGTERM);
    if (0 != pthread_sigmask(SIG_BLOCK, &intandterm, 0))
        throw InternalError(PALUDIS_HERE, "pthread_sigmask failed");

    pid_t child(fork());
    if (0 == child)
    {
        pid_t child_child(fork());
        if (0 == child_child)
        {
            /* The pid that does the exec */

            /* clear any SIGINT or SIGTERM handlers we inherit, and unblock signals */
            struct sigaction act;
            act.sa_handler = SIG_DFL;
            act.sa_flags = 0;
            sigaction(SIGINT,  &act, 0);
            sigaction(SIGTERM, &act, 0);
            if (0 != pthread_sigmask(SIG_UNBLOCK, &intandterm, 0))
                std::cerr << "pthread_sigmask failed: " + stringify(strerror(errno)) + "'" << std::endl;
            try
            {
                if (cmd.pipe_command_handler())
                {
                    close(pipe_command_reader->read_fd());
                    pipe_command_reader->clear_read_fd();

                    close(pipe_command_response->write_fd());
                    pipe_command_response->clear_write_fd();
                }

                if (cmd.captured_stdout_stream())
                {
                    close(captured_stdout->read_fd());
                    captured_stdout->clear_read_fd();
                }

                if (cmd.captured_stderr_stream())
                {
                    close(captured_stderr->read_fd());
                    captured_stderr->clear_read_fd();
                }

                if (cmd.input_stream())
                {
                    close(input_stream->write_fd());
                    input_stream->clear_write_fd();
                }

                close(internal_command_reader->read_fd());
                internal_command_reader->clear_read_fd();
                close(internal_command_reader->write_fd());
                internal_command_reader->clear_write_fd();

                do_chdir(cmd);
                do_env(cmd);

                if (cmd.pipe_command_handler())
                {
                    setenv("PALUDIS_PIPE_COMMAND_WRITE_FD", stringify(pipe_command_reader->write_fd()).c_str(), 1);
                    setenv("PALUDIS_PIPE_COMMAND_READ_FD", stringify(pipe_command_response->read_fd()).c_str(), 1);
                }

                if (cmd.captured_stdout_stream())
                {
                    if (-1 == dup2(captured_stdout->write_fd(), 1))
                        throw RunCommandError("captured stdout dup2 failed: " + stringify(strerror(errno)));
                }
                else if (-1 != stdout_write_fd)
                {
                    if (-1 == dup2(stdout_write_fd, 1))
                        throw RunCommandError("stdout dup2 failed: " + stringify(strerror(errno)));

                    if (-1 != stdout_close_fd)
                        close(stdout_close_fd);
                }

                if (cmd.captured_stderr_stream())
                {
                    if (-1 == dup2(captured_stderr->write_fd(), 2))
                        throw RunCommandError("captured stderr dup2 failed: " + stringify(strerror(errno)));
                }
                else if (-1 != stderr_write_fd)
                {
                    if (-1 == dup2(stderr_write_fd, 2))
                        throw RunCommandError("stderr dup2 failed: " + stringify(strerror(errno)));

                    if (-1 != stderr_close_fd)
                        close(stderr_close_fd);
                }

                if (cmd.input_stream())
                {
                    int cmd_input_fd;

                    if (-1 == cmd.input_fd())
                        cmd_input_fd = dup(input_stream->read_fd());
                    else
                        cmd_input_fd = dup2(input_stream->read_fd(), cmd.input_fd());

                    if (-1 == cmd_input_fd)
                        throw RunCommandError("input dup2 failed: " + stringify(strerror(errno)));

                    if (! cmd.input_fd_env_var().empty())
                        setenv(cmd.input_fd_env_var().c_str(), stringify(cmd_input_fd).c_str(), 1);
                }

                do_uid_gid(cmd);

                execl("/bin/sh", "sh", "-c", command.c_str(), static_cast<char *>(0));
                throw RunCommandError("execl /bin/sh -c '" + command + "' failed:"
                        + stringify(strerror(errno)));
            }
            catch (const Exception & e)
            {
                std::cerr << "exec of '" << command << "' failed due to exception '" << e.message()
                    << "' (" << e.what() << ")" << std::endl;
                std::exit(123);
            }
            catch (...)
            {
                std::cerr << "exec of '" << command << "' failed due to unknown exception" << std::endl;
                std::exit(124);
            }
        }
        else if (-1 == child_child)
        {
            std::cerr << "fork failed: " + stringify(strerror(errno)) + "'" << std::endl;
            std::exit(125);
        }
        else
        {
            /* The pid that waits for the exec pid and then writes to the done pipe. */

            /* On SIGINT or SIGTERM, just output a notice. */
            struct sigaction act, old_act;
            act.sa_handler = &wait_handler;
            act.sa_flags = SA_RESTART;
            sigemptyset(&act.sa_mask);
            sigaddset(&act.sa_mask, SIGINT);
            sigaddset(&act.sa_mask, SIGTERM);

            sigaction(SIGINT, 0, &old_act);
            if (SIG_DFL != old_act.sa_handler && SIG_IGN != old_act.sa_handler)
                sigaction(SIGINT, &act, 0);
            sigaction(SIGTERM, 0, &old_act);
            if (SIG_DFL != old_act.sa_handler && SIG_IGN != old_act.sa_handler)
                sigaction(SIGTERM, &act, 0);

            if (0 != pthread_sigmask(SIG_UNBLOCK, &intandterm, 0))
                std::cerr << "pthread_sigmask failed: " + stringify(strerror(errno)) + "'" << std::endl;

            if (cmd.pipe_command_handler())
            {
                close(pipe_command_reader->read_fd());
                pipe_command_reader->clear_read_fd();
                close(pipe_command_reader->write_fd());
                pipe_command_reader->clear_write_fd();

                close(pipe_command_response->read_fd());
                pipe_command_response->clear_read_fd();
                close(pipe_command_response->write_fd());
                pipe_command_response->clear_write_fd();
            }

            if (cmd.captured_stdout_stream())
            {
                close(captured_stdout->read_fd());
                captured_stdout->clear_read_fd();
                close(captured_stdout->write_fd());
                captured_stdout->clear_write_fd();
            }

            if (cmd.captured_stderr_stream())
            {
                close(captured_stderr->read_fd());
                captured_stderr->clear_read_fd();
                close(captured_stderr->write_fd());
                captured_stderr->clear_write_fd();
            }

            if (cmd.input_stream())
            {
                close(input_stream->read_fd());
                input_stream->clear_read_fd();
                close(input_stream->write_fd());
                input_stream->clear_write_fd();
            }

            close(internal_command_reader->read_fd());
            internal_command_reader->clear_read_fd();

            int status(-1);

            stdout_write_fd = -1;
            stdout_close_fd = -1;
            stderr_write_fd = -1;
            stderr_close_fd = -1;

            int ret(-1);
            while (true)
            {
                if (-1 == waitpid(child_child, &status, 0))
                {
                    if (errno == EINTR)
                        std::cerr << "wait failed: '" + stringify(strerror(errno)) + "', trying once more" << std::endl;
                    else
                    {
                        std::cerr << "wait failed: '" + stringify(strerror(errno)) + "'" << std::endl;
                        break;
                    }
                }
                else
                {
                    ret = (WIFSIGNALED(status) ? WTERMSIG(status) + 128 : WEXITSTATUS(status));
                    break;
                }
            }

            {
                SafeOFStream stream(internal_command_reader->write_fd());
                stream << "EXIT " << ret << std::endl;
            }
        }

        _exit(0);
    }
    else if (-1 == child)
        throw RunCommandError("fork failed: " + stringify(strerror(errno)));
    else
    {
        /* Our original pid */

        /* Restore SIGINT and SIGTERM handling */
        if (0 != pthread_sigmask(SIG_UNBLOCK, &intandterm, 0))
            throw InternalError(PALUDIS_HERE, "pthread_sigmask failed");

        if (cmd.pipe_command_handler())
        {
            close(pipe_command_reader->write_fd());
            pipe_command_reader->clear_write_fd();
            close(pipe_command_response->read_fd());
            pipe_command_response->clear_read_fd();
        }

        if (cmd.captured_stdout_stream())
        {
            close(captured_stdout->write_fd());
            captured_stdout->clear_write_fd();
        }

        if (cmd.captured_stderr_stream())
        {
            close(captured_stderr->write_fd());
            captured_stderr->clear_write_fd();
        }

        if (cmd.input_stream())
        {
            close(input_stream->read_fd());
            input_stream->clear_read_fd();
        }

        close(internal_command_reader->write_fd());
        internal_command_reader->clear_write_fd();

        std::string pipe_command_buffer, internal_command_buffer;
        while (true)
        {
            fd_set read_fds, write_fds;
            FD_ZERO(&read_fds);
            FD_ZERO(&write_fds);
            int max_fd(0);

            if (cmd.pipe_command_handler())
            {
                FD_SET(pipe_command_reader->read_fd(), &read_fds);
                max_fd = std::max(max_fd, pipe_command_reader->read_fd());
            }

            if (cmd.captured_stdout_stream())
            {
                FD_SET(captured_stdout->read_fd(), &read_fds);
                max_fd = std::max(max_fd, captured_stdout->read_fd());
            }

            if (cmd.captured_stderr_stream())
            {
                FD_SET(captured_stderr->read_fd(), &read_fds);
                max_fd = std::max(max_fd, captured_stderr->read_fd());
            }

            if (cmd.input_stream() && -1 != input_stream->write_fd())
            {
                FD_SET(input_stream->write_fd(), &write_fds);
                max_fd = std::max(max_fd, input_stream->write_fd());
            }

            FD_SET(internal_command_reader->read_fd(), &read_fds);
            max_fd = std::max(max_fd, internal_command_reader->read_fd());

            timespec tv;
            tv.tv_sec = 5;
            tv.tv_nsec = 0;

            int retval(pselect(max_fd + 1, &read_fds, &write_fds, 0, &tv, 0));
            if (-1 == retval)
                throw RunCommandError("select failed: " + stringify(strerror(errno)));
            else if (0 == retval)
            {
                Log::get_instance()->message("util.system.wait", ll_debug, lc_context) << "Waiting for child " << child << " to finish";
                continue;
            }
            else
            {
                char buf[1024];

                if (cmd.captured_stdout_stream() && FD_ISSET(captured_stdout->read_fd(), &read_fds))
                {
                    int r;
                    if (((r = read(captured_stdout->read_fd(), buf, 1024))) > 0)
                    {
                        *cmd.captured_stdout_stream() << std::string(buf, r);
                        /* don't look at the other FDs yet to avoid a partial read from being snipped
                         * when capturing output */
                        continue;
                    }
                }

                if (cmd.captured_stderr_stream() && FD_ISSET(captured_stderr->read_fd(), &read_fds))
                {
                    int r;
                    if (((r = read(captured_stderr->read_fd(), buf, 1024))) > 0)
                    {
                        *cmd.captured_stderr_stream() << std::string(buf, r);
                        /* don't look at the other FDs yet to avoid a partial read from being snipped
                         * when capturing output */
                        continue;
                    }
                }

                if (cmd.pipe_command_handler() && FD_ISSET(pipe_command_reader->read_fd(), &read_fds))
                {
                    int r;
                    if (((r = read(pipe_command_reader->read_fd(), buf, 1024))) > 0)
                        pipe_command_buffer.append(std::string(buf, r));
                }

                if (FD_ISSET(internal_command_reader->read_fd(), &read_fds))
                {
                    int r;
                    if (((r = read(internal_command_reader->read_fd(), buf, 1024))) > 0)
                        internal_command_buffer.append(std::string(buf, r));
                }

                if (cmd.input_stream() && (-1 != input_stream->write_fd())
                        && FD_ISSET(input_stream->write_fd(), &write_fds))
                {
                    bool eof(! cmd.input_stream()->good());

                    while (! eof)
                    {
                        char c;
                        if (cmd.input_stream()->get(c).good())
                        {
                            int w(write(input_stream->write_fd(), &c, 1));
                            if (0 == w || (-1 == w && (errno == EAGAIN || errno == EWOULDBLOCK)))
                            {
                                cmd.input_stream()->unget();
                                break;
                            }
                            else if (-1 == w)
                                throw RunCommandError("write failed: " + stringify(strerror(errno)));
                        }
                        else
                            eof = true;
                    }

                    if (eof)
                    {
                        if (0 != close(input_stream->write_fd()))
                            throw RunCommandError("close failed: " + stringify(strerror(errno)));
                        input_stream->clear_write_fd();
                    }
                }
            }

            if (! pipe_command_buffer.empty())
                Log::get_instance()->message("util.system.pipe_command_buffer", ll_debug, lc_context) <<
                    "pipe_command_buffer is '" << pipe_command_buffer << "'";
            if (! internal_command_buffer.empty())
                Log::get_instance()->message("util.system.internal_command_buffer", ll_debug, lc_context) <<
                    "internal_command_buffer is '" << internal_command_buffer << "'";

            while (! pipe_command_buffer.empty())
            {
                std::string::size_type n_p(pipe_command_buffer.find('\0'));
                if (std::string::npos == n_p)
                    break;

                std::string op(pipe_command_buffer.substr(0, n_p));
                pipe_command_buffer.erase(0, n_p + 1);

                std::string response;
                if (cmd.pipe_command_handler())
                {
                    response = cmd.pipe_command_handler()(op);
                    Log::get_instance()->message("util.system.pipe_command_op", ll_debug, lc_context)
                        << "Pipe command op '" << op << "' response '" << response << "'";
                }
                else
                    Log::get_instance()->message("util.system.no_pipe_op_handler", ll_warning, lc_context)
                        << "Pipe command op '" << op << "' was requested but no handler defined. This is probably a bug...";

                ssize_t n(0);
                while (! response.empty())
                {
                    n = write(pipe_command_response->write_fd(), response.c_str(), response.length());
                    if (-1 == n)
                        throw InternalError(PALUDIS_HERE, "write failed: " + stringify(strerror(errno)));
                    else
                        response.erase(0, n);
                }

                char c(0);
                n = write(pipe_command_response->write_fd(), &c, 1);
                if (1 != n)
                    throw InternalError(PALUDIS_HERE, "write failed: " + stringify(strerror(errno)));
            }

            while (! internal_command_buffer.empty())
            {
                std::string::size_type n_p(internal_command_buffer.find('\n'));
                if (std::string::npos == n_p)
                    break;

                std::string op(internal_command_buffer.substr(0, n_p));
                internal_command_buffer.erase(0, n_p + 1);
                if (0 == op.compare(0, 4, "EXIT"))
                {
                    op.erase(0, 4);
                    int status(-1);
                    Log::get_instance()->message("util.system.exit_op", ll_debug, lc_context) << "Got exit op '" << op << "'";
                    if (-1 == waitpid(child, &status, 0))
                        std::cerr << "wait failed: " + stringify(strerror(errno)) + "'" << std::endl;
                    return destringify<int>(strip_leading(strip_trailing(op, " \r\n\t"), " \r\n\t"));
                }
                else
                    throw InternalError(PALUDIS_HERE, "unknown op '" + op + "' on internal_command_buffer");
            }
        }
    }

    throw InternalError(PALUDIS_HERE, "should never be reached");
}

void
paludis::become_command(const Command & cmd)
{
    Context context("When becoming command '" + stringify(cmd.command()) + "':");

    std::string extras(build_extras(cmd));
    std::string command(cmd.command());

    cmd.echo_to_stderr();
    Log::get_instance()->message("util.system.execl_parent", ll_debug, lc_no_context) << "execl /bin/sh -c " << command
        << " " << extras;

    /* The double fork with the ignoring CLD in the middle may or may not be
     * necessary, and it probably isn't, but POSIX appears to suggest that
     * doing this will guarantee that the feeding process won't be a zombie,
     * whereas it doesn't if we don't. Unless it doesn't. */

    /* Temporarily disable SIGINT and SIGTERM to this thread, so that we can set up signal
     * handlers. */
    sigset_t intandterm;
    sigemptyset(&intandterm);
    sigaddset(&intandterm, SIGINT);
    sigaddset(&intandterm, SIGTERM);
    if (0 != pthread_sigmask(SIG_BLOCK, &intandterm, 0))
        throw InternalError(PALUDIS_HERE, "pthread_sigmask failed");

    std::tr1::shared_ptr<Pipe> input_stream;
    if (cmd.input_stream())
    {
        input_stream.reset(new Pipe);

        int cmd_input_fd;

        if (-1 == cmd.input_fd())
            cmd_input_fd = dup(input_stream->read_fd());
        else
            cmd_input_fd = dup2(input_stream->read_fd(), cmd.input_fd());

        if (-1 == cmd_input_fd)
            throw RunCommandError("input dup2 failed: " + stringify(strerror(errno)));

        if (! cmd.input_fd_env_var().empty())
            setenv(cmd.input_fd_env_var().c_str(), stringify(cmd_input_fd).c_str(), 1);
    }

    pid_t child(fork());
    if (0 == child)
    {
        if (cmd.input_stream())
        {
            close(input_stream->read_fd());
            input_stream->clear_read_fd();
        }

        /* Ignore CLD. POSIX may or may not say that if we do this, our child will
         * not become a zombie. */
        struct sigaction act;
        act.sa_handler = SIG_IGN;
        act.sa_flags = 0;
        sigaction(SIGCLD, &act, 0);

        pid_t child_child(fork());
        if (0 == child_child)
        {
            /* Restore SIGINT and SIGTERM handling */
            if (0 != pthread_sigmask(SIG_UNBLOCK, &intandterm, 0))
                throw InternalError(PALUDIS_HERE, "pthread_sigmask failed");

            /* Feed in any input things */
            if (cmd.input_stream())
            {
                while (true)
                {
                    char c;
                    if (cmd.input_stream()->get(c).good())
                    {
                        int w(write(input_stream->write_fd(), &c, 1));
                        if (w != 1)
                            throw RunCommandError("write failed: " + stringify(strerror(errno)));
                    }
                    else
                        break;
                }

                if (0 != close(input_stream->write_fd()))
                    throw RunCommandError("close failed: " + stringify(strerror(errno)));
                input_stream->clear_write_fd();
            }

            /* we're done */
            _exit(0);
        }
        else
        {
            _exit(0);
        }
    }
    else if (-1 == child)
        throw RunCommandError("fork failed: " + stringify(strerror(errno)));
    else
    {
        /* Our original pid, which gets exec()ed */

        if (cmd.input_stream())
        {
            close(input_stream->write_fd());
            input_stream->clear_write_fd();
        }

        /* clear any SIGINT or SIGTERM handlers we inherit, and unblock signals */
        struct sigaction act;
        act.sa_handler = SIG_DFL;
        act.sa_flags = 0;
        sigaction(SIGINT,  &act, 0);
        sigaction(SIGTERM, &act, 0);
        if (0 != pthread_sigmask(SIG_UNBLOCK, &intandterm, 0))
            std::cerr << "pthread_sigmask failed: " + stringify(strerror(errno)) + "'" << std::endl;

        /* wait until the child is done */
        while (true)
        {
            int status(-1);
            if (-1 == waitpid(child, &status, 0))
            {
                if (errno == EINTR)
                    std::cerr << "wait failed: '" + stringify(strerror(errno)) + "', trying once more" << std::endl;
                else
                {
                    std::cerr << "wait failed: '" + stringify(strerror(errno)) + "'" << std::endl;
                    break;
                }
            }
            else
                break;
        }

        try
        {
            do_chdir(cmd);
            do_env(cmd);
            do_uid_gid(cmd);

            execl("/bin/sh", "sh", "-c", command.c_str(), static_cast<char *>(0));
            throw RunCommandError("execl /bin/sh -c '" + command + "' failed:"
                    + stringify(strerror(errno)));
        }
        catch (const Exception & e)
        {
            std::cerr << "exec of '" << command << "' failed due to exception '" << e.message()
                << "' (" << e.what() << ")" << std::endl;
            std::exit(123);
        }
        catch (...)
        {
            std::cerr << "exec of '" << command << "' failed due to unknown exception" << std::endl;
            std::exit(124);
        }
    }

    throw InternalError(PALUDIS_HERE, "should never be reached");
}

std::string
Command::command() const
{
    return _imp->command;
}

std::string
Command::chdir() const
{
    return _imp->chdir;
}

bool
Command::clearenv() const
{
    return _imp->clearenv;
}

Command::ConstIterator
Command::begin_setenvs() const
{
    return ConstIterator(_imp->setenv_values.begin());
}

Command::ConstIterator
Command::end_setenvs() const
{
    return ConstIterator(_imp->setenv_values.end());
}

void
Command::echo_to_stderr() const
{
    if (! _imp->echo_to_stderr)
        return;

    std::cerr << command().c_str() << std::endl;
}

Command &
Command::with_echo_to_stderr()
{
    _imp->echo_to_stderr = true;
    return *this;
}

Command &
Command::with_prefix_discard_blank_output()
{
    _imp->prefix_discard_blank_output = true;
    return *this;
}

Command &
Command::with_prefix_blank_lines()
{
    _imp->prefix_blank_lines = true;
    return *this;
}

Command &
Command::with_stdout_prefix(const std::string & s)
{
    _imp->stdout_prefix = s;
    return *this;
}

Command &
Command::with_stderr_prefix(const std::string & s)
{
    _imp->stderr_prefix = s;
    return *this;
}

Command &
Command::with_pipe_command_handler(const std::tr1::function<std::string (const std::string &)> & f)
{
    _imp->pipe_command_handler = f;
    return *this;
}

std::string
Command::stdout_prefix() const
{
    return _imp->stdout_prefix;
}

std::string
Command::stderr_prefix() const
{
    return _imp->stderr_prefix;
}

bool
Command::prefix_discard_blank_output() const
{
    return _imp->prefix_discard_blank_output;
}

bool
Command::prefix_blank_lines() const
{
    return _imp->prefix_blank_lines;
}

const std::tr1::function<std::string (const std::string &)> &
Command::pipe_command_handler() const
{
    return _imp->pipe_command_handler;
}

std::ostream *
Command::captured_stdout_stream() const
{
    return _imp->captured_stdout_stream;
}

std::ostream *
Command::captured_stderr_stream() const
{
    return _imp->captured_stderr_stream;
}

std::istream *
Command::input_stream() const
{
    return _imp->input_stream;
}

int
Command::input_fd() const
{
    return _imp->input_fd;
}

const std::string
Command::input_fd_env_var() const
{
    return _imp->input_fd_env_var;
}

bool
Command::ptys() const
{
    return _imp->ptys;
}

std::string
paludis::get_user_name(const uid_t u)
{
    const struct passwd * const p(getpwuid(u));
    if (p)
        return stringify(p->pw_name);
    else
    {
        Context c("When getting user name for uid '" + stringify(u) + "':");
        Log::get_instance()->message("util.system.getpwuid", ll_warning, lc_context) <<
            "getpwuid(" << u << ") returned null";
        return stringify(u);
    }
}

std::string
paludis::get_group_name(const gid_t u)
{
    const struct group * const p(getgrgid(u));
    if (p)
        return stringify(p->gr_name);
    else
    {
        Context c("When getting group name for gid '" + stringify(u) + "':");
        Log::get_instance()->message("util.system.getgrgid", ll_warning, lc_context) <<
            "getgrgid(" << u << + ") returned null";
        return stringify(u);
    }
}

template class WrappedForwardIterator<Command::ConstIteratorTag, const std::pair<const std::string, std::string> >;

