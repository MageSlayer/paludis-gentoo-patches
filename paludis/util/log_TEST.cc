/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2011 Ciaran McCreesh
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

#include <paludis/util/log.hh>

#include <sstream>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    struct Monkey
    {
    };

    std::ostream &
    operator<< (std::ostream & s, const Monkey & m)
    {
        if (s)
            throw m;

        return s;
    }

    int throws_a_monkey() PALUDIS_ATTRIBUTE((noreturn));
    int throws_a_monkey()
    {
        throw Monkey();
    }

    Monkey throws_a_monkey_when_stringified()
    {
        return Monkey();
    }
}

TEST(Log, Messages)
{
    Log::destroy_instance();

    ASSERT_TRUE(Log::get_instance());
    EXPECT_TRUE(Log::get_instance() == Log::get_instance());

    std::stringstream s;
    Log::get_instance()->set_log_stream(&s);
    Log::get_instance()->set_log_level(ll_debug);

    EXPECT_TRUE(s.str().empty());
    Log::get_instance()->message("test.log", ll_debug, lc_no_context) << "one";
    EXPECT_TRUE(! s.str().empty());
    EXPECT_TRUE(std::string::npos != s.str().find("one"));

    std::stringstream t;
    Log::get_instance()->set_log_stream(&t);
    EXPECT_TRUE(t.str().empty());

    Log::get_instance()->set_log_level(ll_warning);
    Log::get_instance()->message("test.log", ll_debug, lc_no_context) << "two";
    EXPECT_TRUE(t.str().empty());
    Log::get_instance()->message("test.log", ll_warning, lc_no_context) << "three" << "." << 14;
    EXPECT_TRUE(! t.str().empty());
    EXPECT_TRUE(std::string::npos == t.str().find("one"));
    EXPECT_TRUE(std::string::npos == t.str().find("two"));
    EXPECT_TRUE(std::string::npos != t.str().find("three.14"));
}

TEST(Log, Exceptions)
{
    Log::destroy_instance();

    std::stringstream s;
    Log::get_instance()->set_log_stream(&s);
    Log::get_instance()->set_log_level(ll_debug);
    EXPECT_TRUE(s.str().empty());

    EXPECT_THROW(Log::get_instance()->message("test.log", ll_debug, lc_no_context) << throws_a_monkey(), Monkey);
    EXPECT_TRUE(s.str().empty());
    EXPECT_THROW(Log::get_instance()->message("test.log", ll_debug, lc_no_context)
            << "one" << throws_a_monkey() << "two", Monkey);
    EXPECT_TRUE(s.str().empty());

    EXPECT_THROW(Log::get_instance()->message("test.log", ll_debug, lc_no_context) << throws_a_monkey_when_stringified(), Monkey);
    EXPECT_TRUE(s.str().empty());
    EXPECT_THROW(Log::get_instance()->message("test.log", ll_debug, lc_no_context)
            << "one" << throws_a_monkey_when_stringified() << "two", Monkey);
    EXPECT_TRUE(s.str().empty());
}

