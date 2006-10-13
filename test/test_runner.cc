/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "test_framework.hh"
#include "test_runner.hh"
#include <paludis/util/stringify.hh>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <signal.h>
#if defined(__GLIBC__)
#include <execinfo.h>
#endif
#include <unistd.h>
#include <sys/types.h>

/** \file
 * Implementation of the default test runner.
 *
 * \ingroup grptestframework
 */

using namespace test;

namespace
{
    void do_backtrace()
    {
#if defined(__GLIBC__)
        void * bt[50];
        size_t sz = backtrace(bt, 50);
        char * * symbols = backtrace_symbols(bt, sz);

        std::cerr << "Stack dump:" << std::endl;
        for (unsigned n(0) ; n < sz ; ++n)
            std::cerr << "  * " << symbols[n] << std::endl;

        std::free(symbols);
#endif
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
    {
        std::ifstream ppid(("/proc/" + paludis::stringify(getppid()) + "/cmdline").c_str());
        if (ppid)
        {
            std::string cmd;
            std::getline(ppid, cmd, '\0');
            std::string::size_type slash_pos(cmd.rfind('/'));
            if (std::string::npos != slash_pos)
                cmd.erase(0, slash_pos);
            if (cmd != "gdb")
            {
                signal(SIGALRM, &timeout_handler);
                signal(SIGSEGV, &segfault_handler);
            }
            else
                TestCaseList::use_alarm = false;
        }
    }

    std::cout << "Test program " << argv[0] << ":" << std::endl;
    return TestCaseList::run_tests() ? EXIT_SUCCESS : EXIT_FAILURE;
}

