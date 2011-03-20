/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2011 Ciaran McCreesh
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

#include <paludis/util/save.hh>

#include <gtest/gtest.h>

using namespace paludis;

TEST(Save, Works)
{
    std::string s("one");
    EXPECT_EQ("one", s);
    {
        Save<std::string> save_s(&s);
        EXPECT_EQ("one", s);
        s = "two";
        EXPECT_EQ("two", s);
    }
    EXPECT_EQ("one", s);
    {
        Save<std::string> save_s(&s, "three");
        EXPECT_EQ("three", s);
        {
            Save<std::string> save_s_2(&s, "four");
            EXPECT_EQ("four", s);
        }
        EXPECT_EQ("three", s);
    }
    EXPECT_EQ("one", s);
}

TEST(RunOnDestruction, Works)
{
    std::string s("one");
    EXPECT_EQ("one", s);
    {
        RunOnDestruction save_s(std::bind(&std::string::clear, &s));
        EXPECT_EQ("one", s);
    }
    EXPECT_EQ("", s);
}

