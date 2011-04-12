/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/accounts/accounts_id.hh>
#include <paludis/repositories/accounts/accounts_dep_key.hh>
#include <paludis/repositories/accounts/accounts_installed_mask.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/options.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/output_manager.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/action.hh>
#include <paludis/user_dep_spec.hh>
#include <algorithm>

using namespace paludis;
using namespace paludis::accounts_repository;

namespace
{
    struct AccountsIDBehaviours :
        Singleton<AccountsIDBehaviours>
    {
        std::shared_ptr<Set<std::string> > behaviours_value;
        std::shared_ptr<LiteralMetadataStringSetKey> behaviours_key;

        AccountsIDBehaviours() :
            behaviours_value(std::make_shared<Set<std::string>>()),
            behaviours_key(std::make_shared<LiteralMetadataStringSetKey>("behaviours", "behaviours", mkt_internal, behaviours_value))
        {
            behaviours_value->insert("unbinaryable");
            behaviours_value->insert("unchrootable");
        }
    };
}

namespace paludis
{
    template <>
    struct Imp<AccountsID>
    {
        const Environment * const env;

        const QualifiedPackageName name;
        const VersionSpec version;
        const RepositoryName repository_name;

        const std::shared_ptr<const LiteralMetadataValueKey<FSPath> > fs_location_key;
        const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > from_repositories_key;
        const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > behaviours_key;
        const std::shared_ptr<const AccountsInstalledMask> mask;

        const bool is_user;

        mutable Mutex mutex;
        mutable bool has_file_keys;
        mutable bool has_metadata_keys;

        mutable std::shared_ptr<const LiteralMetadataValueKey<std::string> > username_key;
        mutable std::shared_ptr<const LiteralMetadataValueKey<std::string> > gecos_key;
        mutable std::shared_ptr<const LiteralMetadataValueKey<std::string> > preferred_uid_key;
        mutable std::shared_ptr<const LiteralMetadataValueKey<std::string> > primary_group_key;
        mutable std::shared_ptr<const LiteralMetadataStringSetKey> extra_groups_key;
        mutable std::shared_ptr<const LiteralMetadataValueKey<std::string> > home_key;
        mutable std::shared_ptr<const LiteralMetadataValueKey<std::string> > shell_key;
        mutable std::shared_ptr<const AccountsDepKey> dependencies_key;

        mutable std::shared_ptr<const LiteralMetadataValueKey<std::string> > groupname_key;
        mutable std::shared_ptr<const LiteralMetadataValueKey<std::string> > preferred_gid_key;

        Imp(const Environment * const e,
                const QualifiedPackageName & q, const RepositoryName & r,
                const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > & f,
                const FSPath & l, const bool u, const bool m) :
            env(e),
            name(q),
            version("0", { }),
            repository_name(r),
            fs_location_key(std::make_shared<LiteralMetadataValueKey<FSPath>>("location", "Location", mkt_internal, l)),
            from_repositories_key(f),
            behaviours_key(AccountsIDBehaviours::get_instance()->behaviours_key),
            mask(m ? std::make_shared<AccountsInstalledMask>() : make_null_shared_ptr()),
            is_user(u),
            has_file_keys(false),
            has_metadata_keys(false)
        {
        }
    };
}

AccountsID::AccountsID(const Environment * const e,
        const QualifiedPackageName & q, const RepositoryName & r,
        const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > & f, const FSPath & l,
        const bool u, const bool m) :
    _imp(e, q, r, f, l, u, m)
{
    if (_imp->mask)
        add_mask(_imp->mask);
}

AccountsID::~AccountsID()
{
}

void
AccountsID::_add_metadata_keys() const
{
    Lock lock(_imp->mutex);

    if (_imp->has_metadata_keys)
        return;

    add_metadata_key(_imp->fs_location_key);
    add_metadata_key(_imp->from_repositories_key);

    if (_imp->username_key)
        add_metadata_key(_imp->username_key);
    if (_imp->gecos_key)
        add_metadata_key(_imp->gecos_key);
    if (_imp->preferred_uid_key)
        add_metadata_key(_imp->preferred_uid_key);
    if (_imp->primary_group_key)
        add_metadata_key(_imp->primary_group_key);
    if (_imp->extra_groups_key)
        add_metadata_key(_imp->extra_groups_key);
    if (_imp->shell_key)
        add_metadata_key(_imp->shell_key);
    if (_imp->home_key)
        add_metadata_key(_imp->home_key);
    if (_imp->dependencies_key)
        add_metadata_key(_imp->dependencies_key);

    if (_imp->groupname_key)
        add_metadata_key(_imp->groupname_key);
    if (_imp->preferred_gid_key)
        add_metadata_key(_imp->preferred_gid_key);

    _imp->has_metadata_keys = true;
}

void
AccountsID::_need_file_keys() const
{
    if (_imp->has_file_keys)
        return;

    Lock lock(_imp->mutex);

    KeyValueConfigFile k(_imp->fs_location_key->value(), { },
            &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

    /* also need to change the handlers if any of the raw names are changed */

    if (_imp->is_user)
    {
        _imp->username_key = std::make_shared<LiteralMetadataValueKey<std::string>>("username", "Username",
                    mkt_significant, stringify(name().package()));

        if (! k.get("gecos").empty())
            _imp->gecos_key = std::make_shared<LiteralMetadataValueKey<std::string>>("gecos", "Description",
                        mkt_significant, k.get("gecos"));

        if (! k.get("preferred_uid").empty())
            _imp->preferred_uid_key = std::make_shared<LiteralMetadataValueKey<std::string>>("preferred_uid", "Preferred UID",
                        mkt_normal, k.get("preferred_uid"));

        if (! k.get("shell").empty())
            _imp->shell_key = std::make_shared<LiteralMetadataValueKey<std::string>>("shell", "Shell",
                        mkt_normal, k.get("shell"));

        if (! k.get("home").empty())
            _imp->home_key = std::make_shared<LiteralMetadataValueKey<std::string>>("home", "Home Directory",
                        mkt_normal, k.get("home"));

        std::shared_ptr<Set<std::string> > all_groups_s(std::make_shared<Set<std::string>>());

        if (! k.get("extra_groups").empty())
        {
            std::shared_ptr<Set<std::string> > groups_s(std::make_shared<Set<std::string>>());
            tokenise_whitespace(k.get("extra_groups"), groups_s->inserter());
            std::copy(groups_s->begin(), groups_s->end(), all_groups_s->inserter());
            _imp->extra_groups_key = std::make_shared<LiteralMetadataStringSetKey>("extra_groups", "Extra Groups",
                        mkt_normal, groups_s);
        }

        if (! k.get("primary_group").empty())
        {
            _imp->primary_group_key = std::make_shared<LiteralMetadataValueKey<std::string>>("primary_group", "Default Group",
                        mkt_normal, k.get("primary_group"));
            all_groups_s->insert(k.get("primary_group"));
        }

        if (! all_groups_s->empty())
            _imp->dependencies_key = std::make_shared<AccountsDepKey>(_imp->env, all_groups_s);
    }
    else
    {
        _imp->groupname_key = std::make_shared<LiteralMetadataValueKey<std::string>>("groupname", "Groupname",
                    mkt_significant, stringify(name().package()));

        if (! k.get("preferred_gid").empty())
            _imp->preferred_gid_key = std::make_shared<LiteralMetadataValueKey<std::string>>("preferred_gid", "Preferred GID",
                        mkt_normal, k.get("preferred_gid"));
    }

    _imp->has_file_keys = true;
}

void
AccountsID::need_keys_added() const
{
    if (! _imp->has_file_keys)
        _need_file_keys();

    if (! _imp->has_metadata_keys)
        _add_metadata_keys();
}

void
AccountsID::clear_metadata_keys() const
{
    Lock lock(_imp->mutex);
    _imp->has_metadata_keys = false;
    PackageID::clear_metadata_keys();
}

void
AccountsID::need_masks_added() const
{
}

const QualifiedPackageName
AccountsID::name() const
{
    return _imp->name;
}

const VersionSpec
AccountsID::version() const
{
    return _imp->version;
}

const RepositoryName
AccountsID::repository_name() const
{
    return _imp->repository_name;
}

const std::string
AccountsID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            return stringify(name()) + "-" + stringify(version()) + "::" + stringify(repository_name());

        case idcf_no_version:
            return stringify(name()) + "::" + stringify(repository_name());

        case idcf_version:
            return stringify(version());

        case idcf_no_name:
            return stringify(version()) + "::" + stringify(repository_name());

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

PackageDepSpec
AccountsID::uniquely_identifying_spec() const
{
    return parse_user_package_dep_spec(stringify(name()) + "::" + stringify(repository_name()),
            _imp->env, { });
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > >
AccountsID::virtual_for_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
AccountsID::keywords_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
AccountsID::provide_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >
AccountsID::contains_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > >
AccountsID::contained_in_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
AccountsID::build_dependencies_key() const
{
    _need_file_keys();
    return _imp->dependencies_key;
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
AccountsID::run_dependencies_key() const
{
    _need_file_keys();
    return _imp->dependencies_key;
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
AccountsID::post_dependencies_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
AccountsID::suggested_dependencies_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
AccountsID::dependencies_key() const
{
    _need_file_keys();
    return _imp->dependencies_key;
}

const std::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
AccountsID::fetches_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
AccountsID::homepage_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<std::string> >
AccountsID::short_description_key() const
{
    if (_imp->is_user)
    {
        _need_file_keys();
        return _imp->gecos_key;
    }
    else
        return _imp->groupname_key;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
AccountsID::long_description_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Contents> > >
AccountsID::contents_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataTimeKey>
AccountsID::installed_time_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
AccountsID::from_repositories_key() const
{
    return _imp->from_repositories_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
AccountsID::fs_location_key() const
{
    return _imp->fs_location_key;
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
AccountsID::behaviours_key() const
{
    return _imp->behaviours_key;
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > >
AccountsID::choices_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<SlotName> >
AccountsID::slot_key() const
{
    return make_null_shared_ptr();
}

std::shared_ptr<const Set<std::string> >
AccountsID::breaks_portage() const
{
    return std::make_shared<Set<std::string>>();
}

bool
AccountsID::arbitrary_less_than_comparison(const PackageID &) const
{
    return false;
}

std::size_t
AccountsID::extra_hash_value() const
{
    return 0;
}

bool
AccountsID::supports_action(const SupportsActionTestBase & test) const
{
    return visitor_cast<const SupportsActionTest<InstallAction> >(test);
}

namespace
{
    std::shared_ptr<OutputManager> this_output_manager(const std::shared_ptr<OutputManager> & o, const Action &)
    {
        return o;
    }

    void used_this_for_config_protect(std::string & s, const std::string & v)
    {
        s = v;
    }

    bool ignore_nothing(const FSPath &)
    {
        return false;
    }
}

void
AccountsID::perform_action(Action & action) const
{
    Timestamp build_start_time(Timestamp::now());

    const InstallAction * const install_action(visitor_cast<const InstallAction>(action));
    if (! install_action)
        throw ActionFailedError("Unsupported action: " + action.simple_name());

    if (! (*install_action->options.destination()).destination_interface())
        throw ActionFailedError("Can't install '" + stringify(*this)
                + "' to destination '" + stringify(install_action->options.destination()->name())
                + "' because destination does not provide destination_interface");

    std::shared_ptr<OutputManager> output_manager(install_action->options.make_output_manager()(
                *install_action));

    std::string used_config_protect;

    MergeParams merge_params(make_named_values<MergeParams>(
                n::build_start_time() = build_start_time,
                n::check() = true,
                n::environment_file() = FSPath("/dev/null"),
                n::image_dir() = fs_location_key()->value(),
                n::merged_entries() = std::make_shared<FSPathSet>(),
                n::options() = MergerOptions() + mo_rewrite_symlinks + mo_allow_empty_dirs,
                n::output_manager() = output_manager,
                n::package_id() = shared_from_this(),
                n::perform_uninstall() = install_action->options.perform_uninstall(),
                n::replacing() = install_action->options.replacing(),
                n::used_this_for_config_protect() = std::bind(
                    &used_this_for_config_protect, std::ref(used_config_protect), std::placeholders::_1)
                ));

    switch (install_action->options.want_phase()("check_merge"))
    {
        case wp_yes:
            {
                merge_params.check() = true;
                (*install_action->options.destination()).destination_interface()->merge(merge_params);
            }
            break;

        case wp_skip:
            break;

        case wp_abort:
            throw ActionAbortedError("Told to abort install");

        case last_wp:
            throw InternalError(PALUDIS_HERE, "bad WantPhase");
    }

    switch (install_action->options.want_phase()("merge"))
    {
        case wp_yes:
            {
                merge_params.check() = false;
                (*install_action->options.destination()).destination_interface()->merge(merge_params);
            }
            break;

        case wp_skip:
            break;

        case wp_abort:
            throw ActionAbortedError("Told to abort install");

        case last_wp:
            throw InternalError(PALUDIS_HERE, "bad WantPhase");
    }

    for (PackageIDSequence::ConstIterator i(install_action->options.replacing()->begin()), i_end(install_action->options.replacing()->end()) ;
            i != i_end ; ++i)
    {
        Context local_context("When cleaning '" + stringify(**i) + "':");
        auto repo(_imp->env->fetch_repository((*i)->repository_name()));
        if (repo->format_key() && repo->format_key()->value() == "installed-accounts"
                && (*i)->name() == name())
            continue;
        else
        {
            UninstallActionOptions uo(make_named_values<UninstallActionOptions>(
                        n::config_protect() = used_config_protect,
                        n::if_for_install_id() = shared_from_this(),
                        n::ignore_for_unmerge() = &ignore_nothing,
                        n::is_overwrite() = false,
                        n::make_output_manager() = std::bind(&this_output_manager, output_manager, std::placeholders::_1),
                        n::override_contents() = make_null_shared_ptr()
                        ));
            install_action->options.perform_uninstall()(*i, uo);
        }
    }

    output_manager->succeeded();
}

