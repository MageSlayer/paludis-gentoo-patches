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

#ifndef PALUDIS_GUARD_TEST_TEST_FRAMEWORK_HH
#define PALUDIS_GUARD_TEST_TEST_FRAMEWORK_HH 1

#include <paludis/stringify.hh>
#include <string>
#include <memory>
#include <list>
#include <sstream>

/** \file
 * Test framework class definitions.
 *
 * \ingroup Test
 */

namespace test
{
    /**
     * RAII suffix marker for TestCase context.
     *
     * \ingroup Test
     */
    class TestMessageSuffix
    {
        private:
            static std::list<std::string> _suffixes;

        public:
            /**
             * Constructor.
             */
            explicit TestMessageSuffix(const std::string & s, bool write = false);

            /**
             * Destructor.
             */
            ~TestMessageSuffix()
            {
                _suffixes.pop_back();
            }

            /**
             * Our suffixes.
             */
            static std::string suffixes();
    };

    /**
     * Base TestCase class.
     *
     * \ingroup Test
     */
    class TestCase
    {
        private:
            struct Impl;
            std::auto_ptr<Impl> _impl;

        protected:
            /**
             * Check that a given assertion is true.
             */
            void check(const char * const function, const char * const file,
                    const long line, bool was_ok, const std::string & message) const;

            /**
             * Run the tests (override this in descendents).
             */
            virtual void run() = 0;

        public:
            /**
             * Constructor.
             */
            TestCase(const std::string & name);

            /**
             * Destructor.
             */
            virtual ~TestCase();

            /**
             * Call our run() function, and normalise exceptions.
             */
            void call_run();

            /**
             * Return our name.
             */
            std::string name() const;

            /**
             * Are we repeatable? Override if not.
             */
            virtual bool repeatable() const
            {
                return true;
            }

            /**
             * After how many seconds should we timeout?
             */
            virtual unsigned max_run_time() const
            {
                return 5;
            }
    };

    /**
     * Thrown if a TestCase failed.
     *
     * \ingroup Test
     * \ingroup Exception
     */
    class TestFailedException : public std::exception
    {
        private:
            const std::string _message;

        public:
            /**
             * Constructor.
             */
            TestFailedException(const char * const function, const char * const file,
                    const long line, const std::string & message) throw ();

            /**
             * Destructor.
             */
            virtual ~TestFailedException() throw ();

            /**
             * Description.
             */
            const char * what() const throw ()
            {
                return _message.c_str();
            }
    };

    /**
     * A list of TestCase instances.
     *
     * \ingroup Test
     */
    class TestCaseList
    {
        private:
            TestCaseList();
            ~TestCaseList();

            static std::list<TestCase *> & _get_test_case_list()
            {
                static std::list<TestCase *> l;
                return l;
            }

        public:
            /**
             * Register a TestCase instance.
             */
            inline static void register_test_case(TestCase * const test_case)
            {
                _get_test_case_list().push_back(test_case);
            }

            /**
             * Run all tests.
             */
            static bool run_tests();
    };

    void set_exception_to_debug_string(std::string (*) (const std::exception &));
}

/**
 * Check that a == b.
 */
#define TEST_CHECK_EQUAL(a, b) \
    do { \
        check(__PRETTY_FUNCTION__, __FILE__, __LINE__, a == b, \
                "Expected '" #a "' to equal '" + paludis::stringify(b) + \
                "' but got '" + paludis::stringify(a) + "'"); \
    } while (false)

/**
 * Check that stringify(a) == stringify(b).
 */
#define TEST_CHECK_STRINGIFY_EQUAL(a, b) \
    do { \
        check(__PRETTY_FUNCTION__, __FILE__, __LINE__, paludis::stringify(a) == paludis::stringify(b), \
                "Expected '" #a "' to equal '" + paludis::stringify(b) + \
                "' but got '" + paludis::stringify(a) + "'"); \
    } while (false)

/**
 * Check that a is true.
 */
#define TEST_CHECK(a) \
    do { \
        check(__PRETTY_FUNCTION__, __FILE__, __LINE__, a, \
                "Check '" #a "' failed"); \
    } while (false)

/**
 * Check that a throws an exception of type b.
 */
#define TEST_CHECK_THROWS(a, b) \
    do { \
        try { \
            a; \
            check(__PRETTY_FUNCTION__, __FILE__, __LINE__, false, \
                    "Expected exception of type '" #b "' not thrown"); \
        } catch (b &) { \
            TEST_CHECK(true); \
        } \
    } while (false)

#endif

