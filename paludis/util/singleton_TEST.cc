/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2010, 2011, 2012, 2013 Ciaran McCreesh
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
#include <paludis/util/thread_pool.hh>

#include <functional>
#include <algorithm>
#include <vector>
#include <string>

#include <gtest/gtest.h>

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
                std::unique_lock<std::mutex> lock(mutex);
                ++instances;
            }

        public:
            std::string s;

            static std::mutex mutex;
            static int instances;
    };

    int MyThreadedClass::instances = 0;
    std::mutex MyThreadedClass::mutex;

    static void thread_func(void * * const p) throw ()
    {
        *p = MyThreadedClass::get_instance();
    }
}

TEST(Singleton, Works)
{
    ASSERT_EQ(0, MyClass::instances);
    ASSERT_TRUE(nullptr != MyClass::get_instance());
    ASSERT_EQ(1, MyClass::instances);
    ASSERT_TRUE(MyClass::get_instance() == MyClass::get_instance());
    ASSERT_TRUE(MyClass::get_instance()->s.empty());
    MyClass::get_instance()->s = "foo";
    ASSERT_EQ("foo", MyClass::get_instance()->s);
}

TEST(Singleton, Threaded)
{
    using namespace std::placeholders;
    const int c = 100;

    std::vector<void *> a(c, static_cast<void *>(nullptr));
    ASSERT_EQ(0, MyThreadedClass::instances);
    ASSERT_TRUE(c == std::count(a.begin(), a.end(), static_cast<void *>(nullptr)));
    {
        ThreadPool p;
        for (int x(0) ; x < c ; ++x)
            p.create_thread(std::bind(&thread_func, &a[x]));
    }
    ASSERT_EQ(1, MyThreadedClass::instances);
    ASSERT_TRUE(0 == std::count(a.begin(), a.end(), static_cast<void *>(nullptr)));
    ASSERT_TRUE(c == std::count(a.begin(), a.end(), MyThreadedClass::get_instance()));
}

TEST(Singleton, Delete)
{
    ASSERT_EQ(0, MyClassTwo::instances);
    ASSERT_TRUE(nullptr != MyClassTwo::get_instance());
    ASSERT_EQ(1, MyClassTwo::instances);
    ASSERT_TRUE(MyClassTwo::get_instance() == MyClassTwo::get_instance());
    ASSERT_TRUE(MyClassTwo::get_instance()->s.empty());
    MyClassTwo::get_instance()->s = "foo";
    ASSERT_EQ("foo", MyClassTwo::get_instance()->s);
    MyClassTwo::destroy_instance();
    ASSERT_EQ(MyClassTwo::instances, 0);
    ASSERT_TRUE(nullptr != MyClassTwo::get_instance());
    ASSERT_EQ(1, MyClassTwo::instances);
    ASSERT_TRUE(MyClassTwo::get_instance()->s.empty());
}

TEST(Singleton, Recursive)
{
    ASSERT_THROW(MyRecursiveClass * PALUDIS_ATTRIBUTE((unused)) x =
            MyRecursiveClass::get_instance(), InternalError);
}

