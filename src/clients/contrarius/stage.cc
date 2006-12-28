/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
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

#include <paludis/environment/default/default_environment.hh>
#include <paludis/environment/default/default_config.hh>
#include <paludis/util/log.hh>
#include <string>

#include "stage.hh"
#include "install.hh"

using namespace paludis;

namespace
{
    const PackageDepAtom * make_atom(const HostTupleName & target,
        const std::string & name,
        const std::string & default_name,
        const std::string & version)
    {
        return new PackageDepAtom("cross-" + stringify(target) + "/"
                + (name.empty() ? default_name : name) + (version.empty() ? "" : "-" + version));
    }
}

#include <src/clients/contrarius/contrarius_stage_options-sr.cc>

ContrariusStageOptions::ContrariusStageOptions(const HostTupleName & _target,
        const std::string & binutils_name,
        const std::string & binutils_version,
        const std::string & gcc_name,
        const std::string & gcc_version,
        const std::string & headers_name,
        const std::string & headers_version,
        const std::string & libc_name,
        const std::string & libc_version) :
    target(_target),
    binutils(::make_atom(target, binutils_name, "binutils", binutils_version)),
    gcc(::make_atom(target, gcc_name, "gcc", gcc_version)),
    headers(::make_atom(target, headers_name, "linux-headers", headers_version)),
    libc(::make_atom(target, libc_name, "glibc", libc_version))
{
}

int
BinutilsStage::build(const StageOptions &) const
{
    Context context("When building BinutilsStage:");

    DefaultConfig::get_instance()->clear_forced_use_config();

    return do_install(_options.binutils);
}

bool
BinutilsStage::is_rebuild() const
{
    return (! DefaultEnvironment::get_instance()->package_database()->query(_options.binutils, is_installed_only)->empty());
}

int
KernelHeadersStage::build(const StageOptions &) const
{
    Context context("When building KernelHeadersStage:");

    DefaultConfig::get_instance()->clear_forced_use_config();

    DefaultConfig::get_instance()->add_forced_use_config(UseConfigEntry(
            _options.headers, UseFlagName("crosscompile_opts_headers-only"),
            use_disabled));

    return do_install(_options.headers);
}

bool
KernelHeadersStage::is_rebuild() const
{
    return (! DefaultEnvironment::get_instance()->package_database()->query(_options.headers, is_installed_only)->empty());
}

int
MinimalStage::build(const StageOptions &) const
{
    Context context("When executing MinimalStage:");

    DefaultConfig::get_instance()->clear_forced_use_config();

    DefaultConfig::get_instance()->add_forced_use_config(UseConfigEntry(
            _options.gcc, UseFlagName("boundschecking"), use_disabled));
    DefaultConfig::get_instance()->add_forced_use_config(UseConfigEntry(
            _options.gcc, UseFlagName("fortran"), use_disabled));
    DefaultConfig::get_instance()->add_forced_use_config(UseConfigEntry(
            _options.gcc, UseFlagName("gtk"), use_disabled));
    DefaultConfig::get_instance()->add_forced_use_config(UseConfigEntry(
            _options.gcc, UseFlagName("gcj"), use_disabled));
    DefaultConfig::get_instance()->add_forced_use_config(UseConfigEntry(
            _options.gcc, UseFlagName("mudflap"), use_disabled));
    DefaultConfig::get_instance()->add_forced_use_config(UseConfigEntry(
            _options.gcc, UseFlagName("objc"), use_disabled));
    DefaultConfig::get_instance()->add_forced_use_config(UseConfigEntry(
            _options.gcc, UseFlagName("objc-gc"), use_disabled));
    DefaultConfig::get_instance()->add_forced_use_config(UseConfigEntry(
            _options.gcc, UseFlagName("nocxx"), use_enabled));
    DefaultConfig::get_instance()->add_forced_use_config(UseConfigEntry(
            _options.gcc, UseFlagName("crosscompile_opts_bootstrap"), use_enabled));

    return do_install(_options.gcc);
}

bool
MinimalStage::is_rebuild() const
{
   return (! DefaultEnvironment::get_instance()->package_database()
            ->query(_options.gcc, is_installed_only)->empty());
}

int
LibCStage::build(const StageOptions &) const
{
    Context context("When building LibCStage:");

    DefaultConfig::get_instance()->clear_forced_use_config();

    return do_install(_options.libc);
}

bool
LibCStage::is_rebuild() const
{
    return (! DefaultEnvironment::get_instance()->package_database()
            ->query(_options.libc, is_installed_only)->empty());
}

int
FullStage::build(const StageOptions &) const
{
    Context context("When building FullStage:");

    DefaultConfig::get_instance()->clear_forced_use_config();

    DefaultConfig::get_instance()->add_forced_use_config(UseConfigEntry(
            _options.gcc, UseFlagName("boundschecking"), use_disabled));
    DefaultConfig::get_instance()->add_forced_use_config(UseConfigEntry(
            _options.gcc, UseFlagName("gtk"), use_disabled));
    DefaultConfig::get_instance()->add_forced_use_config(UseConfigEntry(
            _options.gcc, UseFlagName("gcj"), use_disabled));
    DefaultConfig::get_instance()->add_forced_use_config(UseConfigEntry(
            _options.gcc, UseFlagName("mudflap"), use_disabled));
    DefaultConfig::get_instance()->add_forced_use_config(UseConfigEntry(
            _options.gcc, UseFlagName("objc"), use_disabled));
    DefaultConfig::get_instance()->add_forced_use_config(UseConfigEntry(
            _options.gcc, UseFlagName("objc-gc"), use_disabled));

    return do_install(_options.gcc);
}

bool
FullStage::is_rebuild() const
{
    PackageDatabaseEntryCollection::ConstPointer c(
            DefaultEnvironment::get_instance()->package_database()->query(_options.gcc, is_installed_only));

    if (c->empty())
        return false;

    return (! DefaultEnvironment::get_instance()->query_use(UseFlagName("nocxx"), &(*c->last())));
}
