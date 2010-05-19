/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/hashes.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/system.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/hook.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/choice.hh>
#include <tr1/functional>
#include <tr1/unordered_map>
#include <string>
#include <list>

using namespace paludis;

typedef std::tr1::unordered_map<SetName, std::tr1::shared_ptr<const SetSpecTree>, Hash<SetName> > Sets;

namespace paludis
{
    template<>
    struct Implementation<TestEnvironment>
    {
        std::tr1::shared_ptr<PackageDatabase> package_database;
        std::string paludis_command;
        std::tr1::unordered_map<std::string, Tribool> override_want_choice_enabled;
        FSEntry root;
        Sets sets;

        Implementation(Environment * const e, const FSEntry & r) :
            package_database(new PackageDatabase(e)),
            paludis_command(""),
            root(r)
        {
        }
    };
}

TestEnvironment::TestEnvironment() :
    PrivateImplementationPattern<TestEnvironment>(new Implementation<TestEnvironment>(this, FSEntry("/"))),
    _imp(PrivateImplementationPattern<TestEnvironment>::_imp)
{
}

TestEnvironment::TestEnvironment(const FSEntry & r) :
    PrivateImplementationPattern<TestEnvironment>(new Implementation<TestEnvironment>(this, r)),
    _imp(PrivateImplementationPattern<TestEnvironment>::_imp)
{
}

TestEnvironment::~TestEnvironment()
{
}

bool
TestEnvironment::accept_keywords(const std::tr1::shared_ptr<const KeywordNameSet> & k, const PackageID &) const
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

uid_t
TestEnvironment::reduced_uid() const
{
    return destringify<int>(getenv_with_default("PALUDIS_REDUCED_UID", stringify(getuid())));
}

gid_t
TestEnvironment::reduced_gid() const
{
    return destringify<int>(getenv_with_default("PALUDIS_REDUCED_GID", stringify(getgid())));
}

const FSEntry
TestEnvironment::root() const
{
    return _imp->root;
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
    return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
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
TestEnvironment::mask_for_user(const PackageID &, const bool) const
{
    return std::tr1::shared_ptr<const Mask>();
}

bool
TestEnvironment::unmasked_by_user(const PackageID &) const
{
    return false;
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

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
TestEnvironment::format_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::string> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
TestEnvironment::config_location_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >();
}

const Tribool
TestEnvironment::want_choice_enabled(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const Choice> & c,
        const UnprefixedChoiceName & v
        ) const
{
    std::string s(stringify(c->prefix()) + ":" + stringify(v));
    if (_imp->override_want_choice_enabled.end() != _imp->override_want_choice_enabled.find(s))
        return _imp->override_want_choice_enabled.find(s)->second;

    if (stringify(v) == "pkgname")
    {
        if ("enabled" == stringify(id->name().package()))
            return Tribool(true);
        else if ("disabled" == stringify(id->name().package()))
            return Tribool(false);
        else
            return Tribool(indeterminate);
    }

    if (std::string::npos != stringify(v).find("enabled"))
        return Tribool(true);
    else if (std::string::npos != stringify(v).find("disabled"))
        return Tribool(false);
    else
        return Tribool(indeterminate);
}

const std::string
TestEnvironment::value_for_choice_parameter(
        const std::tr1::shared_ptr<const PackageID> &,
        const std::tr1::shared_ptr<const Choice> &,
        const UnprefixedChoiceName &
        ) const
{
    return "";
}

std::tr1::shared_ptr<const Set<UnprefixedChoiceName> >
TestEnvironment::known_choice_value_names(
        const std::tr1::shared_ptr<const PackageID> &,
        const std::tr1::shared_ptr<const Choice> &
        ) const
{
    return make_shared_ptr(new Set<UnprefixedChoiceName>);
}

const std::tr1::shared_ptr<OutputManager>
TestEnvironment::create_output_manager(const CreateOutputManagerInfo &) const
{
    return make_shared_ptr(new StandardOutputManager);
}

void
TestEnvironment::set_want_choice_enabled(const ChoicePrefixName & p, const UnprefixedChoiceName & n, const Tribool v)
{
    _imp->override_want_choice_enabled[stringify(p) + ":" + stringify(n)] = v;
}

void
TestEnvironment::populate_sets() const
{
}

const std::tr1::shared_ptr<Repository>
TestEnvironment::repository_from_new_config_file(const FSEntry &)
{
    throw InternalError(PALUDIS_HERE, "can't create repositories on the fly for TestEnvironment");
}

void
TestEnvironment::update_config_files_for_package_move(const PackageDepSpec &, const QualifiedPackageName &) const
{
}

