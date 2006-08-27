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

#include <paludis/ebin.hh>
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
 * Implementation for ebin.hh things.
 *
 * \ingroup grpebininterface
 */

using namespace paludis;

#include <paludis/ebin-sr.cc>

EbinCommand::EbinCommand(const EbinCommandParams & p) :
    params(p)
{
}

EbinCommand::~EbinCommand()
{
}

bool
EbinCommand::success()
{
    return true;
}

bool
EbinCommand::use_sandbox() const
{
    return true;
}

bool
EbinCommand::failure()
{
    return false;
}

bool
EbinCommand::operator() ()
{
    std::string ebin_cmd(getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis") +
            "/ebuild.bash '" +
            stringify(params.ebin_dir) + "/" +
            stringify(params.db_entry->name.package) + "-" +
            stringify(params.db_entry->version) +
            ".ebin' " + commands());

    if (use_sandbox())
        ebin_cmd = make_sandbox_command(ebin_cmd);

    MakeEnvCommand cmd(extend_command(make_env_command(ebin_cmd)
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
                ("SRC_REPOSITORY", stringify(params.src_repository))
                ("PKGDIR", stringify(params.pkgdir))
                ("PALUDIS_TMPDIR", stringify(params.buildroot))
                ("PALUDIS_CONFIG_DIR", SYSCONFDIR "/paludis/")
                ("PALUDIS_BASHRC_FILES", params.environment->bashrc_files())
                ("PALUDIS_HOOK_DIRS", params.environment->hook_dirs())
                ("PALUDIS_COMMAND", params.environment->paludis_command())
                ("KV", kernel_version())
                ("PALUDIS_EBUILD_LOG_LEVEL", Log::get_instance()->log_level_string())
                ("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))));

    if (do_run_command(add_portage_vars(cmd)))
        return success();
    else
        return failure();
}

MakeEnvCommand
EbinCommand::add_portage_vars(const MakeEnvCommand & cmd) const
{
    return cmd
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
EbinCommand::do_run_command(const std::string & cmd)
{
    return 0 == run_command(cmd);
}

std::string
EbinFetchCommand::commands() const
{
    return "fetch_bin";
}

bool
EbinFetchCommand::failure()
{
    throw PackageFetchActionError("Fetch failed for '" + stringify(
                *params.db_entry) + "'");
}

MakeEnvCommand
EbinFetchCommand::extend_command(const MakeEnvCommand & cmd)
{
    MakeEnvCommand result(cmd
            ("B", fetch_params.b)
            ("FLAT_BIN_URI", fetch_params.flat_bin_uri)
            ("ROOT", fetch_params.root)
            ("PALUDIS_PROFILE_DIR", stringify(*fetch_params.profiles->begin()))
            ("PALUDIS_PROFILE_DIRS", join(fetch_params.profiles->begin(),
                                          fetch_params.profiles->end(), " ")));

    return result;
}

EbinFetchCommand::EbinFetchCommand(const EbinCommandParams & p,
        const EbinFetchCommandParams & f) :
    EbinCommand(p),
    fetch_params(f)
{
}

std::string
EbinInstallCommand::commands() const
{
    return "init_bin unpack_bin setup strip preinst merge postinst tidyup";
}

bool
EbinInstallCommand::failure()
{
    throw PackageInstallActionError("Install failed for '" + stringify(
                *params.db_entry) + "'");
}

MakeEnvCommand
EbinInstallCommand::extend_command(const MakeEnvCommand & cmd)
{
    MakeEnvCommand result(cmd
            ("B", install_params.b)
            ("USE", install_params.use)
            ("USE_EXPAND", install_params.use_expand)
            ("ROOT", install_params.root)
            ("PALUDIS_EBUILD_OVERRIDE_CONFIG_PROTECT_MASK",
                install_params.disable_cfgpro ? "/" : "")
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

EbinInstallCommand::EbinInstallCommand(const EbinCommandParams & p,
        const EbinInstallCommandParams & f) :
    EbinCommand(p),
    install_params(f)
{
}


