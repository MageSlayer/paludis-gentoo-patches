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
            values.insert(std::make_pair("0", EAPI("0", tr1::shared_ptr<SupportedEAPI>(new SupportedEAPI(
                                    pds_pm_eapi_0, pds_pm_eapi_0_strict, iuse_pm_eapi_0, iuse_pm_eapi_0_strict, false, false)))));

            values.insert(std::make_pair("", EAPI("", tr1::shared_ptr<SupportedEAPI>(new SupportedEAPI(
                                    pds_pm_eapi_0, pds_pm_eapi_0_strict, iuse_pm_eapi_0, iuse_pm_eapi_0_strict, false, false)))));

            values.insert(std::make_pair("paludis-1", EAPI("paludis-1", tr1::shared_ptr<SupportedEAPI>(new SupportedEAPI(
                                    pds_pm_permissive, pds_pm_permissive, iuse_pm_permissive, iuse_pm_permissive, true, true)))));

            values.insert(std::make_pair("CRAN-1", EAPI("CRAN-1", tr1::shared_ptr<SupportedEAPI>(new SupportedEAPI(
                                    pds_pm_permissive, pds_pm_permissive, iuse_pm_permissive, iuse_pm_permissive, true, false)))));
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

