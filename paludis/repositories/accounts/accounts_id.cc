/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/options.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/output_manager.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/repository.hh>
#include <paludis/action.hh>
#include <paludis/user_dep_spec.hh>
#include <algorithm>

using namespace paludis;
using namespace paludis::accounts_repository;

namespace paludis
{
    template <>
    struct Implementation<AccountsID>
    {
        const Environment * const env;

        const QualifiedPackageName name;
        const VersionSpec version;
        const std::tr1::shared_ptr<const Repository> repository;

        const std::tr1::shared_ptr<const LiteralMetadataValueKey<FSEntry> > fs_location_key;
        const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > > from_repositories_key;
        const std::tr1::shared_ptr<const AccountsInstalledMask> mask;

        const bool is_user;

        mutable Mutex mutex;
        mutable bool has_file_keys;
        mutable bool has_metadata_keys;

        mutable std::tr1::shared_ptr<const LiteralMetadataValueKey<std::string> > username_key;
        mutable std::tr1::shared_ptr<const LiteralMetadataValueKey<std::string> > gecos_key;
        mutable std::tr1::shared_ptr<const LiteralMetadataValueKey<std::string> > preferred_uid_key;
        mutable std::tr1::shared_ptr<const LiteralMetadataValueKey<std::string> > primary_group_key;
        mutable std::tr1::shared_ptr<const LiteralMetadataStringSetKey> extra_groups_key;
        mutable std::tr1::shared_ptr<const LiteralMetadataValueKey<std::string> > home_key;
        mutable std::tr1::shared_ptr<const LiteralMetadataValueKey<std::string> > shell_key;
        mutable std::tr1::shared_ptr<const AccountsDepKey> dependencies_key;

        mutable std::tr1::shared_ptr<const LiteralMetadataValueKey<std::string> > groupname_key;
        mutable std::tr1::shared_ptr<const LiteralMetadataValueKey<std::string> > preferred_gid_key;

        Implementation(const Environment * const e,
                const QualifiedPackageName & q, const std::tr1::shared_ptr<const Repository> & r,
                const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > > & f,
                const FSEntry & l, const bool u, const bool m) :
            env(e),
            name(q),
            version("0", VersionSpecOptions()),
            repository(r),
            fs_location_key(new LiteralMetadataValueKey<FSEntry>("location", "Location", mkt_internal, l)),
            from_repositories_key(f),
            mask(m ? make_shared_ptr(new AccountsInstalledMask) : make_null_shared_ptr()),
            is_user(u),
            has_file_keys(false),
            has_metadata_keys(false)
        {
        }
    };
}

AccountsID::AccountsID(const Environment * const e,
        const QualifiedPackageName & q, const std::tr1::shared_ptr<const Repository> & r,
        const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > > & f, const FSEntry & l,
        const bool u, const bool m) :
    PrivateImplementationPattern<AccountsID>(new Implementation<AccountsID>(e, q, r, f, l, u, m)),
    _imp(PrivateImplementationPattern<AccountsID>::_imp)
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

    KeyValueConfigFile k(_imp->fs_location_key->value(), KeyValueConfigFileOptions(),
            &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

    /* also need to change the handlers if any of the raw names are changed */

    if (_imp->is_user)
    {
        _imp->username_key.reset(new LiteralMetadataValueKey<std::string>("username", "Username",
                    mkt_significant, stringify(name().package())));

        if (! k.get("gecos").empty())
            _imp->gecos_key.reset(new LiteralMetadataValueKey<std::string>("gecos", "Description",
                        mkt_significant, k.get("gecos")));

        if (! k.get("preferred_uid").empty())
            _imp->preferred_uid_key.reset(new LiteralMetadataValueKey<std::string>("preferred_uid", "Preferred UID",
                        mkt_normal, k.get("preferred_uid")));

        if (! k.get("shell").empty())
            _imp->shell_key.reset(new LiteralMetadataValueKey<std::string>("shell", "Shell",
                        mkt_normal, k.get("shell")));

        if (! k.get("home").empty())
            _imp->home_key.reset(new LiteralMetadataValueKey<std::string>("home", "Home Directory",
                        mkt_normal, k.get("home")));

        std::tr1::shared_ptr<Set<std::string> > all_groups_s(new Set<std::string>);

        if (! k.get("extra_groups").empty())
        {
            std::tr1::shared_ptr<Set<std::string> > groups_s(new Set<std::string>);
            tokenise_whitespace(k.get("extra_groups"), groups_s->inserter());
            std::copy(groups_s->begin(), groups_s->end(), all_groups_s->inserter());
            _imp->extra_groups_key.reset(new LiteralMetadataStringSetKey("extra_groups", "Extra Groups",
                        mkt_normal, groups_s));
        }

        if (! k.get("primary_group").empty())
        {
            _imp->primary_group_key.reset(new LiteralMetadataValueKey<std::string>("primary_group", "Default Group",
                        mkt_normal, k.get("primary_group")));
            all_groups_s->insert(k.get("primary_group"));
        }

        if (! all_groups_s->empty())
            _imp->dependencies_key.reset(new AccountsDepKey(_imp->env, all_groups_s));
    }
    else
    {
        _imp->groupname_key.reset(new LiteralMetadataValueKey<std::string>("groupname", "Groupname",
                    mkt_significant, stringify(name().package())));

        if (! k.get("preferred_gid").empty())
            _imp->preferred_gid_key.reset(new LiteralMetadataValueKey<std::string>("preferred_gid", "Preferred GID",
                        mkt_normal, k.get("preferred_gid")));
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

const std::tr1::shared_ptr<const Repository>
AccountsID::repository() const
{
    return _imp->repository;
}

const std::string
AccountsID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            return stringify(name()) + "-" + stringify(version()) + "::" + stringify(repository()->name());

        case idcf_no_version:
            return stringify(name()) + "::" + stringify(repository()->name());

        case idcf_version:
            return stringify(version());

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

PackageDepSpec
AccountsID::uniquely_identifying_spec() const
{
    return parse_user_package_dep_spec(stringify(name()) + "::" + stringify(repository()->name()),
            _imp->env, UserPackageDepSpecOptions());
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
AccountsID::virtual_for_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
AccountsID::keywords_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
AccountsID::provide_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >
AccountsID::contains_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
AccountsID::contained_in_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
AccountsID::build_dependencies_key() const
{
    _need_file_keys();
    return _imp->dependencies_key;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
AccountsID::run_dependencies_key() const
{
    _need_file_keys();
    return _imp->dependencies_key;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
AccountsID::post_dependencies_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
AccountsID::suggested_dependencies_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
AccountsID::dependencies_key() const
{
    _need_file_keys();
    return _imp->dependencies_key;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
AccountsID::fetches_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
AccountsID::homepage_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
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

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
AccountsID::long_description_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >
AccountsID::contents_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataTimeKey>
AccountsID::installed_time_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
AccountsID::from_repositories_key() const
{
    return _imp->from_repositories_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
AccountsID::fs_location_key() const
{
    return _imp->fs_location_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<bool> >
AccountsID::transient_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > >
AccountsID::choices_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<SlotName> >
AccountsID::slot_key() const
{
    return make_null_shared_ptr();
}

std::tr1::shared_ptr<const Set<std::string> >
AccountsID::breaks_portage() const
{
    return make_shared_ptr(new Set<std::string>);
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
    return simple_visitor_cast<const SupportsActionTest<InstallAction> >(test);
}

namespace
{
    std::tr1::shared_ptr<OutputManager> this_output_manager(const std::tr1::shared_ptr<OutputManager> & o, const Action &)
    {
        return o;
    }

    void used_this_for_config_protect(std::string & s, const std::string & v)
    {
        s = v;
    }

    bool ignore_nothing(const FSEntry &)
    {
        return false;
    }
}

void
AccountsID::perform_action(Action & action) const
{
    const InstallAction * const install_action(simple_visitor_cast<const InstallAction>(action));
    if (! install_action)
        throw ActionFailedError("Unsupported action: " + stringify(action));

    if (! (*install_action->options.destination()).destination_interface())
        throw ActionFailedError("Can't install '" + stringify(*this)
                + "' to destination '" + stringify(install_action->options.destination()->name())
                + "' because destination does not provide destination_interface");

    std::tr1::shared_ptr<OutputManager> output_manager(install_action->options.make_output_manager()(
                *install_action));

    std::string used_config_protect;

    switch (install_action->options.want_phase()("merge"))
    {
        case wp_yes:
            {
                (*install_action->options.destination()).destination_interface()->merge(
                        make_named_values<MergeParams>(
                            value_for<n::environment_file>(FSEntry("/dev/null")),
                            value_for<n::image_dir>(fs_location_key()->value()),
                            value_for<n::merged_entries>(make_shared_ptr(new FSEntrySet)),
                            value_for<n::options>(MergerOptions() + mo_rewrite_symlinks + mo_allow_empty_dirs),
                            value_for<n::output_manager>(output_manager),
                            value_for<n::package_id>(shared_from_this()),
                            value_for<n::perform_uninstall>(install_action->options.perform_uninstall()),
                            value_for<n::used_this_for_config_protect>(std::tr1::bind(
                                    &used_this_for_config_protect, std::tr1::ref(used_config_protect), std::tr1::placeholders::_1))
                            ));
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
        if ((*i)->repository()->format_key() && (*i)->repository()->format_key()->value() == "installed-accounts"
                && (*i)->name() == name())
            continue;
        else
        {
            UninstallActionOptions uo(make_named_values<UninstallActionOptions>(
                        value_for<n::config_protect>(used_config_protect),
                        value_for<n::if_for_install_id>(shared_from_this()),
                        value_for<n::ignore_for_unmerge>(&ignore_nothing),
                        value_for<n::is_overwrite>(false),
                        value_for<n::make_output_manager>(std::tr1::bind(&this_output_manager, output_manager, std::tr1::placeholders::_1))
                        ));
            install_action->options.perform_uninstall()(*i, uo);
        }
    }

    output_manager->succeeded();
}

