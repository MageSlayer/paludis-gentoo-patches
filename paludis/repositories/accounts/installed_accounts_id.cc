/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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

#include <paludis/repositories/accounts/installed_accounts_id.hh>
#include <paludis/repositories/accounts/accounts_dep_key.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/options.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/log.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/repository.hh>
#include <paludis/action.hh>
#include <paludis/user_dep_spec.hh>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <list>
#include <vector>

using namespace paludis;
using namespace paludis::accounts_repository;

namespace paludis
{
    template <>
    struct Implementation<InstalledAccountsID>
    {
        const Environment * const env;

        const QualifiedPackageName name;
        const VersionSpec version;
        const std::shared_ptr<const Repository> repository;

        static const std::shared_ptr<Set<std::string> > behaviours_set;
        const std::shared_ptr<const LiteralMetadataStringSetKey> behaviours_key;

        mutable Mutex mutex;
        mutable std::shared_ptr<const AccountsDepKey> dependencies_key;

        const bool is_user;

        Implementation(const Environment * const e,
                const QualifiedPackageName & q, const std::shared_ptr<const Repository> & r,
                const bool u) :
            env(e),
            name(q),
            version("0", VersionSpecOptions()),
            repository(r),
            behaviours_key(new LiteralMetadataStringSetKey("behaviours", "Behaviours", mkt_internal,
                        behaviours_set)),
            is_user(u)
        {
        }
    };
}

namespace
{
    std::shared_ptr<Set<std::string> > make_behaviours()
    {
        std::shared_ptr<Set<std::string> > result(new Set<std::string>);
        result->insert("transient");
        result->insert("used");
        return result;
    }
}

const std::shared_ptr<Set<std::string> > Implementation<InstalledAccountsID>::behaviours_set = make_behaviours();

InstalledAccountsID::InstalledAccountsID(const Environment * const e,
        const QualifiedPackageName & q, const std::shared_ptr<const Repository> & r, const bool u) :
    PrivateImplementationPattern<InstalledAccountsID>(new Implementation<InstalledAccountsID>(e, q, r, u)),
    _imp(PrivateImplementationPattern<InstalledAccountsID>::_imp)
{
    add_metadata_key(_imp->behaviours_key);
}

InstalledAccountsID::~InstalledAccountsID()
{
}

void
InstalledAccountsID::need_keys_added() const
{
    Lock lock(_imp->mutex);

    if (_imp->is_user && ! _imp->dependencies_key)
    {
        std::shared_ptr<Set<std::string> > groups(new Set<std::string>);

        /* depend upon our primary group */
        {
            int pwd_buf_sz(sysconf(_SC_GETPW_R_SIZE_MAX));
            if (-1 == pwd_buf_sz || pwd_buf_sz > 1024 * 128)
            {
                Log::get_instance()->message("accounts.getpw_r_size_max", ll_warning, lc_context) <<
                    "Got dodgy value " << pwd_buf_sz << " from sysconf(_SC_GETPW_R_SIZE_MAX)";
                pwd_buf_sz = 1024 * 128;
            }
            std::vector<char> pwd_buf(pwd_buf_sz);

            struct passwd pwd;
            struct passwd * pwd_result;

            if (0 == getpwnam_r(stringify(name().package()).c_str(), &pwd, &pwd_buf[0], pwd_buf_sz, &pwd_result))
            {
                int grp_buf_sz(sysconf(_SC_GETGR_R_SIZE_MAX));
                if (-1 == grp_buf_sz || grp_buf_sz > 1024 * 128)
                {
                    Log::get_instance()->message("accounts.getgr_r_size_max", ll_warning, lc_context) <<
                        "Got dodgy value " << grp_buf_sz << " from sysconf(_SC_GETPW_R_SIZE_MAX)";
                    grp_buf_sz = 1024 * 128;
                }
                std::vector<char> grp_buf(grp_buf_sz);

                struct group grp;
                struct group * grp_result;
                if (0 == getgrgid_r(pwd.pw_gid, &grp, &grp_buf[0], grp_buf_sz, &grp_result))
                {
                    /* really we should only do this if the group in question is managed by accounts. users
                     * might have accounts installed by hand with a group that's unmanaged. */
                    groups->insert(stringify(grp.gr_name));
                }
                else
                    Log::get_instance()->message("accounts.getgrgid_r", ll_warning, lc_context) <<
                        "getgrgid_r failed for " << name();
            }
            else
                Log::get_instance()->message("accounts.getpwnam_r", ll_warning, lc_context) <<
                    "getpwnam_r failed for " << name();
        }

        /* ...and our secondary groups */
        {
            /* first person who gets annoyed by this not existing gets to implement it. */
        }

        _imp->dependencies_key.reset(new AccountsDepKey(_imp->env, groups));
        add_metadata_key(_imp->dependencies_key);
    }
}

void
InstalledAccountsID::clear_metadata_keys() const
{
}

void
InstalledAccountsID::need_masks_added() const
{
}

const QualifiedPackageName
InstalledAccountsID::name() const
{
    return _imp->name;
}

const VersionSpec
InstalledAccountsID::version() const
{
    return _imp->version;
}

const std::shared_ptr<const Repository>
InstalledAccountsID::repository() const
{
    return _imp->repository;
}

const std::string
InstalledAccountsID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            return stringify(name()) + "-" + stringify(version()) + "::" + stringify(repository()->name());

        case idcf_no_version:
            return stringify(name()) + "::" + stringify(repository()->name());

        case idcf_version:
            return stringify(version());

        case idcf_no_name:
            return stringify(version()) + "::" + stringify(repository()->name());

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

PackageDepSpec
InstalledAccountsID::uniquely_identifying_spec() const
{
    return parse_user_package_dep_spec(stringify(name()) + "::" + stringify(repository()->name()),
            _imp->env, UserPackageDepSpecOptions());
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > >
InstalledAccountsID::virtual_for_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
InstalledAccountsID::keywords_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
InstalledAccountsID::provide_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >
InstalledAccountsID::contains_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > >
InstalledAccountsID::contained_in_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledAccountsID::build_dependencies_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledAccountsID::run_dependencies_key() const
{
    need_keys_added();
    return _imp->dependencies_key;
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledAccountsID::post_dependencies_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledAccountsID::dependencies_key() const
{
    need_keys_added();
    return _imp->dependencies_key;
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledAccountsID::suggested_dependencies_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
InstalledAccountsID::fetches_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
InstalledAccountsID::homepage_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<std::string> >
InstalledAccountsID::short_description_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<std::string> >
InstalledAccountsID::long_description_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Contents> > >
InstalledAccountsID::contents_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataTimeKey>
InstalledAccountsID::installed_time_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
InstalledAccountsID::from_repositories_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<FSEntry> >
InstalledAccountsID::fs_location_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
InstalledAccountsID::behaviours_key() const
{
    return _imp->behaviours_key;
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > >
InstalledAccountsID::choices_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<SlotName> >
InstalledAccountsID::slot_key() const
{
    return make_null_shared_ptr();
}

std::shared_ptr<const Set<std::string> >
InstalledAccountsID::breaks_portage() const
{
    return std::make_shared<Set<std::string> >();
}

bool
InstalledAccountsID::arbitrary_less_than_comparison(const PackageID &) const
{
    return false;
}

std::size_t
InstalledAccountsID::extra_hash_value() const
{
    return 0;
}

bool
InstalledAccountsID::supports_action(const SupportsActionTestBase &) const
{
    return false;
}

void
InstalledAccountsID::perform_action(Action & action) const
{
    throw ActionFailedError("Unsupported action: " + action.simple_name());
}

