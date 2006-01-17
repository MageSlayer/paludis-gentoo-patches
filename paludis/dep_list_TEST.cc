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

#include "paludis.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

namespace test_cases
{
    /**
     * \test Test dep list: one package.
     *
     * \ingroup Test
     */
    struct DepListOnePackageTest : TestCase
    {
        DepListOnePackageTest() : TestCase("one package") { }

        void run()
        {
            TestEnvironment e;

            /* t/one exists */
            FakeRepository::Pointer repo(new FakeRepository(RepositoryName("repo")));
            repo->add_version(CategoryNamePart("t"), PackageNamePart("one"), VersionSpec("1.0"))->
                set(vmk_slot, "slot1");
            e.package_db()->add_repository(repo);

            FakeRepository::Pointer installed(new FakeRepository(RepositoryName("installed")));
            e.installed_db()->add_repository(installed);

            DepList d(&e);
            d.add(DepParser::parse("t/one"));
            TEST_CHECK(d.begin() != d.end());
            TEST_CHECK_EQUAL(std::distance(d.begin(), d.end()), 1);
            TEST_CHECK_EQUAL(*d.begin(), DepListEntry(
                        QualifiedPackageName(CategoryNamePart("t"), PackageNamePart("one")),
                        VersionSpec("1.0"), SlotName("slot1"), RepositoryName("repo")));
        }
    } test_dep_list_one_package;

    /**
     * \test Test dep list: one installed package.
     *
     * \ingroup Test
     */
    struct DepListOneInstalledPackageTest : TestCase
    {
        DepListOneInstalledPackageTest() : TestCase("one installed package") { }

        void run()
        {
            TestEnvironment e;

            /* t/one exists and is already installed. */
            FakeRepository::Pointer repo(new FakeRepository(RepositoryName("repo")));
            repo->add_version(CategoryNamePart("t"), PackageNamePart("one"), VersionSpec("1.0"))->
                set(vmk_slot, "slot1");
            e.package_db()->add_repository(repo);

            FakeRepository::Pointer installed(new FakeRepository(RepositoryName("installed")));
            installed->add_version(CategoryNamePart("t"), PackageNamePart("one"), VersionSpec("0.9"))->
                set(vmk_slot, "slot1");
            e.installed_db()->add_repository(installed);

            DepList d(&e);
            d.add(DepParser::parse("t/one"));
            TEST_CHECK(d.begin() == d.end());
        }
    } test_dep_list_one_installed_package;

    /**
     * \test Test dep list: dep.
     *
     * \ingroup Test
     */
    struct DepListDepTest : TestCase
    {
        DepListDepTest() : TestCase("dep") { }

        void run()
        {
            TestEnvironment e;

            /* t/one DEPENDs upon t/two */
            FakeRepository::Pointer repo(new FakeRepository(RepositoryName("repo")));

            repo->add_version(CategoryNamePart("t"), PackageNamePart("one"), VersionSpec("1.0"))->
                set(vmk_slot, "slot1").
                set(vmk_depend, "t/two");

            repo->add_version(CategoryNamePart("t"), PackageNamePart("two"), VersionSpec("1.0"))->
                set(vmk_slot, "slot2");

            e.package_db()->add_repository(repo);

            FakeRepository::Pointer installed(new FakeRepository(RepositoryName("installed")));
            e.installed_db()->add_repository(installed);

            DepList d(&e);
            d.add(DepParser::parse("t/one"));
            DepList::Iterator di(d.begin()), di_end(d.end());
            TEST_CHECK(di != di_end);
            TEST_CHECK_EQUAL(*di, DepListEntry(
                        QualifiedPackageName(CategoryNamePart("t"), PackageNamePart("two")),
                        VersionSpec("1.0"), SlotName("slot2"), RepositoryName("repo")));
            TEST_CHECK(++di != di_end);
            TEST_CHECK_EQUAL(*di, DepListEntry(
                        QualifiedPackageName(CategoryNamePart("t"), PackageNamePart("one")),
                        VersionSpec("1.0"), SlotName("slot1"), RepositoryName("repo")));
            TEST_CHECK(++di == di_end);
        }
    } test_dep_list_dep;

    /**
     * \test Test dep list: pdep.
     *
     * \ingroup Test
     */
    struct DepListPDepTest : TestCase
    {
        DepListPDepTest() : TestCase("pdep") { }

        void run()
        {
            TestEnvironment e;

            /* t/one PDEPENDs upon t/two */
            FakeRepository::Pointer repo(new FakeRepository(RepositoryName("repo")));

            repo->add_version(CategoryNamePart("t"), PackageNamePart("one"), VersionSpec("1.0"))->
                set(vmk_slot, "slot1").
                set(vmk_pdepend, "t/two");

            repo->add_version(CategoryNamePart("t"), PackageNamePart("two"), VersionSpec("1.0"))->
                set(vmk_slot, "slot2");

            e.package_db()->add_repository(repo);

            FakeRepository::Pointer installed(new FakeRepository(RepositoryName("installed")));
            e.installed_db()->add_repository(installed);

            DepList d(&e);
            d.add(DepParser::parse("t/one"));
            DepList::Iterator di(d.begin()), di_end(d.end());
            TEST_CHECK(di != di_end);
            TEST_CHECK_EQUAL(*di, DepListEntry(
                        QualifiedPackageName(CategoryNamePart("t"), PackageNamePart("one")),
                        VersionSpec("1.0"), SlotName("slot1"), RepositoryName("repo")));
            TEST_CHECK(++di != di_end);
            TEST_CHECK_EQUAL(*di, DepListEntry(
                        QualifiedPackageName(CategoryNamePart("t"), PackageNamePart("two")),
                        VersionSpec("1.0"), SlotName("slot2"), RepositoryName("repo")));
            TEST_CHECK(++di == di_end);
        }
    } test_dep_list_pdep;

    /**
     * \test Test dep list: common deps.
     *
     * \ingroup Test
     */
    struct DepListCommonDepsTest : TestCase
    {
        DepListCommonDepsTest() : TestCase("common deps") { }

        void run()
        {
            TestEnvironment e;

            /* t/one DEPENDs upon ( t/two t/three ) ; t/two DEPENDs upon t/three */
            FakeRepository::Pointer repo(new FakeRepository(RepositoryName("repo")));

            repo->add_version(CategoryNamePart("t"), PackageNamePart("one"), VersionSpec("1.0"))->
                set(vmk_slot, "slot1").
                set(vmk_depend, "t/two t/three");

            repo->add_version(CategoryNamePart("t"), PackageNamePart("two"), VersionSpec("1.0"))->
                set(vmk_slot, "slot2").
                set(vmk_depend, "t/three");

            repo->add_version(CategoryNamePart("t"), PackageNamePart("three"), VersionSpec("1.0"))->
                set(vmk_slot, "slot3");

            e.package_db()->add_repository(repo);

            FakeRepository::Pointer installed(new FakeRepository(RepositoryName("installed")));
            e.installed_db()->add_repository(installed);

            DepList d(&e);
            d.add(DepParser::parse("t/one"));
            DepList::Iterator di(d.begin()), di_end(d.end());
            TEST_CHECK(di != di_end);
            TEST_CHECK_EQUAL(*di, DepListEntry(
                        QualifiedPackageName(CategoryNamePart("t"), PackageNamePart("three")),
                        VersionSpec("1.0"), SlotName("slot3"), RepositoryName("repo")));
            TEST_CHECK(++di != di_end);
            TEST_CHECK_EQUAL(*di, DepListEntry(
                        QualifiedPackageName(CategoryNamePart("t"), PackageNamePart("two")),
                        VersionSpec("1.0"), SlotName("slot2"), RepositoryName("repo")));
            TEST_CHECK(++di != di_end);
            TEST_CHECK_EQUAL(*di, DepListEntry(
                        QualifiedPackageName(CategoryNamePart("t"), PackageNamePart("one")),
                        VersionSpec("1.0"), SlotName("slot1"), RepositoryName("repo")));
            TEST_CHECK(++di == di_end);
        }
    } test_dep_list_common_deps;

    /**
     * \test Test dep list: dep failure.
     *
     * \ingroup Test
     */
    struct DepListDepFailureTest : TestCase
    {
        DepListDepFailureTest() : TestCase("dep failure") { }

        void run_d(VersionMetadataKey dep_kind)
        {
            TestMessageSuffix suffix(stringify(dep_kind), true);
            TestEnvironment e;

            /* t/one DEPENDs upon t/two and t/three. t/three has unresolvable
             * deps. */
            FakeRepository::Pointer repo(new FakeRepository(RepositoryName("repo")));

            repo->add_version(CategoryNamePart("t"), PackageNamePart("one"), VersionSpec("1.0"))->
                set(vmk_slot, "slot1").
                set(dep_kind, "t/two t/three");

            repo->add_version(CategoryNamePart("t"), PackageNamePart("two"), VersionSpec("1.0"))->
                set(vmk_slot, "slot2");

            repo->add_version(CategoryNamePart("t"), PackageNamePart("three"), VersionSpec("1.0"))->
                set(vmk_slot, "slot3").
                set(dep_kind, "t/bad");

            e.package_db()->add_repository(repo);

            FakeRepository::Pointer installed(new FakeRepository(RepositoryName("installed")));
            e.installed_db()->add_repository(installed);

            DepList d(&e);
            TEST_CHECK_THROWS(d.add(DepParser::parse("t/one")), Exception);
            TEST_CHECK(d.begin() == d.end());

            /* try again, with some stuff already in the dep list. */
            d.add(DepParser::parse("t/two"));
            DepList::Iterator di(d.begin()), di_end(d.end());
            TEST_CHECK(di != di_end);
            TEST_CHECK_EQUAL(*di, DepListEntry(
                        QualifiedPackageName(CategoryNamePart("t"), PackageNamePart("two")),
                        VersionSpec("1.0"), SlotName("slot2"), RepositoryName("repo")));
            TEST_CHECK(++di == di_end);

            TEST_CHECK_THROWS(d.add(DepParser::parse("t/one")), Exception);
            di = d.begin();
            TEST_CHECK(di != di_end);
            TEST_CHECK_EQUAL(*di, DepListEntry(
                        QualifiedPackageName(CategoryNamePart("t"), PackageNamePart("two")),
                        VersionSpec("1.0"), SlotName("slot2"), RepositoryName("repo")));
            TEST_CHECK(++di == di_end);
        }

        void run()
        {
            run_d(vmk_depend);
            run_d(vmk_rdepend);
            run_d(vmk_pdepend);
        }
    } test_dep_list_dep_failure;

    /**
     * \test Test dep list: indirect circular deps.
     *
     * \ingroup Test
     */
    struct DepListIndirectCircularDepTest : TestCase
    {
        DepListIndirectCircularDepTest() : TestCase("indirect circular deps") { }

        void run()
        {
            TestEnvironment e;

            /* t/one DEPENDs upon t/two. t/two DEPENDs upon t/three. t/three DEPENDs
             * upon t/two */
            FakeRepository::Pointer repo(new FakeRepository(RepositoryName("repo")));

            repo->add_version(CategoryNamePart("t"), PackageNamePart("one"), VersionSpec("1.0"))->
                set(vmk_slot, "slot1").
                set(vmk_depend, "t/two");

            repo->add_version(CategoryNamePart("t"), PackageNamePart("two"), VersionSpec("1.0"))->
                set(vmk_slot, "slot2").
                set(vmk_depend, "t/three");

            repo->add_version(CategoryNamePart("t"), PackageNamePart("three"), VersionSpec("1.0"))->
                set(vmk_slot, "slot3").
                set(vmk_depend, "t/two");

            e.package_db()->add_repository(repo);

            FakeRepository::Pointer installed(new FakeRepository(RepositoryName("installed")));
            e.installed_db()->add_repository(installed);

            DepList d(&e);
            TEST_CHECK_THROWS(d.add(DepParser::parse("t/one")), CircularDependencyError);
            TEST_CHECK(d.begin() == d.end());

            installed->add_version(CategoryNamePart("t"), PackageNamePart("three"), VersionSpec("1.0"));
            d.add(DepParser::parse("t/one"));

            DepList::Iterator di(d.begin()), di_end(d.end());
            TEST_CHECK(di != di_end);
            TEST_CHECK_EQUAL(*di, DepListEntry(
                        QualifiedPackageName(CategoryNamePart("t"), PackageNamePart("two")),
                        VersionSpec("1.0"), SlotName("slot2"), RepositoryName("repo")));
            TEST_CHECK(++di != di_end);
            TEST_CHECK_EQUAL(*di, DepListEntry(
                        QualifiedPackageName(CategoryNamePart("t"), PackageNamePart("one")),
                        VersionSpec("1.0"), SlotName("slot1"), RepositoryName("repo")));
            TEST_CHECK(++di == di_end);
        }
    } test_dep_list_indirect_circular_deps;

    /**
     * \test Test dep list: self circular deps.
     *
     * \ingroup Test
     */
    struct DepListSelfCircularDepTest : TestCase
    {
        DepListSelfCircularDepTest() : TestCase("self circular deps") { }

        void run()
        {
            TestEnvironment e;

            /* t/one DEPENDs upon t/two. t/two DEPENDs upon t/two. */
            FakeRepository::Pointer repo(new FakeRepository(RepositoryName("repo")));

            repo->add_version(CategoryNamePart("t"), PackageNamePart("one"), VersionSpec("1.0"))->
                set(vmk_slot, "slot1").
                set(vmk_depend, "t/two");

            repo->add_version(CategoryNamePart("t"), PackageNamePart("two"), VersionSpec("1.0"))->
                set(vmk_slot, "slot2").
                set(vmk_depend, "t/two");

            e.package_db()->add_repository(repo);

            FakeRepository::Pointer installed(new FakeRepository(RepositoryName("installed")));
            e.installed_db()->add_repository(installed);

            DepList d(&e);
            TEST_CHECK_THROWS(d.add(DepParser::parse("t/one")), CircularDependencyError);
            TEST_CHECK(d.begin() == d.end());

            installed->add_version(CategoryNamePart("t"), PackageNamePart("two"), VersionSpec("1.0"));
            d.add(DepParser::parse("t/one"));

            DepList::Iterator di(d.begin()), di_end(d.end());
            TEST_CHECK(di != di_end);
            TEST_CHECK_EQUAL(*di, DepListEntry(
                        QualifiedPackageName(CategoryNamePart("t"), PackageNamePart("one")),
                        VersionSpec("1.0"), SlotName("slot1"), RepositoryName("repo")));
            TEST_CHECK(++di == di_end);
        }
    } test_dep_list_self_circular_deps;

    /**
     * \test Test dep list: met or dep.
     *
     * \ingroup Test
     */
    struct DepListMetOrDepTest : TestCase
    {
        DepListMetOrDepTest() : TestCase("met or dep") { }

        void run()
        {
            TestEnvironment e;

            /* t/one DEPENDs upon || ( t/two t/three ). t/three is already
             * installed */
            FakeRepository::Pointer repo(new FakeRepository(RepositoryName("repo")));

            repo->add_version(CategoryNamePart("t"), PackageNamePart("one"), VersionSpec("1.0"))->
                set(vmk_slot, "slot1").
                set(vmk_depend, "|| ( t/two t/three )");

            e.package_db()->add_repository(repo);

            FakeRepository::Pointer installed(new FakeRepository(RepositoryName("installed")));

            installed->add_version(CategoryNamePart("t"), PackageNamePart("three"), VersionSpec("1.0"))->
                set(vmk_slot, "slot2");
            e.installed_db()->add_repository(installed);

            DepList d(&e);
            d.add(DepParser::parse("t/one"));
            DepList::Iterator di(d.begin()), di_end(d.end());
            TEST_CHECK(di != di_end);
            TEST_CHECK_EQUAL(*di, DepListEntry(
                        QualifiedPackageName(CategoryNamePart("t"), PackageNamePart("one")),
                        VersionSpec("1.0"), SlotName("slot1"), RepositoryName("repo")));
            TEST_CHECK(++di == di_end);
        }
    } test_dep_list_met_or_dep;

    /**
     * \test Test dep list: complex met or dep.
     *
     * \ingroup Test
     */
    struct DepListComplexMetOrDepTest : TestCase
    {
        DepListComplexMetOrDepTest() : TestCase("complex met or dep") { }

        void run_n(int n)
        {
            TestEnvironment e;
            TestMessageSuffix(stringify(n), true);

            /* t/one DEPENDs upon
             * || (
             *     ( t/two t/three )
             *     (
             *         t/four
             *         || (
             *             t/five
             *             ( t/six t/seven )
             *         )
             *      )
             *  )
             */
            FakeRepository::Pointer repo(new FakeRepository(RepositoryName("repo")));

            repo->add_version(CategoryNamePart("t"), PackageNamePart("one"), VersionSpec("1.0"))->
                set(vmk_slot, "1").
                set(vmk_depend, "|| ( ( t/two t/three ) ( t/four || ( t/five ( t/six t/seven ) ) ) )");

            e.package_db()->add_repository(repo);

            FakeRepository::Pointer installed(new FakeRepository(RepositoryName("installed")));

            if (n == 1)
            {
                installed->add_version(CategoryNamePart("t"), PackageNamePart("two"), VersionSpec("1.0"));
                installed->add_version(CategoryNamePart("t"), PackageNamePart("three"), VersionSpec("1.0"));
            }
            else if (n == 2)
            {
                installed->add_version(CategoryNamePart("t"), PackageNamePart("four"), VersionSpec("1.0"));
                installed->add_version(CategoryNamePart("t"), PackageNamePart("five"), VersionSpec("1.0"));
            }
            else if (n == 3)
            {
                installed->add_version(CategoryNamePart("t"), PackageNamePart("four"), VersionSpec("1.0"));
                installed->add_version(CategoryNamePart("t"), PackageNamePart("six"), VersionSpec("1.0"));
                installed->add_version(CategoryNamePart("t"), PackageNamePart("seven"), VersionSpec("1.0"));
            }

            e.installed_db()->add_repository(installed);

            DepList d(&e);
            d.add(DepParser::parse("t/one"));
            DepList::Iterator di(d.begin()), di_end(d.end());
            TEST_CHECK(di != di_end);
            TEST_CHECK_EQUAL(*di, DepListEntry(
                        QualifiedPackageName(CategoryNamePart("t"), PackageNamePart("one")),
                        VersionSpec("1.0"), SlotName("1"), RepositoryName("repo")));
            TEST_CHECK(++di == di_end);
        }

        void run()
        {
            for (int n = 1 ; n <= 3 ; ++n)
                run_n(n);
        }
    } test_dep_list_complex_met_or_dep;

    /**
     * \test Test dep list: complex unmet or dep.
     *
     * \ingroup Test
     */
    struct DepListComplexUnmetOrDepTest : TestCase
    {
        DepListComplexUnmetOrDepTest() : TestCase("complex unmet or dep") { }

        void run_n(int n)
        {
            TestEnvironment e;
            TestMessageSuffix suffix(stringify(n), true);

            /* t/one DEPENDs upon
             * || (
             *     ( t/two t/three )
             *     (
             *         t/four
             *         || (
             *             t/five
             *             ( t/six t/seven )
             *         )
             *      )
             *  )
             */
            FakeRepository::Pointer repo(new FakeRepository(RepositoryName("repo")));

            repo->add_version(CategoryNamePart("t"), PackageNamePart("one"), VersionSpec("1.0"))->
                set(vmk_slot, "1").
                set(vmk_depend, "|| ( ( t/two t/three ) ( t/four || ( t/five ( t/six t/seven ) ) ) )");

            e.package_db()->add_repository(repo);

            FakeRepository::Pointer installed(new FakeRepository(RepositoryName("installed")));

            if (n == 1)
            {
                installed->add_version(CategoryNamePart("t"), PackageNamePart("two"), VersionSpec("1.0"));
            }
            else if (n == 2)
            {
                installed->add_version(CategoryNamePart("t"), PackageNamePart("three"), VersionSpec("1.0"));
            }
            else if (n == 3)
            {
                installed->add_version(CategoryNamePart("t"), PackageNamePart("five"), VersionSpec("1.0"));
            }
            else if (n == 4)
            {
                installed->add_version(CategoryNamePart("t"), PackageNamePart("four"), VersionSpec("1.0"));
                installed->add_version(CategoryNamePart("t"), PackageNamePart("seven"), VersionSpec("1.0"));
            }

            e.installed_db()->add_repository(installed);

            DepList d(&e);
            TEST_CHECK(d.begin() == d.end());
            TEST_CHECK_THROWS(d.add(DepParser::parse("t/one")), DepListError);
            TEST_CHECK(d.begin() == d.end());
        }

        void run()
        {
            for (int n = 1 ; n <= 4 ; ++n)
                run_n(n);
        }
    } test_dep_list_complex_unmet_or_dep;

    /**
     * \test Test dep list: or all use dep.
     *
     * \ingroup Test
     */
    struct DepListOrAllUseDepTest : TestCase
    {
        DepListOrAllUseDepTest() : TestCase("or all use dep") { }

        void run()
        {
            TestEnvironment e;

            FakeRepository::Pointer repo(new FakeRepository(RepositoryName("repo")));

            repo->add_version(CategoryNamePart("t"), PackageNamePart("one"), VersionSpec("1.0"))->
                set(vmk_slot, "1").
                set(vmk_depend, "|| ( t/two ( off? ( t/three ) ) )");

            repo->add_version(CategoryNamePart("t"), PackageNamePart("two"), VersionSpec("1.0"))->
                set(vmk_slot, "1");

            repo->add_version(CategoryNamePart("t"), PackageNamePart("three"), VersionSpec("1.0"))->
                set(vmk_slot, "1");

            e.package_db()->add_repository(repo);

            FakeRepository::Pointer installed(new FakeRepository(RepositoryName("installed")));
            e.installed_db()->add_repository(installed);

            DepList d(&e);
            d.add(DepParser::parse("t/one"));
            DepList::Iterator di(d.begin()), di_end(d.end());
            TEST_CHECK(di != di_end);
            TEST_CHECK_EQUAL(*di, DepListEntry(
                        QualifiedPackageName(CategoryNamePart("t"), PackageNamePart("one")),
                        VersionSpec("1.0"), SlotName("1"), RepositoryName("repo")));
            TEST_CHECK(++di == di_end);
        }
    } test_dep_list_or_all_use;

}

