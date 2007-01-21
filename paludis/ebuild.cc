/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/about.hh>
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
#include <unistd.h>

/** \file
 * Implementation for ebuild.hh things.
 *
 * \ingroup grpebuildinterface
 */

using namespace paludis;

#include <paludis/ebuild-sr.cc>

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
            stringify(params.ebuild_dir) + "/" +
            stringify(params.db_entry->name.package) + "-" +
            stringify(params.db_entry->version) +
            ".ebuild' " + commands());

    if (use_sandbox())
        ebuild_cmd = make_sandbox_command(ebuild_cmd);

    MakeEnvCommand cmd(extend_command(make_env_command(ebuild_cmd)
                ("P", stringify(params.db_entry->name.package) + "-" +
                 stringify(params.db_entry->version.remove_revision()))
                ("PV", stringify(params.db_entry->version.remove_revision()))
                ("PR", stringify(params.db_entry->version.revision_only()))
                ("PN", stringify(params.db_entry->name.package))
                ("PVR", stringify(params.db_entry->version))
                ("PF", stringify(params.db_entry->name.package) + "-" +
                 stringify(params.db_entry->version))
                ("CATEGORY", stringify(params.db_entry->name.category))
                ("REPOSITORY", stringify(params.db_entry->repository))
                ("FILESDIR", stringify(params.files_dir))
                ("ECLASSDIR", stringify(*params.eclassdirs->begin()))
                ("ECLASSDIRS", join(params.eclassdirs->begin(),
                                    params.eclassdirs->end(), " "))
                ("PORTDIR", stringify(params.portdir))
                ("DISTDIR", stringify(params.distdir))
                ("PKGMANAGER", PALUDIS_PACKAGE "-" + stringify(PALUDIS_VERSION_MAJOR) + "." +
                         stringify(PALUDIS_VERSION_MINOR) + "." +
                         stringify(PALUDIS_VERSION_MICRO) +
                         (std::string(PALUDIS_SUBVERSION_REVISION).empty() ?
                          std::string("") : "-r" + std::string(PALUDIS_SUBVERSION_REVISION)))
                ("PALUDIS_TMPDIR", stringify(params.buildroot))
                ("PALUDIS_CONFIG_DIR", SYSCONFDIR "/paludis/")
                ("PALUDIS_BASHRC_FILES", params.environment->bashrc_files())
                ("PALUDIS_HOOK_DIRS", params.environment->hook_dirs())
                ("PALUDIS_FETCHERS_DIRS", params.environment->fetchers_dirs())
                ("PALUDIS_SYNCERS_DIRS", params.environment->syncers_dirs())
                ("PALUDIS_COMMAND", params.environment->paludis_command())
                ("KV", kernel_version())
                ("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
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
        ("PORTAGE_ACTUAL_DISTDIR", stringify(params.distdir))
        ("PORTAGE_BASHRC", "/dev/null")
        ("PORTAGE_BUILDDIR", stringify(params.buildroot) + "/" +
             stringify(params.db_entry->name.category) + "/" +
             stringify(params.db_entry->name.package) + "-" +
             stringify(params.db_entry->version))
        ("PORTAGE_CALLER", params.environment->paludis_command())
        ("PORTAGE_GID", "0")
        ("PORTAGE_INST_GID", "0")
        ("PORTAGE_INST_UID", "0")
        ("PORTAGE_MASTER_PID", stringify(::getpid()))
        ("PORTAGE_NICENCESS", stringify(::getpriority(PRIO_PROCESS, 0)))
        ("PORTAGE_TMPDIR", stringify(params.buildroot))
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
        _metadata->deps.build_depend_string = f.get("DEPEND");
        _metadata->deps.run_depend_string = f.get("RDEPEND");
        _metadata->slot = SlotName(f.get("SLOT"));
        _metadata->get_ebuild_interface()->src_uri = f.get("SRC_URI");
        _metadata->get_ebuild_interface()->restrict_string = f.get("RESTRICT");
        _metadata->homepage = f.get("HOMEPAGE");
        _metadata->license_string = f.get("LICENSE");
        _metadata->description = f.get("DESCRIPTION");
        _metadata->get_ebuild_interface()->keywords = f.get("KEYWORDS");
        _metadata->get_ebuild_interface()->eclass_keywords = f.get("E_KEYWORDS");
        _metadata->get_ebuild_interface()->inherited = f.get("INHERITED");
        _metadata->get_ebuild_interface()->iuse = f.get("IUSE");
        _metadata->deps.post_depend_string = f.get("PDEPEND");
        _metadata->get_ebuild_interface()->provide_string = f.get("PROVIDE");
        _metadata->eapi = f.get("EAPI");

        if (0 == prog.exit_status())
            ok = true;
    }
    catch (const NameError & e)
    {
        Log::get_instance()->message(ll_warning, lc_context, "Caught exception '" +
                stringify(e.message()) + "' (" + stringify(e.what()) +
                ") when generating cache for '" + stringify(*params.db_entry) + "'");
    }

    if (ok)
        return true;
    else
    {
        Log::get_instance()->message(ll_warning, lc_context, "Could not generate cache for '"
                + stringify(*params.db_entry) + "'");
        _metadata->eapi = "UNKNOWN";

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
    if (fetch_params.no_fetch)
        return "nofetch";
    else
        return "fetch";
}

bool
EbuildFetchCommand::failure()
{
    throw PackageFetchActionError("Fetch failed for '" + stringify(
                *params.db_entry) + "'");
}

MakeEnvCommand
EbuildFetchCommand::extend_command(const MakeEnvCommand & cmd)
{
    MakeEnvCommand result(cmd
            ("A", fetch_params.a)
            ("AA", fetch_params.aa)
            ("USE", fetch_params.use)
            ("USE_EXPAND", fetch_params.use_expand)
            ("FLAT_SRC_URI", fetch_params.flat_src_uri)
            ("ROOT", fetch_params.root)
            ("PALUDIS_USE_SAFE_RESUME", fetch_params.safe_resume ? "oohyesplease" : "")
            ("PALUDIS_PROFILE_DIR", stringify(*fetch_params.profiles->begin()))
            ("PALUDIS_PROFILE_DIRS", join(fetch_params.profiles->begin(),
                                          fetch_params.profiles->end(), " ")));

    for (AssociativeCollection<std::string, std::string>::Iterator
            i(fetch_params.expand_vars->begin()),
            j(fetch_params.expand_vars->end()) ; i != j ; ++i)
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
    return "init setup unpack compile test install strip preinst "
        "merge postinst tidyup";
}

bool
EbuildInstallCommand::failure()
{
    throw PackageInstallActionError("Install failed for '" + stringify(
                *params.db_entry) + "'");
}

MakeEnvCommand
EbuildInstallCommand::extend_command(const MakeEnvCommand & cmd)
{
    std::string debug_build;
    do
    {
        switch (install_params.debug_build)
        {
            case ido_none:
                debug_build = "none";
                continue;

            case ido_split:
                debug_build = "split";
                continue;

            case ido_internal:
                debug_build = "internal";
                continue;
        }

        throw InternalError(PALUDIS_HERE, "Bad debug_build value");
    }
    while (false);

    MakeEnvCommand result(cmd
            ("A", install_params.a)
            ("AA", install_params.aa)
            ("USE", install_params.use)
            ("USE_EXPAND", install_params.use_expand)
            ("ROOT", install_params.root)
            ("PALUDIS_EBUILD_OVERRIDE_CONFIG_PROTECT_MASK",
                install_params.disable_cfgpro ? "/" : "")
            ("PALUDIS_DEBUG_BUILD", debug_build)
            ("PALUDIS_PROFILE_DIR", stringify(*install_params.profiles->begin()))
            ("PALUDIS_PROFILE_DIRS", join(install_params.profiles->begin(),
                                          install_params.profiles->end(), " "))
            ("SLOT", stringify(install_params.slot)));

    for (AssociativeCollection<std::string, std::string>::Iterator
            i(install_params.expand_vars->begin()),
            j(install_params.expand_vars->end()) ; i != j ; ++i)
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
    if (uninstall_params.unmerge_only)
        return "unmerge";
    else
        return "prerm unmerge postrm";
}

bool
EbuildUninstallCommand::failure()
{
    throw PackageUninstallActionError("Uninstall failed for '" + stringify(
                *params.db_entry) + "'");
}

MakeEnvCommand
EbuildUninstallCommand::extend_command(const MakeEnvCommand & cmd)
{
    MakeEnvCommand result(cmd
            ("ROOT", uninstall_params.root)
            ("PALUDIS_EBUILD_OVERRIDE_CONFIG_PROTECT_MASK",
                uninstall_params.disable_cfgpro ? "/" : ""));

    if (uninstall_params.load_environment)
        result = result
            ("PALUDIS_LOAD_ENVIRONMENT", stringify(*uninstall_params.load_environment))
            ("PALUDIS_SKIP_INHERIT", "yes");

    return result;
}

EbuildUninstallCommand::EbuildUninstallCommand(const EbuildCommandParams & p,
        const EbuildUninstallCommandParams & f) :
    EbuildCommand(p),
    uninstall_params(f)
{
}

