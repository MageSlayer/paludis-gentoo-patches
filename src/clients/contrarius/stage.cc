/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk
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
#include <paludis/util/sequence.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/options.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/package_database.hh>
#include <paludis/environments/adapted/adapted_environment.hh>
#include <paludis/user_dep_spec.hh>
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
    tokenise_whitespace(TargetConfig::get_instance()->aux(), std::back_inserter(packages));

    for (std::list<std::string>::const_iterator p(packages.begin()), p_end(packages.end()) ;
            p != p_end ; ++p)
        if ((*_env)[selection::SomeArbitraryVersion(
                    generator::Matches(parse_user_package_dep_spec(*p, _env.get(), UserPackageDepSpecOptions())) |
                    filter::InstalledAtRoot(_env->root()))]->empty())
            return false;

    return true;
}

int
BinutilsStage::build(const StageOptions &) const
{
    Context context("When building BinutilsStage:");

    std::tr1::shared_ptr<PackageDepSpec> binutils(new PackageDepSpec(
                parse_user_package_dep_spec(TargetConfig::get_instance()->binutils(), _env.get(),
                    UserPackageDepSpecOptions())));

    _env->clear_adaptions();

    return 0 == do_install(_env, stringify(*binutils));
}

bool
BinutilsStage::is_rebuild() const
{
    return (! (*_env)[selection::SomeArbitraryVersion(
                generator::Matches(parse_user_package_dep_spec(
                        TargetConfig::get_instance()->binutils(), _env.get(), UserPackageDepSpecOptions())) |
                filter::InstalledAtRoot(_env->root()))]->empty());
}

int
KernelHeadersStage::build(const StageOptions &) const
{
    Context context("When building KernelHeadersStage:");

    std::tr1::shared_ptr<PackageDepSpec> headers(new PackageDepSpec(
                parse_user_package_dep_spec(TargetConfig::get_instance()->headers(),
                    _env.get(), UserPackageDepSpecOptions())));

    _env->clear_adaptions();

    _env->adapt_use(headers, UseFlagName("crosscompile_opts_headers-only"), use_enabled);

    return 0 == do_install(_env, stringify(*headers));
}

bool
KernelHeadersStage::is_rebuild() const
{
    return (! (*_env)[selection::SomeArbitraryVersion(
                generator::Matches(parse_user_package_dep_spec(TargetConfig::get_instance()->headers(),
                        _env.get(), UserPackageDepSpecOptions())) |
                filter::InstalledAtRoot(_env->root()))]->empty());
}

int
MinimalStage::build(const StageOptions &) const
{
    Context context("When executing MinimalStage:");

    std::tr1::shared_ptr<PackageDepSpec> gcc(new PackageDepSpec(parse_user_package_dep_spec(
                    TargetConfig::get_instance()->gcc(),
                    _env.get(), UserPackageDepSpecOptions())));

    _env->clear_adaptions();

    _env->adapt_use(gcc, UseFlagName("boundschecking"), use_disabled);
    _env->adapt_use(gcc, UseFlagName("fortran"), use_disabled);
    _env->adapt_use(gcc, UseFlagName("gtk"), use_disabled);
    _env->adapt_use(gcc, UseFlagName("gcj"), use_disabled);
    _env->adapt_use(gcc, UseFlagName("mudflap"), use_disabled);
    _env->adapt_use(gcc, UseFlagName("objc"), use_disabled);
    _env->adapt_use(gcc, UseFlagName("objc-gc"), use_disabled);
    _env->adapt_use(gcc, UseFlagName("nocxx"), use_enabled);
    _env->adapt_use(gcc, UseFlagName("crosscompile_opts_bootstrap"), use_enabled);

    return 0 == do_install(_env, stringify(*gcc));
}

bool
MinimalStage::is_rebuild() const
{
    return (! (*_env)[selection::SomeArbitraryVersion(
                generator::Matches(parse_user_package_dep_spec(
                        TargetConfig::get_instance()->gcc(), _env.get(), UserPackageDepSpecOptions())) |
                filter::InstalledAtRoot(_env->root()))]->empty());
}

int
LibCHeadersStage::build(const StageOptions &) const
{
    Context context("When building LIbCHeaderStage:");

    std::tr1::shared_ptr<PackageDepSpec> libc(new PackageDepSpec(
                parse_user_package_dep_spec(TargetConfig::get_instance()->libc(), _env.get(),
                    UserPackageDepSpecOptions())));

    _env->clear_adaptions();

    _env->adapt_use(libc, UseFlagName("crosscompile_opts_headers-only"), use_enabled);

    return 0 == do_install(_env, stringify(*libc));
}

bool
LibCHeadersStage::is_rebuild() const
{
    return (! (*_env)[selection::SomeArbitraryVersion(
                generator::Matches(parse_user_package_dep_spec(
                        TargetConfig::get_instance()->libc(), _env.get(), UserPackageDepSpecOptions())) |
                filter::InstalledAtRoot(_env->root()))]->empty());
}

int
LibCStage::build(const StageOptions &) const
{
    Context context("When building LibCStage:");

    std::tr1::shared_ptr<PackageDepSpec> libc(new PackageDepSpec(
                parse_user_package_dep_spec(TargetConfig::get_instance()->libc(), _env.get(),
                    UserPackageDepSpecOptions())));

    _env->clear_adaptions();

    return 0 == do_install(_env, stringify(*libc));
}

bool
LibCStage::is_rebuild() const
{
    std::tr1::shared_ptr<const PackageIDSequence> c((*_env)[selection::BestVersionOnly(
                generator::Matches(parse_user_package_dep_spec(TargetConfig::get_instance()->libc(),
                        _env.get(), UserPackageDepSpecOptions())) |
                filter::InstalledAtRoot(_env->root()))]);

    if (c->empty())
        return false;

    return (! _env->query_use(UseFlagName("crosscompile_opts_headers-only"), **c->last()));
}

int
FullStage::build(const StageOptions &) const
{
    Context context("When building FullStage:");

    std::tr1::shared_ptr<PackageDepSpec> gcc(new PackageDepSpec(parse_user_package_dep_spec(
                    TargetConfig::get_instance()->gcc(), _env.get(),
                    UserPackageDepSpecOptions())));

    _env->clear_adaptions();

    _env->adapt_use(gcc, UseFlagName("boundschecking"), use_disabled);
    _env->adapt_use(gcc, UseFlagName("gtk"), use_disabled);
    _env->adapt_use(gcc, UseFlagName("gcj"), use_disabled);
    _env->adapt_use(gcc, UseFlagName("mudflap"), use_disabled);
    _env->adapt_use(gcc, UseFlagName("objc"), use_disabled);
    _env->adapt_use(gcc, UseFlagName("objc-gc"), use_disabled);

    return 0 == do_install(_env, stringify(*gcc));
}

bool
FullStage::is_rebuild() const
{
    std::tr1::shared_ptr<const PackageIDSequence> c((*_env)[selection::BestVersionOnly(
                generator::Matches(parse_user_package_dep_spec(TargetConfig::get_instance()->gcc(),
                        _env.get(), UserPackageDepSpecOptions())) |
                filter::InstalledAtRoot(_env->root()))]);

    if (c->empty())
        return false;

    return (! _env->query_use(UseFlagName("nocxx"), **c->last()));
}

