/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/e_repository_sets.hh>
#include <paludis/repositories/e/glsa.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/eapi.hh>

#include <paludis/action-fwd.hh>
#include <paludis/dep_spec.hh>
#include <paludis/elike_slot_requirement.hh>
#include <paludis/environment.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/generator.hh>
#include <paludis/match_package.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_id.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/selection.hh>
#include <paludis/set_file.hh>
#include <paludis/slot.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_requirements.hh>

#include <paludis/util/config_file.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/log.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/fs_iterator.hh>

#include <functional>
#include <algorithm>
#include <list>
#include <map>
#include <set>

#include "config.h"

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    /**
     * Imp data for ERepositorySets.
     *
     * \ingroup grperepository
     */
    template<>
    struct Imp<ERepositorySets>
    {
        const Environment * const environment;
        const ERepository * const e_repository;
        const erepository::ERepositoryParams params;

        Imp(const Environment * const e, const ERepository * const p,
                const erepository::ERepositoryParams & k) :
            environment(e),
            e_repository(p),
            params(k)
        {
        }
    };
}

ERepositorySets::ERepositorySets(const Environment * const e, const ERepository * const p,
        const erepository::ERepositoryParams & k) :
    _imp(e, p, k)
{
}

ERepositorySets::~ERepositorySets()
{
}

const std::shared_ptr<const SetSpecTree>
ERepositorySets::package_set(const SetName & ss) const
{
    using namespace std::placeholders;

    if ("system" == ss.value())
        throw InternalError(PALUDIS_HERE, "system set should've been handled by ERepository");
    else if ("security" == ss.value())
        return security_set(false);
    else if ("insecurity" == ss.value())
        return security_set(true);

    std::pair<SetName, SetFileSetOperatorMode> s(find_base_set_name_and_suffix_mode(ss));

    if ((_imp->params.setsdir() / (stringify(s.first) + ".conf")).stat().exists())
    {
        FSPath ff(_imp->params.setsdir() / (stringify(s.first) + ".conf"));
        Context context("When loading package set '" + stringify(s.first) + "' from '" + stringify(ff) + "':");

        SetFile f(make_named_values<SetFileParams>(
                    n::environment() = _imp->environment,
                    n::file_name() = ff,
                    n::parser() = std::bind(&parse_user_package_dep_spec, _1, _imp->environment, UserPackageDepSpecOptions() + updso_no_disambiguation + updso_throw_if_set, filter::All()),
                    n::set_operator_mode() = s.second,
                    n::type() = sft_paludis_conf
                    ));

        return f.contents();
    }
    else
        return make_null_shared_ptr();
}

std::shared_ptr<const SetNameSet>
ERepositorySets::sets_list() const
{
    Context context("While generating the list of sets:");

    std::shared_ptr<SetNameSet> result(std::make_shared<SetNameSet>());
    result->insert(SetName("insecurity"));
    result->insert(SetName("security"));
    result->insert(SetName("system"));

    if (_imp->params.setsdir().stat().exists())
    {
        using namespace std::placeholders;

        for (FSIterator f(_imp->params.setsdir(), { }), f_end ; f != f_end ; ++f)
        {
            if (! is_file_with_extension(*f, ".conf", { }))
                continue;

            try
            {
                result->insert(SetName(strip_trailing_string(f->basename(), ".conf")));
            }
            catch (const NameError & e)
            {
                Log::get_instance()->message("e.sets.failure", ll_warning, lc_context) << "Skipping set '"
                    << *f << "' due to exception '" << e.message() << "' (" << e.what() << ")";
            }
        }
    }

    return result;
}

namespace
{
    bool
    match_range(const Environment * const env, const EAPI & eapi,
            const std::shared_ptr<const PackageID> & id, const erepository::GLSARange & r, const VersionSpecOptions & ver_options, const ELikePackageDepSpecOptions & pds_options)
    {
        if (r.slot() != "*")
        {
            try
            {
                PackageDepSpec spec(parse_elike_package_dep_spec(stringify(id->name()) + ":" + r.slot(), eapi.supported()->package_dep_spec_parse_options(),
                            eapi.supported()->version_spec_options()));
                if (! match_package(*env, spec, id, make_null_shared_ptr(), { }))
                    return false;
            }
            catch (const SlotNameError &)
            {
                throw GLSAError("Got bad slot '" + r.slot() + "'");
            }
        }

        VersionOperatorValue our_op(static_cast<VersionOperatorValue>(-1));
        std::string ver(r.version());
        if (r.op() == "le")
            our_op = vo_less_equal;
        if (r.op() == "lt")
            our_op = vo_less;
        if (r.op() == "eq")
        {
            if (! ver.empty() && '*' == ver.at(ver.length() - 1))
            {
                ver.erase(ver.length() - 1);
                our_op = pds_options[epdso_nice_equal_star] ? vo_nice_equal_star : vo_stupid_equal_star;
            }
            else
                our_op = vo_equal;
        }
        if (r.op() == "gt")
            our_op = vo_greater;
        if (r.op() == "ge")
            our_op = vo_greater_equal;

        if (static_cast<VersionOperatorValue>(-1) != our_op)
            return (VersionOperator(our_op).as_version_spec_comparator()(id->version(), VersionSpec(ver, ver_options)));

        if (0 == r.op().compare(0, 1, "r"))
        {
            return id->version().remove_revision() == VersionSpec(ver, ver_options).remove_revision() &&
                match_range(env, eapi, id, make_named_values<erepository::GLSARange>(
                            n::op() = r.op().substr(1),
                            n::slot() = r.slot(),
                            n::version() = r.version()),
                        ver_options, pds_options);
        }

        throw GLSAError("Got bad op '" + r.op() + "'");
    }

    bool
    is_vulnerable(const Environment * const env, const EAPI & eapi,
            const GLSAPackage & glsa_pkg, const std::shared_ptr<const PackageID> & id, const VersionSpecOptions & ver_options, const ELikePackageDepSpecOptions & pds_options)
    {
        /* a package is affected if it matches any vulnerable line, except if it matches
         * any unaffected line. */
        bool vulnerable(false);
        for (GLSAPackage::RangesConstIterator r(glsa_pkg.begin_vulnerable()), r_end(glsa_pkg.end_vulnerable()) ;
                r != r_end && ! vulnerable ; ++r)
            if (match_range(env, eapi, id, *r, ver_options, pds_options))
                vulnerable = true;

        if (! vulnerable)
            return false;

        for (GLSAPackage::RangesConstIterator r(glsa_pkg.begin_unaffected()), r_end(glsa_pkg.end_unaffected()) ;
                r != r_end && vulnerable ; ++r)
            if (match_range(env, eapi, id, *r, ver_options, pds_options))
                vulnerable = false;

        return vulnerable;
    }
}

const std::shared_ptr<const SetSpecTree>
ERepositorySets::security_set(bool insecurity) const
{
    Context context("When building security or insecurity package set:");

    std::shared_ptr<SetSpecTree> security_packages(std::make_shared<SetSpecTree>(std::make_shared<AllDepSpec>()));

    if (!_imp->params.securitydir().stat().is_directory_or_symlink_to_directory())
        return security_packages;

    for (FSIterator f(_imp->params.securitydir(), { }), f_end ; f != f_end; ++f)
    {
        if (! is_file_with_prefix_extension(*f, "glsa-", ".xml", { }))
            continue;

        Context local_context("When parsing security advisory '" + stringify(*f) + "':");

        const std::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string(_imp->e_repository->eapi_for_file(*f)));
        if (! eapi->supported())
            throw GLSAError("Can't use advisory '" + stringify(*f) +
                    "' because it uses an unsupported EAPI");

        const VersionSpecOptions ver_options(eapi->supported()->version_spec_options());
        const ELikePackageDepSpecOptions pds_options(eapi->supported()->package_dep_spec_parse_options());

        try
        {
            std::shared_ptr<const GLSA> glsa(GLSA::create_from_xml_file(stringify(*f)));
            Context local_local_context("When handling GLSA '" + glsa->id() + "' from '" +
                    stringify(*f) + "':");

            for (GLSA::PackagesConstIterator glsa_pkg(glsa->begin_packages()),
                    glsa_pkg_end(glsa->end_packages()) ; glsa_pkg != glsa_pkg_end ; ++glsa_pkg)
            {
                std::shared_ptr<const PackageIDSequence> candidates;
                if (insecurity)
                    candidates = (*_imp->environment)[selection::AllVersionsSorted(generator::Package(glsa_pkg->name()))];
                else
                    candidates = (*_imp->environment)[selection::AllVersionsSorted(
                            generator::Package(glsa_pkg->name()) |
                            filter::InstalledAtRoot(_imp->environment->preferred_root_key()->parse_value()))];

                for (PackageIDSequence::ConstIterator c(candidates->begin()), c_end(candidates->end()) ;
                        c != c_end ; ++c)
                {
                    if (! is_vulnerable(_imp->environment, *eapi, *glsa_pkg, *c, ver_options, pds_options))
                        continue;

                    if (insecurity)
                    {
                        std::shared_ptr<PackageDepSpec> spec(std::make_shared<PackageDepSpec>(
                                    make_package_dep_spec({ })
                                    .package((*c)->name())
                                    .version_requirement(make_named_values<VersionRequirement>(
                                            n::version_operator() = vo_equal,
                                            n::version_spec() = (*c)->version()))
                                    .in_repository((*c)->repository_name())));
                        security_packages->top()->append(spec);
                    }
                    else
                    {
                        Context local_local_local_context("When finding upgrade for '" + stringify(glsa_pkg->name()) + ":"
                                + ((*c)->slot_key() ? stringify((*c)->slot_key()->parse_value().parallel_value()) : "(none)") + "'");

                        /* we need to find the best not vulnerable installable package that isn't masked
                         * that's in the same slot as our vulnerable installed package. */
                        bool ok(false);
                        std::shared_ptr<const PackageIDSequence> available(
                                (*_imp->environment)[selection::AllVersionsSorted(
                                    generator::Package(glsa_pkg->name()) |
                                    filter::SameSlot(*c) |
                                    filter::SupportsAction<InstallAction>() |
                                    filter::NotMasked())]);

                        for (PackageIDSequence::ReverseConstIterator r(available->rbegin()), r_end(available->rend()) ; r != r_end ; ++r)
                        {
                            if (is_vulnerable(_imp->environment, *eapi, *glsa_pkg, *r, ver_options, pds_options))
                            {
                                Log::get_instance()->message("e.glsa.skipping_vulnerable", ll_debug, lc_context)
                                    << "Skipping '" << **r << "' due to is_vulnerable match";
                                continue;
                            }

                            std::shared_ptr<PackageDepSpec> spec(std::make_shared<PackageDepSpec>(make_package_dep_spec({ })
                                        .package((*r)->name())
                                        .version_requirement(make_named_values<VersionRequirement>(
                                                n::version_operator() = vo_equal,
                                                n::version_spec() = (*r)->version()))
                                        .in_repository((*r)->repository_name())));
                            security_packages->top()->append(spec);
                            ok = true;
                            break;
                        }

                        if (! ok)
                            throw GLSAError("Could not determine upgrade path to resolve '"
                                    + glsa->id() + ": " + glsa->title() + "' for package '"
                                    + stringify(**c) + "'");
                    }
                }
            }
        }
        catch (const GLSAError & e)
        {
            Log::get_instance()->message("e.glsa.failure", ll_warning, lc_context)
                << "Cannot use GLSA '" << *f << "' due to exception '" << e.message() << "' (" << e.what() << ")";
        }
        catch (const NameError & e)
        {
            Log::get_instance()->message("e.glsa.failure", ll_warning, lc_context)
                << "Cannot use GLSA '" << *f << "' due to exception '" << e.message() << "' (" << e.what() << ")";
        }
    }

    return security_packages;
}

