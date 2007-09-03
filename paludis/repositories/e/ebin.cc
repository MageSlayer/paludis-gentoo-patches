/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/repositories/e/ebin.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/environment.hh>
#include <paludis/util/system.hh>
#include <paludis/util/log.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/sequence.hh>
#include <paludis/about.hh>
#include <paludis/package_id.hh>
#include <paludis/action.hh>
#include <paludis/metadata_key.hh>
#include <sys/resource.h>
#include <sys/time.h>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <unistd.h>

using namespace paludis;

#include <paludis/repositories/e/ebin-se.cc>
#include <paludis/repositories/e/ebin-sr.cc>

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
EbinCommand::do_run_command(const Command & cmd)
{
    return 0 == run_command(cmd);
}

bool
EbinCommand::operator() ()
{
    Command cmd(getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis") +
            "/ebuild.bash '-' " + commands());

    if (use_sandbox())
        cmd.with_sandbox();

    tr1::shared_ptr<const FSEntrySequence> syncers_dirs(params.environment->syncers_dirs());
    tr1::shared_ptr<const FSEntrySequence> bashrc_files(params.environment->bashrc_files());
    tr1::shared_ptr<const FSEntrySequence> fetchers_dirs(params.environment->fetchers_dirs());
    tr1::shared_ptr<const FSEntrySequence> hook_dirs(params.environment->hook_dirs());

    cmd = extend_command(cmd
            .with_setenv("P", stringify(params.package_id->name().package) + "-" +
                stringify(params.package_id->version().remove_revision()))
            .with_setenv("PV", stringify(params.package_id->version().remove_revision()))
            .with_setenv("PR", stringify(params.package_id->version().revision_only()))
            .with_setenv("PN", stringify(params.package_id->name().package))
            .with_setenv("PVR", stringify(params.package_id->version()))
            .with_setenv("PF", stringify(params.package_id->name().package) + "-" +
                stringify(params.package_id->version()))
            .with_setenv("CATEGORY", stringify(params.package_id->name().category))
            .with_setenv("REPOSITORY", stringify(params.package_id->repository()->name()))
            .with_setenv("PORTDIR", stringify(params.portdir))
            .with_setenv("DISTDIR", stringify(params.distdir))
            .with_setenv("PKGMANAGER", PALUDIS_PACKAGE "-" + stringify(PALUDIS_VERSION_MAJOR) + "." +
                stringify(PALUDIS_VERSION_MINOR) + "." +
                stringify(PALUDIS_VERSION_MICRO) +
                (std::string(PALUDIS_SUBVERSION_REVISION).empty() ?
                 std::string("") : "-r" + std::string(PALUDIS_SUBVERSION_REVISION)))
            .with_setenv("PALUDIS_TMPDIR", stringify(params.builddir))
            .with_setenv("PALUDIS_CONFIG_DIR", SYSCONFDIR "/paludis/")
            .with_setenv("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "))
            .with_setenv("PALUDIS_HOOK_DIRS", join(hook_dirs->begin(), hook_dirs->end(), " "))
            .with_setenv("PALUDIS_FETCHERS_DIRS", join(fetchers_dirs->begin(), fetchers_dirs->end(), " "))
            .with_setenv("PALUDIS_SYNCERS_DIRS", join(syncers_dirs->begin(), syncers_dirs->end(), " "))
            .with_setenv("PALUDIS_COMMAND", params.environment->paludis_command())
            .with_setenv("PALUDIS_REDUCED_GID", stringify(params.environment->reduced_gid()))
            .with_setenv("PALUDIS_REDUCED_UID", stringify(params.environment->reduced_uid()))
            .with_setenv("KV", kernel_version())
            .with_setenv("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
            .with_setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis")));

    if (do_run_command(add_portage_vars(cmd)))
        return success();
    else
        return failure();
}

Command
EbinCommand::add_portage_vars(const Command & cmd) const
{
    return Command(cmd)
        .with_setenv("PORTAGE_BASHRC", "/dev/null")
        .with_setenv("PORTAGE_BUILDDIR", stringify(params.builddir) + "/" +
             stringify(params.package_id->name().category) + "/" +
             stringify(params.package_id->name().package) + "-" +
             stringify(params.package_id->version()))
        .with_setenv("PORTAGE_CALLER", params.environment->paludis_command())
        .with_setenv("PORTAGE_GID", "0")
        .with_setenv("PORTAGE_INST_GID", "0")
        .with_setenv("PORTAGE_INST_UID", "0")
        .with_setenv("PORTAGE_MASTER_PID", stringify(::getpid()))
        .with_setenv("PORTAGE_NICENCESS", stringify(::getpriority(PRIO_PROCESS, 0)))
        .with_setenv("PORTAGE_TMPDIR", stringify(params.builddir))
        .with_setenv("PORTAGE_TMPFS", "/dev/shm")
        .with_setenv("PORTAGE_WORKDIR_MODE", "0700");
}

std::string
EbinFetchCommand::commands() const
{
    return "fetchbin";
}

bool
EbinFetchCommand::failure()
{
    throw FetchActionError("Fetch failed for '" + stringify(*params.package_id) + "'");
}

Command
EbinFetchCommand::extend_command(const Command & cmd)
{
    Command result(Command(cmd)
            .with_setenv("B", fetch_params.b)
            .with_setenv("FLAT_BIN_URI", fetch_params.flat_bin_uri)
            .with_setenv("ROOT", fetch_params.root)
            .with_setenv("PALUDIS_USE_SAFE_RESUME", fetch_params.safe_resume ? "oohyesplease" : "")
            .with_setenv("ROOT", fetch_params.root)
            .with_setenv("PALUDIS_USE_SAFE_RESUME", fetch_params.safe_resume ? "oohyesplease" : ""));

    if (fetch_params.userpriv)
        result.with_uid_gid(params.environment->reduced_uid(), params.environment->reduced_gid());

    return result;
}

EbinFetchCommand::EbinFetchCommand(const EbinCommandParams & p, const EbinFetchCommandParams & f) :
    EbinCommand(p),
    fetch_params(f)
{
}

std::string
EbinInstallCommand::commands() const
{
    switch (install_params.phase)
    {
        case ebin_ip_prepare:
            return "prepare";

        case ebin_ip_initbinenv:
            return "initbin saveenv";

        case ebin_ip_setup:
            return "loadenv setup saveenv";

        case ebin_ip_unpackbin:
            return "loadenv unpackbin saveenv";

        case ebin_ip_preinstall:
            return "loadenv strip preinst saveenv";

        case ebin_ip_postinstall:
            return "loadenv postinst saveenv";

        case ebin_ip_tidyup:
            return "tidyup";

        case last_ebin_ip:
            ;
    }

    throw InternalError(PALUDIS_HERE, "Bad phase");
}

bool
EbinInstallCommand::failure()
{
    throw InstallActionError("Install failed for '" + stringify(*params.package_id) + "'");
}

Command
EbinInstallCommand::extend_command(const Command & cmd)
{
    std::string debug_build;
    do
    {
        switch (install_params.debug_build)
        {
            case iado_none:
                debug_build = "none";
                continue;

            case iado_split:
                debug_build = "split";
                continue;

            case iado_internal:
                debug_build = "internal";
                continue;

            case last_iado:
                break;
        }

        throw InternalError(PALUDIS_HERE, "Bad debug_build value");
    }
    while (false);

    Command result(Command(cmd)
            .with_setenv("B", install_params.b)
            .with_setenv("ROOT", install_params.root)
            .with_setenv("PALUDIS_LOADSAVEENV_DIR", stringify(install_params.loadsaveenv_dir))
            .with_setenv("PALUDIS_CONFIG_PROTECT", install_params.config_protect)
            .with_setenv("PALUDIS_CONFIG_PROTECT_MASK", install_params.config_protect_mask)
            .with_setenv("PALUDIS_EBUILD_OVERRIDE_CONFIG_PROTECT_MASK",
                install_params.disable_cfgpro ? "/" : "")
            .with_setenv("PALUDIS_DEBUG_BUILD", debug_build)
            .with_setenv("SLOT", stringify(install_params.slot)));

    return result;
}

EbinInstallCommand::EbinInstallCommand(const EbinCommandParams & p, const EbinInstallCommandParams & i) :
    EbinCommand(p),
    install_params(i)
{
}

EbinMergeCommand::EbinMergeCommand(const EbinCommandParams & p, const EbinMergeCommandParams & m) :
    params(p),
    merge_params(m)
{
}

void
EbinMergeCommand::operator() ()
{
    std::string tar(strip_trailing_string(stringify(merge_params.pkg_file_name), ".bz2"));

    Command build_tarball(Command("tar pcvf '" + stringify(tar) + "' ./")
            .with_chdir(merge_params.image)
            .with_echo_to_stderr());

    if (0 != run_command(build_tarball))
        throw InstallActionError("Error creating '" + stringify(tar) + "'");

    Command create_env(Command("cp -f '" + stringify(merge_params.environment_file) + "' '.paludis-binpkg-environment'")
            .with_chdir(merge_params.image)
            .with_echo_to_stderr());

    Command add_env_to_tarball(Command("tar rpf '" + stringify(tar) +
                "' '.paludis-binpkg-environment'")
            .with_chdir(merge_params.image)
            .with_echo_to_stderr());

    if (0 != run_command(create_env) || 0 != run_command(add_env_to_tarball))
        throw InstallActionError("Error adding environment to '" + tar + "'");

    Command clean_env(Command("rm -f '.paludis-binpkg-environment'")
            .with_chdir(merge_params.image)
            .with_echo_to_stderr());
    if (0 != run_command(clean_env))
        Log::get_instance()->message(ll_warning, lc_context, "Cleaning environment failed");

    Command compress_tarball(Command("bzip2 '" + tar + "'")
            .with_chdir(merge_params.image)
            .with_echo_to_stderr());

    if (0 != run_command(compress_tarball))
        throw InstallActionError("Error compressing '" + tar + "'");
}

