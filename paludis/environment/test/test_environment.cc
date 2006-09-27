/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/environment/test/test_environment.hh>
#include <paludis/util/collection_concrete.hh>
#include <map>
#include <string>

/** \file
 * Implementation of TestEnvironment.
 *
 * \ingroup grptestenvironment
 */

using namespace paludis;

TestEnvironment::TestEnvironment() :
    Environment(PackageDatabase::Pointer(new PackageDatabase(this)))
{
}

bool
TestEnvironment::query_use(const UseFlagName & u, const PackageDatabaseEntry *) const
{
    return (std::string::npos != u.data().find("enabled"));
}

bool
TestEnvironment::accept_keyword(const KeywordName & k, const PackageDatabaseEntry * const) const
{
    return k == KeywordName("test");
}

bool
TestEnvironment::accept_license(const std::string &, const PackageDatabaseEntry * const) const
{
    return true;
}

bool
TestEnvironment::query_user_masks(const PackageDatabaseEntry &) const
{
    return false;
}

bool
TestEnvironment::query_user_unmasks(const PackageDatabaseEntry &) const
{
    return false;
}

namespace
{
    static const std::multimap<std::string, std::string> test_environment_mirrors;
}

TestEnvironment::MirrorIterator
TestEnvironment::begin_mirrors(const std::string &) const
{
    return MirrorIterator(test_environment_mirrors.begin());
}

TestEnvironment::MirrorIterator
TestEnvironment::end_mirrors(const std::string &) const
{
    return MirrorIterator(test_environment_mirrors.end());
}


