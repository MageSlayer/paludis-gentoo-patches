/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2010 Ciaran McCreesh
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

#include <paludis/util/singleton.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/thread.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <functional>
#include <algorithm>
#include <vector>
#include <string>

#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

namespace
{
    class MyClass :
        public Singleton<MyClass>
    {
        friend class Singleton<MyClass>;

        private:
            MyClass()
            {
                ++instances;
            }

        public:
            std::string s;

            static int instances;
    };

    int MyClass::instances = 0;

    class MyClassTwo :
        public Singleton<MyClassTwo>
    {
        friend class Singleton<MyClassTwo>;

        private:
            MyClassTwo()
            {
                ++instances;
            }

            ~MyClassTwo()
            {
                --instances;
            }

        public:
            std::string s;

            static int instances;
    };

    int MyClassTwo::instances = 0;

    class MyRecursiveClass :
        public Singleton<MyRecursiveClass>
    {
        friend class Singleton<MyRecursiveClass>;

        public:
            std::string s;

        private:
            MyRecursiveClass() :
                s(MyRecursiveClass::get_instance()->s)
            {
            }
    };

    class MyThreadedClass :
        public Singleton<MyThreadedClass>
    {
        friend class Singleton<MyThreadedClass>;

        private:
            MyThreadedClass()
            {
                Lock l(mutex);
                ++instances;
            }

        public:
            std::string s;

            static Mutex mutex;
            static int instances;
    };

    int MyThreadedClass::instances = 0;
    Mutex MyThreadedClass::mutex;
}

namespace test_cases
{
    struct SingletonPatternTest : TestCase
    {
        SingletonPatternTest() : TestCase("singleton test") { }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            TEST_CHECK_EQUAL(MyClass::instances, 0);
            TEST_CHECK(0 != MyClass::get_instance());
            TEST_CHECK_EQUAL(MyClass::instances, 1);
            TEST_CHECK(MyClass::get_instance() == MyClass::get_instance());
            TEST_CHECK(MyClass::get_instance()->s.empty());
            MyClass::get_instance()->s = "foo";
            TEST_CHECK_EQUAL(MyClass::get_instance()->s, "foo");
        }
    } test_singleton_pattern;

    struct SingletonThreadedTest : TestCase
    {
        SingletonThreadedTest() : TestCase("singleton threaded test") { }

        bool repeatable() const
        {
            return false;
        }

        static void thread_func(void * * const p) throw ()
        {
            *p = MyThreadedClass::get_instance();
        }

        void run()
        {
            using namespace std::placeholders;
            const int c = 100;

            std::vector<void *> a(c, static_cast<void *>(0));
            TEST_CHECK_EQUAL(MyThreadedClass::instances, 0);
            TEST_CHECK(c == std::count(a.begin(), a.end(), static_cast<void *>(0)));
            {
                std::vector<std::shared_ptr<Thread> > t(c);
                for (int x(0) ; x < c ; ++x)
                    t[x] = make_shared_ptr(new Thread(std::bind(&thread_func, &a[x])));
            }
            TEST_CHECK_EQUAL(MyThreadedClass::instances, 1);
            TEST_CHECK(0 == std::count(a.begin(), a.end(), static_cast<void *>(0)));
            TEST_CHECK(c == std::count(a.begin(), a.end(), MyThreadedClass::get_instance()));
        }
    } test_singleton_threaded;

    struct SingletonPatternDeleteTest : TestCase
    {
        SingletonPatternDeleteTest() : TestCase("singleton delete test") { }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            TEST_CHECK_EQUAL(MyClassTwo::instances, 0);
            TEST_CHECK(0 != MyClassTwo::get_instance());
            TEST_CHECK_EQUAL(MyClassTwo::instances, 1);
            TEST_CHECK(MyClassTwo::get_instance() == MyClassTwo::get_instance());
            TEST_CHECK(MyClassTwo::get_instance()->s.empty());
            MyClassTwo::get_instance()->s = "foo";
            TEST_CHECK_EQUAL(MyClassTwo::get_instance()->s, "foo");
            MyClassTwo::destroy_instance();
            TEST_CHECK_EQUAL(MyClassTwo::instances, 0);
            TEST_CHECK(0 != MyClassTwo::get_instance());
            TEST_CHECK_EQUAL(MyClassTwo::instances, 1);
            TEST_CHECK(MyClassTwo::get_instance()->s.empty());
        }
    } test_singleton_pattern_delete;

    struct SingletonPatternRecursiveTest : TestCase
    {
        SingletonPatternRecursiveTest() : TestCase("singleton recursive test") { }

        void run()
        {
            TEST_CHECK_THROWS(MyRecursiveClass * PALUDIS_ATTRIBUTE((unused)) x =
                    MyRecursiveClass::get_instance(), InternalError);
        }
    } test_singleton_pattern_recurse;
}

