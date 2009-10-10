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

#include <paludis/repositories/accounts/passwd_accounts_handler.hh>
#include <paludis/util/system.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/set.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/join.hh>
#include <paludis/output_manager.hh>
#include <paludis/action.hh>
#include <paludis/repository.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_id.hh>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

using namespace paludis;
using namespace paludis::accounts_repository;

void
PasswdAccountsHandler::merge(const MergeParams & params)
{
    Context context("When installing '" + stringify(*params.package_id()) + "':");

    params.output_manager()->stdout_stream() << ">>> Installing " << *params.package_id() << " using passwd handler" << std::endl;

    if (params.package_id()->end_metadata() != params.package_id()->find_metadata("groupname"))
        merge_group(params);
    else
        merge_user(params);

    params.output_manager()->stdout_stream() << ">>> Finished installing " << *params.package_id() << std::endl;
}

void
PasswdAccountsHandler::merge_user(const MergeParams & params)
{
    std::string username;
    do
    {
        PackageID::MetadataConstIterator m(params.package_id()->find_metadata("username"));
        if (params.package_id()->end_metadata() == m)
            throw ActionFailedError("Key 'username' for '" + stringify(*params.package_id()) + "' does not exist");

        const MetadataValueKey<std::string> * k(simple_visitor_cast<const MetadataValueKey<std::string> >(**m));
        if (! k)
            throw ActionFailedError("Key 'username' for '" + stringify(*params.package_id()) + "' is not a string key");

        username = k->value();

        if (0 != getpwnam(username.c_str()))
            throw ActionFailedError("User '" + username + "' already exists");
    } while (false);

    std::string gecos;
    do
    {
        PackageID::MetadataConstIterator m(params.package_id()->find_metadata("gecos"));
        if (params.package_id()->end_metadata() == m)
            break;

        const MetadataValueKey<std::string> * k(simple_visitor_cast<const MetadataValueKey<std::string> >(**m));
        if (! k)
            throw ActionFailedError("Key 'gecos' for '" + stringify(*params.package_id()) + "' is not a string key");

        gecos = k->value();

        if (std::string::npos != gecos.find('\''))
            throw ActionFailedError("Value for key 'gecos' for '" + stringify(*params.package_id()) + "' must not contain a quote");

        if (! gecos.empty())
            gecos = " -c '" + gecos + "'";
    } while (false);

    std::string preferred_uid;
    do
    {
        PackageID::MetadataConstIterator m(params.package_id()->find_metadata("preferred_uid"));
        if (params.package_id()->end_metadata() == m)
            break;

        const MetadataValueKey<std::string> * k(simple_visitor_cast<const MetadataValueKey<std::string> >(**m));
        if (! k)
            throw ActionFailedError("Key 'preferred_uid' for '" + stringify(*params.package_id()) + "' is not a string key");

        preferred_uid = k->value();

        if (std::string::npos != preferred_uid.find_first_not_of("0123456789"))
            throw ActionFailedError("Value for key 'preferred_uid' for '" + stringify(*params.package_id()) + "' must be a number");

        uid_t uid(destringify<uid_t>(preferred_uid));
        if (getpwuid(uid))
        {
            params.output_manager()->stdout_stream() << ">>> Preferred UID " << uid << " already in use, not specifying an ID" << std::endl;
            preferred_uid = "";
        }

        if (! preferred_uid.empty())
            preferred_uid = " -u '" + preferred_uid + "'";
    } while (false);

    std::string primary_group;
    do
    {
        PackageID::MetadataConstIterator m(params.package_id()->find_metadata("primary_group"));
        if (params.package_id()->end_metadata() == m)
            break;

        const MetadataValueKey<std::string> * k(simple_visitor_cast<const MetadataValueKey<std::string> >(**m));
        if (! k)
            throw ActionFailedError("Key 'primary_group' for '" + stringify(*params.package_id()) + "' is not a string key");

        primary_group = k->value();

        if (std::string::npos != primary_group.find('\''))
            throw ActionFailedError("Value for key 'primary_group' for '" + stringify(*params.package_id()) + "' must not contain a quote");

        if (! primary_group.empty())
            primary_group = " -g '" + primary_group + "'";
    } while (false);

    std::string extra_groups;
    do
    {
        PackageID::MetadataConstIterator m(params.package_id()->find_metadata("extra_groups"));
        if (params.package_id()->end_metadata() == m)
            break;

        const MetadataCollectionKey<Set<std::string> > * k(simple_visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**m));
        if (! k)
            throw ActionFailedError("Key 'extra_groups' for '" + stringify(*params.package_id()) + "' is not a string set key");

        extra_groups = join(k->value()->begin(), k->value()->end(), ",");

        if (std::string::npos != extra_groups.find('\''))
            throw ActionFailedError("Value for key 'extra_groups' for '" + stringify(*params.package_id()) + "' must not contain a quote");

        if (! extra_groups.empty())
            extra_groups = " -G '" + extra_groups + "'";
    } while (false);

    std::string shell;
    do
    {
        PackageID::MetadataConstIterator m(params.package_id()->find_metadata("shell"));
        if (params.package_id()->end_metadata() == m)
            break;

        const MetadataValueKey<std::string> * k(simple_visitor_cast<const MetadataValueKey<std::string> >(**m));
        if (! k)
            throw ActionFailedError("Key 'shell' for '" + stringify(*params.package_id()) + "' is not a string key");

        shell = k->value();

        if (std::string::npos != shell.find('\''))
            throw ActionFailedError("Value for key 'shell' for '" + stringify(*params.package_id()) + "' must not contain a quote");

        if (! shell.empty())
            shell = " -s '" + shell + "'";
    } while (false);

    std::string home;
    do
    {
        PackageID::MetadataConstIterator m(params.package_id()->find_metadata("home"));
        if (params.package_id()->end_metadata() == m)
            break;

        const MetadataValueKey<std::string> * k(simple_visitor_cast<const MetadataValueKey<std::string> >(**m));
        if (! k)
            throw ActionFailedError("Key 'home' for '" + stringify(*params.package_id()) + "' is not a string key");

        home = k->value();

        if (std::string::npos != home.find('\''))
            throw ActionFailedError("Value for key 'home' for '" + stringify(*params.package_id()) + "' must not contain a quote");

        if (! home.empty())
            home = " -d '" + home + "'";
    } while (false);

    Command cmd("useradd -r " + username + preferred_uid + gecos + primary_group + extra_groups + shell + home);
    cmd.with_echo_to_stderr();
    int exit_status(run_command(cmd));

    if (0 != exit_status)
        throw ActionFailedError("Install of '" + stringify(*params.package_id()) + "' failed because useradd returned "
                + stringify(exit_status));
}

void
PasswdAccountsHandler::merge_group(const MergeParams & params)
{
    std::string groupname;
    do
    {
        PackageID::MetadataConstIterator m(params.package_id()->find_metadata("groupname"));
        if (params.package_id()->end_metadata() == m)
            throw ActionFailedError("Key 'groupname' for '" + stringify(*params.package_id()) + "' does not exist");

        const MetadataValueKey<std::string> * k(simple_visitor_cast<const MetadataValueKey<std::string> >(**m));
        if (! k)
            throw ActionFailedError("Key 'groupname' for '" + stringify(*params.package_id()) + "' is not a string key");

        groupname = k->value();

        if (0 != getgrnam(groupname.c_str()))
            throw ActionFailedError("Group '" + groupname + "' already exists");
    } while (false);

    std::string preferred_gid;
    do
    {
        PackageID::MetadataConstIterator m(params.package_id()->find_metadata("preferred_gid"));
        if (params.package_id()->end_metadata() == m)
            break;

        const MetadataValueKey<std::string> * k(simple_visitor_cast<const MetadataValueKey<std::string> >(**m));
        if (! k)
            throw ActionFailedError("Key 'preferred_gid' for '" + stringify(*params.package_id()) + "' is not a string key");

        preferred_gid = k->value();

        if (std::string::npos != preferred_gid.find_first_not_of("0123456789"))
            throw ActionFailedError("Value for key 'preferred_gid' for '" + stringify(*params.package_id()) + "' must be a number");

        uid_t gid(destringify<uid_t>(preferred_gid));
        if (getgrgid(gid))
        {
            params.output_manager()->stdout_stream() << ">>> Preferred GID " << gid << " already in use, not specifying an ID" << std::endl;
            preferred_gid = "";
        }

        if (! preferred_gid.empty())
            preferred_gid = " -g '" + preferred_gid + "'";
    } while (false);

    Command cmd("groupadd -r " + groupname + preferred_gid);
    cmd.with_echo_to_stderr();
    int exit_status(run_command(cmd));

    if (0 != exit_status)
        throw ActionFailedError("Install of '" + stringify(*params.package_id()) + "' failed because groupadd returned "
                + stringify(exit_status));
}

