/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2009, 2010, 2011 Ciaran McCreesh
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
#include <algorithm>
#include <iostream>
#include <list>
#include <paludis/util/attributes.hh>
#include <paludis/util/pimp-impl.hh>
#include <unistd.h>
#include <cstdlib>
#include <sys/time.h>

/** \file
 * Imp for test framework classes.
 *
 * \ingroup grptestframework
 */

using namespace test;

#ifndef DOXYGEN
namespace
{
    std::string exception_to_debug_string(const std::exception & e)
    {
        return e.what() + std::string(" (no further information)");
    }

    class DebugStringHolder
    {
        private:
            std::string (*_f) (const std::exception &);

            DebugStringHolder() :
                _f(&exception_to_debug_string)
            {
            }

        public:
            static DebugStringHolder * get_instance()
            {
                static DebugStringHolder _instance;
                return &_instance;
            }

            void set(std::string (*f) (const std::exception &))
            {
                _f = f;
            }

            std::string (*get() const) (const std::exception &)
            {
                return _f;
            }
    };
}

void
test::set_exception_to_debug_string(std::string (*f) (const std::exception &))
{
    DebugStringHolder::get_instance()->set(f);
}

std::string (* test::get_exception_to_debug_string()) (const std::exception &)
{
    return DebugStringHolder::get_instance()->get();
}

namespace paludis
{
    template<>
    struct Imp<TestMessageSuffix>
    {
        static std::list<std::string> suffixes;
    };

    std::list<std::string> Imp<TestMessageSuffix>::suffixes;
}

std::string
TestMessageSuffix::suffixes()
{
    std::string result;
    std::list<std::string>::const_iterator i(paludis::Imp<TestMessageSuffix>::suffixes.begin()),
        end(paludis::Imp<TestMessageSuffix>::suffixes.end());

    while (i != end)
    {
        result += *i++;
        if (end != i)
            result += ", ";
    }
    return result;
}

TestMessageSuffix::TestMessageSuffix(const std::string & s, bool write) :
    _imp()
{
    paludis::Imp<TestMessageSuffix>::suffixes.push_back(s);
    if (write)
        std::cout << "[" << s << "]" << std::flush;
}

TestMessageSuffix::~TestMessageSuffix()
{
    paludis::Imp<TestMessageSuffix>::suffixes.pop_back();
}

namespace paludis
{
    template<>
    struct Imp<TestCase>
    {
        const std::string name;
        mutable const char * reached_file;
        mutable const char * reached_function;
        mutable long reached_line;

        Imp(const std::string & the_name) :
            name(the_name),
            reached_file(NULL),
            reached_function(NULL),
            reached_line(0)
        {
        }
    };
}

TestCase::TestCase(const std::string & our_name) :
    _imp(our_name)
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
TestCase::reached(const char * const file, const char * const function,
        const long line) const
{
    _imp->reached_file = file;
    _imp->reached_function = function;
    _imp->reached_line = line;
}

void
TestCase::call_run()
{
    try
    {
        _imp->reached_file = NULL;
        _imp->reached_function = NULL;
        _imp->reached_line = 0;

        run();
    }
    catch (TestFailedException)
    {
        throw;
    }
    catch (std::exception &e)
    {
        throw TestFailedException(_imp->reached_function ? _imp->reached_file : __PRETTY_FUNCTION__,
                _imp->reached_file ? _imp->reached_file : __FILE__, _imp->reached_line ? _imp->reached_line : __LINE__,
                "Test threw unexpected exception " + (DebugStringHolder::get_instance()->get())(e));
    }
    catch (...)
    {
        throw TestFailedException(_imp->reached_function ? _imp->reached_file : __PRETTY_FUNCTION__,
                _imp->reached_file ? _imp->reached_file : __FILE__, _imp->reached_line ? _imp->reached_line : __LINE__,
                "Test threw unexpected unknown exception");
    }
}

std::string
TestCase::name() const
{
    return _imp->name;
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

namespace
{
    std::list<TestCase *> *
    get_test_case_list()
    {
        static std::list<TestCase *> l;
        return &l;
    }
}

TestCaseList::TestCaseList()
{
}

TestCaseList::~TestCaseList()
{
}

void
TestCaseList::register_test_case(TestCase * const t)
{
    get_test_case_list()->push_back(t);
}

class RunTest
{
    private:
        bool * const _had_a_failure;
        const std::string _named_test;

    public:
        RunTest(bool * had_a_failure, const std::string & named_test) :
            _had_a_failure(had_a_failure),
            _named_test(named_test)
        {
        }

        void operator() (TestCase * test_case) const;
};

void
RunTest::operator() (TestCase * test_case) const
{
    bool had_local_failure(false);

    if ((! _named_test.empty()) && (_named_test != test_case->name()))
    {
        std::cout << "* \"" << test_case->name() << "\": (skip due to NAMED_TEST_ONLY)" << std::endl;
        return;
    }

    std::cout << "* \"" << test_case->name() << "\": " << std::flush;

    int extra_fd = -1;
    if (getenv("PALUDIS_TESTS_REAL_STDOUT_FD"))
    {
        std::stringstream s;
        s << getenv("PALUDIS_TESTS_REAL_STDOUT_FD");
        s >> extra_fd;

        std::stringstream t;
        t << ">>> ";
        if (getenv("PALUDIS_TEST_PROGRAM"))
            t << getenv("PALUDIS_TEST_PROGRAM");
        else
            t << "Unknown Test Program";
        t << " test " << test_case->name() << "..." << std::endl;

        std::string buf(t.str());
        while (! buf.empty())
        {
            int n(write(extra_fd, buf.c_str(), buf.length()));
            if (-1 == n)
                break;
            buf.erase(0, n);
        }
    }

    for (int repeat = 0 ; repeat < 2 ; ++repeat)
    {
        if (test_case->skip())
        {
            std::cout << "(skip)" << std::endl;
            break;
        }

        if (0 != repeat)
            std::cout << "  (repeat): " << std::flush;

        struct timeval start_tv;
        ::gettimeofday(&start_tv, 0);

        try
        {
            if (TestCaseList::use_alarm)
                alarm(test_case->max_run_time());
            test_case->call_run();
            if (TestCaseList::use_alarm)
                alarm(0);
        }
        catch (std::exception &e)
        {
            std::cout << "!{" << std::endl << (DebugStringHolder::get_instance()->get())(e) <<
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
        {
            struct timeval tv;
            ::gettimeofday(&tv, 0);
            const unsigned long delta(((tv.tv_sec - start_tv.tv_sec) * 1000) + ((tv.tv_usec - start_tv.tv_usec) / 1000));
            std::cout << " OK (" << delta << "ms)";
        }

        std::cout << std::endl;

        if (! test_case->repeatable())
            break;
    }
}

bool
TestCaseList::run_tests()
{
    bool had_a_failure(get_test_case_list()->empty());

    std::string named_test;
    if (getenv("NAMED_TEST_ONLY"))
        named_test = getenv("NAMED_TEST_ONLY");

    std::for_each(
            get_test_case_list()->begin(),
            get_test_case_list()->end(),
            RunTest(&had_a_failure, named_test));

    return ! had_a_failure;
}

bool
TestCaseList::use_alarm(true);

#endif

