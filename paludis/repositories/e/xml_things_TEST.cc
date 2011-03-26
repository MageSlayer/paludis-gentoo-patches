/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/glsa.hh>

#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

#include <gtest/gtest.h>

using namespace paludis;

TEST(GLSA, GLSA12345678)
{
    std::shared_ptr<GLSA> glsa(GLSA::create_from_xml_file("xml_things_TEST_dir/glsa-123456-78.xml"));
    ASSERT_TRUE(bool(glsa));

    EXPECT_EQ("123456-78", glsa->id());
    EXPECT_EQ("Kittens: Too Adorable", glsa->title());

    EXPECT_EQ(1, std::distance(glsa->begin_packages(), glsa->end_packages()));
    EXPECT_EQ("animal-feline/kitten", stringify(glsa->begin_packages()->name()));
    EXPECT_EQ(0, std::distance(glsa->begin_packages()->begin_archs(),
                glsa->begin_packages()->end_archs()));

    EXPECT_EQ(1, std::distance(glsa->begin_packages()->begin_unaffected(),
                glsa->begin_packages()->end_unaffected()));
    EXPECT_EQ("ge", glsa->begin_packages()->begin_unaffected()->op());
    EXPECT_EQ("1.23", glsa->begin_packages()->begin_unaffected()->version());

    EXPECT_EQ(1, std::distance(glsa->begin_packages(), glsa->end_packages()));
    EXPECT_EQ(1, std::distance(glsa->begin_packages()->begin_vulnerable(),
                glsa->begin_packages()->end_vulnerable()));
    EXPECT_EQ("lt", glsa->begin_packages()->begin_vulnerable()->op());
    EXPECT_EQ("1.22", glsa->begin_packages()->begin_vulnerable()->version());
}

TEST(GLSA, GLSA98765432)
{
    std::shared_ptr<GLSA> glsa(GLSA::create_from_xml_file("xml_things_TEST_dir/glsa-987654-32.xml"));
    ASSERT_TRUE(bool(glsa));

    EXPECT_EQ("987654-32", glsa->id());
    EXPECT_EQ("Python: Retarded", glsa->title());

    EXPECT_EQ(1, std::distance(glsa->begin_packages(), glsa->end_packages()));
    EXPECT_EQ("dev-lang/python", stringify(glsa->begin_packages()->name()));
    EXPECT_EQ(3, std::distance(glsa->begin_packages()->begin_archs(),
                glsa->begin_packages()->end_archs()));
    EXPECT_EQ("mips,sparc,x86", join(glsa->begin_packages()->begin_archs(),
                glsa->begin_packages()->end_archs(), ","));

    EXPECT_EQ(1, std::distance(glsa->begin_packages()->begin_unaffected(),
                glsa->begin_packages()->end_unaffected()));
    EXPECT_EQ("ge", glsa->begin_packages()->begin_unaffected()->op());
    EXPECT_EQ("12.34", glsa->begin_packages()->begin_unaffected()->version());

    EXPECT_EQ(1, std::distance(glsa->begin_packages(), glsa->end_packages()));
    EXPECT_EQ(1, std::distance(glsa->begin_packages()->begin_vulnerable(),
                glsa->begin_packages()->end_vulnerable()));
    EXPECT_EQ("lt", glsa->begin_packages()->begin_vulnerable()->op());
    EXPECT_EQ("12.34", glsa->begin_packages()->begin_vulnerable()->version());
}

