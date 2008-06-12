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

#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/hook.hh>
#include <tr1/functional>
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
    PrivateImplementationPattern<TestEnvironment>(new Implementation<TestEnvironment>(this)),
    _imp(PrivateImplementationPattern<TestEnvironment>::_imp)
{
}

TestEnvironment::~TestEnvironment()
{
}

bool
TestEnvironment::query_use(const UseFlagName & u, const PackageID & p) const
{
    if (UseFlagName("pkgname") == u)
        return PackageNamePart("enabled") == p.name().package;

    return (std::string::npos != u.data().find("enabled"));
}

bool
TestEnvironment::accept_keywords(std::tr1::shared_ptr<const KeywordNameSet> k, const PackageID &) const
{
    return k->end() != k->find(KeywordName("test")) || k->end() != k->find(KeywordName("*"));
}

bool
TestEnvironment::accept_license(const std::string &, const PackageID &) const
{
    return true;
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

const std::tr1::shared_ptr<const PackageID>
TestEnvironment::fetch_package_id(const QualifiedPackageName & q,
        const VersionSpec & v, const RepositoryName & r) const
{
    using namespace std::tr1::placeholders;

    std::tr1::shared_ptr<const PackageIDSequence> ids(package_database()->fetch_repository(r)->package_ids(q));
    for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
            i != i_end ; ++i)
        if (v == (*i)->version())
            return *i;
    throw NoSuchPackageError(stringify(q) + "-" + stringify(v) + "::" + stringify(r));
}

std::tr1::shared_ptr<SetSpecTree::ConstItem>
TestEnvironment::local_set(const SetName &) const
{
    return std::tr1::shared_ptr<SetSpecTree::ConstItem>();
}

uid_t
TestEnvironment::reduced_uid() const
{
    return getuid();
}

gid_t
TestEnvironment::reduced_gid() const
{
    return getgid();
}

const FSEntry
TestEnvironment::root() const
{
    return FSEntry("/");
}

std::tr1::shared_ptr<const MirrorsSequence>
TestEnvironment::mirrors(const std::string & s) const
{
    std::tr1::shared_ptr<MirrorsSequence> result(new MirrorsSequence);

    if (s == "example")
    {
        result->push_back("http://example-mirror-1/example-mirror-1/");
        result->push_back("http://example-mirror-2/example-mirror-2/");
    }

    return result;
}

HookResult
TestEnvironment::perform_hook(const Hook &) const
{
    return HookResult(0, "");
}

std::tr1::shared_ptr<const FSEntrySequence>
TestEnvironment::hook_dirs() const
{
    return make_shared_ptr(new FSEntrySequence);
}

const std::tr1::shared_ptr<const Mask>
TestEnvironment::mask_for_breakage(const PackageID &) const
{
    return std::tr1::shared_ptr<const Mask>();
}

const std::tr1::shared_ptr<const Mask>
TestEnvironment::mask_for_user(const PackageID &) const
{
    return std::tr1::shared_ptr<const Mask>();
}

bool
TestEnvironment::unmasked_by_user(const PackageID &) const
{
    return false;
}

std::tr1::shared_ptr<const UseFlagNameSet>
TestEnvironment::known_use_expand_names(const UseFlagName &, const PackageID &) const
{
    return make_shared_ptr(new UseFlagNameSet);
}

std::tr1::shared_ptr<SetSpecTree::ConstItem>
TestEnvironment::world_set() const
{
    return std::tr1::shared_ptr<SetSpecTree::ConstItem>();
}

void
TestEnvironment::add_to_world(const QualifiedPackageName &) const
{
}

void
TestEnvironment::remove_from_world(const QualifiedPackageName &) const
{
}

void
TestEnvironment::add_to_world(const SetName &) const
{
}

void
TestEnvironment::remove_from_world(const SetName &) const
{
}

void
TestEnvironment::need_keys_added() const
{
}

