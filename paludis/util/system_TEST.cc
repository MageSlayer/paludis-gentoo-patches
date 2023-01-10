/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/util/system.hh>
#include <paludis/util/log.hh>
#include <paludis/util/thread_pool.hh>

#include <functional>
#include <cctype>
#include <sched.h>

#include <gtest/gtest.h>

using namespace paludis;

TEST(GetenvWithDefault, Works)
{
    EXPECT_TRUE(! getenv_with_default("HOME", "!").empty());
    EXPECT_EQ('/', getenv_with_default("HOME", "!").at(0));
    EXPECT_EQ("moo", getenv_with_default("THEREISNOSUCHVARIABLE", "moo"));
}

TEST(GetenvOrError, Works)
{
    EXPECT_TRUE(! getenv_or_error("HOME").empty());
}

TEST(GetenvError, Throws)
{
    EXPECT_THROW(getenv_or_error("THEREISNOSUCHVARIABLE"), GetenvError);
}

TEST(KernelVersion, Works)
{
    EXPECT_TRUE(! kernel_version().empty());
#if defined(__linux__) || defined(__FreeBSD__)
    EXPECT_TRUE(isdigit(kernel_version().at(0)));
    EXPECT_TRUE('.' == kernel_version().at(1));
#else
#  error You need to write a sanity test for kernel_version() for your platform.
#endif
}

