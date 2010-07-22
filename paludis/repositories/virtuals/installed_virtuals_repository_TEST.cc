/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <paludis/repositories/virtuals/installed_virtuals_repository.hh>
#include <paludis/environments/test/test_environment.hh>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct InstalledVirtualsRepositoryTest : TestCase
    {
        InstalledVirtualsRepositoryTest() : TestCase("installed virtuals repository") { }

        void run()
        {
            TestEnvironment env;
            std::shared_ptr<const InstalledVirtualsRepository> r(new InstalledVirtualsRepository(&env, FSEntry("/")));
        }
    } test_installed_virtuals_repository;
}

