/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/do_pretend_action.hh>
#include <paludis/repositories/e/check_userpriv.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/eapi_phase.hh>
#include <paludis/repositories/e/make_use.hh>
#include <paludis/repositories/e/myoptions_requirements_verifier.hh>
#include <paludis/repositories/e/required_use_verifier.hh>
#include <paludis/repositories/e/ebuild.hh>
#include <paludis/repositories/e/can_skip_phase.hh>

#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <paludis/util/make_null_shared_ptr.hh>

#include <paludis/dep_spec_flattener.hh>
#include <paludis/metadata_key.hh>
#include <paludis/environment.hh>
#include <paludis/action.hh>

#include <algorithm>

using namespace paludis;
using namespace paludis::erepository;

bool
paludis::erepository::do_pretend_action(
        const Environment * const env,
        const ERepository * const repo,
        const std::shared_ptr<const ERepositoryID> & id,
        const PretendAction & a)
{
    using namespace std::placeholders;

    Context context("When running pretend for '" + stringify(*id) + "':");

    if (! id->eapi()->supported())
        return false;

    bool result(true), can_pretend(true);

    if ((! id->raw_myoptions_key()) && (! id->required_use_key()))
        if (id->eapi()->supported()->ebuild_phases()->ebuild_pretend().empty())
            return result;

    bool userpriv_restrict;
    {
        DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec> restricts(env, id);
        if (id->restrict_key())
            id->restrict_key()->parse_value()->top()->accept(restricts);

        userpriv_restrict =
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::bind(std::equal_to<std::string>(), std::bind(std::mem_fn(&StringDepSpec::text), _1), "userpriv")) ||
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::bind(std::equal_to<std::string>(), std::bind(std::mem_fn(&StringDepSpec::text), _1), "nouserpriv"));
    }
    bool userpriv_ok((! userpriv_restrict) && (env->reduced_gid() != getgid()) &&
            check_userpriv(FSPath(repo->params().builddir()), env, id->eapi()->supported()->userpriv_cannot_use_root()));

    std::string use(make_use(env, *id, repo->profile()));
    std::shared_ptr<Map<std::string, std::string> > expand_vars(make_expand(
                env, *id, repo->profile()));

    std::shared_ptr<const FSPathSequence> exlibsdirs(repo->layout()->exlibsdirs(id->name()));

    std::shared_ptr<OutputManager> output_manager;

    if (id->raw_myoptions_key())
    {
        MyOptionsRequirementsVerifier verifier(env, id);
        id->raw_myoptions_key()->parse_value()->top()->accept(verifier);

        if (verifier.unmet_requirements() && ! verifier.unmet_requirements()->empty())
        {
            EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_bad_options());
            if (phases.begin_phases() == phases.end_phases())
                throw InternalError(PALUDIS_HERE, "using myoptions but no ebuild_bad_options phase");

            for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
                    phase != phase_end ; ++phase)
            {
                if (! output_manager)
                    output_manager = a.options.make_output_manager()(a);

                EbuildCommandParams command_params(make_named_values<EbuildCommandParams>(
                            n::builddir() = repo->params().builddir(),
                            n::clearenv() = phase->option("clearenv"),
                            n::commands() = join(phase->begin_commands(), phase->end_commands(), " "),
                            n::distdir() = repo->params().distdir(),
                            n::ebuild_dir() = repo->layout()->package_directory(id->name()),
                            n::ebuild_file() = id->fs_location_key()->parse_value(),
                            n::eclassdirs() = repo->params().eclassdirs(),
                            n::environment() = env,
                            n::exlibsdirs() = exlibsdirs,
                            n::files_dir() = repo->layout()->package_directory(id->name()) / "files",
                            n::maybe_output_manager() = output_manager,
                            n::package_builddir() = repo->params().builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-bad_options"),
                            n::package_id() = id,
                            n::permitted_directories() = make_null_shared_ptr(),
                            n::portdir() =
                                (repo->params().master_repositories() && ! repo->params().master_repositories()->empty()) ?
                                (*repo->params().master_repositories()->begin())->params().location() : repo->params().location(),
                            n::root() = a.options.destination()->installed_root_key() ?
                                stringify(a.options.destination()->installed_root_key()->parse_value()) :
                                "/",
                            n::sandbox() = phase->option("sandbox"),
                            n::sydbox() = phase->option("sydbox"),
                            n::userpriv() = phase->option("userpriv") && userpriv_ok
                            ));

                EbuildBadOptionsCommand bad_options_cmd(command_params,
                        make_named_values<EbuildBadOptionsCommandParams>(
                            n::expand_vars() = expand_vars,
                            n::profiles() = repo->params().profiles(),
                            n::profiles_with_parents() = repo->profile()->profiles_with_parents(),
                            n::unmet_requirements() = verifier.unmet_requirements(),
                            n::use() = use,
                            n::use_expand() = join(repo->profile()->use_expand()->begin(), repo->profile()->use_expand()->end(), " "),
                            n::use_expand_hidden() = join(repo->profile()->use_expand_hidden()->begin(), repo->profile()->use_expand_hidden()->end(), " ")
                            ));

                if (! bad_options_cmd())
                    throw ActionFailedError("Bad options phase died");
            }

            result = false;
        }
    }

    if (id->required_use_key())
    {
        RequiredUseVerifier verifier(env, id);
        id->required_use_key()->parse_value()->top()->accept(verifier);

        if (verifier.unmet_requirements() && ! verifier.unmet_requirements()->empty())
        {
            EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_bad_options());
            if (phases.begin_phases() == phases.end_phases())
                throw InternalError(PALUDIS_HERE, "using required_use but no ebuild_bad_options phase");

            for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
                    phase != phase_end ; ++phase)
            {
                if (! output_manager)
                    output_manager = a.options.make_output_manager()(a);

                EbuildCommandParams command_params(make_named_values<EbuildCommandParams>(
                            n::builddir() = repo->params().builddir(),
                            n::clearenv() = phase->option("clearenv"),
                            n::commands() = join(phase->begin_commands(), phase->end_commands(), " "),
                            n::distdir() = repo->params().distdir(),
                            n::ebuild_dir() = repo->layout()->package_directory(id->name()),
                            n::ebuild_file() = id->fs_location_key()->parse_value(),
                            n::eclassdirs() = repo->params().eclassdirs(),
                            n::environment() = env,
                            n::exlibsdirs() = exlibsdirs,
                            n::files_dir() = repo->layout()->package_directory(id->name()) / "files",
                            n::maybe_output_manager() = output_manager,
                            n::package_builddir() = repo->params().builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-bad_options"),
                            n::package_id() = id,
                            n::permitted_directories() = make_null_shared_ptr(),
                            n::portdir() =
                                (repo->params().master_repositories() && ! repo->params().master_repositories()->empty()) ?
                                (*repo->params().master_repositories()->begin())->params().location() : repo->params().location(),
                            n::root() = a.options.destination()->installed_root_key() ?
                                stringify(a.options.destination()->installed_root_key()->parse_value()) :
                                "/",
                            n::sandbox() = phase->option("sandbox"),
                            n::sydbox() = phase->option("sydbox"),
                            n::userpriv() = phase->option("userpriv") && userpriv_ok
                            ));

                EbuildBadOptionsCommand bad_options_cmd(command_params,
                        make_named_values<EbuildBadOptionsCommandParams>(
                            n::expand_vars() = expand_vars,
                            n::profiles() = repo->params().profiles(),
                            n::profiles_with_parents() = repo->profile()->profiles_with_parents(),
                            n::unmet_requirements() = verifier.unmet_requirements(),
                            n::use() = use,
                            n::use_expand() = join(repo->profile()->use_expand()->begin(), repo->profile()->use_expand()->end(), " "),
                            n::use_expand_hidden() = join(repo->profile()->use_expand_hidden()->begin(), repo->profile()->use_expand_hidden()->end(), " ")
                            ));

                if (! bad_options_cmd())
                    throw ActionFailedError("Bad options phase died");
            }

            result = false;
            can_pretend = false;

        }
    }

    if (id->eapi()->supported()->ebuild_phases()->ebuild_pretend().empty() || ! can_pretend)
        return result;

    EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_pretend());
    for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
        if (can_skip_phase(env, id, *phase))
            continue;

        if (! output_manager)
            output_manager = a.options.make_output_manager()(a);

        EbuildCommandParams command_params(make_named_values<EbuildCommandParams>(
                n::builddir() = repo->params().builddir(),
                n::clearenv() = phase->option("clearenv"),
                n::commands() = join(phase->begin_commands(), phase->end_commands(), " "),
                n::distdir() = repo->params().distdir(),
                n::ebuild_dir() = repo->layout()->package_directory(id->name()),
                n::ebuild_file() = id->fs_location_key()->parse_value(),
                n::eclassdirs() = repo->params().eclassdirs(),
                n::environment() = env,
                n::exlibsdirs() = exlibsdirs,
                n::files_dir() = repo->layout()->package_directory(id->name()) / "files",
                n::maybe_output_manager() = output_manager,
                n::package_builddir() = repo->params().builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-pretend"),
                n::package_id() = id,
                n::permitted_directories() = make_null_shared_ptr(),
                n::portdir() =
                    (repo->params().master_repositories() && ! repo->params().master_repositories()->empty()) ?
                    (*repo->params().master_repositories()->begin())->params().location() : repo->params().location(),
                n::root() = a.options.destination()->installed_root_key() ?
                    stringify(a.options.destination()->installed_root_key()->parse_value()) :
                    "/",
                n::sandbox() = phase->option("sandbox"),
                n::sydbox() = phase->option("sydbox"),
                n::userpriv() = phase->option("userpriv") && userpriv_ok
                ));

        EbuildPretendCommand pretend_cmd(command_params,
                make_named_values<EbuildPretendCommandParams>(
                    n::destination() = a.options.destination(),
                    n::expand_vars() = expand_vars,
                    n::is_from_pbin() = id->eapi()->supported()->is_pbin(),
                    n::profiles() = repo->params().profiles(),
                    n::profiles_with_parents() = repo->profile()->profiles_with_parents(),
                    n::replacing_ids() = a.options.replacing(),
                    n::use() = use,
                    n::use_expand() = join(repo->profile()->use_expand()->begin(), repo->profile()->use_expand()->end(), " "),
                    n::use_expand_hidden() = join(repo->profile()->use_expand_hidden()->begin(), repo->profile()->use_expand_hidden()->end(), " ")
                    ));

        if (! pretend_cmd())
            return false;
    }

    return result;
}

