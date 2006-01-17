/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
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

#include "test_runner.hh"

#include "test_runner.hh"
#include "test_framework.hh"
#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>

/** \file
 * Implementation of the default test runner.
 *
 * \ingroup Test
 */

using namespace test;

/**
 * Called if we take too long.
 */
void
timeout_handler(int)
{
    std::cerr << "Exiting due to timeout!" << std::endl;
    std::exit(EXIT_FAILURE);
}

int
main(int, char * argv[])
{
    signal(SIGALRM, &timeout_handler);
    std::cout << "Test program " << argv[0] << ":" << std::endl;
    return TestCaseList::run_tests() ? EXIT_SUCCESS : EXIT_FAILURE;
}

