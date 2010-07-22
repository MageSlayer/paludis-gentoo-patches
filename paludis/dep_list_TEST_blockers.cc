/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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

#include "dep_list_TEST.hh"
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

using namespace paludis;
using namespace test;

namespace test_cases
{
    struct DepListTestCaseBasicBlock : DepListTestCaseBase
    {
        DepListTestCaseBasicBlock() : DepListTestCaseBase("block") { }

        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("!cat/two");
            installed_repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
        }

        void check_lists()
        {
            TEST_CHECK(true);
            DepList d(&env, DepListOptions());
            d.options()->blocks() = dl_blocks_error;
            TEST_CHECK_THROWS(d.add(parse_user_package_dep_spec(merge_target, &env, UserPackageDepSpecOptions()),
                        env.default_destinations()), DepListError);
            TEST_CHECK(d.begin() == d.end());

            d.options()->blocks() = dl_blocks_accumulate;
            d.add(parse_user_package_dep_spec(merge_target, &env, UserPackageDepSpecOptions()), env.default_destinations());
            TEST_CHECK_EQUAL(std::distance(d.begin(), d.end()), 2);
            TEST_CHECK_EQUAL(d.begin()->kind(), dlk_block);
            TEST_CHECK_STRINGIFY_EQUAL(*d.begin()->package_id(), "cat/two-1:0::installed");
            TEST_CHECK_EQUAL(next(d.begin())->kind(), dlk_package);
            TEST_CHECK_STRINGIFY_EQUAL(*next(d.begin())->package_id(), "cat/one-1:0::repo");
        }
    } test_dep_list_basic_block;

    struct DepListTestCaseListBlock : DepListTestCaseBase
    {
        DepListTestCaseListBlock() : DepListTestCaseBase("block on list ignore") { }

        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two");
            repo->add_version("cat", "two", "1")->build_dependencies_key()->set_from_string("!cat/two");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_list_block;

#ifdef ENABLE_VIRTUALS_REPOSITORY
    struct DepListTestCaseSelfBlock : DepListTestCaseBase
    {
        DepListTestCaseSelfBlock() : DepListTestCaseBase("self block via provide on list") { }

        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two");
            std::shared_ptr<FakePackageID> two_m(repo->add_version("cat", "two", "1"));
            two_m->build_dependencies_key()->set_from_string("!virtual/two");
            two_m->provide_key()->set_from_string("virtual/two");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("virtual/two-1::virtuals (virtual for cat/two-1:0::repo)");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_self_block_list;

    struct DepListTestCaseProvidedBlock : DepListTestCaseBase
    {
        DepListTestCaseProvidedBlock() : DepListTestCaseBase("provided block") { }

        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two");
            std::shared_ptr<FakePackageID> two_m(repo->add_version("cat", "two", "1"));
            two_m->build_dependencies_key()->set_from_string("!virtual/two");
            two_m->provide_key()->set_from_string("virtual/two");
            installed_repo->add_version("other", "two", "1")->provide_key()->set_from_string("virtual/two");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
        }

        void check_lists()
        {
            TEST_CHECK(true);
            DepList d(&env, DepListOptions());
            d.options()->blocks() = dl_blocks_error;
            TEST_CHECK_THROWS(d.add(PackageDepSpec(parse_user_package_dep_spec(merge_target, &env, UserPackageDepSpecOptions())),
                        env.default_destinations()), DepListError);
            TEST_CHECK(d.begin() == d.end());

            d.options()->blocks() = dl_blocks_accumulate;
            d.add(PackageDepSpec(parse_user_package_dep_spec(merge_target, &env, UserPackageDepSpecOptions())), env.default_destinations());
            TEST_CHECK_EQUAL(std::distance(d.begin(), d.end()), 4);
            TEST_CHECK_EQUAL(d.begin()->kind(), dlk_block);
            TEST_CHECK_STRINGIFY_EQUAL(*d.begin()->package_id(), "virtual/two-1::installed-virtuals (virtual for other/two-1:0::installed)");
            TEST_CHECK_EQUAL(next(d.begin())->kind(), dlk_package);
            TEST_CHECK_STRINGIFY_EQUAL(*next(d.begin())->package_id(), "cat/two-1:0::repo");
            TEST_CHECK_EQUAL(next(next(d.begin()))->kind(), dlk_provided);
            TEST_CHECK_STRINGIFY_EQUAL(*next(next(d.begin()))->package_id(), "virtual/two-1::virtuals (virtual for cat/two-1:0::repo)");
            TEST_CHECK_EQUAL(next(next(next(d.begin())))->kind(), dlk_package);
            TEST_CHECK_STRINGIFY_EQUAL(*next(next(next(d.begin())))->package_id(), "cat/one-1:0::repo");
        }
    } test_dep_list_provided_block;
#endif

    struct DepListTestCaseBlockOnList : DepListTestCaseBase
    {
        DepListTestCaseBlockOnList() : DepListTestCaseBase("block on list") { }

        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two cat/three");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1")->build_dependencies_key()->set_from_string("!cat/two");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
        }

        void check_lists()
        {
            TEST_CHECK(true);
            DepList d(&env, DepListOptions());
            d.options()->blocks() = dl_blocks_error;
            TEST_CHECK_THROWS(d.add(PackageDepSpec(parse_user_package_dep_spec(merge_target, &env, UserPackageDepSpecOptions())),
                        env.default_destinations()), DepListError);
            TEST_CHECK(d.begin() == d.end());
        }
    } test_dep_list_block_on_list;

    struct DepListTestCaseNoBlockOnReplaced : DepListTestCaseBase
    {
        DepListTestCaseNoBlockOnReplaced() : DepListTestCaseBase("no block on replaced") { }

        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string(">=cat/two-2 cat/three");
            repo->add_version("cat", "two", "2");
            repo->add_version("cat", "three", "1")->build_dependencies_key()->set_from_string("!<cat/two-2");
            installed_repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-2:0::repo");
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_no_block_on_replaced;

    struct DepListTestCaseNoBlockOnNoUpgrade : DepListTestCaseBase
    {
        DepListTestCaseNoBlockOnNoUpgrade() : DepListTestCaseBase("no block on no upgrade") { }

        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two");
            repo->add_version("cat", "two", "1")->build_dependencies_key()->set_from_string("!cat/two");
            installed_repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::installed");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_no_block_on_no_upgrade;

#ifdef ENABLE_VIRTUALS_REPOSITORY
    struct DepListTestCaseNoBlockOnNoUpgradeViaProvided : DepListTestCaseBase
    {
        DepListTestCaseNoBlockOnNoUpgradeViaProvided() : DepListTestCaseBase("no block on no upgrade via provided") { }

        void populate_repo()
        {
            std::shared_ptr<FakePackageID> one_m(repo->add_version("cat", "one", "1"));
            one_m->provide_key()->set_from_string("virtual/one");
            one_m->build_dependencies_key()->set_from_string("!virtual/one");
            one_m->run_dependencies_key()->set_from_string("!virtual/one");
            std::shared_ptr<FakePackageID> i_one_m(installed_repo->add_version("cat", "one", "1"));
            i_one_m->provide_key()->set_from_string("virtual/one");
            i_one_m->run_dependencies_key()->set_from_string("!virtual/one");
        }

        void populate_expected()
        {
            merge_target = "virtual/one";
            expected.push_back("cat/one-1:0::repo");
            expected.push_back("virtual/one-1::virtuals (virtual for cat/one-1:0::repo)");
        }
    } test_dep_list_no_block_on_no_upgrade_via_provided;

    struct DepListTestCaseNoBlockOnNoReinstallViaProvided : DepListTestCaseBase
    {
        DepListTestCaseNoBlockOnNoReinstallViaProvided() : DepListTestCaseBase("no block on no reinstall via provided") { }

        void populate_repo()
        {
            std::shared_ptr<FakePackageID> one_m(repo->add_version("cat", "one", "1"));
            one_m->provide_key()->set_from_string("virtual/one");
            one_m->build_dependencies_key()->set_from_string("!virtual/one");
            one_m->run_dependencies_key()->set_from_string("!virtual/one");
            std::shared_ptr<FakePackageID> i_one_m(installed_repo->add_version("cat", "one", "1"));
            i_one_m->provide_key()->set_from_string("virtual/one");
            i_one_m->run_dependencies_key()->set_from_string("!virtual/one");
        }

        void populate_expected()
        {
            merge_target = "virtual/one";
            expected.push_back("cat/one-1:0::repo");
            expected.push_back("virtual/one-1::virtuals (virtual for cat/one-1:0::repo)");
        }

        virtual void set_options(DepListOptions & p)
        {
            p.reinstall() = dl_reinstall_always;
        }
    } test_dep_list_no_block_on_no_reinstall_via_provided;

    struct DepListTestCaseNoBlockOnReplacedProvide : DepListTestCaseBase
    {
        DepListTestCaseNoBlockOnReplacedProvide() : DepListTestCaseBase("no block on replaced provide") { }

        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two");
            std::shared_ptr<FakePackageID> two_m(repo->add_version("cat", "two", "2"));
            two_m->provide_key()->set_from_string("virtual/two");
            two_m->build_dependencies_key()->set_from_string("!virtual/two");
            installed_repo->add_version("cat", "two", "1")->provide_key()->set_from_string("virtual/two");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-2:0::repo");
            expected.push_back("virtual/two-2::virtuals (virtual for cat/two-2:0::repo)");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_no_block_on_replaced_provide;
#endif

    struct DepListTestCaseRestrictedOlderSelf : DepListTestCaseBase
    {
        DepListTestCaseRestrictedOlderSelf() : DepListTestCaseBase("restricted older self") { }

        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two");
            repo->add_version("cat", "two", "2")->build_dependencies_key()->set_from_string("!<cat/two-2");
            installed_repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
        }

        void check_lists()
        {
            TEST_CHECK(true);
            DepList d(&env, DepListOptions());
            d.options()->blocks() = dl_blocks_error;
            TEST_CHECK_THROWS(d.add(PackageDepSpec(parse_user_package_dep_spec(merge_target, &env, UserPackageDepSpecOptions())),
                        env.default_destinations()), DepListError);
            TEST_CHECK(d.begin() == d.end());
        }
    } test_dep_list_restricted_older_self;

#ifdef ENABLE_VIRTUALS_REPOSITORY
    struct DepListTestCaseRestrictedOlderSelfProvide : DepListTestCaseBase
    {
        DepListTestCaseRestrictedOlderSelfProvide() : DepListTestCaseBase("restricted older self provide") { }

        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two");
            std::shared_ptr<FakePackageID> two_m(repo->add_version("cat", "two", "2"));
            two_m->build_dependencies_key()->set_from_string("!<virtual/two-2");
            two_m->provide_key()->set_from_string("virtual/two");
            installed_repo->add_version("cat", "two", "1")->provide_key()->set_from_string("virtual/two");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
        }

        void check_lists()
        {
            TEST_CHECK(true);
            DepList d(&env, DepListOptions());
            d.options()->blocks() = dl_blocks_error;
            TEST_CHECK_THROWS(d.add(PackageDepSpec(parse_user_package_dep_spec(merge_target, &env, UserPackageDepSpecOptions())),
                        env.default_destinations()), DepListError);
            TEST_CHECK(d.begin() == d.end());
        }
    } test_dep_list_restricted_older_self_provide;

    struct DepListTestCaseRestrictedOlderOtherProvide : DepListTestCaseBase
    {
        DepListTestCaseRestrictedOlderOtherProvide() : DepListTestCaseBase("restricted older other provide") { }

        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->build_dependencies_key()->set_from_string("cat/two");
            std::shared_ptr<FakePackageID> two_m(repo->add_version("cat", "two", "2"));
            two_m->build_dependencies_key()->set_from_string("!<virtual/two-2");
            two_m->provide_key()->set_from_string("virtual/two");
            installed_repo->add_version("other", "two", "1")->provide_key()->set_from_string("virtual/two");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
        }

        void check_lists()
        {
            TEST_CHECK(true);
            DepList d(&env, DepListOptions());
            d.options()->blocks() = dl_blocks_error;
            TEST_CHECK_THROWS(d.add(PackageDepSpec(parse_user_package_dep_spec(merge_target, &env, UserPackageDepSpecOptions())),
                        env.default_destinations()), DepListError);
            TEST_CHECK(d.begin() == d.end());
        }
    } test_dep_list_restricted_older_other_provide;
#endif
}


