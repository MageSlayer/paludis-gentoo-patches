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
#include <algorithm>
#include <iostream>
#include <list>
#include <paludis/util/attributes.hh>
#include <unistd.h>

/** \file
 * Implementation for test framework classes.
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
    struct Implementation<TestMessageSuffix> :
        InternalCounted<Implementation<TestMessageSuffix> >
    {
        static std::list<std::string> suffixes;
    };

    std::list<std::string> Implementation<TestMessageSuffix>::suffixes;
}

std::string
TestMessageSuffix::suffixes()
{
    std::string result;
    std::list<std::string>::const_iterator i(paludis::Implementation<TestMessageSuffix>::suffixes.begin()),
        end(paludis::Implementation<TestMessageSuffix>::suffixes.end());

    while (i != end)
    {
        result += *i++;
        if (end != i)
            result += ", ";
    }
    return result;
}

TestMessageSuffix::TestMessageSuffix(const std::string & s, bool write) :
    paludis::PrivateImplementationPattern<TestMessageSuffix>(
            new paludis::Implementation<TestMessageSuffix>)
{
    paludis::Implementation<TestMessageSuffix>::suffixes.push_back(s);
    if (write)
        std::cout << "[" << s << "]" << std::flush;
}

TestMessageSuffix::~TestMessageSuffix()
{
    paludis::Implementation<TestMessageSuffix>::suffixes.pop_back();
}

namespace paludis
{
    template<>
    struct Implementation<TestCase> :
        InternalCounted<Implementation<TestCase> >
    {
        const std::string name;

        Implementation(const std::string & the_name) :
            name(the_name)
        {
        }
    };
}

TestCase::TestCase(const std::string & our_name) :
    paludis::PrivateImplementationPattern<TestCase>(new paludis::Implementation<TestCase>(our_name))
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
                "Test threw unexpected exception " + (DebugStringHolder::get_instance()->get())(e));
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
        if (test_case->skip())
        {
            std::cout << "(skip)" << std::endl;
            break;
        }

        if (0 != repeat)
            std::cout << "  (repeat): " << std::flush;

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
            std::cout << " OK";

        std::cout << std::endl;

        if (! test_case->repeatable())
            break;
    }
}

bool
TestCaseList::run_tests()
{
    bool had_a_failure(get_test_case_list()->empty());

    std::for_each(
            get_test_case_list()->begin(),
            get_test_case_list()->end(),
            RunTest(&had_a_failure));

    return ! had_a_failure;
}

bool
TestCaseList::use_alarm(true);

#endif

