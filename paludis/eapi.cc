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

#include <paludis/eapi.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/hashed_containers.hh>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<EAPIData>
    {
        MakeHashedMap<std::string, EAPI>::Type values;

        Implementation()
        {
            values.insert(std::make_pair("0", EAPI("0", tr1::shared_ptr<SupportedEAPI>(new SupportedEAPI(SupportedEAPI::create()
                                    .package_dep_spec_parse_mode(pds_pm_eapi_0)
                                    .strict_package_dep_spec_parse_mode(pds_pm_eapi_0_strict)
                                    .dependency_spec_tree_parse_mode(dst_pm_eapi_0)
                                    .iuse_flag_parse_mode(iuse_pm_eapi_0)
                                    .strict_iuse_flag_parse_mode(iuse_pm_eapi_0_strict)
                                    .breaks_portage(false)
                                    .has_pkg_pretend(false)
                                    .want_aa_var(true)
                                    .want_arch_var(true)
                                    .want_portage_emulation_vars(true)
                                    .require_use_expand_in_iuse(false))))));

            values.insert(std::make_pair("", values.find("0")->second));

            values.insert(std::make_pair("paludis-1", EAPI("paludis-1", tr1::shared_ptr<SupportedEAPI>(new SupportedEAPI(SupportedEAPI::create()
                                    .package_dep_spec_parse_mode(pds_pm_permissive)
                                    .strict_package_dep_spec_parse_mode(pds_pm_permissive)
                                    .dependency_spec_tree_parse_mode(dst_pm_paludis_1)
                                    .iuse_flag_parse_mode(iuse_pm_permissive)
                                    .strict_iuse_flag_parse_mode(iuse_pm_permissive)
                                    .breaks_portage(true)
                                    .has_pkg_pretend(true)
                                    .want_aa_var(false)
                                    .want_arch_var(true)
                                    .want_portage_emulation_vars(false)
                                    .require_use_expand_in_iuse(false))))));

            values.insert(std::make_pair("CRAN-1", EAPI("CRAN-1", tr1::shared_ptr<SupportedEAPI>(new SupportedEAPI(SupportedEAPI::create()
                                    .package_dep_spec_parse_mode(pds_pm_permissive)
                                    .strict_package_dep_spec_parse_mode(pds_pm_eapi_0_strict)
                                    .dependency_spec_tree_parse_mode(dst_pm_eapi_0)
                                    .iuse_flag_parse_mode(iuse_pm_permissive)
                                    .strict_iuse_flag_parse_mode(iuse_pm_permissive)
                                    .breaks_portage(true)
                                    .has_pkg_pretend(false)
                                    .want_aa_var(false)
                                    .want_arch_var(false)
                                    .want_portage_emulation_vars(false)
                                    .require_use_expand_in_iuse(false))))));

            values.insert(std::make_pair("exheres-0", EAPI("exheres-0", tr1::shared_ptr<SupportedEAPI>(new SupportedEAPI(SupportedEAPI::create()
                                    .package_dep_spec_parse_mode(pds_pm_exheres_0)
                                    .strict_package_dep_spec_parse_mode(pds_pm_exheres_0)
                                    .dependency_spec_tree_parse_mode(dst_pm_exheres_0)
                                    .iuse_flag_parse_mode(iuse_pm_exheres_0)
                                    .strict_iuse_flag_parse_mode(iuse_pm_exheres_0)
                                    .breaks_portage(true)
                                    .has_pkg_pretend(true)
                                    .want_aa_var(false)
                                    .want_arch_var(false)
                                    .want_portage_emulation_vars(false)
                                    .require_use_expand_in_iuse(true))))));
        }
    };
}

#include <paludis/eapi-sr.cc>

EAPIData::EAPIData() :
    PrivateImplementationPattern<EAPIData>(new Implementation<EAPIData>)
{
}

EAPIData::~EAPIData()
{
}

EAPI
EAPIData::eapi_from_string(const std::string & s) const
{
    MakeHashedMap<std::string, EAPI>::Type::const_iterator i(_imp->values.find(s));
    if (i != _imp->values.end())
        return i->second;

    return EAPI(s, tr1::shared_ptr<SupportedEAPI>());
}

EAPI
EAPIData::unknown_eapi() const
{
    return EAPI("UNKNOWN", tr1::shared_ptr<SupportedEAPI>());
}

