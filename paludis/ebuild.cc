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

#include <paludis/ebuild.hh>
#include <paludis/util/system.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/pstream.hh>
#include <paludis/util/log.hh>
#include <paludis/environment.hh>
#include <paludis/config_file.hh>
#include <paludis/portage_dep_parser.hh>
#include <sys/resource.h>
#include <sys/time.h>

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
                ("PVR", stringify(params.get<ecpk_db_entry>()->get<pde_version>()))
                ("PF", stringify(params.get<ecpk_db_entry>()->get<pde_name>().get<qpn_package>()) + "-" +
                 stringify(params.get<ecpk_db_entry>()->get<pde_version>()))
                ("CATEGORY", stringify(params.get<ecpk_db_entry>()->get<pde_name>().get<qpn_category>()))
                ("REPOSITORY", stringify(params.get<ecpk_db_entry>()->get<pde_repository>()))
                ("FILESDIR", stringify(params.get<ecpk_files_dir>()))
                ("ECLASSDIR", stringify(*params.get<ecpk_eclassdirs>()->begin()))
                ("ECLASSDIRS", join(params.get<ecpk_eclassdirs>()->begin(),
                                    params.get<ecpk_eclassdirs>()->end(), " "))
                ("PORTDIR", stringify(params.get<ecpk_portdir>()))
                ("DISTDIR", stringify(params.get<ecpk_distdir>()))
                ("PALUDIS_TMPDIR", stringify(params.get<ecpk_buildroot>()))
                ("PALUDIS_CONFIG_DIR", SYSCONFDIR "/paludis/")
                ("PALUDIS_BASHRC_FILES", params.get<ecpk_environment>()->bashrc_files())
                ("PALUDIS_HOOK_DIRS", params.get<ecpk_environment>()->hook_dirs())
                ("PALUDIS_COMMAND", params.get<ecpk_environment>()->paludis_command())
                ("KV", kernel_version())
                ("PALUDIS_EBUILD_LOG_LEVEL", Log::get_instance()->log_level_string())
                ("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))));

    if (do_run_command(add_portage_vars(cmd)))
        return success();
    else
        return failure();
}

MakeEnvCommand
EbuildCommand::add_portage_vars(const MakeEnvCommand & cmd) const
{
    return cmd
        ("PORTAGE_ACTUAL_DISTDIR", stringify(params.get<ecpk_distdir>()))
        ("PORTAGE_BASHRC", "/dev/null")
        ("PORTAGE_BUILDDIR", stringify(params.get<ecpk_buildroot>()) + "/" +
             stringify(params.get<ecpk_db_entry>()->get<pde_name>().get<qpn_category>()) + "/" +
             stringify(params.get<ecpk_db_entry>()->get<pde_name>().get<qpn_package>()) + "-" +
             stringify(params.get<ecpk_db_entry>()->get<pde_version>()))
        ("PORTAGE_CALLER", params.get<ecpk_environment>()->paludis_command())
        ("PORTAGE_GID", "0")
        ("PORTAGE_INST_GID", "0")
        ("PORTAGE_INST_UID", "0")
        ("PORTAGE_MASTER_PID", stringify(::getpid()))
        ("PORTAGE_NICENCESS", stringify(::getpriority(PRIO_PROCESS, 0)))
        ("PORTAGE_TMPDIR", stringify(params.get<ecpk_buildroot>()))
        ("PORTAGE_TMPFS", "/dev/shm")
        ("PORTAGE_WORKDIR_MODE", "0700");
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
    _metadata.assign(new VersionMetadata::Ebuild(PortageDepParser::parse_depend));

    bool ok(false);
    try
    {
        _metadata->get<vm_deps>().set<vmd_build_depend_string>(f.get("DEPEND"));
        _metadata->get<vm_deps>().set<vmd_run_depend_string>(f.get("RDEPEND"));
        _metadata->set<vm_slot>(SlotName(f.get("SLOT")));
        _metadata->get_ebuild_interface()->set<evm_src_uri>(f.get("SRC_URI"));
        _metadata->get_ebuild_interface()->set<evm_restrict>(f.get("RESTRICT"));
        _metadata->set<vm_homepage>(f.get("HOMEPAGE"));
        _metadata->set<vm_license>(f.get("LICENSE"));
        _metadata->set<vm_description>(f.get("DESCRIPTION"));
        _metadata->get_ebuild_interface()->set<evm_keywords>(f.get("KEYWORDS"));
        _metadata->get_ebuild_interface()->set<evm_inherited>(f.get("INHERITED"));
        _metadata->get_ebuild_interface()->set<evm_iuse>(f.get("IUSE"));
        _metadata->get<vm_deps>().set<vmd_post_depend_string>(f.get("PDEPEND"));
        _metadata->get_ebuild_interface()->set<evm_provide>(f.get("PROVIDE"));
        _metadata->set<vm_eapi>(f.get("EAPI"));
        _metadata->get_ebuild_interface()->set<evm_virtual>("");

        if (0 == prog.exit_status())
            ok = true;
    }
    catch (const NameError &)
    {
    }

    if (ok)
        return true;
    else
    {
        Log::get_instance()->message(ll_warning, lc_context, "Could not generate cache for '"
                + stringify(*params.get<ecpk_db_entry>()) + "'");
        _metadata->set<vm_eapi>("UNKNOWN");

        return false;
    }
}

EbuildVariableCommand::EbuildVariableCommand(const EbuildCommandParams & p,
        const std::string & var) :
    EbuildCommand(p),
    _var(var)
{
}

std::string
EbuildVariableCommand::commands() const
{
    return "variable";
}

bool
EbuildVariableCommand::failure()
{
    return EbuildCommand::failure();
}

MakeEnvCommand
EbuildVariableCommand::extend_command(const MakeEnvCommand & cmd)
{
    return cmd("PALUDIS_VARIABLE", _var);
}

bool
EbuildVariableCommand::do_run_command(const std::string & cmd)
{
    PStream prog(cmd);
    _result = strip_trailing_string(
            std::string((std::istreambuf_iterator<char>(prog)),
                std::istreambuf_iterator<char>()), "\n");

    return (0 == prog.exit_status());
}

std::string
EbuildFetchCommand::commands() const
{
    if (fetch_params.get<ecfpk_no_fetch>())
        return "nofetch";
    else
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
            ("AA", fetch_params.get<ecfpk_aa>())
            ("USE", fetch_params.get<ecfpk_use>())
            ("USE_EXPAND", fetch_params.get<ecfpk_use_expand>())
            ("FLAT_SRC_URI", fetch_params.get<ecfpk_flat_src_uri>())
            ("ROOT", fetch_params.get<ecfpk_root>())
            ("PALUDIS_PROFILE_DIR", stringify(*fetch_params.get<ecfpk_profiles>()->begin()))
            ("PALUDIS_PROFILE_DIRS", join(fetch_params.get<ecfpk_profiles>()->begin(),
                                          fetch_params.get<ecfpk_profiles>()->end(), " ")));

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
            ("AA", install_params.get<ecipk_aa>())
            ("USE", install_params.get<ecipk_use>())
            ("USE_EXPAND", install_params.get<ecipk_use_expand>())
            ("ROOT", install_params.get<ecipk_root>())
            ("PALUDIS_EBUILD_OVERRIDE_CONFIG_PROTECT_MASK",
                install_params.get<ecipk_disable_cfgpro>() ? "/" : "")
            ("PALUDIS_PROFILE_DIR", stringify(*install_params.get<ecipk_profiles>()->begin()))
            ("PALUDIS_PROFILE_DIRS", join(install_params.get<ecipk_profiles>()->begin(),
                                          install_params.get<ecipk_profiles>()->end(), " "))
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

    if (uninstall_params.get<ecupk_load_environment>())
        result = result
            ("PALUDIS_LOAD_ENVIRONMENT", stringify(*uninstall_params.get<ecupk_load_environment>()))
            ("PALUDIS_SKIP_INHERIT", "yes");

    return result;
}

EbuildUninstallCommand::EbuildUninstallCommand(const EbuildCommandParams & p,
        const EbuildUninstallCommandParams & f) :
    EbuildCommand(p),
    uninstall_params(f)
{
}

