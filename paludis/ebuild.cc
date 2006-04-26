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

#include "ebuild.hh"
#include <paludis/util/system.hh>
#include <paludis/util/pstream.hh>
#include <paludis/util/log.hh>
#include <paludis/environment.hh>
#include <paludis/config_file.hh>

/** \file
 * Implementation for ebuild.hh things.
 *
 * \ingroup grpebuildinterface
 */

using namespace paludis;

EbuildCommand::EbuildCommand(const EbuildCommandParams & p) :
    params(p)
{
}

EbuildCommand::~EbuildCommand()
{
}

bool
EbuildCommand::success()
{
    return true;
}

bool
EbuildCommand::use_sandbox() const
{
    return true;
}

bool
EbuildCommand::failure()
{
    return false;
}

bool
EbuildCommand::operator() ()
{
    std::string ebuild_cmd(getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis") +
            "/ebuild.bash '" +
            stringify(params.get<ecpk_ebuild_dir>()) + "/" +
            stringify(params.get<ecpk_db_entry>()->get<pde_name>().get<qpn_package>()) + "-" +
            stringify(params.get<ecpk_db_entry>()->get<pde_version>()) +
            ".ebuild' " + commands());

    if (use_sandbox())
        ebuild_cmd = make_sandbox_command(ebuild_cmd);

    MakeEnvCommand cmd(extend_command(make_env_command(ebuild_cmd)
                ("P", stringify(params.get<ecpk_db_entry>()->get<pde_name>().get<qpn_package>()) + "-" +
                 stringify(params.get<ecpk_db_entry>()->get<pde_version>().remove_revision()))
                ("PV", stringify(params.get<ecpk_db_entry>()->get<pde_version>().remove_revision()))
                ("PR", stringify(params.get<ecpk_db_entry>()->get<pde_version>().revision_only()))
                ("PN", stringify(params.get<ecpk_db_entry>()->get<pde_name>().get<qpn_package>()))
                ("PVR", stringify(params.get<ecpk_db_entry>()->get<pde_version>().remove_revision()) + "-" +
                 stringify(params.get<ecpk_db_entry>()->get<pde_version>().revision_only()))
                ("PF", stringify(params.get<ecpk_db_entry>()->get<pde_name>().get<qpn_package>()) + "-" +
                 stringify(params.get<ecpk_db_entry>()->get<pde_version>()))
                ("CATEGORY", stringify(params.get<ecpk_db_entry>()->get<pde_name>().get<qpn_category>()))
                ("FILESDIR", stringify(params.get<ecpk_files_dir>()))
                ("ECLASSDIR", stringify(params.get<ecpk_eclass_dir>()))
                ("PORTDIR", stringify(params.get<ecpk_portdir>()))
                ("DISTDIR", stringify(params.get<ecpk_distdir>()))
                ("PALUDIS_TMPDIR", BIGTEMPDIR "/paludis/")
                ("PALUDIS_CONFIG_DIR", SYSCONFDIR "/paludis/")
                ("PALUDIS_BASHRC_FILES", params.get<ecpk_environment>()->bashrc_files())
                ("PALUDIS_HOOK_DIRS", params.get<ecpk_environment>()->hook_dirs())
                ("PALUDIS_COMMAND", params.get<ecpk_environment>()->paludis_command())
                ("KV", kernel_version())
                ("PALUDIS_EBUILD_LOG_LEVEL", Log::get_instance()->log_level_string())
                ("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))));

    if (do_run_command(cmd))
        return success();
    else
        return failure();
}

bool
EbuildCommand::do_run_command(const std::string & cmd)
{
    return 0 == run_command(cmd);
}

EbuildMetadataCommand::EbuildMetadataCommand(const EbuildCommandParams & p) :
    EbuildCommand(p),
    _metadata(0)
{
}

std::string
EbuildMetadataCommand::commands() const
{
    return "metadata";
}

bool
EbuildMetadataCommand::failure()
{
    return EbuildCommand::failure();
}

MakeEnvCommand
EbuildMetadataCommand::extend_command(const MakeEnvCommand & cmd)
{
    return cmd;
}

bool
EbuildMetadataCommand::do_run_command(const std::string & cmd)
{
    PStream prog(cmd);
    KeyValueConfigFile f(&prog);
    _metadata.assign(new VersionMetadata);

    _metadata->set(vmk_depend,      f.get("DEPEND"));
    _metadata->set(vmk_rdepend,     f.get("RDEPEND"));
    _metadata->set(vmk_slot,        f.get("SLOT"));
    _metadata->set(vmk_src_uri,     f.get("SRC_URI"));
    _metadata->set(vmk_restrict,    f.get("RESTRICT"));
    _metadata->set(vmk_homepage,    f.get("HOMEPAGE"));
    _metadata->set(vmk_license,     f.get("LICENSE"));
    _metadata->set(vmk_description, f.get("DESCRIPTION"));
    _metadata->set(vmk_keywords,    f.get("KEYWORDS"));
    _metadata->set(vmk_inherited,   f.get("INHERITED"));
    _metadata->set(vmk_iuse,        f.get("IUSE"));
    _metadata->set(vmk_pdepend,     f.get("PDEPEND"));
    _metadata->set(vmk_provide,     f.get("PROVIDE"));
    _metadata->set(vmk_eapi,        f.get("EAPI"));
    _metadata->set(vmk_virtual, "");
    _metadata->set(vmk_e_keywords,  f.get("E_KEYWORDS"));

    if (prog.exit_status())
    {
        Log::get_instance()->message(ll_warning, "Could not generate cache for '"
                + stringify(*params.get<ecpk_db_entry>()) + "'");
        _metadata->set(vmk_eapi, "UNKNOWN");

        return false;
    }
    return true;
}

std::string
EbuildFetchCommand::commands() const
{
    return "fetch";
}

bool
EbuildFetchCommand::failure()
{
    throw PackageFetchActionError("Fetch failed for '" + stringify(
                *params.get<ecpk_db_entry>()) + "'");
}

MakeEnvCommand
EbuildFetchCommand::extend_command(const MakeEnvCommand & cmd)
{
    MakeEnvCommand result(cmd
            ("A", fetch_params.get<ecfpk_a>())
            ("USE", fetch_params.get<ecfpk_use>())
            ("USE_EXPAND", fetch_params.get<ecfpk_use_expand>())
            ("FLAT_SRC_URI", fetch_params.get<ecfpk_flat_src_uri>())
            ("ROOT", fetch_params.get<ecfpk_root>())
            ("PALUDIS_PROFILE_DIR", fetch_params.get<ecfpk_profile>()));

    for (std::map<std::string, std::string>::const_iterator
            i(fetch_params.get<ecfpk_expand_vars>().begin()),
            j(fetch_params.get<ecfpk_expand_vars>().end()) ; i != j ; ++i)
        result = result(i->first, i->second);

    return result;
}

EbuildFetchCommand::EbuildFetchCommand(const EbuildCommandParams & p,
        const EbuildFetchCommandParams & f) :
    EbuildCommand(p),
    fetch_params(f)
{
}

std::string
EbuildInstallCommand::commands() const
{
    if (install_params.get<ecipk_merge_only>())
        return "merge";
    else
        return "init setup unpack compile test install strip preinst "
            "merge postinst tidyup";
}

bool
EbuildInstallCommand::failure()
{
    throw PackageInstallActionError("Install failed for '" + stringify(
                *params.get<ecpk_db_entry>()) + "'");
}

MakeEnvCommand
EbuildInstallCommand::extend_command(const MakeEnvCommand & cmd)
{
    MakeEnvCommand result(cmd
            ("A", install_params.get<ecipk_a>())
            ("USE", install_params.get<ecipk_use>())
            ("USE_EXPAND", install_params.get<ecipk_use_expand>())
            ("ROOT", install_params.get<ecipk_root>())
            ("PALUDIS_EBUILD_OVERRIDE_CONFIG_PROTECT_MASK",
                install_params.get<ecipk_disable_cfgpro>() ? "/" : "")
            ("PALUDIS_PROFILE_DIR", install_params.get<ecipk_profile>())
            ("SLOT", stringify(install_params.get<ecipk_slot>())));

    for (std::map<std::string, std::string>::const_iterator
            i(install_params.get<ecipk_expand_vars>().begin()),
            j(install_params.get<ecipk_expand_vars>().end()) ; i != j ; ++i)
        result = result(i->first, i->second);

    return result;
}

EbuildInstallCommand::EbuildInstallCommand(const EbuildCommandParams & p,
        const EbuildInstallCommandParams & f) :
    EbuildCommand(p),
    install_params(f)
{
}

std::string
EbuildUninstallCommand::commands() const
{
    if (uninstall_params.get<ecupk_unmerge_only>())
        return "unmerge";
    else
        return "prerm unmerge postrm";
}

bool
EbuildUninstallCommand::failure()
{
    throw PackageUninstallActionError("Uninstall failed for '" + stringify(
                *params.get<ecpk_db_entry>()) + "'");
}

MakeEnvCommand
EbuildUninstallCommand::extend_command(const MakeEnvCommand & cmd)
{
    MakeEnvCommand result(cmd
            ("ROOT", uninstall_params.get<ecupk_root>())
            ("PALUDIS_EBUILD_OVERRIDE_CONFIG_PROTECT_MASK",
                uninstall_params.get<ecupk_disable_cfgpro>() ? "/" : ""));

    return result;
}

EbuildUninstallCommand::EbuildUninstallCommand(const EbuildCommandParams & p,
        const EbuildUninstallCommandParams & f) :
    EbuildCommand(p),
    uninstall_params(f)
{
}

