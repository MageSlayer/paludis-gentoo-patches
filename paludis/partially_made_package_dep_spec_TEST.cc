/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/package_dep_spec_constraint.hh>

#include <paludis/util/stringify.hh>
#include <paludis/util/options.hh>

#include <gtest/gtest.h>

using namespace paludis;

TEST(PartiallyMadePackageDepSpec, CatAndPkgBecomesQPN)
{
    PackageDepSpec p(make_package_dep_spec({ })
            .category_name_part(CategoryNamePart("cat"))
            .package_name_part(PackageNamePart("pkg"))
            );

    EXPECT_EQ("cat/pkg", stringify(p));

    ASSERT_TRUE(bool(p.package_name_constraint()));
    EXPECT_EQ("cat/pkg", stringify(p.package_name_constraint()->name()));

    EXPECT_FALSE(bool(p.category_name_part_constraint()));
    EXPECT_FALSE(bool(p.package_name_part_constraint()));
}

TEST(PartiallyMadePackageDepSpec, QPNClearsCatPkg)
{
    PackageDepSpec p(make_package_dep_spec({ })
            .category_name_part(CategoryNamePart("cat"))
            .package_name_part(PackageNamePart("pkg"))
            .package(QualifiedPackageName("other/name"))
            );

    EXPECT_EQ("other/name", stringify(p));

    ASSERT_TRUE(bool(p.package_name_constraint()));
    EXPECT_EQ("other/name", stringify(p.package_name_constraint()->name()));

    EXPECT_FALSE(bool(p.category_name_part_constraint()));
    EXPECT_FALSE(bool(p.package_name_part_constraint()));
}

