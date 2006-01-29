/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "test_runner.hh"
#include "test_framework.hh"

#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>

/** \file
 * Implementation of the default test runner.
 *
 * \ingroup Test
 */

using namespace test;

namespace
{
    void do_backtrace()
    {
        void * bt[50];
        size_t sz = backtrace(bt, 50);
        char * * symbols = backtrace_symbols(bt, sz);

        std::cerr << "Stack dump:" << std::endl;
        for (unsigned n(0) ; n < sz ; ++n)
            std::cerr << "  * " << symbols[n] << std::endl;

        std::free(symbols);
    }
}

void timeout_handler(int)
{
    std::cerr << std::endl << "Test aborted due to timeout!" << std::endl;
    do_backtrace();
    std::exit(EXIT_FAILURE);
}

void segfault_handler(int)
{
    std::cerr << std::endl << "Test aborted due to segmentation fault!" << std::endl;
    do_backtrace();
    std::exit(EXIT_FAILURE);
}

int
main(int, char * argv[])
{
    signal(SIGALRM, &timeout_handler);
    signal(SIGSEGV, &segfault_handler);

    std::cout << "Test program " << argv[0] << ":" << std::endl;
    return TestCaseList::run_tests() ? EXIT_SUCCESS : EXIT_FAILURE;
}

