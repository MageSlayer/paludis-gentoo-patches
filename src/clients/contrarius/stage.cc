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

#include <paludis/util/log.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/query.hh>
#include <string>

#include "stage.hh"
#include "install.hh"

using namespace paludis;

namespace
{
    const PackageDepSpec * make_spec(const HostTupleName & target,
        const std::string & name,
        const std::string & default_name,
        const std::string & version)
    {
        std::tr1::shared_ptr<VersionRequirements> v;
        if (! version.empty())
        {
            v.reset(new VersionRequirements::Concrete);
            v->push_back(VersionRequirement(vo_equal, VersionSpec(version)));
        }

        return new PackageDepSpec(
                std::tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName("cross-" + stringify(target) + "/"
                        + (name.empty() ? default_name : name))),
                std::tr1::shared_ptr<CategoryNamePart>(),
                std::tr1::shared_ptr<PackageNamePart>(),
                v, vr_and);
    }
}

#include <src/clients/contrarius/contrarius_stage_options-sr.cc>

ContrariusStageOptions::ContrariusStageOptions(
        std::tr1::shared_ptr<Environment> env,
        const HostTupleName & _target,
        const std::string & binutils_name,
        const std::string & binutils_version,
        const std::string & gcc_name,
        const std::string & gcc_version,
        const std::string & headers_name,
        const std::string & headers_version,
        const std::string & libc_name,
        const std::string & libc_version) :
    environment(env),
    target(_target),
    binutils(::make_spec(target, binutils_name, "binutils", binutils_version)),
    gcc(::make_spec(target, gcc_name, "gcc", gcc_version)),
    headers(::make_spec(target, headers_name, "linux-headers", headers_version)),
    libc(::make_spec(target, libc_name, "glibc", libc_version))
{
}

int
BinutilsStage::build(const StageOptions &) const
{
    Context context("When building BinutilsStage:");

    _options.environment->clear_forced_use();

    return 0 == do_install(_options.environment, _options.binutils);
}

bool
BinutilsStage::is_rebuild() const
{
    return (! _options.environment->package_database()->query(
                query::Matches(*_options.binutils) & query::InstalledAtRoot(_options.environment->root()),
                qo_whatever)->empty());
}

int
KernelHeadersStage::build(const StageOptions &) const
{
    Context context("When building KernelHeadersStage:");

    _options.environment->clear_forced_use();

    _options.environment->force_use(
            _options.headers, UseFlagName("crosscompile_opts_headers-only"),
            use_disabled);

    return 0 == do_install(_options.environment, _options.headers);
}

bool
KernelHeadersStage::is_rebuild() const
{
    return (! _options.environment->package_database()->query(
                query::Matches(*_options.headers) & query::InstalledAtRoot(_options.environment->root()),
                qo_whatever)->empty());
}

int
MinimalStage::build(const StageOptions &) const
{
    Context context("When executing MinimalStage:");

    _options.environment->clear_forced_use();

    _options.environment->force_use(_options.gcc, UseFlagName("boundschecking"), use_disabled);
    _options.environment->force_use(_options.gcc, UseFlagName("fortran"), use_disabled);
    _options.environment->force_use(_options.gcc, UseFlagName("gtk"), use_disabled);
    _options.environment->force_use(_options.gcc, UseFlagName("gcj"), use_disabled);
    _options.environment->force_use(_options.gcc, UseFlagName("mudflap"), use_disabled);
    _options.environment->force_use(_options.gcc, UseFlagName("objc"), use_disabled);
    _options.environment->force_use(_options.gcc, UseFlagName("objc-gc"), use_disabled);
    _options.environment->force_use(_options.gcc, UseFlagName("nocxx"), use_enabled);
    _options.environment->force_use(_options.gcc, UseFlagName("crosscompile_opts_bootstrap"), use_enabled);

    return 0 == do_install(_options.environment, _options.gcc);
}

bool
MinimalStage::is_rebuild() const
{
   return (! _options.environment->package_database()->query(
               query::Matches(*_options.gcc) & query::InstalledAtRoot(_options.environment->root()),
               qo_whatever)->empty());
}

int
LibCStage::build(const StageOptions &) const
{
    Context context("When building LibCStage:");

    _options.environment->clear_forced_use();

    return 0 == do_install(_options.environment, _options.libc);
}

bool
LibCStage::is_rebuild() const
{
    std::tr1::shared_ptr<const PackageDatabaseEntryCollection> c(
            _options.environment->package_database()->query(
                query::Matches(*_options.libc) & query::InstalledAtRoot(_options.environment->root()),
                qo_whatever));

    if (c->empty())
        return false;

    return (! _options.environment->query_use(UseFlagName("crosscompile_opts_headers-only"), &(*c->last())));
}

int
FullStage::build(const StageOptions &) const
{
    Context context("When building FullStage:");

    _options.environment->clear_forced_use();

    _options.environment->force_use(_options.gcc, UseFlagName("boundschecking"), use_disabled);
    _options.environment->force_use(_options.gcc, UseFlagName("gtk"), use_disabled);
    _options.environment->force_use(_options.gcc, UseFlagName("gcj"), use_disabled);
    _options.environment->force_use(_options.gcc, UseFlagName("mudflap"), use_disabled);
    _options.environment->force_use(_options.gcc, UseFlagName("objc"), use_disabled);
    _options.environment->force_use(_options.gcc, UseFlagName("objc-gc"), use_disabled);

    return 0 == do_install(_options.environment, _options.gcc);
}

bool
FullStage::is_rebuild() const
{
    std::tr1::shared_ptr<const PackageDatabaseEntryCollection> c(
            _options.environment->package_database()->query(
                query::Matches(*_options.gcc) & query::InstalledAtRoot(_options.environment->root()), qo_whatever));

    if (c->empty())
        return false;

    return (! _options.environment->query_use(UseFlagName("nocxx"), &(*c->last())));
}

