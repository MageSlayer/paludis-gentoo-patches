/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#include <string>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>

int
main(int argc, char *argv[])
{
    int argi(1);
    std::string stdout_prefix, stderr_prefix;
    bool wrap_blanks(false);
    bool discard_blank_output(false);

    for ( ; argi < argc ; ++argi)
    {
        std::string s(argv[argi]);
        if (s == "--")
        {
            ++argi;
            break;
        }
        else if (s == "--wrap-blanks")
            wrap_blanks = true;
        else if (s == "--discard-blank-output")
            discard_blank_output = true;
        else if (s == "--stdout-prefix")
        {
            if (++argi >= argc)
            {
                std::cerr << argv[0] << ": no argument for --stdout-prefix" << std::endl;
                return EXIT_FAILURE;
            }
            stdout_prefix = argv[argi];
        }
        else if (s == "--stderr-prefix")
        {
            if (++argi >= argc)
            {
                std::cerr << argv[0] << ": no argument for --stderr-prefix" << std::endl;
                return EXIT_FAILURE;
            }
            stderr_prefix = argv[argi];
        }
        else
        {
            std::cerr << argv[0] << ": bad argument '" << s << "'" << std::endl;
            return EXIT_FAILURE;
        }
    }

    if (argi >= argc)
    {
        std::cerr << argv[0] << ": no -- found" << std::endl;
        return EXIT_FAILURE;
    }

    int stdout_pipes[2], stderr_pipes[2];

    if (0 != pipe(stdout_pipes))
    {
        std::cerr << argv[0] << ": pipe failed: " << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    if (0 != pipe(stderr_pipes))
    {
        std::cerr << argv[0] << ": pipe failed: " << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    pid_t pid(fork());

    if (0 == pid)
    {
        if (-1 == dup2(stdout_pipes[1], 1))
        {
            std::cerr << argv[0] << ": dup2 failed: " << strerror(errno) << std::endl;
            return EXIT_FAILURE;
        }
        close(stdout_pipes[0]);

        if (-1 == dup2(stderr_pipes[1], 2))
        {
            std::cerr << argv[0] << ": dup2 failed: " << strerror(errno) << std::endl;
            return EXIT_FAILURE;
        }
        close(stderr_pipes[0]);

        execvp(argv[argi], &argv[argi]);
        std::cerr << argv[0] << ": execvp failed: " << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }
    else if (-1 == pid)
    {
        std::cerr << argv[0] << ": fork failed: " << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }
    else
    {
        close(stdout_pipes[1]);
        close(stderr_pipes[1]);
        bool stdout_done(false), stdout_prefix_shown(false), stdout_had_interesting_char(false),
             stderr_done(false), stderr_prefix_shown(false), stderr_had_interesting_char(false),
             stdout_had_non_blanks(! discard_blank_output), stderr_had_non_blanks(! discard_blank_output);
        unsigned stdout_blanks(0), stderr_blanks(0);
        while ((! stdout_done) || (! stderr_done))
        {
            fd_set fds;
            int ret;

            FD_ZERO(&fds);
            FD_SET(stdout_pipes[0], &fds);
            FD_SET(stderr_pipes[0], &fds);

            ret = select(std::max(stdout_pipes[0], stderr_pipes[0]) + 1, &fds, 0, 0, 0);
            if (-1 == ret)
            {
                std::cerr << argv[0] << ": select failed: " << strerror(errno) << std::endl;
                return EXIT_FAILURE;
            }
            else if (ret)
            {
                char buf[1024];
                if (FD_ISSET(stdout_pipes[0], &fds))
                {
                    int c;
                    if (0 >= ((c = read(stdout_pipes[0], buf, 1024))))
                        stdout_done = true;
                    else
                    {
                        std::string to_write(buf, c);

                        for (std::string::size_type p(0) ; p < to_write.length() ; ++p)
                        {
                            if (to_write.at(p) != '\n')
                            {
                                if (! stdout_had_non_blanks)
                                {
                                    for (unsigned x(0) ; x < stdout_blanks ; ++x)
                                        if (wrap_blanks)
                                            std::cout << stdout_prefix << std::endl;
                                        else
                                            std::cout << std::endl;
                                    stdout_had_non_blanks = true;
                                }
                                stdout_had_interesting_char = true;
                            }
                            else if (! stdout_had_non_blanks)
                            {
                                ++stdout_blanks;
                                continue;
                            }

                            if (! stdout_prefix_shown)
                            {
                                if (stdout_had_interesting_char || wrap_blanks)
                                    std::cout << stdout_prefix;
                                stdout_prefix_shown = true;
                            }

                            if (to_write.at(p) == '\n')
                            {
                                stdout_had_interesting_char = false;
                                stdout_prefix_shown = false;
                            }

                            std::cout << to_write.at(p);
                        }

                        std::cout << std::flush;
                    }
                }

                if (FD_ISSET(stderr_pipes[0], &fds))
                {
                    int c;
                    if (0 >= ((c = read(stderr_pipes[0], buf, 1024))))
                        stderr_done = true;
                    else
                    {
                        std::string to_write(buf, c);

                        for (std::string::size_type p(0) ; p < to_write.length() ; ++p)
                        {
                            if (to_write.at(p) != '\n')
                            {
                                if (! stderr_had_non_blanks)
                                {
                                    for (unsigned x(0) ; x < stderr_blanks ; ++x)
                                        if (wrap_blanks)
                                            std::cerr << stderr_prefix << std::endl;
                                        else
                                            std::cerr << std::endl;
                                    stderr_had_non_blanks = true;
                                }
                                stderr_had_interesting_char = true;
                            }
                            else if (! stderr_had_non_blanks)
                            {
                                ++stderr_blanks;
                                continue;
                            }

                            if (! stderr_prefix_shown)
                            {
                                if (stderr_had_interesting_char || wrap_blanks)
                                    std::cerr << stderr_prefix;
                                stderr_prefix_shown = true;
                            }

                            if (to_write.at(p) == '\n')
                            {
                                stderr_had_interesting_char = false;
                                stderr_prefix_shown = false;
                            }

                            std::cerr << to_write.at(p);
                        }

                        std::cerr << std::flush;
                    }
                }
            }
            else
                break;
        }

        int status(-1);
        if (-1 == wait(&status))
        {
            std::cerr << argv[0] << ": wait failed: " << strerror(errno) << std::endl;
            return EXIT_FAILURE;
        }
        return WIFSIGNALED(status) ? WTERMSIG(status) + 128 : WEXITSTATUS(status);
    }
}

