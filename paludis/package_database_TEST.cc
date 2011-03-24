/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/environments/test/test_environment.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_named_values.hh>

#include <paludis/package_database.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/filter.hh>

#include <gtest/gtest.h>

using namespace paludis;

TEST(PackageDatabase, Repositories)
{
    TestEnvironment e;
    PackageDatabase & p(*e.package_database());

    const std::shared_ptr<FakeRepository> r1(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &e,
                    n::name() = RepositoryName("repo1")
                    )));
    const std::shared_ptr<FakeRepository> r2(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &e,
                    n::name() = RepositoryName("repo2")
                    )));

    EXPECT_THROW(p.fetch_repository(RepositoryName("repo1")), NoSuchRepositoryError);
    EXPECT_THROW(p.fetch_repository(RepositoryName("repo2")), NoSuchRepositoryError);

    p.add_repository(10, r1);
    ASSERT_TRUE(bool(p.fetch_repository(RepositoryName("repo1"))));
    EXPECT_EQ(RepositoryName("repo1"), p.fetch_repository(RepositoryName("repo1"))->name());
    EXPECT_THROW(p.fetch_repository(RepositoryName("repo2")), NoSuchRepositoryError);

    EXPECT_THROW(p.add_repository(10, r1), DuplicateRepositoryError);

    p.add_repository(11, r2);
    ASSERT_TRUE(bool(p.fetch_repository(RepositoryName("repo1"))));
    EXPECT_EQ(RepositoryName("repo1"), p.fetch_repository(RepositoryName("repo1"))->name());
    ASSERT_TRUE(bool(p.fetch_repository(RepositoryName("repo2"))));
    EXPECT_EQ(RepositoryName("repo2"), p.fetch_repository(RepositoryName("repo2"))->name());

    EXPECT_THROW(p.add_repository(10, r1), DuplicateRepositoryError);
    EXPECT_THROW(p.add_repository(5, r2), DuplicateRepositoryError);

    ASSERT_TRUE(bool(p.fetch_repository(RepositoryName("repo1"))));
    EXPECT_EQ(RepositoryName("repo1"), p.fetch_repository(RepositoryName("repo1"))->name());
    ASSERT_TRUE(bool(p.fetch_repository(RepositoryName("repo2"))));
    EXPECT_EQ(RepositoryName("repo2"), p.fetch_repository(RepositoryName("repo2"))->name());

    EXPECT_TRUE(! p.more_important_than(RepositoryName("repo1"), RepositoryName("repo2")));
    EXPECT_TRUE(p.more_important_than(RepositoryName("repo2"), RepositoryName("repo1")));
    EXPECT_TRUE(! p.more_important_than(RepositoryName("repo2"), RepositoryName("repo2")));
    EXPECT_TRUE(! p.more_important_than(RepositoryName("repo1"), RepositoryName("repo1")));
}

namespace
{
    struct CoolFakeRepository :
        FakeRepository
    {
        CoolFakeRepository(const Environment * const e, const RepositoryName & rn) :
            FakeRepository(make_named_values<FakeRepositoryParams>(
                        n::environment() = e,
                        n::name() = rn
                        ))
        {
        }

        std::shared_ptr<const CategoryNamePartSet> unimportant_category_names(const RepositoryContentMayExcludes &) const
        {
            std::shared_ptr<CategoryNamePartSet> result(std::make_shared<CategoryNamePartSet>());
            result->insert(CategoryNamePart("bad-cat1"));
            result->insert(CategoryNamePart("bad-cat2"));
            return result;
        }
    };
}

TEST(PackageDatabase, Disambiguation)
{
    TestEnvironment e;
    PackageDatabase & p(*e.package_database());

    std::shared_ptr<FakeRepository> r1(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &e,
                    n::name() = RepositoryName("repo1"))));
    r1->add_version(CategoryNamePart("cat-one") + PackageNamePart("pkg-one"), VersionSpec("0", { }));
    r1->add_version(CategoryNamePart("cat-one") + PackageNamePart("pkg-two"), VersionSpec("0", { }));
    r1->add_version(CategoryNamePart("cat-two") + PackageNamePart("pkg-two"), VersionSpec("0", { }));
    r1->add_version(CategoryNamePart("cat-two") + PackageNamePart("pkg-three"), VersionSpec("0", { }));
    p.add_repository(10, r1);

    std::shared_ptr<FakeRepository> r2(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &e,
                    n::name() = RepositoryName("repo2"))));
    r2->add_version(CategoryNamePart("cat-three") + PackageNamePart("pkg-three"), VersionSpec("0", { }));
    r2->add_version(CategoryNamePart("cat-three") + PackageNamePart("pkg-four"), VersionSpec("0", { }));
    p.add_repository(10, r2);

    std::shared_ptr<FakeRepository> r3(std::make_shared<CoolFakeRepository>(&e, RepositoryName("repo3")));
    r3->add_version(CategoryNamePart("bad-cat1") + PackageNamePart("pkg-important"), VersionSpec("0", { }));
    r3->add_version(CategoryNamePart("good-cat1") + PackageNamePart("pkg-important"), VersionSpec("0", { }));

    r3->add_version(CategoryNamePart("good-cat1") + PackageNamePart("pkg-installed"), VersionSpec("0", { }));
    r3->add_version(CategoryNamePart("good-cat2") + PackageNamePart("pkg-installed"), VersionSpec("0", { }));

    r3->add_version(CategoryNamePart("bad-cat1") + PackageNamePart("pkg-fail1"), VersionSpec("0", { }));
    r3->add_version(CategoryNamePart("bad-cat2") + PackageNamePart("pkg-fail1"), VersionSpec("0", { }));

    r3->add_version(CategoryNamePart("bad-cat1") + PackageNamePart("pkg-fail2"), VersionSpec("0", { }));
    r3->add_version(CategoryNamePart("bad-cat2") + PackageNamePart("pkg-fail2"), VersionSpec("0", { }));

    r3->add_version(CategoryNamePart("good-cat1") + PackageNamePart("pkg-fail3"), VersionSpec("0", { }));
    r3->add_version(CategoryNamePart("good-cat2") + PackageNamePart("pkg-fail3"), VersionSpec("0", { }));

    r3->add_version(CategoryNamePart("good-cat1") + PackageNamePart("pkg-fail4"), VersionSpec("0", { }));
    r3->add_version(CategoryNamePart("good-cat2") + PackageNamePart("pkg-fail4"), VersionSpec("0", { }));

    r3->add_version(CategoryNamePart("avail-cat") + PackageNamePart("pkg-foo"), VersionSpec("0", { }));
    p.add_repository(10, r3);

    std::shared_ptr<FakeInstalledRepository> r4(std::make_shared<FakeInstalledRepository>(
                make_named_values<FakeInstalledRepositoryParams>(
                    n::environment() = &e,
                    n::name() = RepositoryName("repo4"),
                    n::suitable_destination() = true,
                    n::supports_uninstall() = true
                    )));
    r4->add_version(CategoryNamePart("good-cat1") + PackageNamePart("pkg-installed"), VersionSpec("0", { }));
    r4->add_version(CategoryNamePart("good-cat1") + PackageNamePart("pkg-fail4"), VersionSpec("0", { }));
    r4->add_version(CategoryNamePart("good-cat2") + PackageNamePart("pkg-fail4"), VersionSpec("0", { }));
    r4->add_version(CategoryNamePart("inst-cat") + PackageNamePart("pkg-foo"), VersionSpec("0", { }));
    p.add_repository(10, r4);

    EXPECT_EQ("cat-one/pkg-one", stringify(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-one"))));
    EXPECT_EQ("cat-three/pkg-four", stringify(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-four"))));
    EXPECT_EQ("good-cat1/pkg-important", stringify(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-important"))));
    EXPECT_EQ("good-cat1/pkg-installed", stringify(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-installed"))));

    EXPECT_THROW(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-two")), AmbiguousPackageNameError);
    EXPECT_THROW(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-three")), AmbiguousPackageNameError);

    EXPECT_THROW(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-fail1")), AmbiguousPackageNameError);
    EXPECT_THROW(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-fail2")), AmbiguousPackageNameError);
    EXPECT_THROW(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-fail3")), AmbiguousPackageNameError);
    EXPECT_THROW(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-fail4")), AmbiguousPackageNameError);

    EXPECT_THROW(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-five")), NoSuchPackageError);

    EXPECT_THROW(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-one"),
                filter::SupportsAction<ConfigAction>()), NoSuchPackageError);
    EXPECT_EQ("inst-cat/pkg-foo", stringify(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-foo"))));
    EXPECT_EQ("avail-cat/pkg-foo", stringify(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-foo"), filter::SupportsAction<InstallAction>())));
    EXPECT_THROW(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-foo"), filter::All(), false), AmbiguousPackageNameError);

}

