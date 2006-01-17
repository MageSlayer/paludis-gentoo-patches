/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "visitor_pattern.hh"
#include "visitor_pattern-impl.hh"
#include "deleter.hh"
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <list>
#include <algorithm>
#include <sstream>

using namespace test;
using namespace paludis;

/** \file
 * Tests for visitor_pattern.hh .
 *
 * \ingroup Test
 */

#ifndef DOXYGEN

class MyVisitorBase;

class MyBaseClass : public virtual VisitableInterface<MyVisitorBase>
{
    protected:
        MyBaseClass()
        {
        }
};

class MyClassOne : public MyBaseClass,
                   public Visitable<MyClassOne, MyVisitorBase>
{
    public:
        MyClassOne()
        {
        }

        int get_one_thing() const
        {
            return 1;
        }
};

class MyClassTwo : public MyBaseClass,
                   public Visitable<MyClassTwo, MyVisitorBase>
{
    public:
        MyClassTwo()
        {
        }

        std::string get_two_thing() const
        {
            return "two";
        }
};

class MyClassThree : public MyBaseClass,
                     public Visitable<MyClassThree, MyVisitorBase>
{
    public:
        MyClassThree()
        {
        }
};

class MyVisitorBase : public Visits<MyClassOne>,
                      public Visits<MyClassTwo>,
                      public Visits<MyClassThree>
{
    protected:
        MyVisitorBase()
        {
        }
};

class MyStringifyVisitor : public MyVisitorBase
{
    private:
        std::stringstream s;

    public:
        MyStringifyVisitor()
        {
        }

        void visit(const MyClassOne * const one)
        {
            s << one->get_one_thing();
        }

        void visit(const MyClassTwo * const two)
        {
            s << two->get_two_thing();
        }

        void visit(const MyClassThree * const)
        {
            s << "---";
        }

        std::string value() const
        {
            return s.str();
        }
};

class MyCountingVisitor : public MyVisitorBase
{
    private:
        int _value;

    public:
        MyCountingVisitor() :
            _value(0)
        {
        }

        void visit(const MyClassOne * const)
        {
            _value += 1;
        }

        void visit(const MyClassTwo * const)
        {
            _value += 2;
        }

        void visit(const MyClassThree * const)
        {
            _value += 3;
        }

        int value() const
        {
            return _value;
        }
};

#endif

namespace test_cases
{
    /**
     * \test Test visitor pattern.
     *
     * \ingroup Test
     */
    struct VisitorTest : TestCase
    {
        VisitorTest() : TestCase("visitor") { }

        void run()
        {
            std::list<MyBaseClass *> items;
            items.push_back(new MyClassOne);
            items.push_back(new MyClassTwo);
            items.push_back(new MyClassThree);
            items.push_back(new MyClassOne);

            MyStringifyVisitor v;
            std::for_each(items.begin(), items.end(), std::bind2nd(std::mem_fun(&MyBaseClass::accept), &v));
            TEST_CHECK_EQUAL(v.value(), "1two---1");

            MyCountingVisitor c;
            std::for_each(items.begin(), items.end(), std::bind2nd(std::mem_fun(&MyBaseClass::accept), &c));
            TEST_CHECK_EQUAL(c.value(), 1 + 2 + 3 + 1);

            std::for_each(items.begin(), items.end(), Deleter());
        }
    } test_visitor;
}
