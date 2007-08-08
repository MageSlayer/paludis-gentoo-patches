/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "no_config_environment.hh"
#include <test/test_runner.hh>
#include <test/test_framework.hh>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct NoConfigEnvironmentTest : TestCase
    {
        NoConfigEnvironmentTest() : TestCase("no config environment") { }

        void run()
        {
            NoConfigEnvironment e(no_config_environment::Params::create()
                    .repository_dir(FSEntry("no_config_environment_TEST_dir/repo"))
                    .write_cache(FSEntry("/var/empty"))
                    .accept_unstable(false)
                    .repository_type(no_config_environment::ncer_auto)
                    .disable_metadata_cache(false)
                    .master_repository_dir(FSEntry("/var/empty")));

            TEST_CHECK(e.package_database());
        }
    } test_no_config_environment;
}

