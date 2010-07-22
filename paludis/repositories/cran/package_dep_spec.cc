/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/repositories/cran/package_dep_spec.hh>
#include <paludis/repositories/cran/normalise.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/dep_spec.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_spec.hh>
#include <paludis/version_requirements.hh>
#include <vector>

using namespace paludis;
using namespace paludis::cranrepository;

namespace
{
    struct CRANPackageDepSpecData :
        PackageDepSpecData
    {
        std::string unnormalised_package_name;
        std::shared_ptr<const QualifiedPackageName> package_v;
        std::shared_ptr<VersionRequirements> version_requirements_v;

        virtual std::string as_string() const
        {
            std::string result(unnormalised_package_name);

            if (version_requirements_v && ! version_requirements_v->empty())
            {
                result.append(" (");
                bool first(true);
                for (VersionRequirements::ConstIterator r(version_requirements_v->begin()),
                        r_end(version_requirements_v->end()) ; r != r_end ; ++r)
                {
                    if (! first)
                        result.append(", ");
                    first = false;
                    result.append(stringify(r->version_operator()));
                    result.append(" ");
                    result.append(stringify(r->version_spec()));
                }
                result.append(")");
            }

            return result;
        }

        virtual std::shared_ptr<const QualifiedPackageName> package_ptr() const
        {
            return package_v;
        }

        virtual std::shared_ptr<const PackageNamePart> package_name_part_ptr() const
        {
            return std::shared_ptr<const PackageNamePart>();
        }

        virtual std::shared_ptr<const CategoryNamePart> category_name_part_ptr() const
        {
            return std::shared_ptr<const CategoryNamePart>();
        }

        virtual std::shared_ptr<const VersionRequirements> version_requirements_ptr() const
        {
            return version_requirements_v;
        }

        virtual VersionRequirementsMode version_requirements_mode() const
        {
            return vr_and;
        }

        virtual std::shared_ptr<const SlotRequirement> slot_requirement_ptr() const
        {
            return std::shared_ptr<const SlotRequirement>();
        }

        virtual std::shared_ptr<const RepositoryName> from_repository_ptr() const
        {
            return std::shared_ptr<const RepositoryName>();
        }

        virtual std::shared_ptr<const RepositoryName> in_repository_ptr() const
        {
            return std::shared_ptr<const RepositoryName>();
        }

        virtual std::shared_ptr<const AdditionalPackageDepSpecRequirements> additional_requirements_ptr() const
        {
            return std::shared_ptr<const AdditionalPackageDepSpecRequirements>();
        }

        virtual std::shared_ptr<const MetadataSectionKey> annotations_key() const
        {
            return std::shared_ptr<const MetadataSectionKey>();
        }

        CRANPackageDepSpecData & version_requirement(const VersionRequirement & v)
        {
            if (! version_requirements_v)
                version_requirements_v.reset(new VersionRequirements);
            version_requirements_v->push_back(v);
            return *this;
        }

        CRANPackageDepSpecData & package(const std::string & s)
        {
            unnormalised_package_name = s;
            if ("R" == s)
                package_v.reset(new QualifiedPackageName("dev-lang/R"));
            else
                package_v.reset(new QualifiedPackageName(CategoryNamePart("cran") + PackageNamePart(cran_name_to_internal(s))));

            return *this;
        }

        virtual std::shared_ptr<const InstallableToRepository> installable_to_repository_ptr() const
        {
            return make_null_shared_ptr();
        }

        virtual std::shared_ptr<const FSEntry> installed_at_path_ptr() const
        {
            return make_null_shared_ptr();
        }

        virtual std::shared_ptr<const InstallableToPath> installable_to_path_ptr() const
        {
            return make_null_shared_ptr();
        }

        virtual const PartiallyMadePackageDepSpecOptions options_for_partially_made_package_dep_spec() const
        {
            return PartiallyMadePackageDepSpecOptions();
        }
    };
}

PackageDepSpec
paludis::cranrepository::parse_cran_package_dep_spec(const std::string & ss)
{
    Context context("When parsing CRAN package dep spec '" + ss + "':");

    std::shared_ptr<CRANPackageDepSpecData> data(new CRANPackageDepSpecData);
    std::string s(ss);

    std::string::size_type p(s.find('('));
    if (std::string::npos != p)
    {
        std::string restrictions(s.substr(p));
        s.erase(p);
        s = strip_trailing(s, " \r\n\t");

        if (restrictions.empty() || (')' != restrictions.at(restrictions.length() - 1)))
            throw PackageDepSpecError("Invalid () part in '" + ss + "'");
        restrictions = strip_leading(strip_trailing(restrictions.substr(1, restrictions.length() - 2), " \r\n\t"), " \r\n\t");

        std::vector<std::string> tokens;
        tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(restrictions, ",", "", std::back_inserter(tokens));
        if (tokens.empty())
            throw PackageDepSpecError("Invalid empty () part in '" + ss + "'");

        for (std::vector<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
                t != t_end ; ++t)
        {
            std::vector<std::string> subtokens;
            tokenise_whitespace(*t, std::back_inserter(subtokens));
            if (subtokens.size() == 1)
            {
                std::string::size_type vp(subtokens[0].find_first_not_of("<>="));
                if (std::string::npos != vp)
                {
                    subtokens.push_back(subtokens[0].substr(vp));
                    subtokens[0].erase(vp);
                }
            }

            if (subtokens.size() != 2)
                throw PackageDepSpecError("Invalid () entry '" + *t + "' in '" + ss + "'");
            data->version_requirement(make_named_values<VersionRequirement>(
                        n::version_operator() = VersionOperator(subtokens[0]),
                        n::version_spec() = VersionSpec(cran_version_to_internal(subtokens[1]), VersionSpecOptions())));
        }
    }

    data->package(s);

    return PackageDepSpec(data);
}

