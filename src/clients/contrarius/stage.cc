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
#include <paludis/util/tokeniser.hh>
#include <paludis/config_file.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/query.hh>
#include <string>
#include <tr1/memory>

#include "stage.hh"
#include "target_config.hh"
#include "install.hh"

using namespace paludis;

int
AuxiliaryStage::build(const StageOptions &) const
{
    Context context("When building AuxiliaryStage:");

    return 0 == do_install(_env, TargetConfig::get_instance()->aux());
}

bool
AuxiliaryStage::is_rebuild() const
{
    std::list<std::string> packages;
    WhitespaceTokeniser::get_instance()->tokenise(TargetConfig::get_instance()->aux(), std::back_inserter(packages));

    for (std::list<std::string>::const_iterator p(packages.begin()), p_end(packages.end()) ;
            p != p_end ; ++p)
        if ( _env->package_database()->query(
                    query::Matches(PackageDepSpec(*p)) &
                        query::InstalledAtRoot(_env->root()),
                    qo_whatever)->empty())
            return false;

    return true;
}

int
BinutilsStage::build(const StageOptions &) const
{
    Context context("When building BinutilsStage:");

    std::tr1::shared_ptr<PackageDepSpec> binutils(new PackageDepSpec(TargetConfig::get_instance()->binutils(),
                pds_pm_unspecific));

    _env->clear_forced_use();

    return 0 == do_install(_env, stringify(*binutils));
}

bool
BinutilsStage::is_rebuild() const
{
    return (! _env->package_database()->query(
                query::Matches(PackageDepSpec(TargetConfig::get_instance()->binutils(), pds_pm_unspecific)) &
                    query::InstalledAtRoot(_env->root()),
                qo_whatever)->empty());
}

int
KernelHeadersStage::build(const StageOptions &) const
{
    Context context("When building KernelHeadersStage:");

    std::tr1::shared_ptr<PackageDepSpec> headers(new PackageDepSpec(TargetConfig::get_instance()->headers(),
                pds_pm_unspecific));

    _env->clear_forced_use();

    _env->force_use(headers, UseFlagName("crosscompile_opts_headers-only"), use_enabled);

    return 0 == do_install(_env, stringify(*headers));
}

bool
KernelHeadersStage::is_rebuild() const
{
    return (! _env->package_database()->query(
                query::Matches(PackageDepSpec(TargetConfig::get_instance()->headers(), pds_pm_unspecific)) &
                    query::InstalledAtRoot(_env->root()),
                qo_whatever)->empty());
}

int
MinimalStage::build(const StageOptions &) const
{
    Context context("When executing MinimalStage:");

    std::tr1::shared_ptr<PackageDepSpec> gcc(new PackageDepSpec(TargetConfig::get_instance()->gcc(),
                pds_pm_unspecific));

    _env->clear_forced_use();

    _env->force_use(gcc, UseFlagName("boundschecking"), use_disabled);
    _env->force_use(gcc, UseFlagName("fortran"), use_disabled);
    _env->force_use(gcc, UseFlagName("gtk"), use_disabled);
    _env->force_use(gcc, UseFlagName("gcj"), use_disabled);
    _env->force_use(gcc, UseFlagName("mudflap"), use_disabled);
    _env->force_use(gcc, UseFlagName("objc"), use_disabled);
    _env->force_use(gcc, UseFlagName("objc-gc"), use_disabled);
    _env->force_use(gcc, UseFlagName("nocxx"), use_enabled);
    _env->force_use(gcc, UseFlagName("crosscompile_opts_bootstrap"), use_enabled);

    return 0 == do_install(_env, stringify(*gcc));
}

bool
MinimalStage::is_rebuild() const
{
   return (! _env->package_database()->query(
                query::Matches(PackageDepSpec(TargetConfig::get_instance()->gcc(), pds_pm_unspecific)) &
                    query::InstalledAtRoot(_env->root()),
                qo_whatever)->empty());
}

int
LibCStage::build(const StageOptions &) const
{
    Context context("When building LibCStage:");

    std::tr1::shared_ptr<PackageDepSpec> libc(new PackageDepSpec(TargetConfig::get_instance()->libc(),
                pds_pm_unspecific));

    _env->clear_forced_use();

    return 0 == do_install(_env, stringify(*libc));
}

bool
LibCStage::is_rebuild() const
{
    std::tr1::shared_ptr<const PackageDatabaseEntryCollection> c(_env->package_database()->query(
                query::Matches(PackageDepSpec(TargetConfig::get_instance()->libc(), pds_pm_unspecific)) &
                    query::InstalledAtRoot(_env->root()),
                qo_whatever));

    if (c->empty())
        return false;

    return (! _env->query_use(UseFlagName("crosscompile_opts_headers-only"), &(*c->last())));
}

int
FullStage::build(const StageOptions &) const
{
    Context context("When building FullStage:");

    std::tr1::shared_ptr<PackageDepSpec> gcc(new PackageDepSpec(TargetConfig::get_instance()->gcc(),
                pds_pm_unspecific));

    _env->clear_forced_use();

    _env->force_use(gcc, UseFlagName("boundschecking"), use_disabled);
    _env->force_use(gcc, UseFlagName("gtk"), use_disabled);
    _env->force_use(gcc, UseFlagName("gcj"), use_disabled);
    _env->force_use(gcc, UseFlagName("mudflap"), use_disabled);
    _env->force_use(gcc, UseFlagName("objc"), use_disabled);
    _env->force_use(gcc, UseFlagName("objc-gc"), use_disabled);

    return 0 == do_install(_env, stringify(*gcc));
}

bool
FullStage::is_rebuild() const
{
    std::tr1::shared_ptr<const PackageDatabaseEntryCollection> c(_env->package_database()->query(
                query::Matches(PackageDepSpec(TargetConfig::get_instance()->gcc(), pds_pm_unspecific)) &
                    query::InstalledAtRoot(_env->root()),
                qo_whatever));

    if (c->empty())
        return false;

    return (! _env->query_use(UseFlagName("nocxx"), &(*c->last())));
}

