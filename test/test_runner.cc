/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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
#include <exception>
#include <signal.h>
#if defined(__GLIBC__)
#include <execinfo.h>
#endif
#include <unistd.h>
#include <sys/types.h>

#include "config.h"
#ifdef HAVE_CXA_DEMANGLE
#  include <cxxabi.h>
#endif

/** \file
 * Imp of the default test runner.
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
        int sz = backtrace(bt, 50);
        char * * symbols = backtrace_symbols(bt, sz);

        std::cerr << "Stack dump:" << std::endl;
        for (int n(0) ; n < sz ; ++n)
        {
            std::string sym(symbols[n]);

#ifdef HAVE_CXA_DEMANGLE
            std::string::size_type p, q;
            if (std::string::npos != ((p = sym.find("(_Z"))) && std::string::npos != ((q = sym.find("+0x", p))))
            {
                ++p;
                int status(0);
                char * const name(abi::__cxa_demangle(sym.substr(p, q - p).c_str(), 0, 0, &status));
                if (0 == status)
                {
                    sym = sym.substr(0, p) + name + sym.substr(q);
                    std::free(name);
                }
            }
#endif

            std::cerr << "  * " << sym << std::endl;
        }

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
    try
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
    catch (const std::exception & e)
    {
        std::cout << "Uncaught exception " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}

