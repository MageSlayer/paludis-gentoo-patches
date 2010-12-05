/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/repositories/e/do_info_action.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/eapi_phase.hh>
#include <paludis/repositories/e/check_userpriv.hh>
#include <paludis/repositories/e/make_use.hh>
#include <paludis/repositories/e/ebuild.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/action.hh>
#include <paludis/metadata_key.hh>
#include <paludis/environment.hh>
#include <paludis/output_manager.hh>

#include <algorithm>

using namespace paludis;
using namespace paludis::erepository;

void
paludis::erepository::do_info_action(
        const Environment * const env,
        const ERepository * const repo,
        const std::shared_ptr<const ERepositoryID> & id,
        const InfoAction & a)
{
    using namespace std::placeholders;

    Context context("When infoing '" + stringify(*id) + "':");

    std::shared_ptr<OutputManager> output_manager(a.options.make_output_manager()(a));

    bool userpriv_restrict;
    {
        DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec> restricts(env);
        if (id->restrict_key())
            id->restrict_key()->value()->top()->accept(restricts);

        userpriv_restrict =
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::bind(std::equal_to<std::string>(), std::bind(std::mem_fn(&StringDepSpec::text), _1), "userpriv")) ||
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::bind(std::equal_to<std::string>(), std::bind(std::mem_fn(&StringDepSpec::text), _1), "nouserpriv"));
    }
    bool userpriv_ok((! userpriv_restrict) && (env->reduced_gid() != getgid()) &&
            check_userpriv(FSPath(repo->params().builddir()), env, id->eapi()->supported()->userpriv_cannot_use_root()));

    /* make use */
    std::string use(make_use(env, *id, repo->profile()));

    /* add expand to use (iuse isn't reliable for use_expand things), and make the expand
     * environment variables */
    std::shared_ptr<Map<std::string, std::string> > expand_vars(make_expand(
                env, *id, repo->profile()));

    std::shared_ptr<const FSPathSequence> exlibsdirs(repo->layout()->exlibsdirs(id->name()));

    EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_info());
    for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
        if (phase->option("installed=true"))
            continue;

        EbuildCommandParams command_params(make_named_values<EbuildCommandParams>(
                n::builddir() = repo->params().builddir(),
                n::clearenv() = phase->option("clearenv"),
                n::commands() = join(phase->begin_commands(), phase->end_commands(), " "),
                n::distdir() = repo->params().distdir(),
                n::ebuild_dir() = repo->layout()->package_directory(id->name()),
                n::ebuild_file() = id->fs_location_key()->value(),
                n::eclassdirs() = repo->params().eclassdirs(),
                n::environment() = env,
                n::exlibsdirs() = exlibsdirs,
                n::files_dir() = repo->layout()->package_directory(id->name()) / "files",
                n::maybe_output_manager() = output_manager,
                n::package_builddir() = repo->params().builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-info"),
                n::package_id() = id,
                n::portdir() =
                    (repo->params().master_repositories() && ! repo->params().master_repositories()->empty()) ?
                    (*repo->params().master_repositories()->begin())->params().location() : repo->params().location(),
                n::root() = stringify(env->preferred_root_key()->value()),
                n::sandbox() = phase->option("sandbox"),
                n::sydbox() = phase->option("sydbox"),
                n::userpriv() = phase->option("userpriv") && userpriv_ok
                ));

        EbuildInfoCommandParams info_params(
                make_named_values<EbuildInfoCommandParams>(
                n::expand_vars() = expand_vars,
                n::info_vars() = repo->info_vars_key() ?
                    repo->info_vars_key()->value() : std::make_shared<const Set<std::string>>(),
                n::load_environment() = static_cast<const FSPath *>(0),
                n::profiles() = repo->params().profiles(),
                n::profiles_with_parents() = repo->profile()->profiles_with_parents(),
                n::use() = use,
                n::use_ebuild_file() = true,
                n::use_expand() = join(repo->profile()->use_expand()->begin(), repo->profile()->use_expand()->end(), " "),
                n::use_expand_hidden() = join(repo->profile()->use_expand_hidden()->begin(), repo->profile()->use_expand_hidden()->end(), " ")
                ));

        EbuildInfoCommand cmd(command_params, info_params);
        cmd();
    }

    output_manager->succeeded();
}

