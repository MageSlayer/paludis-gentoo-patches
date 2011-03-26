/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/util/pimp-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/system.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/env_var_names.hh>

#include <paludis/standard_output_manager.hh>
#include <paludis/package_id.hh>
#include <paludis/hook.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/choice.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/repository.hh>


#include <functional>
#include <unordered_map>
#include <string>
#include <list>

using namespace paludis;

typedef std::unordered_map<SetName, std::shared_ptr<const SetSpecTree>, Hash<SetName> > Sets;

namespace paludis
{
    template<>
    struct Imp<TestEnvironment>
    {
        std::unordered_map<std::string, Tribool> override_want_choice_enabled;
        FSPath root;
        Sets sets;
        std::shared_ptr<LiteralMetadataValueKey<FSPath> > preferred_root_key;
        std::shared_ptr<LiteralMetadataValueKey<FSPath> > system_root_key;

        Imp(const FSPath & r) :
            root(r),
            preferred_root_key(std::make_shared<LiteralMetadataValueKey<FSPath>>("root", "Root", mkt_normal, root)),
            system_root_key(std::make_shared<LiteralMetadataValueKey<FSPath>>("system_root", "System Root", mkt_normal, FSPath("/")))
        {
        }
    };
}

TestEnvironment::TestEnvironment() :
    _imp(FSPath("/"))
{
    add_metadata_key(_imp->preferred_root_key);
    add_metadata_key(_imp->system_root_key);
}

TestEnvironment::TestEnvironment(const FSPath & r) :
    _imp(r)
{
    add_metadata_key(_imp->preferred_root_key);
    add_metadata_key(_imp->system_root_key);
}

TestEnvironment::~TestEnvironment()
{
}

bool
TestEnvironment::accept_keywords(const std::shared_ptr<const KeywordNameSet> & k, const std::shared_ptr<const PackageID> &) const
{
    return k->end() != k->find(KeywordName("test")) || k->end() != k->find(KeywordName("*"));
}

bool
TestEnvironment::accept_license(const std::string &, const std::shared_ptr<const PackageID> &) const
{
    return true;
}

const std::shared_ptr<const PackageID>
TestEnvironment::fetch_package_id(const QualifiedPackageName & q,
        const VersionSpec & v, const RepositoryName & r) const
{
    using namespace std::placeholders;

    std::shared_ptr<const PackageIDSequence> ids(fetch_repository(r)->package_ids(q, { }));
    for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
            i != i_end ; ++i)
        if (v == (*i)->version())
            return *i;
    throw NoSuchPackageError(stringify(q) + "-" + stringify(v) + "::" + stringify(r));
}

uid_t
TestEnvironment::reduced_uid() const
{
    return destringify<int>(getenv_with_default(env_vars::reduced_uid, stringify(getuid())));
}

gid_t
TestEnvironment::reduced_gid() const
{
    return destringify<int>(getenv_with_default(env_vars::reduced_gid, stringify(getgid())));
}

std::shared_ptr<const MirrorsSequence>
TestEnvironment::mirrors(const std::string & s) const
{
    std::shared_ptr<MirrorsSequence> result(std::make_shared<MirrorsSequence>());

    if (s == "example")
    {
        result->push_back("http://example-mirror-1/example-mirror-1/");
        result->push_back("http://example-mirror-2/example-mirror-2/");
    }

    return result;
}

HookResult
TestEnvironment::perform_hook(
        const Hook &,
        const std::shared_ptr<OutputManager> &) const
{
    return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
}

std::shared_ptr<const FSPathSequence>
TestEnvironment::hook_dirs() const
{
    return std::make_shared<FSPathSequence>();
}

const std::shared_ptr<const Mask>
TestEnvironment::mask_for_breakage(const std::shared_ptr<const PackageID> &) const
{
    return std::shared_ptr<const Mask>();
}

const std::shared_ptr<const Mask>
TestEnvironment::mask_for_user(const std::shared_ptr<const PackageID> &, const bool) const
{
    return std::shared_ptr<const Mask>();
}

bool
TestEnvironment::unmasked_by_user(const std::shared_ptr<const PackageID> &, const std::string &) const
{
    return false;
}

bool
TestEnvironment::add_to_world(const QualifiedPackageName &) const
{
    return false;
}

bool
TestEnvironment::remove_from_world(const QualifiedPackageName &) const
{
    return false;
}

bool
TestEnvironment::add_to_world(const SetName &) const
{
    return false;
}

bool
TestEnvironment::remove_from_world(const SetName &) const
{
    return false;
}

void
TestEnvironment::need_keys_added() const
{
}

const std::shared_ptr<const MetadataValueKey<std::string> >
TestEnvironment::format_key() const
{
    return std::shared_ptr<const MetadataValueKey<std::string> >();
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
TestEnvironment::config_location_key() const
{
    return std::shared_ptr<const MetadataValueKey<FSPath> >();
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
TestEnvironment::preferred_root_key() const
{
    return _imp->preferred_root_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
TestEnvironment::system_root_key() const
{
    return _imp->system_root_key;
}

const Tribool
TestEnvironment::want_choice_enabled(
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const Choice> & c,
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
        const std::shared_ptr<const PackageID> &,
        const std::shared_ptr<const Choice> &,
        const UnprefixedChoiceName &
        ) const
{
    return "";
}

std::shared_ptr<const Set<UnprefixedChoiceName> >
TestEnvironment::known_choice_value_names(
        const std::shared_ptr<const PackageID> &,
        const std::shared_ptr<const Choice> &
        ) const
{
    return std::make_shared<Set<UnprefixedChoiceName>>();
}

const std::shared_ptr<OutputManager>
TestEnvironment::create_output_manager(const CreateOutputManagerInfo &) const
{
    return std::make_shared<StandardOutputManager>();
}

void
TestEnvironment::set_want_choice_enabled(const ChoicePrefixName & p, const UnprefixedChoiceName & n, const Tribool v)
{
    _imp->override_want_choice_enabled[stringify(p) + ":" + stringify(n)] = v;
}

Tribool
TestEnvironment::interest_in_suggestion(
        const std::shared_ptr<const PackageID> &,
        const PackageDepSpec &) const
{
    return indeterminate;
}

void
TestEnvironment::populate_sets() const
{
}

const std::shared_ptr<Repository>
TestEnvironment::repository_from_new_config_file(const FSPath &)
{
    throw InternalError(PALUDIS_HERE, "can't create repositories on the fly for TestEnvironment");
}

void
TestEnvironment::update_config_files_for_package_move(const PackageDepSpec &, const QualifiedPackageName &) const
{
}

