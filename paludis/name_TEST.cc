/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2011 Ciaran McCreesh
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

#include <paludis/name.hh>

#include <paludis/util/exception.hh>

#include <gtest/gtest.h>

using namespace paludis;

TEST(QualifiedPackageName, Create)
{
    QualifiedPackageName a("foo/bar1");
}

TEST(QualifiedPackageName, Validation)
{
    QualifiedPackageName a("foo/bar");
    EXPECT_THROW(a = QualifiedPackageName("moo"), NameError);
    EXPECT_THROW(a = QualifiedPackageName("foo/bar!"), NameError);
    EXPECT_THROW(a = QualifiedPackageName("foo/bar/baz"), NameError);
}

TEST(QualifiedPackageName, Compare)
{
    QualifiedPackageName foo1_bar1("foo1/bar1");
    QualifiedPackageName foo1_bar2("foo1/bar2");
    QualifiedPackageName foo2_bar1("foo2/bar1");

    EXPECT_TRUE( (foo1_bar1 <  foo1_bar2));
    EXPECT_TRUE( (foo1_bar1 <= foo1_bar2));
    EXPECT_TRUE(!(foo1_bar1 == foo1_bar2));
    EXPECT_TRUE( (foo1_bar1 != foo1_bar2));
    EXPECT_TRUE(!(foo1_bar1 >= foo1_bar2));
    EXPECT_TRUE(!(foo1_bar1 >  foo1_bar2));

    EXPECT_TRUE(!(foo2_bar1 <  foo1_bar2));
    EXPECT_TRUE(!(foo2_bar1 <= foo1_bar2));
    EXPECT_TRUE(!(foo2_bar1 == foo1_bar2));
    EXPECT_TRUE( (foo2_bar1 != foo1_bar2));
    EXPECT_TRUE( (foo2_bar1 >= foo1_bar2));
    EXPECT_TRUE( (foo2_bar1 >  foo1_bar2));
}

TEST(CategoryNamePart, Create)
{
    CategoryNamePart p("foo");
}

TEST(CategoryNamePart, Validate)
{
    CategoryNamePart p = CategoryNamePart("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-+_");

    EXPECT_THROW(p = CategoryNamePart(""), CategoryNamePartError);
    EXPECT_THROW(p = CategoryNamePart("*"), CategoryNamePartError);
    EXPECT_THROW(p = CategoryNamePart("foo bar"), CategoryNamePartError);
}

TEST(PackageNamePart, Create)
{
    PackageNamePart p("foo");
}

TEST(PackageNamePart, Validate)
{
    PackageNamePart p("foo-200dpi");
    EXPECT_THROW(p = PackageNamePart(""), NameError);
    EXPECT_THROW(p = PackageNamePart("!!!"), NameError);
    EXPECT_THROW(p = PackageNamePart("foo-2"), NameError);
    EXPECT_THROW(p = PackageNamePart("foo-200"), NameError);
}

TEST(PackageNamePart, Comparison)
{
    PackageNamePart p1("p1");
    PackageNamePart p2("p2");

    EXPECT_TRUE( (p1 <  p2));
    EXPECT_TRUE( (p1 <= p2));
    EXPECT_TRUE(!(p1 == p2));
    EXPECT_TRUE(!(p1 >  p2));
    EXPECT_TRUE(!(p1 >= p2));
    EXPECT_TRUE( (p1 != p2));

    EXPECT_TRUE(!(p2 <  p2));
    EXPECT_TRUE( (p2 <= p2));
    EXPECT_TRUE( (p2 == p2));
    EXPECT_TRUE(!(p2 >  p2));
    EXPECT_TRUE( (p2 >= p2));
    EXPECT_TRUE(!(p2 != p2));

    EXPECT_TRUE(!(p2 <  p1));
    EXPECT_TRUE(!(p2 <= p1));
    EXPECT_TRUE(!(p2 == p1));
    EXPECT_TRUE( (p2 >  p1));
    EXPECT_TRUE( (p2 >= p1));
    EXPECT_TRUE( (p2 != p1));
}

TEST(RepositoryName, Create)
{
    RepositoryName r("foo");
}

TEST(RepositoryName, Validation)
{
    RepositoryName r("repo0_-");
    EXPECT_THROW(r = RepositoryName(""), NameError);
    EXPECT_THROW(r = RepositoryName("!!!"), NameError);
    EXPECT_THROW(r = RepositoryName("-foo"), NameError);
    EXPECT_THROW(r = RepositoryName("fo$o"), NameError);
}

TEST(SlotName, Creation)
{
    SlotName s("foo");
}

TEST(SlotName, Validation)
{
    SlotName s("slot0+_.-");
    EXPECT_THROW(s = SlotName(""), NameError);
    EXPECT_THROW(s = SlotName("!!!"), NameError);
    EXPECT_THROW(s = SlotName("-foo"), NameError);
    EXPECT_THROW(s = SlotName(".foo"), NameError);
    EXPECT_THROW(s = SlotName("fo$o"), NameError);
}

TEST(KeywordName, Create)
{
    KeywordName k("foo");
}

TEST(KeywordName, Validation)
{
    KeywordName k("keyword0_-");
    KeywordName k1("~keyword0_-");
    KeywordName k2("-keyword0_-");
    KeywordName k3("*");
    KeywordName k4("-*");
    EXPECT_THROW(k = KeywordName(""), NameError);
    EXPECT_THROW(k = KeywordName("!!!"), NameError);
    EXPECT_THROW(k = KeywordName("~"), NameError);
    EXPECT_THROW(k = KeywordName("-"), NameError);
    EXPECT_THROW(k = KeywordName("@foo"), NameError);
    EXPECT_THROW(k = KeywordName("fo$o"), NameError);
    EXPECT_THROW(k = KeywordName("foo+"), NameError);
}

TEST(SetName, Validation)
{
    SetName k("set0_-");
    SetName k1("set0_-");
    SetName k2("set0*");
    SetName k3("set::foo");
    SetName k4("set::foo*");
    EXPECT_THROW(k = SetName(""), NameError);
    EXPECT_THROW(k = SetName("!!!"), NameError);
    EXPECT_THROW(k = SetName("~"), NameError);
    EXPECT_THROW(k = SetName("-"), NameError);
    EXPECT_THROW(k = SetName("f?oo"), NameError);
    EXPECT_THROW(k = SetName("*"), NameError);
    EXPECT_THROW(k = SetName("?"), NameError);
    EXPECT_THROW(k = SetName("*set"), NameError);
    EXPECT_THROW(k = SetName("set**"), NameError);
    EXPECT_THROW(k = SetName("set*?"), NameError);
    EXPECT_THROW(k = SetName("set?"), NameError);
    EXPECT_THROW(k = SetName("set:::"), NameError);
    EXPECT_THROW(k = SetName("set::"), NameError);
    EXPECT_THROW(k = SetName("set:"), NameError);
    EXPECT_THROW(k = SetName("set:foo"), NameError);
    EXPECT_THROW(k = SetName("set:::foo"), NameError);
    EXPECT_THROW(k = SetName("set::foo::bar"), NameError);
}

