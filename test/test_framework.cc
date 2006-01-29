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

#include "test_framework.hh"
#include <paludis/attributes.hh>
#include <iostream>
#include <algorithm>
#include <unistd.h>

/** \file
 * Implementation for test framework classes.
 *
 * \ingroup Test
 */

using namespace test;

/**
 * Default implementation of exception_to_debug_string, that can be overridden
 * by other libraries.
 *
 * \ingroup Test
 */
std::string exception_to_debug_string(const std::exception &) PALUDIS_ATTRIBUTE((weak,noinline));

std::string exception_to_debug_string(const std::exception & e)
{
    return e.what() + std::string(" (no further information)");
}

#ifndef DOXYGEN

std::list<std::string>
TestMessageSuffix::_suffixes;

std::string
TestMessageSuffix::suffixes()
{
    std::string result;
    std::list<std::string>::const_iterator i(_suffixes.begin()), end(_suffixes.end());
    while (i != end)
    {
        result += *i++;
        if (end != i)
            result += ", ";
    }
    return result;
}

TestMessageSuffix::TestMessageSuffix(const std::string & s, bool write)
{
    _suffixes.push_back(s);
    if (write)
        std::cout << "[" << s << "]" << std::flush;
}

struct TestCase::Impl
{
    const std::string name;

    Impl(const std::string & the_name) :
        name(the_name)
    {
    }
};

TestCase::TestCase(const std::string & name) :
    _impl(new Impl(name))
{
    TestCaseList::register_test_case(this);
}

TestCase::~TestCase()
{
}

void
TestCase::check(const char * const function, const char * const file,
        const long line, bool was_ok, const std::string & message) const
{
    std::cout << "." << std::flush;
    if (! was_ok)
        throw TestFailedException(function, file, line, message);
}

void
TestCase::call_run()
{
    try
    {
        run();
    }
    catch (TestFailedException)
    {
        throw;
    }
    catch (std::exception &e)
    {
        throw TestFailedException(__PRETTY_FUNCTION__, __FILE__, __LINE__,
                "Test threw unexpected exception " + exception_to_debug_string(e));
    }
    catch (...)
    {
        throw TestFailedException(__PRETTY_FUNCTION__, __FILE__, __LINE__,
                "Test threw unexpected unknown exception");
    }
}

std::string
TestCase::name() const
{
    return _impl->name;
}

TestFailedException::TestFailedException(const char * const function, const char * const file,
        const long line, const std::string & message) throw () :
    _message(paludis::stringify(file) + ":" + paludis::stringify(line) + ": in " +
            paludis::stringify(function) + ": " + message + (
                TestMessageSuffix::suffixes().empty() ? std::string("") : " [context: " +
                TestMessageSuffix::suffixes() + "]"))
{
}

TestFailedException::~TestFailedException() throw ()
{
}

TestCaseList::TestCaseList()
{
}

TestCaseList::~TestCaseList()
{
}

class RunTest
{
    private:
        bool * const _had_a_failure;

    public:
        RunTest(bool * had_a_failure) :
            _had_a_failure(had_a_failure)
        {
        }

        void operator() (TestCase * test_case) const;
};

void
RunTest::operator() (TestCase * test_case) const
{
    bool had_local_failure(false);

    std::cout << "* \"" << test_case->name() << "\": " << std::flush;

    for (int repeat = 0 ; repeat < 2 ; ++repeat)
    {
        if (0 != repeat)
            std::cout << "  (repeat): " << std::flush;

        try
        {
            alarm(test_case->max_run_time());
            test_case->call_run();
            alarm(0);
        }
        catch (std::exception &e)
        {
            std::cout << "!{" << std::endl << exception_to_debug_string(e) <<
                std::endl << "  } " << std::flush;
            had_local_failure = true;
            *_had_a_failure = true;
        }
        catch (...)
        {
            std::cout << "!{Unknown exception type} ";
            had_local_failure = true;
            *_had_a_failure = true;
        }

        if (had_local_failure)
            std::cout << " NOT OK";
        else
            std::cout << " OK";

        std::cout << std::endl;

        if (! test_case->repeatable())
            break;
    }
}

bool
TestCaseList::run_tests()
{
    bool had_a_failure(_get_test_case_list().empty());

    std::for_each(
            TestCaseList::_get_test_case_list().begin(),
            TestCaseList::_get_test_case_list().end(),
            RunTest(&had_a_failure));

    return ! had_a_failure;
}

#endif

