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

#include <paludis/dep_spec.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/version_requirements.hh>
#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/clone-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/options.hh>
#include <paludis/util/stringify.hh>

#include <gtest/gtest.h>

using namespace paludis;

TEST(FetchableURIDepSpec, Works)
{
    FetchableURIDepSpec a("foo");
    EXPECT_EQ("foo", a.original_url());
    EXPECT_EQ("", a.renamed_url_suffix());
    EXPECT_EQ("foo", a.filename());

    FetchableURIDepSpec b("fnord -> bar");
    EXPECT_EQ("fnord", b.original_url());
    EXPECT_EQ("bar", b.renamed_url_suffix());
    EXPECT_EQ("bar", b.filename());

    FetchableURIDepSpec c("http://example.com/download/baz");
    EXPECT_EQ("baz", c.filename());
}

TEST(DepSpec, Clone)
{
    TestEnvironment env;
    PackageDepSpec a(parse_user_package_dep_spec("cat/pkg:1::repo[=1|>3.2][foo]",
                &env, { }));

    std::shared_ptr<PackageDepSpec> b(std::static_pointer_cast<PackageDepSpec>(a.clone()));
    EXPECT_EQ(stringify(a), stringify(*b));

    std::shared_ptr<PackageDepSpec> c(std::static_pointer_cast<PackageDepSpec>(a.clone()));
    EXPECT_EQ(stringify(a), stringify(*c));

    BlockDepSpec d("!" + stringify(*c), *c);
    std::shared_ptr<BlockDepSpec> e(std::static_pointer_cast<BlockDepSpec>(d.clone()));
    EXPECT_EQ(stringify(d.blocking()), stringify(e->blocking()));
}

