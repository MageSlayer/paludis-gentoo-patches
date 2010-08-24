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

#include "no_config_environment.hh"
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_named_values.hh>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct NoConfigEnvironmentTest : TestCase
    {
        NoConfigEnvironmentTest() : TestCase("no config environment") { }

        void run()
        {
            NoConfigEnvironment e(make_named_values<no_config_environment::Params>(
                        n::accept_unstable() = false,
                        n::disable_metadata_cache() = false,
                        n::extra_accept_keywords() = "",
                        n::extra_params() = std::shared_ptr<Map<std::string, std::string> >(),
                        n::extra_repository_dirs() = std::make_shared<FSPathSequence>(),
                        n::master_repository_name() = "",
                        n::profiles_if_not_auto() = "",
                        n::repository_dir() = FSPath("no_config_environment_TEST_dir/repo"),
                        n::repository_type() = no_config_environment::ncer_auto,
                        n::write_cache() = FSPath("/var/empty")
                    ));

            TEST_CHECK(bool(e.package_database()));
        }
    } test_no_config_environment;
}

