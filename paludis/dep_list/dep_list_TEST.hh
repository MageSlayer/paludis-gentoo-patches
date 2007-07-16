/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_LIST_DEP_LIST_TEST_HH
#define PALUDIS_GUARD_PALUDIS_DEP_LIST_DEP_LIST_TEST_HH 1

#include <paludis/dep_list/dep_list.hh>
#include <paludis/dep_list/exceptions.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/virtuals/virtuals_repository.hh>
#include <paludis/repositories/virtuals/installed_virtuals_repository.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/environments/test/test_environment.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <string>
#include <list>
#include <ostream>

using namespace paludis;
using namespace test;

#ifndef DOXYGEN

namespace paludis
{
    class DepListEntry;
    std::ostream &
    operator<< (std::ostream & s, const DepListEntry & e)
    {
        s << *e.package_id;

        switch (e.kind)
        {
            case dlk_virtual:
            case dlk_package:
            case dlk_provided:
            case dlk_already_installed:
            case dlk_subpackage:
            case dlk_suggested:
                break;

            case dlk_block:
                s << "(!)";
                break;

            case dlk_masked:
                s << "(M)";
                break;

            case last_dlk:
                ;
        }

        return s;
    }
}

namespace test_cases
{
    /**
     * Convenience base class used by many of the DepList tests.
     *
     */
    class DepListTestCaseBase :
        public TestCase
    {
        protected:
            TestEnvironment env;
            tr1::shared_ptr<FakeRepository> repo;
            tr1::shared_ptr<FakeInstalledRepository> installed_repo;
            tr1::shared_ptr<VirtualsRepository> virtuals_repo;
            tr1::shared_ptr<InstalledVirtualsRepository> installed_virtuals_repo;
            std::list<std::string> expected;
            std::string merge_target;
            bool done_populate;

            /**
             * Constructor.
             */
            DepListTestCaseBase(const std::string & s) :
                TestCase(s),
                env(),
                repo(new FakeRepository(&env, RepositoryName("repo"))),
                installed_repo(new FakeInstalledRepository(&env, RepositoryName("installed"))),
                virtuals_repo(new VirtualsRepository(&env)),
                installed_virtuals_repo(new InstalledVirtualsRepository(&env, FSEntry("/"))),
                done_populate(false)
            {
                env.package_database()->add_repository(4, repo);
                env.package_database()->add_repository(3, installed_repo);
                env.package_database()->add_repository(2, virtuals_repo);
                env.package_database()->add_repository(1, installed_virtuals_repo);
            }

            /**
             * Populate our repo member.
             */
            virtual void populate_repo() = 0;

            /**
             * Populate our expected member.
             */
            virtual void populate_expected() = 0;

            virtual void set_options(DepListOptions &)
            {
            }

            /**
             * Check expected is what we got.
             */
            virtual void check_lists()
            {
                DepList d(&env, DepListOptions());
                set_options(*d.options());
                d.add(PackageDepSpec(merge_target, pds_pm_unspecific), env.default_destinations());
                TEST_CHECK(true);

                TestMessageSuffix s("got={ " + join(d.begin(), d.end(), ", ") + " }", false);
                TestMessageSuffix s2("expected={ " + join(expected.begin(), expected.end(), ", ") + " }", false);

                unsigned n(0);
                std::list<std::string>::const_iterator exp(expected.begin());
                DepList::Iterator got(d.begin());
                while (true)
                {
                    TestMessageSuffix sx(stringify(n++), true);

                    TEST_CHECK((exp == expected.end()) == (got == d.end()));
                    if (got == d.end())
                        break;
                    TEST_CHECK_STRINGIFY_EQUAL(*got, *exp);
                    ++exp;
                    ++got;
                }

                d.clear();
                TEST_CHECK(d.begin() == d.end());
            }

        public:
            void run()
            {
                if (! done_populate)
                {
                    populate_repo();
                    populate_expected();
                    done_populate = true;
                }
                check_lists();
            }
    };
}

#endif

#endif
