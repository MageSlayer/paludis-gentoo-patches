/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2009 Ciaran McCreesh
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
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <iostream>

int
main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " write_fd read_fd" << std::endl;
        return EXIT_FAILURE;
    }

    int write_fd(std::atoi(argv[1])), read_fd(std::atoi(argv[2]));
    if (0 != ::lockf(write_fd, F_LOCK, 0))
    {
        std::cerr << "Error: " << argv[0] << ": lockf failed with " << ::strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    /* copy stdin to the pipe read buffer */
    char buf[1024];
    int c, w;
    while (((c = read(0, buf, 1024))) > 0)
    {
        char * buf_p(buf);
        while (((w = write(write_fd, buf_p, c))) > 0)
        {
            buf_p += w;
            c -= w;
            if (c == 0)
                break;
        }
        if (w == -1)
        {
            std::cerr << "Error: " << argv[0] << ": write failed with " << ::strerror(errno) << std::endl;
            return EXIT_FAILURE;
        }
    }

    if (c == -1)
    {
        std::cerr << "Error: " << argv[0] << ": read failed with " << ::strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    /* append an 'end' marker to the pipe read buffer */
    buf[0] = '\0';
    while (((w = write(write_fd, buf, 1))) == 0)
        sleep(0);
    if (w == -1)
    {
        std::cerr << "Error: " << argv[0] << ": write failed with " << ::strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    /* copy write buffer to stdout until we get a null, and discard that null */
    while (((c = read(read_fd, buf, 1024))) > 0)
    {
        bool done(false);
        if (buf[c - 1] == '\0')
        {
            done = true;
            --c;
        }

        char * buf_p(buf);
        while (((w = write(1, buf_p, c))) > 0)
        {
            c -= w;
            buf_p += w;
            if (c == 0)
                break;
        }

        if (w == -1)
        {
            std::cerr << "Error: " << argv[0] << ": write failed with " << ::strerror(errno) << std::endl;
            return EXIT_FAILURE;
        }

        if (done)
            break;
    }

    if (c == -1)
    {
        std::cerr << "Error: " << argv[0] << ": read failed with " << ::strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    if (0 != ::lockf(write_fd, F_ULOCK, 0))
    {
        std::cerr << "Error: " << argv[0] << ": lockf unlock failed with " << ::strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

