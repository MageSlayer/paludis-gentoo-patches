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

#include <paludis/repositories/e/do_info_action.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/eapi_phase.hh>
#include <paludis/repositories/e/check_userpriv.hh>
#include <paludis/repositories/e/make_use.hh>
#include <paludis/repositories/e/ebuild.hh>

#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>

#include <paludis/dep_spec_flattener.hh>
#include <paludis/action.hh>
#include <paludis/metadata_key.hh>
#include <paludis/environment.hh>
#include <paludis/output_manager.hh>

#include <algorithm>
#include <unistd.h>

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
        DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec> restricts(env, id);
        if (id->restrict_key())
            id->restrict_key()->parse_value()->top()->accept(restricts);

        userpriv_restrict =
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::bind(std::equal_to<>(), std::bind(std::mem_fn(&StringDepSpec::text), _1), "userpriv")) ||
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::bind(std::equal_to<>(), std::bind(std::mem_fn(&StringDepSpec::text), _1), "nouserpriv"));
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
                n::package_builddir() = params.builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-info"),
                n::package_id() = id,
                n::parts() = nullptr,
                n::permitted_directories() = nullptr,
                n::portdir() =
                    (params.master_repositories() && ! params.master_repositories()->empty())
                        ? (*params.master_repositories()->begin())->params().location()
                        : params.location(),
                n::root() = stringify(env->preferred_root_key()->parse_value()),
                n::sandbox() = phase->option("sandbox"),
                n::sydbox() = phase->option("sydbox"),
                n::userpriv() = phase->option("userpriv") && userpriv_ok,
                n::volatile_files() = nullptr
                ));

        EbuildInfoCommandParams info_params(
                make_named_values<EbuildInfoCommandParams>(
                n::expand_vars() = expand_vars,
                n::info_vars() = repo->info_vars_key() ?
                    repo->info_vars_key()->parse_value() : std::make_shared<const Set<std::string>>(),
                n::load_environment() = static_cast<const FSPath *>(nullptr),
                n::profiles() = params.profiles(),
                n::profiles_with_parents() = profile->profiles_with_parents(),
                n::use() = use,
                n::use_ebuild_file() = true,
                n::use_expand() = join(profile->use_expand()->begin(), profile->use_expand()->end(), " "),
                n::use_expand_hidden() = join(profile->use_expand_hidden()->begin(), profile->use_expand_hidden()->end(), " ")
                ));

        EbuildInfoCommand cmd(command_params, info_params);
        cmd();
    }

    output_manager->succeeded();
}

