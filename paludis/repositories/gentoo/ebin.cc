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

#include "ebin.hh"
#include <paludis/portage_dep_parser.hh>
#include <paludis/environment.hh>
#include <paludis/util/system.hh>
#include <paludis/util/log.hh>
#include <paludis/util/strip.hh>
#include <paludis/about.hh>

using namespace paludis;

#include <paludis/repositories/gentoo/ebin-se.cc>
#include <paludis/repositories/gentoo/ebin-sr.cc>

EbinVersionMetadata::EbinVersionMetadata(const SlotName & s) :
    VersionMetadata(
            VersionMetadataBase::create()
            .slot(s)
            .homepage("")
            .description("")
            .eapi("paludis-1")
            .interactive(false),
            VersionMetadataCapabilities::create()
            .ebin_interface(this)
            .ebuild_interface(this)
            .deps_interface(this)
            .license_interface(this)
            .origins_interface(this)
            .cran_interface(0)
            .virtual_interface(0)
            ),
    VersionMetadataDepsInterface(&PortageDepParser::parse_depend),
    VersionMetadataLicenseInterface(&PortageDepParser::parse_license)
{
}

EbinVersionMetadata::~EbinVersionMetadata()
{
}

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

    cmd = extend_command(cmd
            .with_setenv("P", stringify(params.db_entry->name.package) + "-" +
                stringify(params.db_entry->version.remove_revision()))
            .with_setenv("PV", stringify(params.db_entry->version.remove_revision()))
            .with_setenv("PR", stringify(params.db_entry->version.revision_only()))
            .with_setenv("PN", stringify(params.db_entry->name.package))
            .with_setenv("PVR", stringify(params.db_entry->version))
            .with_setenv("PF", stringify(params.db_entry->name.package) + "-" +
                stringify(params.db_entry->version))
            .with_setenv("CATEGORY", stringify(params.db_entry->name.category))
            .with_setenv("REPOSITORY", stringify(params.db_entry->repository))
            .with_setenv("PORTDIR", stringify(params.portdir))
            .with_setenv("PKGDIR", stringify(params.pkgdir))
            .with_setenv("PKGMANAGER", PALUDIS_PACKAGE "-" + stringify(PALUDIS_VERSION_MAJOR) + "." +
                stringify(PALUDIS_VERSION_MINOR) + "." +
                stringify(PALUDIS_VERSION_MICRO) +
                (std::string(PALUDIS_SUBVERSION_REVISION).empty() ?
                 std::string("") : "-r" + std::string(PALUDIS_SUBVERSION_REVISION)))
            .with_setenv("PALUDIS_TMPDIR", stringify(params.buildroot))
            .with_setenv("PALUDIS_CONFIG_DIR", SYSCONFDIR "/paludis/")
            .with_setenv("PALUDIS_BASHRC_FILES", params.environment->bashrc_files())
            .with_setenv("PALUDIS_HOOK_DIRS", params.environment->hook_dirs())
            .with_setenv("PALUDIS_FETCHERS_DIRS", params.environment->fetchers_dirs())
            .with_setenv("PALUDIS_SYNCERS_DIRS", params.environment->syncers_dirs())
            .with_setenv("PALUDIS_COMMAND", params.environment->paludis_command())
            .with_setenv("KV", kernel_version())
            .with_setenv("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
            .with_setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis")));

    if (do_run_command(cmd))
        return success();
    else
        return failure();
}

EbinFetchCommand::EbinFetchCommand(const EbinCommandParams & p, const EbinFetchCommandParams & f) :
    EbinCommand(p),
    fetch_params(f)
{
}

std::string
EbinFetchCommand::commands() const
{
    return "fetchbin";
}

bool
EbinFetchCommand::failure()
{
    throw PackageFetchActionError("Fetch failed for '" + stringify(
                *params.db_entry) + "'");
}

Command
EbinFetchCommand::extend_command(const Command & cmd)
{
    return Command(cmd)
        .with_setenv("B", fetch_params.b)
        .with_setenv("FLAT_BIN_URI", fetch_params.flat_bin_uri)
        .with_setenv("ROOT", fetch_params.root)
        .with_setenv("PALUDIS_USE_SAFE_RESUME", fetch_params.safe_resume ? "oohyesplease" : "");
}

std::string
EbinInstallCommand::commands() const
{
    switch (install_params.phase)
    {
        case ebin_ip_unpack:
            return "initbin unpackbin";

        case ebin_ip_preinstall:
            return "preinst saveenv";

        case ebin_ip_postinstall:
            return "loadenv postinst";

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
    throw PackageInstallActionError("Install failed for '" + stringify(
                *params.db_entry) + "'");
}

Command
EbinInstallCommand::extend_command(const Command & cmd)
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

    switch (install_params.phase)
    {
        case ebin_ip_preinstall:
        case ebin_ip_postinstall:
            result
                .with_setenv("PALUDIS_LOAD_ENVIRONMENT", stringify(params.buildroot /
                            stringify(params.db_entry->name.category) / (
                                stringify(params.db_entry->name.package) + "-" + stringify(params.db_entry->version)) / "temp"
                            / "binpkgenv"))
                .with_setenv("PALUDIS_SKIP_INHERIT", "yes");
            break;

        case ebin_ip_unpack:
        case ebin_ip_tidyup:
        case last_ebin_ip:
            ;
    };

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
        throw PackageInstallActionError("Error creating '" + stringify(tar) + "'");

    Command create_env(Command("cp -f '" + stringify(merge_params.environment_file) + "' '.paludis-binpkg-environment'")
            .with_chdir(merge_params.image)
            .with_echo_to_stderr());

    Command add_env_to_tarball(Command("tar rpf '" + stringify(tar) +
                "' '.paludis-binpkg-environment'")
            .with_chdir(merge_params.image)
            .with_echo_to_stderr());

    if (0 != run_command(create_env) || 0 != run_command(add_env_to_tarball))
        throw PackageInstallActionError("Error adding environment to '" + tar + "'");

    Command clean_env(Command("rm -f '.paludis-binpkg-environment'")
            .with_chdir(merge_params.image)
            .with_echo_to_stderr());
    if (0 != run_command(clean_env))
        Log::get_instance()->message(ll_warning, lc_context, "Cleaning environment failed");

    Command compress_tarball(Command("bzip2 '" + tar + "'")
            .with_chdir(merge_params.image)
            .with_echo_to_stderr());

    if (0 != run_command(compress_tarball))
        throw PackageInstallActionError("Error compressing '" + tar + "'");
}

