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

#include <paludis/repositories/e/do_install_action.hh>
#include <paludis/repositories/e/a_finder.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/a_finder.hh>
#include <paludis/repositories/e/aa_visitor.hh>
#include <paludis/repositories/e/make_use.hh>
#include <paludis/repositories/e/check_userpriv.hh>
#include <paludis/repositories/e/eapi_phase.hh>
#include <paludis/repositories/e/can_skip_phase.hh>
#include <paludis/repositories/e/e_stripper.hh>
#include <paludis/repositories/e/ebuild.hh>
#include <paludis/repositories/e/make_archive_strings.hh>
#include <paludis/repositories/e/permitted_directories.hh>

#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/log.hh>
#include <paludis/util/join.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/util/tokeniser.hh>

#include <paludis/action.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/metadata_key.hh>
#include <paludis/choice.hh>
#include <paludis/elike_choices.hh>
#include <paludis/output_manager.hh>
#include <paludis/partitioning.hh>
#include <paludis/slot.hh>

#include <vector>
#include <algorithm>
#include <set>
#include <unistd.h>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    struct AcceptLicenseFinder
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> id;
        std::stringstream s;

        AcceptLicenseFinder(const Environment * const e, const std::shared_ptr<const PackageID> & i) :
            env(e),
            id(i)
        {
            s << "*";
        }

        void visit(const LicenseSpecTree::NodeType<AllDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const LicenseSpecTree::NodeType<AnyDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const LicenseSpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            if (node.spec()->condition_met(env, id))
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const LicenseSpecTree::NodeType<LicenseDepSpec>::Type & node)
        {
            s << " " << node.spec()->text();
        }
    };

    void used_this_for_config_protect(std::string & s, const std::string & v)
    {
        s = v;
    }

    bool parallel_slot_is_same(const std::shared_ptr<const PackageID> & a,
            const std::shared_ptr<const PackageID> & b)
    {
        if (a->slot_key())
            return b->slot_key() && a->slot_key()->parse_value().parallel_value() == b->slot_key()->parse_value().parallel_value();
        else
            return ! b->slot_key();
    }

    bool ignore_merged(const std::shared_ptr<const FSPathSet> & s, const FSPath & f)
    {
        return s->end() != s->find(f);
    }

    std::shared_ptr<OutputManager> this_output_manager(const std::shared_ptr<OutputManager> & o, const Action &)
    {
        return o;
    }
}

void
paludis::erepository::do_install_action(
        const Environment * const env,
        const ERepository * const repo,
        const std::shared_ptr<const ERepositoryID> & id,
        const InstallAction & install_action)
{
    using namespace std::placeholders;

    Context context("When installing '" + stringify(*id) + "'" +
            (install_action.options.replacing()->empty() ? "" : " replacing { '"
             + join(indirect_iterator(install_action.options.replacing()->begin()),
                 indirect_iterator(install_action.options.replacing()->end()), "', '") + "' }") + ":");

    std::shared_ptr<OutputManager> output_manager(install_action.options.make_output_manager()(install_action));

    bool userpriv_restrict;
    bool test_restrict;
    bool strip_restrict;
    {
        DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec> restricts(env, id);
        if (id->restrict_key())
            id->restrict_key()->parse_value()->top()->accept(restricts);

        userpriv_restrict =
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::bind(std::equal_to<>(), std::bind(std::mem_fn(&StringDepSpec::text), _1), "userpriv")) ||
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::bind(std::equal_to<>(), std::bind(std::mem_fn(&StringDepSpec::text), _1), "nouserpriv"));

        test_restrict =
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::bind(std::equal_to<>(), std::bind(std::mem_fn(&StringDepSpec::text), _1), "test"));

        strip_restrict =
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::bind(std::equal_to<>(), std::bind(std::mem_fn(&StringDepSpec::text), _1), "strip")) ||
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::bind(std::equal_to<>(), std::bind(std::mem_fn(&StringDepSpec::text), _1), "nostrip"));
    }

    std::string archives;
    std::string all_archives;
    std::string accept_license;
    std::tie(archives, all_archives) = make_archives_strings(env, id);

    /* make ACCEPT_LICENSE */
    if (! id->eapi()->supported()->ebuild_environment_variables()->env_accept_license().empty())
    {
        AcceptLicenseFinder g(env, id);
        if (id->license_key())
            id->license_key()->parse_value()->top()->accept(g);

        accept_license = g.s.str();
    }
    else
        accept_license = "ACCEPT_LICENSE-not-set-for-this-EAPI";

    /* Strip trailing space. Some ebuilds rely upon this. From kde-meta.eclass:
     *     [[ -n ${A/${TARBALL}/} ]] && unpack ${A/${TARBALL}/}
     * Rather annoying.
     */
    archives = strip_trailing(archives, " ");
    all_archives = strip_trailing(all_archives, " ");

    /* make use */
    std::string use(make_use(env, *id, repo->profile()));

    /* add expand to use (iuse isn't reliable for use_expand things), and make the expand
     * environment variables */
    std::shared_ptr<Map<std::string, std::string> > expand_vars(make_expand(env, *id, repo->profile()));

    std::shared_ptr<const FSPathSequence> exlibsdirs(repo->layout()->exlibsdirs(id->name()));

    bool userpriv_ok((! userpriv_restrict) && (env->reduced_gid() != getgid()) &&
            check_userpriv(FSPath(repo->params().distdir()),  env, id->eapi()->supported()->userpriv_cannot_use_root()) &&
            check_userpriv(FSPath(repo->params().builddir()), env, id->eapi()->supported()->userpriv_cannot_use_root()));

    FSPath package_builddir(repo->params().builddir() / (stringify(id->name().category()) + "-" +
            stringify(id->name().package()) + "-" + stringify(id->version())));

    std::string used_config_protect;
    auto merged_entries(std::make_shared<FSPathSet>());

    auto permitted_directories(std::make_shared<PermittedDirectories>());
    {
        std::vector<std::string> tokens;
        tokenise_whitespace(id->eapi()->supported()->permitted_directories(), std::back_inserter(tokens));
        for (auto & token : tokens)
        {
            if (token.at(0) == '-')
                permitted_directories->add(FSPath(token.substr(1)), false);
            else if (token.at(0) == '+')
                permitted_directories->add(FSPath(token.substr(1)), true);
            else
                throw InternalError(PALUDIS_HERE, "bad permitted_directories");
        }
    }

    auto parts(std::make_shared<Partitioning>());
    auto choices(id->choices_key()->parse_value());
    auto work_choice(choices->find_by_name_with_prefix(ELikeWorkChoiceValue::canonical_name_with_prefix()));
    auto volatile_files(std::make_shared<FSPathSet>());
    auto destination = install_action.options.destination();

    EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_install());
    for (const auto & phase : phases)
    {
        bool skip(false);
        do
        {
            switch (install_action.options.want_phase()(phase.equal_option("skipname")))
            {
                case wp_yes:
                    continue;

                case wp_skip:
                    skip = true;
                    continue;

                case wp_abort:
                    throw ActionAbortedError("Told to abort install");

                case last_wp:
                    break;
            }

            throw InternalError(PALUDIS_HERE, "bad want_phase");
        } while (false);

        if (skip)
            continue;

        if (can_skip_phase(env, id, phase))
        {
            output_manager->stdout_stream() << "--- No need to do anything for " << phase.equal_option("skipname") << " phase" << std::endl;
            continue;
        }

        if (phase.option("tidyup") && work_choice && ! ELikeWorkChoiceValue::should_remove(work_choice->parameter()))
        {
            output_manager->stdout_stream() << "--- Skipping " << phase.equal_option("skipname")
                << " phase to preserve work" << std::endl;
            continue;
        }

        if (phase.option("merge") || phase.option("check_merge"))
        {
            if (! destination->destination_interface())
                throw ActionFailedError("Can't install '" + stringify(*id)
                        + "' to destination '" + stringify(destination->name())
                        + "' because destination does not provide destination_interface");

            MergerOptions extra_merger_options;
            if (work_choice && ELikeWorkChoiceValue::should_merge_nondestructively(work_choice->parameter()))
                extra_merger_options += mo_nondestructive;

            Timestamp build_start_time(FSPath(package_builddir / "temp" / "build_start_time").stat().mtim());
            destination->destination_interface()->merge(
                    make_named_values<MergeParams>(
                        n::build_start_time() = build_start_time,
                        n::check() = phase.option("check_merge"),
                        n::environment_file() = package_builddir / "temp" / "loadsaveenv",
                        n::image_dir() = package_builddir / "image",
                        n::is_volatile() = [&] (const FSPath & f) { return volatile_files->end() != volatile_files->find(f); },
                        n::merged_entries() = merged_entries,
                        n::options() = id->eapi()->supported()->merger_options() | extra_merger_options,
                        n::output_manager() = output_manager,
                        n::package_id() = id,
                        n::parts() = parts,
                        n::perform_uninstall() = install_action.options.perform_uninstall(),
                        n::permit_destination() =
                            std::bind(&PermittedDirectories::permit,
                                      permitted_directories,
                                      std::placeholders::_1),
                        n::replacing() = install_action.options.replacing(),
                        n::used_this_for_config_protect() = std::bind(
                                &used_this_for_config_protect, std::ref(used_config_protect), std::placeholders::_1),
                        n::want_phase() = install_action.options.want_phase()
                        ));

            if (volatile_files && phase.option("check_merge")) {
                for (auto & file : *volatile_files) {
                    auto vabs = (package_builddir / "image" / file);
                    auto vstat = vabs.stat();
                    if (! vstat.is_regular_file_or_symlink_to_regular_file())
                        throw ActionFailedError("Can't install '" + stringify(*id)
                                + "' to destination '" + stringify(destination->name())
                                + "' because '" + stringify(file) + "' was marked using exvolatile, but it is not a regular "
                                "file or a symlink to a regular file");
                    if (vstat.is_symlink() && 0 == vabs.readlink().compare(0, 1, "/", 0, 1))
                        throw ActionFailedError("Can't install '" + stringify(*id)
                                + "' to destination '" + stringify(destination->name())
                                + "' because '" + stringify(file) + "' was marked using exvolatile, but volatile symlinks"
                                "must not be absolute");
                }
            }
        }
        else if (phase.option("strip"))
        {
            if ((! id->eapi()->supported()->is_pbin()))
            {
                std::string libdir("lib");
                FSPath root(destination->installed_root_key()
                                ? stringify(destination->installed_root_key()->parse_value())
                                : "/");
                if ((root / "usr" / "lib").stat().is_symlink())
                {
                    libdir = (root / "usr" / "lib").readlink();
                    if (std::string::npos != libdir.find_first_of("./"))
                        libdir = "lib";
                }

                Log::get_instance()->message("e.ebuild.libdir", ll_debug, lc_context) << "Using '" << libdir << "' for libdir";

                std::shared_ptr<const ChoiceValue> symbols_choice(choices->find_by_name_with_prefix(
                            ELikeSymbolsChoiceValue::canonical_name_with_prefix()));

                auto dwarf_compression(choices->find_by_name_with_prefix(ELikeDwarfCompressionChoiceValue::canonical_name_with_prefix()));

                EStripper stripper(make_named_values<EStripperOptions>(
                            n::compress_splits() = symbols_choice && symbols_choice->enabled() && ELikeSymbolsChoiceValue::should_compress(
                                symbols_choice->parameter()),
                            n::controllable_strip_dir() = package_builddir / "temp",
                            n::debug_dir() = package_builddir / "image" / "usr" / libdir / "debug",
                            n::dwarf_compression() = dwarf_compression && dwarf_compression->enabled(),
                            n::image_dir() = package_builddir / "image",
                            n::output_manager() = output_manager,
                            n::package_id() = id,
                            n::split() = symbols_choice && symbols_choice->enabled() && ELikeSymbolsChoiceValue::should_split(symbols_choice->parameter()),
                            n::strip_choice() = symbols_choice && symbols_choice->enabled() && ELikeSymbolsChoiceValue::should_strip(symbols_choice->parameter()),
                            n::strip_restrict() = strip_restrict
                            ));
                stripper.strip();
            }
        }
        else if ((! phase.option("prepost")) ||
                (destination->destination_interface() &&
                 destination->destination_interface()->want_pre_post_phases()))
        {
            if (phase.option("optional_tests"))
            {
                if (test_restrict)
                    continue;

                std::shared_ptr<const ChoiceValue> choice(choices->find_by_name_with_prefix(
                            ELikeOptionalTestsChoiceValue::canonical_name_with_prefix()));
                if (choice && ! choice->enabled())
                    continue;
            }
            else if (phase.option("recommended_tests"))
            {
                if (test_restrict)
                    continue;

                std::shared_ptr<const ChoiceValue> choice(choices->find_by_name_with_prefix(
                            ELikeRecommendedTestsChoiceValue::canonical_name_with_prefix()));
                if (choice && ! choice->enabled())
                    continue;
            }
            else if (phase.option("expensive_tests"))
            {
                std::shared_ptr<const ChoiceValue> choice(choices->find_by_name_with_prefix(
                            ELikeExpensiveTestsChoiceValue::canonical_name_with_prefix()));
                if (choice && ! choice->enabled())
                    continue;
            }

            const auto params = repo->params();
            const auto profile = repo->profile();

            EbuildCommandParams command_params(make_named_values<EbuildCommandParams>(
                    n::builddir() = params.builddir(),
                    n::clearenv() = phase.option("clearenv"),
                    n::commands() = join(phase.begin_commands(), phase.end_commands(), " "),
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
                    n::parts() = parts,
                    n::permitted_directories() = permitted_directories,
                    n::portdir() =
                        (params.master_repositories() && ! params.master_repositories()->empty()) ?
                        (*params.master_repositories()->begin())->params().location() : params.location(),
                    n::root() = destination->installed_root_key()
                                    ? stringify(destination->installed_root_key()->parse_value())
                                    : "/",
                    n::sandbox() = phase.option("sandbox"),
                    n::sydbox() = phase.option("sydbox"),
                    n::userpriv() = phase.option("userpriv") && userpriv_ok,
                    n::volatile_files() = volatile_files
                    ));

            EbuildInstallCommandParams install_params(
                    make_named_values<EbuildInstallCommandParams>(
                            n::a() = archives,
                            n::aa() = all_archives,
                            n::accept_license() = accept_license,
                            n::config_protect() = repo->environment_updated_profile_variable("CONFIG_PROTECT"),
                            n::config_protect_mask() = repo->environment_updated_profile_variable("CONFIG_PROTECT_MASK"),
                            n::destination() = destination,
                            n::env_unset() = join(profile->env_unset()->begin(), profile->env_unset()->end(), " "),
                            n::expand_vars() = expand_vars,
                            n::is_from_pbin() = id->eapi()->supported()->is_pbin(),
                            n::loadsaveenv_dir() = package_builddir / "temp",
                            n::profiles() = params.profiles(),
                            n::profiles_with_parents() = profile->profiles_with_parents(),
                            n::replacing_ids() = install_action.options.replacing(),
                            n::slot() = id->slot_key() ? stringify(id->slot_key()->parse_value().raw_value()) : "",
                            n::use() = use,
                            n::use_expand() = join(profile->use_expand()->begin(), profile->use_expand()->end(), " "),
                            n::use_expand_hidden() = join(profile->use_expand_hidden()->begin(), profile->use_expand_hidden()->end(), " ")
                            ));

            EbuildInstallCommand cmd(command_params, install_params);
            try
            {
                cmd();
            }
            catch (const ActionFailedError & e)
            {
                if (work_choice && ELikeWorkChoiceValue::should_remove_on_failure(work_choice->parameter()))
                {
                    for (const auto & tidyup_phase : phases)
                    {
                        if (! tidyup_phase.option("tidyup"))
                            continue;

                        EbuildCommandParams tidyup_command_params(make_named_values<EbuildCommandParams>(
                                    n::builddir() = params.builddir(),
                                    n::clearenv() = tidyup_phase.option("clearenv"),
                                    n::commands() = join(tidyup_phase.begin_commands(), tidyup_phase.end_commands(), " "),
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
                                    n::parts() = parts,
                                    n::permitted_directories() = permitted_directories,
                                    n::portdir() =
                                        (params.master_repositories() && ! params.master_repositories()->empty())
                                            ? (*params.master_repositories()->begin())->params().location()
                                            : params.location(),
                                    n::root() = destination->installed_root_key()
                                                    ?  stringify(destination->installed_root_key()->parse_value())
                                                    : "/",
                                    n::sandbox() = tidyup_phase.option("sandbox"),
                                    n::sydbox() = tidyup_phase.option("sydbox"),
                                    n::userpriv() = tidyup_phase.option("userpriv") && userpriv_ok,
                                    n::volatile_files() = volatile_files
                                        ));

                        EbuildInstallCommandParams tidyup_install_params(
                                make_named_values<EbuildInstallCommandParams>(
                                    n::a() = archives,
                                    n::aa() = all_archives,
                                    n::accept_license() = accept_license,
                                    n::config_protect() = repo->environment_updated_profile_variable("CONFIG_PROTECT"),
                                    n::config_protect_mask() = repo->environment_updated_profile_variable("CONFIG_PROTECT_MASK"),
                                    n::destination() = destination,
                                    n::env_unset() = join(profile->env_unset()->begin(), profile->env_unset()->end(), " "),
                                    n::expand_vars() = expand_vars,
                                    n::is_from_pbin() = id->eapi()->supported()->is_pbin(),
                                    n::loadsaveenv_dir() = package_builddir / "temp",
                                    n::profiles() = params.profiles(),
                                    n::profiles_with_parents() = profile->profiles_with_parents(),
                                    n::replacing_ids() = install_action.options.replacing(),
                                    n::slot() = id->slot_key() ? stringify(id->slot_key()->parse_value().raw_value()) : "",
                                    n::use() = use,
                                    n::use_expand() = join(profile->use_expand()->begin(), profile->use_expand()->end(), " "),
                                    n::use_expand_hidden() = join(profile->use_expand_hidden()->begin(), profile->use_expand_hidden()->end(), " ")
                                    ));

                        EbuildInstallCommand tidyup_cmd(tidyup_command_params, tidyup_install_params);
                        tidyup_cmd();
                    }
                }

                throw;
            }
        }
    }

    /* replacing for pbins is done during the merge */
    if (destination->installed_root_key())
        for (const auto & replaced_id : *install_action.options.replacing())
        {
            Context local_context("When cleaning '" + stringify(*replaced_id) + "':");
            if (replaced_id->name() == id->name() && replaced_id->version() == id->version())
                continue;

            if (id->eapi()->supported()->ebuild_phases()->ebuild_new_upgrade_phase_order())
                if (replaced_id->name() == id->name() && parallel_slot_is_same(replaced_id, id))
                    continue;

            UninstallActionOptions uo(make_named_values<UninstallActionOptions>(
                        n::config_protect() = used_config_protect,
                        n::if_for_install_id() = id,
                        n::ignore_for_unmerge() = std::bind(&ignore_merged, merged_entries,
                            std::placeholders::_1),
                        n::is_overwrite() = false,
                        n::make_output_manager() = std::bind(&this_output_manager, output_manager, std::placeholders::_1),
                        n::override_contents() = nullptr,
                        n::want_phase() = install_action.options.want_phase()
                        ));
            install_action.options.perform_uninstall()(replaced_id, uo);
        }

    output_manager->succeeded();
}
