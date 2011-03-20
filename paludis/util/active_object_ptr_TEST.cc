/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/util/active_object_ptr.hh>

#include <gtest/gtest.h>

using namespace paludis;

TEST(ActiveObjectPtr, Dereference)
{
    ActiveObjectPtr<std::string *> p(new std::string("monkey"));
    EXPECT_EQ(6u, p->length());

    ActiveObjectPtr<std::shared_ptr<std::string> > q(std::make_shared<std::string>("chimp"));
    EXPECT_EQ(5u, q->length());
}

TEST(ActiveObjectPtr, Value)
{
    ActiveObjectPtr<std::string *> p(new std::string("monkey"));
    EXPECT_EQ(6u, p.value()->length());

    ActiveObjectPtr<std::shared_ptr<std::string> > q(
            std::make_shared<std::string>("chimp"));
    EXPECT_EQ(5u, q.value()->length());
}

