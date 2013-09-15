/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011, 2013 Ciaran McCreesh
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

#include <paludis/repositories/e/do_fetch_action.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/eapi_phase.hh>
#include <paludis/repositories/e/check_userpriv.hh>
#include <paludis/repositories/e/a_finder.hh>
#include <paludis/repositories/e/aa_visitor.hh>
#include <paludis/repositories/e/check_fetched_files_visitor.hh>
#include <paludis/repositories/e/fetch_visitor.hh>
#include <paludis/repositories/e/make_use.hh>
#include <paludis/repositories/e/can_skip_phase.hh>
#include <paludis/repositories/e/ebuild.hh>
#include <paludis/repositories/e/make_archive_strings.hh>

#include <paludis/util/strip.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>

#include <paludis/dep_spec_flattener.hh>
#include <paludis/metadata_key.hh>
#include <paludis/environment.hh>
#include <paludis/action.hh>
#include <paludis/output_manager.hh>
#include <paludis/slot.hh>

#include <algorithm>
#include <set>
#include <unistd.h>

using namespace paludis;
using namespace paludis::erepository;

void
paludis::erepository::do_fetch_action(
        const Environment * const env,
        const ERepository * const repo,
        const std::shared_ptr<const ERepositoryID> & id,
        const FetchAction & fetch_action)
{
    using namespace std::placeholders;

    Context context("When fetching '" + stringify(*id) + "':");

    bool fetch_restrict(false), userpriv_restrict(false);
    {
        DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec> restricts(env, id);
        if (id->restrict_key())
            id->restrict_key()->parse_value()->top()->accept(restricts);

        for (DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec>::ConstIterator i(restricts.begin()), i_end(restricts.end()) ;
                i != i_end ; ++i)
        {
            if (id->eapi()->supported()->ebuild_options()->restrict_fetch()->end() !=
                    std::find(id->eapi()->supported()->ebuild_options()->restrict_fetch()->begin(),
                        id->eapi()->supported()->ebuild_options()->restrict_fetch()->end(), (*i)->text()))
                fetch_restrict = true;
            if ("userpriv" == (*i)->text() || "nouserpriv" == (*i)->text())
                userpriv_restrict = true;
        }
    }

    bool fetch_userpriv_ok(env->reduced_gid() != getgid() &&
            check_userpriv(FSPath(repo->params().distdir()), env,
                id->eapi()->supported()->userpriv_cannot_use_root()));

    std::string archives, all_archives;
    std::tie(archives, all_archives) = make_archives_strings(env, id);

    /* Strip trailing space. Some ebuilds rely upon this. From kde-meta.eclass:
     *     [[ -n ${A/${TARBALL}/} ]] && unpack ${A/${TARBALL}/}
     * Rather annoying.
     */
    archives = strip_trailing(archives, " ");
    all_archives = strip_trailing(all_archives, " ");

    std::shared_ptr<OutputManager> output_manager(fetch_action.options.make_output_manager()(fetch_action));

    CheckFetchedFilesVisitor c(env, id, repo->params().distdir(),
            fetch_action.options.fetch_parts()[fp_unneeded], fetch_restrict,
            ((repo->layout()->package_directory(id->name())) / "Manifest"),
            repo->params().use_manifest(),
            output_manager, fetch_action.options.exclude_unmirrorable(),
            fetch_action.options.ignore_unfetched(),
            fetch_action.options.ignore_not_in_manifest());

    if (id->fetches_key())
    {
        auto fetches(id->fetches_key()->parse_value());

        /* always use mirror://gentoo/, where gentoo is the name of our first master repository,
         * or our name if there's no master. */
        std::string mirrors_name(
                (repo->params().master_repositories() && ! repo->params().master_repositories()->empty()) ?
                stringify((*repo->params().master_repositories()->begin())->name()) :
                stringify(repo->name()));

        if (fetch_action.options.fetch_parts()[fp_regulars] && ! fetch_action.options.ignore_unfetched())
        {
            FetchVisitor f(env, id, *id->eapi(),
                    repo->params().distdir(), fetch_action.options.fetch_parts()[fp_unneeded],
                    fetch_userpriv_ok, mirrors_name,
                    id->fetches_key()->initial_label(), fetch_action.options.safe_resume(),
                    output_manager, std::bind(&ERepository::get_mirrors, repo, std::placeholders::_1));
            fetches->top()->accept(f);
        }

        fetches->top()->accept(c);
    }

    if ( (fetch_action.options.fetch_parts()[fp_extras]) && ((c.need_nofetch()) ||
            ((! fetch_action.options.ignore_unfetched()) && (! id->eapi()->supported()->ebuild_phases()->ebuild_fetch_extra().empty()))))
    {
        bool userpriv_ok((! userpriv_restrict) && (env->reduced_gid() != getgid()) &&
                check_userpriv(FSPath(repo->params().builddir()), env,
                    id->eapi()->supported()->userpriv_cannot_use_root()));
        std::string use(make_use(env, *id, repo->profile()));
        std::shared_ptr<Map<std::string, std::string> > expand_vars(make_expand(
                    env, *id, repo->profile()));

        std::shared_ptr<const FSPathSequence> exlibsdirs(repo->layout()->exlibsdirs(id->name()));

        EAPIPhases fetch_extra_phases(id->eapi()->supported()->ebuild_phases()->ebuild_fetch_extra());
        if ((! fetch_action.options.ignore_unfetched()) && (fetch_extra_phases.begin_phases() != fetch_extra_phases.end_phases()))
        {
            FSPath package_builddir(repo->params().builddir() / (stringify(id->name().category()) + "-" +
                    stringify(id->name().package()) + "-" + stringify(id->version()) + "-fetch_extra"));

            for (EAPIPhases::ConstIterator phase(fetch_extra_phases.begin_phases()), phase_end(fetch_extra_phases.end_phases()) ;
                    phase != phase_end ; ++phase)
            {
                bool skip(false);
                do
                {
                    switch (fetch_action.options.want_phase()(phase->equal_option("skipname")))
                    {
                        case wp_yes:
                            continue;

                        case wp_skip:
                            skip = true;
                            continue;

                        case wp_abort:
                            throw ActionAbortedError("Told to abort fetch");

                        case last_wp:
                            break;
                    }

                    throw InternalError(PALUDIS_HERE, "bad want_phase");
                } while (false);

                if (skip)
                    continue;

                if (can_skip_phase(env, id, *phase))
                    continue;

                const auto params = repo->params();
                const auto profile = repo->profile();

                EbuildCommandParams command_params(make_named_values<EbuildCommandParams>(
                        n::builddir() = params.builddir(),
                        n::clearenv() = phase->option("clearenv"),
                        n::commands() = join(phase->begin_commands(), phase->end_commands(), " "),
                        n::distdir() = params.distdir(),
                        n::ebuild_dir() = repo->layout()->package_directory(id->name()),
                        n::ebuild_file() = id->fs_location_key()->parse_value(),
                        n::eclassdirs() = params.eclassdirs(),
                        n::environment() = env,
                        n::exlibsdirs() = exlibsdirs,
                        n::files_dir() = repo->layout()->package_directory(id->name()) / "files",
                        n::maybe_output_manager() = output_manager,
                        n::package_builddir() = package_builddir,
                        n::package_id() = id,
                        n::parts() = nullptr,
                        n::permitted_directories() = nullptr,
                        n::portdir() =
                            (params.master_repositories() && ! params.master_repositories()->empty())
                                ? (*params.master_repositories()->begin())->params().location()
                                : params.location(),
                        n::root() = "/",
                        n::sandbox() = phase->option("sandbox"),
                        n::sydbox() = phase->option("sydbox"),
                        n::userpriv() = phase->option("userpriv") && userpriv_ok,
                        n::volatile_files() = nullptr
                        ));

                EbuildFetchExtraCommand fetch_extra_cmd(command_params, make_named_values<EbuildFetchExtraCommandParams>(
                            n::a() = archives,
                            n::aa() = all_archives,
                            n::expand_vars() = expand_vars,
                            n::loadsaveenv_dir() = package_builddir / "temp",
                            n::profiles() = params.profiles(),
                            n::profiles_with_parents() = profile->profiles_with_parents(),
                            n::slot() = id->slot_key() ? stringify(id->slot_key()->parse_value().raw_value()) : "",
                            n::use() = use,
                            n::use_expand() = join(profile->use_expand()->begin(), profile->use_expand()->end(), " "),
                            n::use_expand_hidden() = join(profile->use_expand_hidden()->begin(), profile->use_expand_hidden()->end(), " ")
                            ));

                if (! fetch_extra_cmd())
                    throw ActionFailedError("Fetch of '" + stringify(*id) + "' failed");
            }
        }

        if (c.need_nofetch())
        {
            EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_nofetch());
            for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
                    phase != phase_end ; ++phase)
            {
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
                        n::package_builddir() = repo->params().builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-nofetch"),
                        n::package_id() = id,
                        n::parts() = nullptr,
                        n::permitted_directories() = nullptr,
                        n::portdir() = (repo->params().master_repositories() && ! repo->params().master_repositories()->empty()) ?
                            (*repo->params().master_repositories()->begin())->params().location() : repo->params().location(),
                        n::root() = "/",
                        n::sandbox() = phase->option("sandbox"),
                        n::sydbox() = phase->option("sydbox"),
                        n::userpriv() = phase->option("userpriv") && userpriv_ok,
                        n::volatile_files() = nullptr
                        ));

                EbuildNoFetchCommand nofetch_cmd(command_params,
                        make_named_values<EbuildNoFetchCommandParams>(
                        n::a() = archives,
                        n::aa() = all_archives,
                        n::expand_vars() = expand_vars,
                        n::profiles() = repo->params().profiles(),
                        n::profiles_with_parents() = repo->profile()->profiles_with_parents(),
                        n::use() = use,
                        n::use_expand() = join(repo->profile()->use_expand()->begin(), repo->profile()->use_expand()->end(), " "),
                        n::use_expand_hidden() = join(repo->profile()->use_expand_hidden()->begin(), repo->profile()->use_expand_hidden()->end(), " ")
                        ));

                if (! nofetch_cmd())
                {
                    std::copy(c.failures()->begin(), c.failures()->end(),
                            fetch_action.options.errors()->back_inserter());
                    throw ActionFailedError("Fetch of '" + stringify(*id) + "' failed");
                }
            }
        }
    }

    if (! c.failures()->empty())
    {
        std::copy(c.failures()->begin(), c.failures()->end(),
                fetch_action.options.errors()->back_inserter());
        throw ActionFailedError("Fetch of '" + stringify(*id) + "' failed");
    }

    output_manager->succeeded();
}


