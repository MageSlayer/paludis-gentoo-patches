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

#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/package_database.hh>
#include <string>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<TestEnvironment>
    {
        std::tr1::shared_ptr<PackageDatabase> package_database;
        std::string paludis_command;

        Implementation(Environment * const e) :
            package_database(new PackageDatabase(e)),
            paludis_command("")
        {
        }
    };
}

TestEnvironment::TestEnvironment() :
    PrivateImplementationPattern<TestEnvironment>(new Implementation<TestEnvironment>(this))
{
}

TestEnvironment::~TestEnvironment()
{
}

bool
TestEnvironment::query_use(const UseFlagName & u, const PackageDatabaseEntry &) const
{
    return (std::string::npos != u.data().find("enabled"));
}

bool
TestEnvironment::accept_keywords(std::tr1::shared_ptr<const KeywordNameCollection> k, const PackageDatabaseEntry &) const
{
    return k->end() != k->find(KeywordName("test")) || k->end() != k->find(KeywordName("*"));
}

std::tr1::shared_ptr<PackageDatabase>
TestEnvironment::package_database()
{
    return _imp->package_database;
}

std::tr1::shared_ptr<const PackageDatabase>
TestEnvironment::package_database() const
{
    return _imp->package_database;
}

std::string
TestEnvironment::paludis_command() const
{
    return _imp->paludis_command;
}

void
TestEnvironment::set_paludis_command(const std::string & s)
{
    _imp->paludis_command = s;
}

