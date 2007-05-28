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
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/system.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/config_file.hh>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<EAPIData>
    {
        MakeHashedMap<std::string, EAPI>::Type values;

        Implementation()
        {
            Context c("When loading EAPI data:");

            for (DirIterator d(getenv_with_default("PALUDIS_EAPIS_DIR", DATADIR "/paludis/eapis")), d_end ;
                    d != d_end ; ++d)
            {
                if (! is_file_with_extension(*d, ".conf", IsFileWithOptions()))
                    continue;

                Context cc("When loading EAPI file '" + stringify(*d) + "':");

                KeyValueConfigFile k(*d, KeyValueConfigFileOptions());

                values.insert(std::make_pair(strip_trailing_string(d->basename(), ".conf"),
                            EAPI(strip_trailing_string(d->basename(), ".conf"), make_shared_ptr(new SupportedEAPI(
                                        SupportedEAPI::create()
                                        .package_dep_spec_parse_mode(destringify<PackageDepSpecParseMode>(
                                                k.get("package_dep_spec_parse_mode")))
                                        .strict_package_dep_spec_parse_mode(destringify<PackageDepSpecParseMode>(
                                                k.get("strict_package_dep_spec_parse_mode")))
                                        .dependency_spec_tree_parse_mode(destringify<DependencySpecTreeParseMode>(
                                                k.get("dependency_spec_tree_parse_mode")))
                                        .iuse_flag_parse_mode(destringify<IUseFlagParseMode>(
                                                k.get("iuse_flag_parse_mode")))
                                        .strict_iuse_flag_parse_mode(destringify<IUseFlagParseMode>(
                                                k.get("strict_iuse_flag_parse_mode")))
                                        .breaks_portage(destringify<bool>(k.get("breaks_portage")))
                                        .has_pkg_pretend(destringify<bool>(k.get("has_pkg_pretend")))
                                        .uri_supports_arrow(destringify<bool>(k.get("uri_supports_arrow")))
                                        .want_aa_var(destringify<bool>(k.get("want_aa_var")))
                                        .want_arch_var(destringify<bool>(k.get("want_arch_var")))
                                        .want_portage_emulation_vars(destringify<bool>(k.get("want_portage_emulation_vars")))
                                        .require_use_expand_in_iuse(destringify<bool>(k.get("require_use_expand_in_iuse")))
                                        )))));
            }

            MakeHashedMap<std::string, EAPI>::Type::const_iterator i(values.find("0"));
            if (i == values.end())
                throw EAPIConfigurationError("No EAPI configuration found for EAPI 0");
            else
                values.insert(std::make_pair("", i->second));
        }
    };
}

#include <paludis/eapi-sr.cc>

EAPIConfigurationError::EAPIConfigurationError(const std::string & s) throw () :
    ConfigurationError("EAPI configuration error: " + s)
{
}

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

