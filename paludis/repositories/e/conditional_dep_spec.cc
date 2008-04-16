/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#include <paludis/repositories/e/conditional_dep_spec.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/destringify.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/dep_spec.hh>
#include <paludis/repository.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/metadata_key_holder.hh>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    struct EConditionalDepSpecData :
        ConditionalDepSpecData
    {
        bool inverse;
        UseFlagName flag;

        const Environment * const env;
        const tr1::shared_ptr<const PackageID> id;

        EConditionalDepSpecData(const std::string & s, const Environment * const e, const tr1::shared_ptr<const PackageID> & i) :
            inverse(false),
            flag(UseFlagName("unset")),
            env(e),
            id(i)
        {
            if (! i)
                Log::get_instance()->message("e.conditional_dep_spec.no_id", ll_warning, lc_context) << "! i";

            if (s.empty())
                throw DepStringParseError(s, "missing use flag name");

            if (s.at(s.length() - 1) != '?')
                throw DepStringParseError(s, "missing ? on use conditional");

            inverse = '!' == s.at(0);
            if (s.length() < (inverse ? 3 : 2))
                throw DepStringParseError(s, "missing flag name on use conditional");

            flag = UseFlagName(s.substr(inverse ? 1 : 0, s.length() - (inverse ? 2 : 1)));

            add_metadata_key(make_shared_ptr(new LiteralMetadataValueKey<std::string> ("Flag", "Flag", mkt_normal, stringify(flag))));
            add_metadata_key(make_shared_ptr(new LiteralMetadataValueKey<std::string> ("Inverse", "Inverse", mkt_normal, stringify(inverse))));
        }

        virtual std::string as_string() const
        {
            return (inverse ? "!" : "") + stringify(flag) + "?";
        }

        virtual bool condition_met() const
        {
            if (! id)
                throw InternalError(PALUDIS_HERE, "! id");

            return env->query_use(flag, *id) ^ inverse;
        }

        virtual bool condition_meetable() const
        {
            if (! id)
                throw InternalError(PALUDIS_HERE, "! id");

            RepositoryUseInterface * const u((*id->repository())[k::use_interface()]);
            if (! u)
                return true;

            if (inverse)
                return ! u->query_use_force(flag, *id);
            else
                return ! u->query_use_mask(flag, *id);
        }

        virtual void need_keys_added() const
        {
        }
    };
}

ConditionalDepSpec
paludis::erepository::parse_e_conditional_dep_spec(const std::string & s,
        const Environment * const env, const tr1::shared_ptr<const PackageID> & id, const EAPI &)
{
    return ConditionalDepSpec(make_shared_ptr(new EConditionalDepSpecData(s, env, id)));
}

UseFlagName
paludis::erepository::conditional_dep_spec_flag(const ConditionalDepSpec & spec)
{
    ConditionalDepSpec::MetadataConstIterator i(spec.find_metadata("Flag"));
    if (i == spec.end_metadata())
        throw InternalError(PALUDIS_HERE, "Spec '" + stringify(spec) + "' has no Flag metadata");
    const MetadataValueKey<std::string>  * key(visitor_cast<const MetadataValueKey<std::string> >(**i));
    if (! key)
        throw InternalError(PALUDIS_HERE, "Spec '" + stringify(spec) + "' has Flag metadata which is not a string");
    return UseFlagName(key->value());
}

bool
paludis::erepository::conditional_dep_spec_is_inverse(const ConditionalDepSpec & spec)
{
    ConditionalDepSpec::MetadataConstIterator i(spec.find_metadata("Inverse"));
    if (i == spec.end_metadata())
        throw InternalError(PALUDIS_HERE, "Spec '" + stringify(spec) + "' has no Inverse metadata");
    const MetadataValueKey<std::string>  * key(visitor_cast<const MetadataValueKey<std::string> >(**i));
    if (! key)
        throw InternalError(PALUDIS_HERE, "Spec '" + stringify(spec) + "' has Inverse metadata which is not a string");
    return destringify<bool>(key->value());
}

