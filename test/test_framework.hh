/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <string>

/** \file
 * Test framework class definitions.
 *
 * \ingroup grptestframework
 */

namespace test
{
    /**
     * RAII suffix marker for TestCase context.
     *
     * \ingroup grptestframework
     */
    class TestMessageSuffix :
        paludis::PrivateImplementationPattern<TestMessageSuffix>
    {
        public:
            /**
             * Constructor.
             */
            explicit TestMessageSuffix(const std::string & s, bool write = false);

            /**
             * Destructor.
             */
            ~TestMessageSuffix();

            /**
             * Our suffixes.
             */
            static std::string suffixes();
    };

    /**
     * Base TestCase class.
     *
     * \ingroup grptestframework
     */
    class TestCase :
        private paludis::PrivateImplementationPattern<TestCase>
    {
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
             * Should we be skipped?
             */
            virtual bool skip() const
            {
                return false;
            }

            /**
             * After how many seconds should we timeout?
             */
            virtual unsigned max_run_time() const
            {
                return 30;
            }
    };

    /**
     * Thrown if a TestCase failed.
     *
     * \ingroup grptestframework
     * \ingroup grpexceptions
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
     * \ingroup grptestframework
     */
    class TestCaseList
    {
        private:
            TestCaseList();
            ~TestCaseList();

        public:
            /**
             * Register a TestCase instance.
             */
            static void register_test_case(TestCase * const test_case);

            /**
             * Run all tests.
             */
            static bool run_tests();

            /**
             * Should we use alarm?
             */
            static bool use_alarm;
    };

    /**
     * Change the function used to get a string description of an exception.
     */
    void set_exception_to_debug_string(std::string (*) (const std::exception &));

    /**
     * Fetch the function used to get a string description of an exception.
     */
    std::string (* get_exception_to_debug_string()) (const std::exception &);

    /**
     * Utility class used by TEST_CHECK_EQUAL.
     */
    struct TwoVarHolder
    {
        bool result;
        std::string s_a;
        std::string s_b;

        template <typename T1_, typename T2_>
        TwoVarHolder(T1_ a, T2_ b) :
            result(a == b),
            s_a(paludis::stringify(a)),
            s_b(paludis::stringify(b))
        {
        }
    };
}

/**
 * Check that a == b.
 */
#define TEST_CHECK_EQUAL(a, b) \
    do { \
        try { \
            test::TwoVarHolder test_h(a, b); \
            check(__PRETTY_FUNCTION__, __FILE__, __LINE__, test_h.result, \
                    "Expected '" #a "' to equal '" + test_h.s_b + \
                    "' but got '" + test_h.s_a + "'"); \
        } catch (const TestFailedException &) { \
            throw; \
        } catch (const std::exception & test_e) { \
            throw TestFailedException(__PRETTY_FUNCTION__, __FILE__, __LINE__, \
                    "Test threw unexpected exception " + test::get_exception_to_debug_string()(test_e) + \
                    " inside a TEST_CHECK_EQUAL block"); \
        } catch (...) { \
            throw TestFailedException(__PRETTY_FUNCTION__, __FILE__, __LINE__, \
                    "Test threw unexpected unknown exception inside a TEST_CHECK_EQUAL block"); \
        } \
    } while (false)

/**
 * Check that stringify(a) == stringify(b).
 */
#define TEST_CHECK_STRINGIFY_EQUAL(a, b) \
    do { \
        try { \
            std::string s_a(paludis::stringify(a)); \
            std::string s_b(paludis::stringify(b)); \
            check(__PRETTY_FUNCTION__, __FILE__, __LINE__, s_a == s_b, \
                    "Expected '" #a "' to equal '" + s_b + \
                    "' but got '" + s_a + "'"); \
        } catch (const TestFailedException &) { \
            throw; \
        } catch (const std::exception & test_e) { \
            throw TestFailedException(__PRETTY_FUNCTION__, __FILE__, __LINE__, \
                    "Test threw unexpected exception " + test::get_exception_to_debug_string()(test_e) + \
                    " inside a TEST_CHECK_STRINGIFY_EQUAL block"); \
        } catch (...) { \
            throw TestFailedException(__PRETTY_FUNCTION__, __FILE__, __LINE__, \
                    "Test threw unexpected unknown exception inside a TEST_CHECK_STRINGIFY_EQUAL block"); \
        } \
    } while (false)

/**
 * Check that a is true.
 */
#define TEST_CHECK(a) \
    do { \
        try { \
            check(__PRETTY_FUNCTION__, __FILE__, __LINE__, a, \
                    "Check '" #a "' failed"); \
        } catch (const TestFailedException &) { \
            throw; \
        } catch (const std::exception & test_e) { \
            throw TestFailedException(__PRETTY_FUNCTION__, __FILE__, __LINE__, \
                    "Test threw unexpected exception " + test::get_exception_to_debug_string()(test_e) + \
                    " inside a TEST_CHECK block"); \
        } catch (...) { \
            throw TestFailedException(__PRETTY_FUNCTION__, __FILE__, __LINE__, \
                    "Test threw unexpected unknown exception inside a TEST_CHECK block"); \
        } \
    } while (false)

/**
 * Check that a throws an exception of type b.
 */
#define TEST_CHECK_THROWS(a, b) \
    do { \
        try { \
            try { \
                a; \
                check(__PRETTY_FUNCTION__, __FILE__, __LINE__, false, \
                        "Expected exception of type '" #b "' not thrown"); \
            } catch (b &) { \
                TEST_CHECK(true); \
            } \
        } catch (const TestFailedException &) { \
            throw; \
        } catch (const std::exception & test_e) { \
            throw TestFailedException(__PRETTY_FUNCTION__, __FILE__, __LINE__, \
                    "Test threw unexpected exception " + test::get_exception_to_debug_string()(test_e) + \
                    " inside a TEST_CHECK_THROWS block"); \
        } catch (...) { \
            throw TestFailedException(__PRETTY_FUNCTION__, __FILE__, __LINE__, \
                    "Test threw unexpected unknown exception inside a TEST_CHECK_THROWS block"); \
        } \
    } while (false)

#endif

