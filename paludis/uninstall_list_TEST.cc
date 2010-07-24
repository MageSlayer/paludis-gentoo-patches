/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/uninstall_list.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/package_database.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/repository_factory.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <string>
#include <list>
#include <ostream>

#include "config.h"

using namespace paludis;
using namespace test;

namespace paludis
{
    std::ostream &
    operator<< (std::ostream & s, const UninstallListEntry & e)
    {
        s << *e.package_id();
        return s;
    }

    template <typename T_>
    T_ make_k_result(T_ t)
    {
        return t;
    }

    template <typename T_>
    std::function<T_ ()> make_k(T_ t)
    {
        return std::bind(&make_k_result<T_>, t);
    }
}

#ifdef ENABLE_VIRTUALS_REPOSITORY
namespace
{
    std::string virtuals_repo_keys(const std::string & key)
    {
        if (key == "format")
            return "virtuals";
        else
            return "";
    }
}
#endif

namespace test_cases
{
    /**
     * Convenience base class used by many of the UninstallList tests.
     *
     */
    class UninstallListTestCaseBase :
        public TestCase
    {
        protected:
            TestEnvironment env;
            std::shared_ptr<FakeInstalledRepository> installed_repo;
#ifdef ENABLE_VIRTUALS_REPOSITORY
            std::shared_ptr<Repository> virtuals_repo;
#endif
            std::shared_ptr<PackageIDSequence> targets;
            std::list<std::string> expected;
            bool done_populate;
            bool unused_target;

            /**
             * Constructor.
             */
            UninstallListTestCaseBase(const std::string & s) :
                TestCase("uninstall list " + s),
                env(),
                installed_repo(std::make_shared<FakeInstalledRepository>(
                            make_named_values<FakeInstalledRepositoryParams>(
                                n::environment() = &env,
                                n::name() = RepositoryName("installed"),
                                n::suitable_destination() = true,
                                n::supports_uninstall() = true
                                ))),
#ifdef ENABLE_VIRTUALS_REPOSITORY
                virtuals_repo(RepositoryFactory::get_instance()->create(&env, virtuals_repo_keys)),
#endif
                targets(std::make_shared<PackageIDSequence>()),
                done_populate(false),
                unused_target(false)
            {
                env.package_database()->add_repository(2, installed_repo);
#ifdef ENABLE_VIRTUALS_REPOSITORY
                env.package_database()->add_repository(1, virtuals_repo);
#endif
            }

            /**
             * Populate our repo member.
             */
            virtual void populate_repo() = 0;

            /**
             * Populate our targets.
             */
            virtual void populate_targets() = 0;

            void add_target(const std::string & p, const std::string & v)
            {
                targets->push_back(
                        env.fetch_package_id(
                            QualifiedPackageName(p),
                            VersionSpec(v, { }),
                            RepositoryName("installed")));
            }

            void add_unused_target()
            {
                unused_target = true;
            }

            /**
             * Populate our expected member.
             */
            virtual void populate_expected() = 0;

            /**
             * Check expected is what we got.
             */
            virtual void check_lists()
            {
                TEST_CHECK(true);
                UninstallList d(&env, options());
                if (unused_target)
                    d.add_unused();
                else
                    for (PackageIDSequence::ConstIterator i(targets->begin()),
                            i_end(targets->end()) ; i != i_end ; ++i)
                        d.add(*i);
                TEST_CHECK(true);

                TestMessageSuffix s("got={ " + join(d.begin(), d.end(), ", ") + " }", false);
                TestMessageSuffix s2("expected={ " + join(expected.begin(), expected.end(), ", ") + " }", false);

                std::list<std::string>::const_iterator exp(expected.begin());
                UninstallList::ConstIterator got(d.begin());
                while (true)
                {
                    TEST_CHECK((exp == expected.end()) == (got == d.end()));
                    if (got == d.end())
                        break;
                    TEST_CHECK_STRINGIFY_EQUAL(*got, *exp);
                    ++exp;
                    ++got;
                }
            }

            virtual UninstallListOptions options()
            {
                return make_named_values<UninstallListOptions>(
                        n::with_dependencies_as_errors() = false,
                        n::with_dependencies_included() = false,
                        n::with_unused_dependencies() = false
                        );
            }

        public:
            void run()
            {
                if (! done_populate)
                {
                    populate_repo();
                    populate_targets();
                    populate_expected();
                    done_populate = true;
                }
                check_lists();
            }
    };

    struct UninstallListSimpleTest :
        public UninstallListTestCaseBase
    {
        UninstallListSimpleTest() : UninstallListTestCaseBase("simple") { }

        void populate_targets()
        {
            add_target("foo/bar", "1");
        }

        void populate_repo()
        {
            installed_repo->add_version("foo", "bar", "1");
        }

        void populate_expected()
        {
            expected.push_back("foo/bar-1:0::installed");
        }
    } uninstall_list_simple_test;

    struct UninstallListRepeatTest :
        public UninstallListTestCaseBase
    {
        UninstallListRepeatTest() : UninstallListTestCaseBase("repeat") { }

        void populate_targets()
        {
            add_target("foo/bar", "1");
            add_target("foo/bar", "1");
        }

        void populate_repo()
        {
            installed_repo->add_version("foo", "bar", "1");
        }

        void populate_expected()
        {
            expected.push_back("foo/bar-1:0::installed");
        }
    } uninstall_list_repeat_test;

    struct UninstallListWithUnusedDepsTest :
        public UninstallListTestCaseBase
    {
        UninstallListWithUnusedDepsTest() : UninstallListTestCaseBase("with unused deps") { }

        void populate_targets()
        {
            add_target("foo/bar", "1");
        }

        void populate_repo()
        {
            installed_repo->add_version("foo", "bar", "1")->build_dependencies_key()->set_from_string("foo/baz");
            installed_repo->add_version("foo", "baz", "2");
        }

        void populate_expected()
        {
            expected.push_back("foo/bar-1:0::installed");
            expected.push_back("foo/baz-2:0::installed");
        }

        UninstallListOptions options()
        {
            return make_named_values<UninstallListOptions>(
                n::with_dependencies_as_errors() = false,
                n::with_dependencies_included() = false,
                n::with_unused_dependencies() = true
                );
        }
    } uninstall_list_with_unused_deps_test;

    struct UninstallListWithUnusedDepsRecursiveTest :
        public UninstallListTestCaseBase
    {
        UninstallListWithUnusedDepsRecursiveTest() : UninstallListTestCaseBase("with unused deps recursive") { }

        void populate_targets()
        {
            add_target("foo/bar", "1");
        }

        void populate_repo()
        {
            installed_repo->add_version("foo", "bar", "1")->build_dependencies_key()->set_from_string("foo/baz");
            installed_repo->add_version("foo", "baz", "2")->build_dependencies_key()->set_from_string("foo/moo");
            installed_repo->add_version("foo", "moo", "3");
        }

        void populate_expected()
        {
            expected.push_back("foo/bar-1:0::installed");
            expected.push_back("foo/baz-2:0::installed");
            expected.push_back("foo/moo-3:0::installed");
        }

        UninstallListOptions options()
        {
            return make_named_values<UninstallListOptions>(
                n::with_dependencies_as_errors() = false,
                n::with_dependencies_included() = false,
                n::with_unused_dependencies() = true
                );
        }
    } uninstall_list_with_unused_deps_recursive_test;

    struct UninstallListWithUnusedDepsWithUsedTest :
        public UninstallListTestCaseBase
    {
        UninstallListWithUnusedDepsWithUsedTest() : UninstallListTestCaseBase("with unused deps with used") { }

        void populate_targets()
        {
            add_target("foo/bar", "1");
        }

        void populate_repo()
        {
            installed_repo->add_version("foo", "bar", "1")->build_dependencies_key()->set_from_string("foo/baz foo/oink");
            installed_repo->add_version("foo", "baz", "2");
            installed_repo->add_version("foo", "moo", "3")->build_dependencies_key()->set_from_string("foo/oink");
            installed_repo->add_version("foo", "oink", "1");
        }

        void populate_expected()
        {
            expected.push_back("foo/bar-1:0::installed");
            expected.push_back("foo/baz-2:0::installed");
        }

        UninstallListOptions options()
        {
            return make_named_values<UninstallListOptions>(
                n::with_dependencies_as_errors() = false,
                n::with_dependencies_included() = false,
                n::with_unused_dependencies() = true
                );
        }
    } uninstall_list_with_unused_deps_with_used_test;

    struct UninstallListWithUnusedDepsWithCrossUsedTest :
        public UninstallListTestCaseBase
    {
        UninstallListWithUnusedDepsWithCrossUsedTest() :
            UninstallListTestCaseBase("with unused deps with cross used") { }

        void populate_targets()
        {
            add_target("foo/moo", "3");
            add_target("foo/bar", "1");
        }

        void populate_repo()
        {
            installed_repo->add_version("foo", "bar", "1")->build_dependencies_key()->set_from_string("foo/baz foo/oink");
            installed_repo->add_version("foo", "baz", "2");
            installed_repo->add_version("foo", "moo", "3")->build_dependencies_key()->set_from_string("foo/oink");
            installed_repo->add_version("foo", "oink", "1");
        }

        void populate_expected()
        {
            expected.push_back("foo/moo-3:0::installed");
            expected.push_back("foo/bar-1:0::installed");
            expected.push_back("foo/baz-2:0::installed");
            expected.push_back("foo/oink-1:0::installed");
        }

        UninstallListOptions options()
        {
            return make_named_values<UninstallListOptions>(
                n::with_dependencies_as_errors() = false,
                n::with_dependencies_included() = false,
                n::with_unused_dependencies() = true
                );
        }
    } uninstall_list_with_unused_deps_with_cross_used_test;

    struct UninstallListWithUnusedDepsWorldTest :
        public UninstallListTestCaseBase
    {
        UninstallListWithUnusedDepsWorldTest() :
            UninstallListTestCaseBase("with unused deps world")
        {
            std::shared_ptr<SetSpecTree> world(std::make_shared<SetSpecTree>(std::make_shared<AllDepSpec>()));
            world->root()->append(std::make_shared<PackageDepSpec>(parse_user_package_dep_spec("foo/moo", &env, { })));
            env.add_set(SetName("world"), SetName("world"), make_k<std::shared_ptr<const SetSpecTree> >(world), false);
        }

        void populate_targets()
        {
            add_target("foo/bar", "1");
        }

        void populate_repo()
        {
            installed_repo->add_version("foo", "bar", "1")->build_dependencies_key()->set_from_string("foo/baz foo/moo");
            installed_repo->add_version("foo", "baz", "2");
            installed_repo->add_version("foo", "moo", "2");
        }

        void populate_expected()
        {
            expected.push_back("foo/bar-1:0::installed");
            expected.push_back("foo/baz-2:0::installed");
        }

        UninstallListOptions options()
        {
            return make_named_values<UninstallListOptions>(
                n::with_dependencies_as_errors() = false,
                n::with_dependencies_included() = false,
                n::with_unused_dependencies() = true
                );
        }
    } uninstall_list_with_unused_deps_world_test;

    struct UninstallListWithUnusedDepsWorldTargetTest :
        public UninstallListTestCaseBase
    {
        UninstallListWithUnusedDepsWorldTargetTest() :
            UninstallListTestCaseBase("with unused deps world target")
        {
            std::shared_ptr<SetSpecTree> world(std::make_shared<SetSpecTree>(std::make_shared<AllDepSpec>()));
            world->root()->append(std::make_shared<PackageDepSpec>(parse_user_package_dep_spec("foo/moo", &env, { })));
            world->root()->append(std::make_shared<PackageDepSpec>(parse_user_package_dep_spec("foo/bar", &env, { })));
            env.add_set(SetName("world"), SetName("world"), make_k(world), false);
        }

        void populate_targets()
        {
            add_target("foo/bar", "1");
        }

        void populate_repo()
        {
            installed_repo->add_version("foo", "bar", "1")->build_dependencies_key()->set_from_string("foo/baz foo/moo");
            installed_repo->add_version("foo", "baz", "2");
            installed_repo->add_version("foo", "moo", "2");
        }

        void populate_expected()
        {
            expected.push_back("foo/bar-1:0::installed");
            expected.push_back("foo/baz-2:0::installed");
        }

        UninstallListOptions options()
        {
            return make_named_values<UninstallListOptions>(
                n::with_dependencies_as_errors() = false,
                n::with_dependencies_included() = false,
                n::with_unused_dependencies() = true
                );
        }
    } uninstall_list_with_unused_deps_world_target_test;

    struct UninstallListWithSlotsTest :
        UninstallListTestCaseBase
    {
        UninstallListWithSlotsTest() :
            UninstallListTestCaseBase("with slots")
        {
            std::shared_ptr<SetSpecTree> world(std::make_shared<SetSpecTree>(std::make_shared<AllDepSpec>()));
            world->root()->append(std::make_shared<PackageDepSpec>(parse_user_package_dep_spec("cat/needs-a", &env, { })));
            world->root()->append(std::make_shared<PackageDepSpec>(parse_user_package_dep_spec("cat/needs-b", &env, { })));
            world->root()->append(std::make_shared<PackageDepSpec>(parse_user_package_dep_spec("cat/needs-c", &env, { })));
            world->root()->append(std::make_shared<PackageDepSpec>(parse_user_package_dep_spec("cat/needs-d", &env, { })));
            env.add_set(SetName("world"), SetName("world"), make_k(world), false);
        }

        void populate_targets()
        {
            add_unused_target();
        }

        void populate_repo()
        {
            installed_repo->add_version("cat", "needs-a", "1")->run_dependencies_key()->set_from_string("cat/a:1");
            installed_repo->add_version("cat", "needs-b", "1")->run_dependencies_key()->set_from_string("cat/b:2");
            installed_repo->add_version("cat", "needs-c", "1")->run_dependencies_key()->set_from_string("cat/c");
            installed_repo->add_version("cat", "needs-d", "1")->run_dependencies_key()->set_from_string("cat/d:*");

            installed_repo->add_version("cat", "a", "1")->set_slot(SlotName("1"));
            installed_repo->add_version("cat", "a", "2")->set_slot(SlotName("2"));

            installed_repo->add_version("cat", "b", "1")->set_slot(SlotName("1"));
            installed_repo->add_version("cat", "b", "2")->set_slot(SlotName("2"));

            installed_repo->add_version("cat", "c", "1")->set_slot(SlotName("1"));
            installed_repo->add_version("cat", "c", "2")->set_slot(SlotName("2"));

            installed_repo->add_version("cat", "d", "1")->set_slot(SlotName("1"));
            installed_repo->add_version("cat", "d", "2")->set_slot(SlotName("2"));
        }

        void populate_expected()
        {
            expected.push_back("cat/a-2:2::installed");
            expected.push_back("cat/b-1:1::installed");
            expected.push_back("cat/d-1:1::installed");
        }

        UninstallListOptions options()
        {
            return make_named_values<UninstallListOptions>(
                n::with_dependencies_as_errors() = false,
                n::with_dependencies_included() = false,
                n::with_unused_dependencies() = false
                );
        }
    } uninstall_list_slots_test;
}

