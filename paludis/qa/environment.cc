/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <map>
#include <paludis/qa/environment.hh>

using namespace paludis;
using namespace paludis::qa;

QAEnvironment::QAEnvironment(const FSEntry & base) :
    Environment(PackageDatabase::Pointer(new PackageDatabase(this)))
{
    MirrorIterator _mirrors;
    std::map<std::string, std::string> keys;

    keys.insert(std::make_pair(std::string("format"), std::string("portage")));
    keys.insert(std::make_pair(std::string("importace"), "1"));
    keys.insert(std::make_pair(std::string("location"), stringify(base)));
    keys.insert(std::make_pair(std::string("cache"), "/var/empty"));
    keys.insert(std::make_pair(std::string("profile"), stringify(base / "profiles" / "base")));

    package_database()->add_repository(
            RepositoryMaker::get_instance()->find_maker("portage")(this,
            package_database().raw_pointer(), keys));
}

QAEnvironment::~QAEnvironment()
{
}

bool
QAEnvironment::query_use(const UseFlagName &, const PackageDatabaseEntry *) const
{
    return false;
}

bool
QAEnvironment::accept_keyword(const KeywordName &, const PackageDatabaseEntry * const) const
{
    return false;
}

bool
QAEnvironment::accept_license(const std::string &, const PackageDatabaseEntry * const) const
{
    return false;
}

bool
QAEnvironment::query_user_masks(const PackageDatabaseEntry &) const
{
    return false;
}

bool
QAEnvironment::query_user_unmasks(const PackageDatabaseEntry &) const
{
    return false;
}
