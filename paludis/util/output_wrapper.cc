/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2010 Ciaran McCreesh
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
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <cstdlib>
#include <sys/types.h>

int
main(int argc, char *argv[])
{
    int argi(1);
    for ( ; argi < argc ; ++argi)
    {
        std::string s(argv[argi]);
        if (s == "--")
        {
            ++argi;
            break;
        }
    }

    if (argi >= argc)
    {
        std::cerr << argv[0] << ": no -- found" << std::endl;
        return EXIT_FAILURE;
    }

    execvp(argv[argi], &argv[argi]);
    std::cerr << argv[0] << ": execvp failed: " << std::strerror(errno) << std::endl;
    return EXIT_FAILURE;
}

