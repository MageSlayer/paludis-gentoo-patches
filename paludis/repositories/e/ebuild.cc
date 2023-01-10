/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2013 Ciaran McCreesh
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

#include <paludis/repositories/e/ebuild.hh>
#include <paludis/repositories/e/ebuild_id.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/pipe_command_handler.hh>

#include <paludis/util/system.hh>
#include <paludis/util/process.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/log.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/options.hh>
#include <paludis/util/map.hh>
#include <paludis/util/join.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/env_var_names.hh>

#include <paludis/about.hh>
#include <paludis/environment.hh>
#include <paludis/util/config_file.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/elike_choices.hh>
#include <paludis/output_manager.hh>

#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <functional>
#include <list>
#include <set>

#include "config.h"

/** \file
 * Imp for ebuild.hh things.
 *
 * \ingroup grpebuildinterface
 */

using namespace paludis;
using namespace paludis::erepository;

EbuildCommand::EbuildCommand(const EbuildCommandParams & p) :
    params(p)
{
}

EbuildCommand::~EbuildCommand() = default;

bool
EbuildCommand::success()
{
    return true;
}

bool
EbuildCommand::failure()
{
    return false;
}

namespace
{
    std::string get_jobs(const std::shared_ptr<const PackageID> & id)
    {
        std::shared_ptr<const ChoiceValue> choice;
        if (id->choices_key())
            choice = id->choices_key()->parse_value()->find_by_name_with_prefix(ELikeJobsChoiceValue::canonical_name_with_prefix());
        if (choice && choice->enabled())
            return choice->parameter();
        else
            return "";
    }

    bool get_trace(const std::shared_ptr<const PackageID> & id)
    {
        std::shared_ptr<const ChoiceValue> choice;
        if (id->choices_key())
            choice = id->choices_key()->parse_value()->find_by_name_with_prefix(ELikeTraceChoiceValue::canonical_name_with_prefix());
        return choice && choice->enabled();
    }
}

bool
EbuildCommand::operator() ()
{
    Context context("When running an ebuild command on '" + stringify(*params.package_id()) + "':");

    const auto & package_id = params.package_id();
    const auto & eapi = package_id->eapi()->supported();

    Process process(ProcessCommand(getenv_with_default(env_vars::ebuild_dir, LIBEXECDIR "/paludis") + "/ebuild.bash '"
                + ebuild_file() + "' " + commands()));

    if (! eapi)
        throw InternalError(PALUDIS_HERE, "Tried to run EbuildCommand on an unsupported EAPI");

    if (params.clearenv())
        process.clearenv();

    if (params.sydbox())
        process.sydbox();
    else if (params.sandbox())
        process.sandbox();

    if (params.userpriv())
    {
        if (eapi->userpriv_cannot_use_root())
        {
            if (0 == params.environment()->reduced_uid() || 0 == params.environment()->reduced_gid())
                if (getenv_with_default(env_vars::bypass_userpriv_checks, "").empty())
                    throw ActionFailedError("Need to be able to use non-0 user and group for userpriv for '" +
                            stringify(*package_id) + "'");
        }
        process
            .setuid_setgid(params.environment()->reduced_uid(), params.environment()->reduced_gid())
            .setenv("USER", params.environment()->reduced_username())
            .setenv("LOGNAME", params.environment()->reduced_username());
    }

    using namespace std::placeholders;

    process.pipe_command_handler("PALUDIS_PIPE_COMMAND",
                                 std::bind(&pipe_command_handler,
                                           params.environment(),
                                           package_id,
                                           params.permitted_directories(),
                                           params.parts(),
                                           params.volatile_files(),
                                           in_metadata_generation(), _1,
                                           params.maybe_output_manager()));

    std::shared_ptr<const FSPathSequence> syncers_dirs(params.environment()->syncers_dirs());
    std::shared_ptr<const FSPathSequence> bashrc_files(params.environment()->bashrc_files());
    std::shared_ptr<const FSPathSequence> fetchers_dirs(params.environment()->fetchers_dirs());
    std::shared_ptr<const FSPathSequence> hook_dirs(params.environment()->hook_dirs());

    const auto & options = eapi->ebuild_options();
    const auto & tools = eapi->tools_options();
    const auto & environment_variables = eapi->ebuild_environment_variables();

    process
        .setenv("PV", stringify(package_id->version().remove_revision()))
        .setenv("PR", stringify(package_id->version().revision_only()))
        .setenv("PN", stringify(package_id->name().package()))
        .setenv("PVR", stringify(package_id->version()))
        .setenv("CATEGORY", stringify(package_id->name().category()))
        .setenv("REPOSITORY", stringify(package_id->repository_name()))
        .setenv("EAPI", stringify(package_id->eapi()->exported_name()))
        .setenv("PKGMANAGER", PALUDIS_PACKAGE "-" + stringify(PALUDIS_VERSION_MAJOR) + "." +
                stringify(PALUDIS_VERSION_MINOR) + "." +
                stringify(PALUDIS_VERSION_MICRO) + stringify(PALUDIS_VERSION_SUFFIX) +
                (std::string(PALUDIS_GIT_HEAD).empty() ?
                 std::string("") : "-git-" + std::string(PALUDIS_GIT_HEAD)))
        .setenv("PALUDIS_TMPDIR", stringify(params.builddir()))
        .setenv("PALUDIS_PACKAGE_BUILDDIR", stringify(params.package_builddir()))
        .setenv("PALUDIS_CONFIG_DIR", SYSCONFDIR "/paludis/")
        .setenv("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "))
        .setenv("PALUDIS_HOOK_DIRS", join(hook_dirs->begin(), hook_dirs->end(), " "))
        .setenv("PALUDIS_FETCHERS_DIRS", join(fetchers_dirs->begin(), fetchers_dirs->end(), " "))
        .setenv("PALUDIS_SYNCERS_DIRS", join(syncers_dirs->begin(), syncers_dirs->end(), " "))
        .setenv("PALUDIS_REDUCED_GID", stringify(params.environment()->reduced_gid()))
        .setenv("PALUDIS_REDUCED_UID", stringify(params.environment()->reduced_uid()))
        .setenv("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
        .setenv("PALUDIS_EBUILD_QUIET", "")
        .setenv("PALUDIS_EBUILD_DIR", getenv_with_default(env_vars::ebuild_dir, LIBEXECDIR "/paludis"))
        .setenv("PALUDIS_UTILITY_PATH_SUFFIXES", options->utility_path_suffixes())
        .setenv("PALUDIS_EBUILD_MODULE_SUFFIXES", options->ebuild_module_suffixes())
        .setenv("PALUDIS_NON_EMPTY_VARIABLES", options->non_empty_variables())
        .setenv("PALUDIS_DIRECTORY_VARIABLES", options->directory_variables())
        .setenv("PALUDIS_EBUILD_MUST_NOT_SET_VARIABLES",
                options->ebuild_must_not_set_variables())
        .setenv("PALUDIS_ECLASS_MUST_NOT_SET_VARIABLES",
                options->eclass_must_not_set_variables())
        .setenv("PALUDIS_SAVE_VARIABLES", options->save_variables())
        .setenv("PALUDIS_SAVE_BASE_VARIABLES", options->save_base_variables())
        .setenv("PALUDIS_SAVE_UNMODIFIABLE_VARIABLES",
                options->save_unmodifiable_variables())
        .setenv("PALUDIS_DIRECTORY_IF_EXISTS_VARIABLES",
                options->directory_if_exists_variables())
        .setenv("PALUDIS_SOURCE_MERGED_VARIABLES",
                options->source_merged_variables())
        .setenv("PALUDIS_BRACKET_MERGED_VARIABLES",
                options->bracket_merged_variables())
        .setenv("PALUDIS_BRACKET_MERGED_VARIABLES_ANNOTATABLE",
                options->bracket_merged_variables_annotatable())
        .setenv("PALUDIS_BRACKET_MERGED_VARIABLES_ANNOTATION",
                options->bracket_merged_variables_annotation())
        .setenv("PALUDIS_MUST_NOT_CHANGE_VARIABLES",
                options->must_not_change_variables())
        .setenv("PALUDIS_MUST_NOT_SET_VARS_STARTING_WITH",
                options->must_not_set_vars_starting_with())
        .setenv("PALUDIS_MUST_NOT_CHANGE_AFTER_SOURCE_VARIABLES",
                options->must_not_change_after_source_variables())
        .setenv("PALUDIS_RDEPEND_DEFAULTS_TO_DEPEND",
                options->rdepend_defaults_to_depend() ? "yes" : "")
        .setenv("PALUDIS_F_FUNCTION_PREFIX", options->f_function_prefix())
        .setenv("PALUDIS_IGNORE_PIVOT_ENV_FUNCTIONS",
                options->ignore_pivot_env_functions())
        .setenv("PALUDIS_IGNORE_PIVOT_ENV_VARIABLES",
                options->ignore_pivot_env_variables())
        .setenv("PALUDIS_LOAD_MODULES", options->load_modules())
        .setenv("PALUDIS_EBUILD_FUNCTIONS", options->ebuild_functions())
        .setenv("PALUDIS_NO_S_WORKDIR_FALLBACK",
                options->no_s_workdir_fallback() ? "yes" : "")
        .setenv("PALUDIS_BINARY_DISTDIR_VARIABLE", environment_variables->env_distdir())
        .setenv("PALUDIS_ECONF_EXTRA_OPTIONS", tools->econf_extra_options())
        .setenv("PALUDIS_ECONF_EXTRA_OPTIONS_HELP_DEPENDENT",
                tools->econf_extra_options_help_dependent())
        .setenv("PALUDIS_UNPACK_UNRECOGNISED_IS_FATAL",
                tools->unpack_unrecognised_is_fatal() ? "yes" : "")
        .setenv("PALUDIS_UNPACK_FIX_PERMISSIONS",
                tools->unpack_fix_permissions() ? "yes" : "")
        .setenv("PALUDIS_UNPACK_ANY_PATH",
                tools->unpack_any_path() ? "yes" : "")
        .setenv("PALUDIS_UNPACK_CASE_INSENSITIVE",
                tools->unpack_case_insensitive() ? "yes" : "")
        .setenv("PALUDIS_UNPACK_SUFFIXES", tools->unpack_suffixes())
        .setenv("PALUDIS_BEST_HAS_VERSION_HOST_ROOT",
                tools->best_has_version_host_root() ? "yes" : "")
        .setenv("PALUDIS_DODOC_R", tools->dodoc_r() ? "yes" : "")
        .setenv("PALUDIS_DOINS_SYMLINK", tools->doins_symlink() ? "yes" : "")
        .setenv("PALUDIS_DOMAN_LANG_FILENAMES",
                tools->doman_lang_filenames() ? "yes" : "")
        .setenv("PALUDIS_DOMAN_LANG_FILENAMES_OVERRIDES",
                tools->doman_lang_filenames_overrides() ? "yes" : "")
        .setenv("PALUDIS_DOSYM_NO_MKDIR", tools->dosym_mkdir() ? "" : "yes")
        .setenv("PALUDIS_NEW_STDIN", tools->new_stdin() ? "yes" : "")
        .setenv("PALUDIS_USE_WITH_ENABLE_EMPTY_THIRD_ARGUMENT",
                tools->use_with_enable_empty_third_argument() ? "yes" : "")
        .setenv("PALUDIS_FAILURE_IS_FATAL", tools->failure_is_fatal() ? "yes" : "")
        .setenv("PALUDIS_DIE_SUPPORTS_DASH_N",
                tools->die_supports_dash_n() ? "yes" : "")
        .setenv("PALUDIS_UNPACK_FROM_VAR", environment_variables->env_distdir())
        .setenv("PALUDIS_IMAGE_DIR_VAR", environment_variables->env_d())
        .setenv("PALUDIS_TEMP_DIR_VAR", environment_variables->env_t())
        .setenv("PALUDIS_NAME_VERSION_REVISION_VAR", environment_variables->env_pf())
        .setenv("PALUDIS_EBUILD_PHASE_VAR", environment_variables->env_ebuild_phase())
        .setenv("PALUDIS_EBUILD_PHASE_FUNC_VAR", environment_variables->env_ebuild_phase_func())
        .setenv("PALUDIS_PIPE_COMMANDS_SUPPORTED", "yes")
        .setenv("PALUDIS_PIPE_COMMANDS_STATUS_SUPPORTED", "yes")
        .setenv("PALUDIS_PIPE_COMMAND_DELIM", "\2")
        .setenv("PALUDIS_BASH_COMPAT", options->bash_compat())
        .setenv("PALUDIS_SHELL_OPTIONS", options->shell_options())
        .setenv("PALUDIS_SHELL_OPTIONS_GLOBAL", options->shell_options_global())
        .setenv("SLOT", "")
        .setenv("PALUDIS_PROFILE_DIR", "")
        .setenv("PALUDIS_PROFILE_DIRS", "")
        .setenv("PALUDIS_PROFILES_DIRS", "")
        .setenv("ROOT", params.root());

    if (! environment_variables->env_merge_type().empty())
        process.setenv(environment_variables->env_merge_type(), "");

    extend_command(process);

    if (! environment_variables->env_kv().empty())
        process.setenv(environment_variables->env_kv(), kernel_version());
    if (! environment_variables->env_portdir().empty())
        process.setenv(environment_variables->env_portdir(), stringify(params.portdir()));
    if (! environment_variables->env_distdir().empty())
        process.setenv(environment_variables->env_distdir(), stringify(params.distdir()));
    if (! environment_variables->env_p().empty())
        process.setenv(environment_variables->env_p(),
                       stringify(package_id->name().package()) + "-" +
                       stringify(package_id->version().remove_revision()));
    if (! environment_variables->env_pf().empty())
        process.setenv(environment_variables->env_pf(),
                       stringify(package_id->name().package()) + "-" +
                       stringify(package_id->version()));
    if (! environment_variables->env_filesdir().empty())
        process.setenv(environment_variables->env_filesdir(), stringify(params.files_dir()));

    if (! eapi->ebuild_metadata_variables()->iuse_effective()->name().empty())
        if (package_id->raw_iuse_effective_key())
        {
            auto iu(package_id->raw_iuse_effective_key()->parse_value());
            process.setenv(eapi->ebuild_metadata_variables()->iuse_effective()->name(), join(iu->begin(), iu->end(), " "));
        }

    if (options->support_eclasses())
        process
            .setenv("ECLASSDIR", stringify(*params.eclassdirs()->begin()))
            .setenv("ECLASSDIRS", join(params.eclassdirs()->begin(), params.eclassdirs()->end(), " "));

    if (options->support_exlibs())
        process
            .setenv("EXLIBSDIRS", join(params.exlibsdirs()->begin(), params.exlibsdirs()->end(), " "));

    if (! environment_variables->env_jobs().empty())
        process
            .setenv("PALUDIS_JOBS_VAR", environment_variables->env_jobs())
            .setenv(environment_variables->env_jobs(), get_jobs(package_id));

    process.setenv("PALUDIS_PREFIX_IMAGE_VAR", environment_variables->env_ed());
    if (! environment_variables->env_eprefix().empty())
        process.setenv(environment_variables->env_eprefix(), "");
    if (! environment_variables->env_eroot().empty())
        process.setenv(environment_variables->env_eroot(), params.root());

    process.setenv("PALUDIS_TRACE", get_trace(package_id) ? "yes" : "");

    if (options->want_portage_emulation_vars())
        add_portage_vars(process);

    if (params.maybe_output_manager())
        process
            .capture_stderr(params.maybe_output_manager()->stderr_stream())
            .capture_stdout(params.maybe_output_manager()->stdout_stream())
            .use_ptys();

    if (do_run_command(process))
        return success();
    else
        return failure();
}

std::string
EbuildCommand::ebuild_file() const
{
    return stringify(params.ebuild_file());
}

void
EbuildCommand::add_portage_vars(Process & process) const
{
    process
        .setenv("PORTAGE_ACTUAL_DISTDIR", stringify(params.distdir()))
        .setenv("PORTAGE_BASHRC", "/dev/null")
        .setenv("PORTAGE_BUILDDIR", stringify(params.package_builddir()))
        .setenv("PORTAGE_GID", "0")
        .setenv("PORTAGE_INST_GID", "0")
        .setenv("PORTAGE_INST_UID", "0")
        .setenv("PORTAGE_MASTER_PID", stringify(::getpid()))
        .setenv("PORTAGE_NICENCESS", stringify(::getpriority(PRIO_PROCESS, 0)))
        .setenv("PORTAGE_TMPDIR", stringify(params.builddir()))
        .setenv("PORTAGE_TMPFS", "/dev/shm")
        .setenv("PORTAGE_WORKDIR_MODE", "0700");
}

bool
EbuildCommand::in_metadata_generation() const
{
    return false;
}

bool
EbuildCommand::do_run_command(Process & process)
{
    return 0 == process.run().wait();
}

EbuildMetadataCommand::EbuildMetadataCommand(const EbuildCommandParams & p) :
    EbuildCommand(p)
{
}

EbuildMetadataCommand::~EbuildMetadataCommand() = default;

std::string
EbuildMetadataCommand::commands() const
{
    return params.commands();
}

bool
EbuildMetadataCommand::failure()
{
    return EbuildCommand::failure();
}

void
EbuildMetadataCommand::extend_command(Process & process)
{
    process
        .setenv("PALUDIS_EBUILD_QUIET", "yes")
        .setuid_setgid(params.environment()->reduced_uid(), params.environment()->reduced_gid())
        ;
}

bool
EbuildMetadataCommand::in_metadata_generation() const
{
    return true;
}

bool
EbuildMetadataCommand::do_run_command(Process & process)
{
    bool ok(false);
    keys = std::make_shared<Map<std::string, std::string>>();

    try
    {
        Context context("When running ebuild command to generate metadata for '" + stringify(*params.package_id()) + "':");

        std::stringstream prog;
        std::stringstream prog_err;
        std::stringstream metadata;
        process
            .capture_stdout(prog)
            .capture_stderr(prog_err)
            .capture_output_to_fd(metadata, -1, "PALUDIS_METADATA_FD");

        int exit_status(process.run().wait());

        KeyValueConfigFile f(metadata, { kvcfo_disallow_continuations, kvcfo_disallow_comments , kvcfo_disallow_space_around_equals,
                kvcfo_disallow_unquoted_values, kvcfo_disallow_source , kvcfo_disallow_variables, kvcfo_preserve_whitespace },
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

        std::copy(f.begin(), f.end(), keys->inserter());
        if (0 == exit_status)
            ok = true;

        captured_stdout = prog.str();
        captured_stderr = prog_err.str();
    }
    catch (const InternalError &)
    {
        throw;
    }
    catch (const Exception & e)
    {
        Log::get_instance()->message("e.ebuild.cache_failure", ll_warning, lc_context) << "Caught exception '"
                << e.message() << "' (" << e.what() << ") when generating cache for '"
                << *params.package_id() << "'";
    }

    if (ok)
        return true;
    else
    {
        Log::get_instance()->message("e.ebuild.cache_failure", ll_warning, lc_context) << "Could not generate cache for '"
            << *params.package_id() << "', stdout says '" << captured_stdout << "' and stderr says '" << captured_stderr << "'";
        keys = std::make_shared<Map<std::string, std::string>>();
        keys->insert("EAPI", EAPIData::get_instance()->unknown_eapi()->name());

        return false;
    }
}

namespace
{
    std::string get(const std::shared_ptr<const Map<std::string, std::string> > & k, const std::string & s)
    {
        Map<std::string, std::string>::ConstIterator i(k->find(s));
        if (k->end() == i)
            return "";
        return i->second;
    }
}

void
EbuildMetadataCommand::load(const std::shared_ptr<const EbuildID> & id)
{
    Context context("When loading generated metadata for '" + stringify(*params.package_id()) + "':");

    if (! keys)
        throw InternalError(PALUDIS_HERE, "keys is 0");

    if (! id->eapi()->supported())
    {
        Log::get_instance()->message("e.ebuild.preload_eapi.unsupported", ll_debug, lc_context)
            << "ID pre-load EAPI '" << id->eapi()->name() << "' not supported";

        if (! captured_stdout.empty())
            id->load_captured_stdout("STDERR", "Captured stdout", mkt_normal, captured_stdout);
        if (! captured_stderr.empty())
            id->load_captured_stderr("STDERR", "Captured stderr", mkt_normal, captured_stderr);

        return;
    }
    else
        Log::get_instance()->message("e.ebuild.preload_eapi.supported", ll_debug, lc_context)
            << "ID pre-load EAPI '" << id->eapi()->name() << "' is supported";

    std::string s(get(keys, id->eapi()->supported()->ebuild_metadata_variables()->eapi()->name()));
    if (s.empty())
    {
        auto repo(params.environment()->fetch_repository(id->repository_name()));
        auto e_repo(std::static_pointer_cast<const ERepository>(repo));
        s = e_repo->params().eapi_when_unspecified();
    }
    if (id->eapi()->name() != s)
    {
        Log::get_instance()->message("e.ebuild.postload_eapi.mismatch", ll_warning, lc_context)
            << "Post-source EAPI '" << s << "' does not match pre-source EAPI '" << id->eapi()->name() << "', treating as unknown";
        id->set_eapi(EAPIData::get_instance()->unknown_eapi()->name());
    }

    if (! id->eapi()->supported())
    {
        Log::get_instance()->message("e.ebuild.postload_eapi.unsupported", ll_debug, lc_context)
            << "ID post-load EAPI '" << id->eapi()->name() << "' not supported";
        if (! captured_stdout.empty())
            id->load_captured_stdout("STDERR", "Captured stdout", mkt_normal, captured_stdout);
        if (! captured_stderr.empty())
            id->load_captured_stderr("STDERR", "Captured stderr", mkt_normal, captured_stderr);
        return;
    }
    else
        Log::get_instance()->message("e.ebuild.postload_eapi.supported", ll_debug, lc_context)
            << "ID post-load EAPI '" << id->eapi()->name() << "' is supported";

    if (! captured_stdout.empty())
        id->load_captured_stdout("STDERR", "Captured stdout", mkt_internal, captured_stdout);
    if (! captured_stderr.empty())
        id->load_captured_stderr("STDERR", "Captured stderr", mkt_internal, captured_stderr);

    const EAPIEbuildMetadataVariables & m(*id->eapi()->supported()->ebuild_metadata_variables());

    if (! m.short_description()->name().empty())
        id->load_short_description(m.short_description()->name(), m.short_description()->description(), get(keys, m.short_description()->name()));

    if (! m.long_description()->name().empty())
    {
        std::string value(get(keys, m.long_description()->name()));
        if (! value.empty())
            id->load_long_description(m.long_description()->name(), m.long_description()->description(), value);
    }

    if (! m.dependencies()->name().empty())
        id->load_dependencies(m.dependencies()->name(), m.dependencies()->description(), get(keys, m.dependencies()->name()));
    else
    {
        if (! m.build_depend()->name().empty())
            id->load_build_depend(m.build_depend()->name(), m.build_depend()->description(), get(keys, m.build_depend()->name()), false);

        if (! m.run_depend()->name().empty())
        {
            if (id->eapi()->supported()->ebuild_options()->rdepend_defaults_to_depend())
            {
                if (! get(keys, "PALUDIS_EBUILD_RDEPEND_WAS_SET").empty())
                    id->load_run_depend(m.run_depend()->name(), m.run_depend()->description(),
                            get(keys, m.run_depend()->name()), false);
                else
                    id->load_run_depend(m.run_depend()->name(), m.run_depend()->description(),
                            get(keys, "PALUDIS_EBUILD_DEPEND") + " " + get(keys, m.run_depend()->name()), false);
            }
            else
                id->load_run_depend(m.run_depend()->name(), m.run_depend()->description(),
                        get(keys, m.run_depend()->name()), false);
        }

        if (! m.pdepend()->name().empty())
            id->load_post_depend(m.pdepend()->name(), m.pdepend()->description(), get(keys, m.pdepend()->name()), false);
    }

    if (! m.slot()->name().empty())
    {
        try
        {
            Context c("When setting SLOT:");
            std::string slot(get(keys, m.slot()->name()));
            if (slot.empty())
            {
                Log::get_instance()->message("e.ebuild.no_slot", ll_qa, lc_context)
                    << "Package '" << *id << "' set SLOT=\"\", using SLOT=\"0\" instead";
                slot = "0";
            }
            id->load_slot(m.slot(), slot);
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.ebuild.bad_slot", ll_warning, lc_context)
                << "Setting SLOT for '" << *id << "' failed due to exception '"
                << e.message() << "' (" << e.what() << ")";
            id->load_slot(m.slot(), "0");
        }
    }

    if (! m.src_uri()->name().empty())
        id->load_src_uri(m.src_uri(), get(keys, m.src_uri()->name()));

    if (! m.homepage()->name().empty())
        id->load_homepage(m.homepage(), get(keys, m.homepage()->name()));

    if (! m.license()->name().empty())
        id->load_license(m.license(), get(keys, m.license()->name()));

    if (! m.iuse()->name().empty())
        id->load_iuse(m.iuse(), get(keys, m.iuse()->name()));

    if (! m.myoptions()->name().empty())
        id->load_myoptions(m.myoptions(), get(keys, m.myoptions()->name()));

    if (! m.required_use()->name().empty())
        id->load_required_use(m.required_use(), get(keys, m.required_use()->name()));

    if (! m.inherited()->name().empty())
        id->load_inherited(m.inherited(), get(keys, m.inherited()->name()));

    if (! m.keywords()->name().empty())
        id->load_keywords(m.keywords(), get(keys, m.keywords()->name()));

    if (! m.restrictions()->name().empty())
        id->load_restrict(m.restrictions(), get(keys, m.restrictions()->name()));

    if (! m.properties()->name().empty())
        id->load_properties(m.properties(), get(keys, m.properties()->name()));

    if (! m.use()->name().empty())
        id->load_use(m.use(), get(keys, m.use()->name()));

    if (! m.generated_from()->name().empty())
        id->load_generated_from(m.generated_from(), get(keys, m.generated_from()->name()));

    if (! m.generated_time()->name().empty())
        id->load_generated_time(m.generated_time()->name(), m.generated_time()->description(), get(keys, m.generated_time()->name()));

    if (! m.generated_using()->name().empty())
        id->load_generated_using(m.generated_using()->name(), m.generated_using()->description(), get(keys, m.generated_using()->name()));

    if (! m.upstream_changelog()->name().empty())
    {
        std::string value(get(keys, m.upstream_changelog()->name()));
        if (! value.empty())
            id->load_upstream_changelog(m.upstream_changelog(), value);
    }

    if (! m.upstream_documentation()->name().empty())
    {
        std::string value(get(keys, m.upstream_documentation()->name()));
        if (! value.empty())
            id->load_upstream_documentation(m.upstream_documentation(), value);
    }

    if (! m.upstream_release_notes()->name().empty())
    {
        std::string value(get(keys, m.upstream_release_notes()->name()));
        if (! value.empty())
            id->load_upstream_release_notes(m.upstream_release_notes(), value);
    }

    if (! m.bugs_to()->name().empty())
    {
        std::string value(get(keys, m.bugs_to()->name()));
        if (! value.empty())
            id->load_bugs_to(m.bugs_to(), value);
    }

    if (! m.remote_ids()->name().empty())
    {
        std::string value(get(keys, m.remote_ids()->name()));
        if (! value.empty())
            id->load_remote_ids(m.remote_ids(), value);
    }

    if (id->eapi()->supported()->is_pbin() && ! m.scm_revision()->name().empty())
    {
        std::string value(get(keys, m.scm_revision()->name()));
        if (! value.empty())
            id->load_scm_revision(m.scm_revision()->name(), m.scm_revision()->description(), value);
    }

    if (! m.defined_phases()->name().empty())
    {
        std::set<std::string> defined_phases;
        std::set<std::string> raw_values;
        std::set<std::string> ebuild_functions;
        std::string raw_value(get(keys, "PALUDIS_DECLARED_FUNCTIONS"));
        tokenise_whitespace(raw_value, std::inserter(raw_values, raw_values.begin()));
        tokenise_whitespace(id->eapi()->supported()->ebuild_options()->ebuild_functions(),
                std::inserter(ebuild_functions, ebuild_functions.end()));

        for (auto function : ebuild_functions)
        {
            if (0 == function.compare(0, 8, "builtin_"))
                continue;

            if (raw_values.end() == raw_values.find(function))
                continue;

            if (0 == function.compare(0, 4, "src_"))
                function.erase(0, 4);
            else if (0 == function.compare(0, 4, "pkg_"))
                function.erase(0, 4);
            else
                throw InternalError(PALUDIS_HERE, "Got strange phase function '" + function + "'");

            defined_phases.insert(function);
        }

        if (defined_phases.empty())
            id->load_defined_phases(m.defined_phases(), "-");
        else
            id->load_defined_phases(m.defined_phases(), join(defined_phases.begin(), defined_phases.end(), " "));
    }
}

EbuildVariableCommand::EbuildVariableCommand(const EbuildCommandParams & p,
        const std::string & var) :
    EbuildCommand(p),
    _var(var)
{
}

std::string
EbuildVariableCommand::commands() const
{
    return params.commands();
}

bool
EbuildVariableCommand::failure()
{
    return EbuildCommand::failure();
}

void
EbuildVariableCommand::extend_command(Process & process)
{
    process
        .setenv("PALUDIS_VARIABLE", _var)
        .setenv("PALUDIS_EBUILD_QUIET", "yes")
        .setuid_setgid(params.environment()->reduced_uid(), params.environment()->reduced_gid());
}

bool
EbuildVariableCommand::do_run_command(Process & process)
{
    std::stringstream prog;
    process.capture_stdout(prog);
    int exit_status(process.run().wait());
    _result = strip_trailing_string(
            std::string((std::istreambuf_iterator<char>(prog)),
                std::istreambuf_iterator<char>()), "\n");

    return (0 == exit_status);
}

std::string
EbuildNoFetchCommand::commands() const
{
    return params.commands();
}

bool
EbuildNoFetchCommand::failure()
{
    throw ActionFailedError("Fetch failed for '" + stringify(*params.package_id()) + "'");
}

void
EbuildNoFetchCommand::extend_command(Process & process)
{
    const auto & package_id = params.package_id();
    const auto & eapi = package_id->eapi()->supported();
    const auto & environment_variables = eapi->ebuild_environment_variables();

    process
        .setenv("PALUDIS_PROFILE_DIR", stringify(*fetch_params.profiles()->begin()))
        .setenv("PALUDIS_PROFILE_DIRS", join(fetch_params.profiles()->begin(),
                    fetch_params.profiles()->end(), " "))
        .setenv("PALUDIS_PROFILES_DIRS", join(fetch_params.profiles_with_parents()->begin(),
                    fetch_params.profiles_with_parents()->end(), " "))
        .setenv("PALUDIS_ARCHIVES_VAR",
                environment_variables->env_a());

    if (! environment_variables->env_a().empty())
        process.setenv(environment_variables->env_a(), fetch_params.a());
    if (! environment_variables->env_aa().empty())
        process.setenv(environment_variables->env_aa(), fetch_params.aa());
    if (! environment_variables->env_use().empty())
        process.setenv(environment_variables->env_use(), fetch_params.use());
    if (! environment_variables->env_use_expand().empty())
        process.setenv(environment_variables->env_use_expand(),
                fetch_params.use_expand());
    if (! environment_variables->env_use_expand_hidden().empty())
        process.setenv(environment_variables->env_use_expand_hidden(),
                fetch_params.use_expand_hidden());

    for (const auto & kv : *fetch_params.expand_vars())
        process.setenv(kv.first, kv.second);
}

EbuildNoFetchCommand::EbuildNoFetchCommand(const EbuildCommandParams & p,
        const EbuildNoFetchCommandParams & f) :
    EbuildCommand(p),
    fetch_params(f)
{
}

std::string
EbuildInstallCommand::commands() const
{
    return params.commands();
}

bool
EbuildInstallCommand::failure()
{
    throw ActionFailedError("Install failed for '" + stringify(*params.package_id()) + "'");
}

void
EbuildInstallCommand::extend_command(Process & process)
{
    const auto & package_id = params.package_id();
    const auto & eapi = package_id->eapi()->supported();
    const auto & environment_variables = eapi->ebuild_environment_variables();

    process
        .setenv("PALUDIS_LOADSAVEENV_DIR", stringify(install_params.loadsaveenv_dir()))
        .setenv("PALUDIS_CONFIG_PROTECT", install_params.config_protect())
        .setenv("PALUDIS_CONFIG_PROTECT_MASK", install_params.config_protect_mask())
        .setenv("PALUDIS_PROFILE_DIR", stringify(*install_params.profiles()->begin()))
        .setenv("PALUDIS_PROFILE_DIRS", join(install_params.profiles()->begin(),
                    install_params.profiles()->end(), " "))
        .setenv("PALUDIS_PROFILES_DIRS", join(install_params.profiles_with_parents()->begin(),
                    install_params.profiles_with_parents()->end(), " "))
        .setenv("PALUDIS_ARCHIVES_VAR",
                eapi->ebuild_environment_variables()->env_a())
        .setenv("SLOT", stringify(install_params.slot()));

    if (! environment_variables->env_a().empty())
        process.setenv(environment_variables->env_a(), install_params.a());
    if (! environment_variables->env_aa().empty())
        process.setenv(environment_variables->env_aa(), install_params.aa());
    if (! environment_variables->env_accept_license().empty())
        process.setenv(environment_variables->env_accept_license(),
                install_params.accept_license());
    if (! environment_variables->env_use().empty())
        process.setenv(environment_variables->env_use(), install_params.use());
    if (! environment_variables->env_use_expand().empty())
        process.setenv(environment_variables->env_use_expand(),
                install_params.use_expand());
    if (! environment_variables->env_use_expand_hidden().empty())
        process.setenv(environment_variables->env_use_expand_hidden(),
                install_params.use_expand_hidden());

    if (! environment_variables->env_replacing_ids().empty())
        process.setenv(environment_variables->env_replacing_ids(),
                join(indirect_iterator(install_params.replacing_ids()->begin()),
                    indirect_iterator(install_params.replacing_ids()->end()), " "));

    if (! environment_variables->env_replacing_versions().empty())
    {
        std::string s;
        for (const auto & id : *install_params.replacing_ids())
            if (id->name() == params.package_id()->name())
            {
                if (! s.empty())
                    s.append(" ");
                s.append(stringify(id->version()));
            }

        process.setenv(environment_variables->env_replacing_versions(), s);
    }

    if (! environment_variables->env_merge_type().empty())
    {
        std::string s;

        if (install_params.destination())
        {
            if (install_params.destination()->installed_root_key())
            {
                if (install_params.is_from_pbin())
                    s = "binary";
                else
                    s = "source";
            }
            else
                s = "buildonly";
        }

        process.setenv(environment_variables->env_merge_type(), s);
    }

    for (const auto & kv : *install_params.expand_vars())
        process.setenv(kv.first, kv.second);
}

EbuildInstallCommand::EbuildInstallCommand(const EbuildCommandParams & p,
        const EbuildInstallCommandParams & f) :
    EbuildCommand(p),
    install_params(f)
{
}

std::string
EbuildUninstallCommand::commands() const
{
    return params.commands();
}

std::string
EbuildUninstallCommand::ebuild_file() const
{
    return "-";
}

bool
EbuildUninstallCommand::failure()
{
    throw ActionFailedError("Uninstall failed for '" + stringify(*params.package_id()) + "'");
}

void
EbuildUninstallCommand::extend_command(Process & process)
{
    const auto & package_id = params.package_id();
    const auto & eapi = package_id->eapi()->supported();
    const auto & environment_variables = eapi->ebuild_environment_variables();

    process
        .setenv("PALUDIS_LOADSAVEENV_DIR", stringify(uninstall_params.loadsaveenv_dir()));

    if (uninstall_params.load_environment())
        process
            .setenv("PALUDIS_LOAD_ENVIRONMENT", stringify(*uninstall_params.load_environment()))
            .setenv("PALUDIS_SKIP_INHERIT", "yes");

    if (! environment_variables->env_replaced_by_id().empty())
        if (uninstall_params.replaced_by())
            process.setenv(environment_variables->env_replaced_by_id(),
                stringify(*uninstall_params.replaced_by()));

    if (! environment_variables->env_replaced_by_version().empty())
        if (uninstall_params.replaced_by())
            process.setenv(environment_variables->env_replaced_by_version(),
                stringify(uninstall_params.replaced_by()->version()));
}

EbuildUninstallCommand::EbuildUninstallCommand(const EbuildCommandParams & p,
        const EbuildUninstallCommandParams & f) :
    EbuildCommand(p),
    uninstall_params(f)
{
}

std::string
EbuildConfigCommand::ebuild_file() const
{
    return "-";
}

std::string
EbuildConfigCommand::commands() const
{
    return params.commands();
}

bool
EbuildConfigCommand::failure()
{
    throw ActionFailedError("Configure failed for '" + stringify(*params.package_id()) + "'");
}

void
EbuildConfigCommand::extend_command(Process & process)
{
    if (config_params.load_environment())
        process
            .setenv("PALUDIS_LOAD_ENVIRONMENT", stringify(*config_params.load_environment()))
            .setenv("PALUDIS_SKIP_INHERIT", "yes");
}

EbuildConfigCommand::EbuildConfigCommand(const EbuildCommandParams & p,
        const EbuildConfigCommandParams & f) :
    EbuildCommand(p),
    config_params(f)
{
}

WriteVDBEntryCommand::WriteVDBEntryCommand(const WriteVDBEntryParams & p) :
    params(p)
{
}

void
WriteVDBEntryCommand::operator() ()
{
    using namespace std::placeholders;

    const auto & package_id = params.package_id();
    const auto & eapi = package_id->eapi()->supported();

    std::string ebuild_cmd(getenv_with_default(env_vars::ebuild_dir, LIBEXECDIR "/paludis") +
            "/write_vdb_entry.bash '" +
            stringify(params.output_directory()) + "' '" +
            stringify(params.environment_file()) + "'");

    std::shared_ptr<const FSPathSequence> syncers_dirs(params.environment()->syncers_dirs());
    std::shared_ptr<const FSPathSequence> bashrc_files(params.environment()->bashrc_files());
    std::shared_ptr<const FSPathSequence> fetchers_dirs(params.environment()->fetchers_dirs());
    std::shared_ptr<const FSPathSequence> hook_dirs(params.environment()->hook_dirs());

    Process process((ProcessCommand(ebuild_cmd)));

    process
        .setenv("PKGMANAGER", PALUDIS_PACKAGE "-" + stringify(PALUDIS_VERSION_MAJOR) + "." +
                stringify(PALUDIS_VERSION_MINOR) + "." +
                stringify(PALUDIS_VERSION_MICRO) +
                (std::string(PALUDIS_GIT_HEAD).empty() ?
                 std::string("") : "-git-" + std::string(PALUDIS_GIT_HEAD)))
        .setenv("EAPI", stringify(params.package_id()->eapi()->exported_name()))
        .setenv("PALUDIS_CONFIG_DIR", SYSCONFDIR "/paludis/")
        .setenv("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "))
        .setenv("PALUDIS_HOOK_DIRS", join(hook_dirs->begin(), hook_dirs->end(), " "))
        .setenv("PALUDIS_FETCHERS_DIRS", join(fetchers_dirs->begin(), fetchers_dirs->end(), " "))
        .setenv("PALUDIS_SYNCERS_DIRS", join(syncers_dirs->begin(), syncers_dirs->end(), " "))
        .setenv("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
        .setenv("PALUDIS_EBUILD_DIR", getenv_with_default(env_vars::ebuild_dir, LIBEXECDIR "/paludis"))
        .setenv("PALUDIS_VDB_FROM_ENV_VARIABLES",
                eapi->ebuild_options()->vdb_from_env_variables())
        .setenv("PALUDIS_VDB_FROM_ENV_UNLESS_EMPTY_VARIABLES",
                eapi->ebuild_options()->vdb_from_env_unless_empty_variables())
        .setenv("PALUDIS_F_FUNCTION_PREFIX",
                eapi->ebuild_options()->f_function_prefix())
        .setenv("PALUDIS_IGNORE_PIVOT_ENV_FUNCTIONS",
                eapi->ebuild_options()->ignore_pivot_env_functions())
        .setenv("PALUDIS_IGNORE_PIVOT_ENV_VARIABLES",
                eapi->ebuild_options()->ignore_pivot_env_variables())
        .setenv("PALUDIS_EBUILD_MODULE_SUFFIXES",
                eapi->ebuild_options()->ebuild_module_suffixes())
        .setenv("PALUDIS_EBUILD_PHASE_VAR",
                eapi->ebuild_environment_variables()->env_ebuild_phase())
        .setenv("PALUDIS_EBUILD_PHASE_FUNC_VAR",
                eapi->ebuild_environment_variables()->env_ebuild_phase_func())
        .setenv("PALUDIS_PIPE_COMMANDS_SUPPORTED", "yes")
        .setenv("PALUDIS_PIPE_COMMANDS_STATUS_SUPPORTED", "yes")
        .pipe_command_handler("PALUDIS_PIPE_COMMAND",
                              std::bind(&pipe_command_handler,
                                        params.environment(),
                                        params.package_id(),
                                        nullptr,
                                        nullptr,
                                        nullptr, false, _1,
                                        params.maybe_output_manager()));

    if (! eapi->ebuild_metadata_variables()->iuse_effective()->name().empty())
        if (package_id->raw_iuse_effective_key())
        {
            auto iu(package_id->raw_iuse_effective_key()->parse_value());
            process.setenv(eapi->ebuild_metadata_variables()->iuse_effective()->name(),
                    join(iu->begin(), iu->end(), " "));
        }

    if (! eapi->ebuild_metadata_variables()->scm_revision()->name().empty())
        if (package_id->scm_revision_key())
            process.setenv(eapi->ebuild_metadata_variables()->scm_revision()->name(),
                    package_id->scm_revision_key()->parse_value());;

    if (params.maybe_output_manager())
        process
            .capture_stderr(params.maybe_output_manager()->stderr_stream())
            .capture_stdout(params.maybe_output_manager()->stdout_stream())
            .use_ptys();

    std::string defined_phases(eapi->ebuild_metadata_variables()->defined_phases()->name());
    if (! defined_phases.empty())
        if (package_id->defined_phases_key())
        {
            auto dp(package_id->defined_phases_key()->parse_value());
            process.setenv(defined_phases, join(dp->begin(), dp->end(), " "));
        }

    if (0 != process.run().wait())
        throw ActionFailedError("Write VDB Entry command failed");
}

VDBPostMergeUnmergeCommand::VDBPostMergeUnmergeCommand(const VDBPostMergeUnmergeCommandParams & p) :
    params(p)
{
}

void
VDBPostMergeUnmergeCommand::operator() ()
{
    if (! getenv_with_default("PALUDIS_NO_GLOBAL_HOOKS", "").empty())
        return;

    Process process(ProcessCommand({ getenv_with_default(env_vars::ebuild_dir, LIBEXECDIR "/paludis") + "/utils/wrapped_ldconfig",
                stringify(params.root()) }));

    if (0 != process.run().wait())
        throw ActionFailedError("VDB Entry post merge commands failed");
}

std::string
EbuildPretendCommand::commands() const
{
    return params.commands();
}

bool
EbuildPretendCommand::failure()
{
    return false;
}

void
EbuildPretendCommand::extend_command(Process & process)
{
    const auto & package_id = params.package_id();
    const auto & eapi = package_id->eapi()->supported();
    const auto & environment_variables = eapi->ebuild_environment_variables();

    process
        .extra_newlines_if_any_output_exists()
        .prefix_stdout(stringify(package_id->name().package()) + "-" +
                stringify(package_id->version()) + "> ")
        .prefix_stderr(stringify(package_id->name().package()) + "-" +
                stringify(package_id->version()) + "> ")
        .setenv("PALUDIS_EBUILD_QUIET", "yes")
        .setenv("PALUDIS_PROFILE_DIR", stringify(*pretend_params.profiles()->begin()))
        .setenv("PALUDIS_PROFILE_DIRS", join(pretend_params.profiles()->begin(),
                    pretend_params.profiles()->end(), " "))
        .setenv("PALUDIS_PROFILES_DIRS", join(pretend_params.profiles_with_parents()->begin(),
                    pretend_params.profiles_with_parents()->end(), " "));

    if (! environment_variables->env_use().empty())
        process.setenv(environment_variables->env_use(), pretend_params.use());
    if (! environment_variables->env_use_expand().empty())
        process.setenv(environment_variables->env_use_expand(), pretend_params.use_expand());
    if (! environment_variables->env_use_expand_hidden().empty())
        process.setenv(environment_variables->env_use_expand_hidden(), pretend_params.use_expand_hidden());

    if (! environment_variables->env_replacing_ids().empty())
        process.setenv(environment_variables->env_replacing_ids(),
                join(indirect_iterator(pretend_params.replacing_ids()->begin()),
                    indirect_iterator(pretend_params.replacing_ids()->end()), " "));

    if (! environment_variables->env_replacing_versions().empty())
    {
        std::string s;
        for (const auto & id : *pretend_params.replacing_ids())
        {
            if (id->name() == params.package_id()->name())
            {
                if (! s.empty())
                    s.append(" ");
                s.append(stringify(id->version()));
            }
        }

        process.setenv(environment_variables->env_replacing_versions(), s);
    }

    for (const auto & kv : *pretend_params.expand_vars())
        process.setenv(kv.first, kv.second);

    process.setuid_setgid(params.environment()->reduced_uid(), params.environment()->reduced_gid());
}

EbuildPretendCommand::EbuildPretendCommand(const EbuildCommandParams & p,
        const EbuildPretendCommandParams & f) :
    EbuildCommand(p),
    pretend_params(f)
{
}

std::string
EbuildInfoCommand::ebuild_file() const
{
    if (info_params.use_ebuild_file())
        return stringify(params.ebuild_file());
    else
        return "-";
}

std::string
EbuildInfoCommand::commands() const
{
    return params.commands();
}

bool
EbuildInfoCommand::failure()
{
    throw ActionFailedError("Info command failed");
}

void
EbuildInfoCommand::extend_command(Process & process)
{
    const auto & package_id = params.package_id();
    const auto & eapi = package_id->eapi()->supported();
    const auto & environment_variables = eapi->ebuild_environment_variables();

    std::string info_vars(join(info_params.info_vars()->begin(), info_params.info_vars()->end(), " "));

    process
        .prefix_stdout("        ")
        .prefix_stderr("        ")
        .setenv("PALUDIS_INFO_VARS", info_vars)
        .setenv("PALUDIS_PROFILE_DIR",
                info_params.profiles()->empty() ? std::string("") : stringify(*info_params.profiles()->begin()))
        .setenv("PALUDIS_PROFILE_DIRS", join(info_params.profiles()->begin(),
                    info_params.profiles()->end(), " "))
        .setenv("PALUDIS_PROFILES_DIRS", join(info_params.profiles_with_parents()->begin(),
                    info_params.profiles_with_parents()->end(), " "));

    if (! environment_variables->env_use().empty())
        process.setenv(environment_variables->env_use(), info_params.use());
    if (! environment_variables->env_use_expand().empty())
        process.setenv(environment_variables->env_use_expand(), info_params.use_expand());
    if (! environment_variables->env_use_expand_hidden().empty())
        process.setenv(environment_variables->env_use_expand_hidden(), info_params.use_expand_hidden());

    for (const auto & kv : *info_params.expand_vars())
        process.setenv(kv.first, kv.second);

    process.setuid_setgid(params.environment()->reduced_uid(), params.environment()->reduced_gid());

    if (info_params.load_environment())
        process
            .setenv("PALUDIS_LOAD_ENVIRONMENT", stringify(*info_params.load_environment()))
            .setenv("PALUDIS_SKIP_INHERIT", "yes");
}

EbuildInfoCommand::EbuildInfoCommand(const EbuildCommandParams & p,
        const EbuildInfoCommandParams & f) :
    EbuildCommand(p),
    info_params(f)
{
}

WriteBinaryEbuildCommand::WriteBinaryEbuildCommand(const WriteBinaryEbuildCommandParams & p) :
    params(p)
{
}

void
WriteBinaryEbuildCommand::operator() ()
{
    using namespace std::placeholders;

    const auto & package_id = params.package_id();
    const auto & eapi = package_id->eapi()->supported();

    if (! EAPIData::get_instance()->eapi_from_string("pbin-1+" + package_id->eapi()->exported_name())->supported())
        throw ActionFailedError("Don't know how to write binary ebuilds using EAPI 'pbin-1+" +
                package_id->eapi()->exported_name());

    std::string ebuild_cmd(getenv_with_default(env_vars::ebuild_dir, LIBEXECDIR "/paludis") +
            "/write_binary_ebuild.bash '" +
            stringify(params.binary_ebuild_location()) + "' '" +
            stringify(params.binary_distdir() / params.binary_dist_base()) + "' '" +
            stringify(params.environment_file()) + "' '" +
            stringify(params.image()) + "'");

    std::shared_ptr<const FSPathSequence> syncers_dirs(params.environment()->syncers_dirs());
    std::shared_ptr<const FSPathSequence> bashrc_files(params.environment()->bashrc_files());
    std::shared_ptr<const FSPathSequence> fetchers_dirs(params.environment()->fetchers_dirs());
    std::shared_ptr<const FSPathSequence> hook_dirs(params.environment()->hook_dirs());

    Process process((ProcessCommand(ebuild_cmd)));

    process
        .setenv("PKGMANAGER", PALUDIS_PACKAGE "-" + stringify(PALUDIS_VERSION_MAJOR) + "." +
                stringify(PALUDIS_VERSION_MINOR) + "." +
                stringify(PALUDIS_VERSION_MICRO) +
                (std::string(PALUDIS_GIT_HEAD).empty() ?
                 std::string("") : "-git-" + std::string(PALUDIS_GIT_HEAD)))
        .setenv("EAPI", stringify(package_id->eapi()->exported_name()))
        .setenv("PALUDIS_CONFIG_DIR", SYSCONFDIR "/paludis/")
        .setenv("PALUDIS_TMPDIR", stringify(params.builddir()))
        .setenv("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "))
        .setenv("PALUDIS_HOOK_DIRS", join(hook_dirs->begin(), hook_dirs->end(), " "))
        .setenv("PALUDIS_FETCHERS_DIRS", join(fetchers_dirs->begin(), fetchers_dirs->end(), " "))
        .setenv("PALUDIS_SYNCERS_DIRS", join(syncers_dirs->begin(), syncers_dirs->end(), " "))
        .setenv("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
        .setenv("PALUDIS_EBUILD_DIR", getenv_with_default(env_vars::ebuild_dir, LIBEXECDIR "/paludis"))
        .setenv("PALUDIS_BINARY_FROM_ENV_VARIABLES",
                eapi->ebuild_options()->binary_from_env_variables())
        .setenv("PALUDIS_F_FUNCTION_PREFIX",
                eapi->ebuild_options()->f_function_prefix())
        .setenv("PALUDIS_IGNORE_PIVOT_ENV_FUNCTIONS",
                eapi->ebuild_options()->ignore_pivot_env_functions())
        .setenv("PALUDIS_IGNORE_PIVOT_ENV_VARIABLES",
                eapi->ebuild_options()->ignore_pivot_env_variables())
        .setenv("PALUDIS_BINARY_URI_PREFIX", params.destination_repository()->params().binary_uri_prefix())
        .setenv("PALUDIS_BINARY_KEYWORDS", params.binary_keywords())
        .setenv("PALUDIS_BINARY_KEYWORDS_VARIABLE", EAPIData::get_instance()->eapi_from_string("pbin-1+"
                    + package_id->eapi()->exported_name())->supported()->ebuild_metadata_variables()->keywords()->name())
        .setenv("PALUDIS_BINARY_DISTDIR_VARIABLE", EAPIData::get_instance()->eapi_from_string("pbin-1+"
                    + package_id->eapi()->exported_name())->supported()->ebuild_environment_variables()->env_distdir())
        .setenv("PALUDIS_BINARY_URI_EXTENSION", params.binary_uri_extension())
        .setenv("PALUDIS_EBUILD_MODULE_SUFFIXES",
                eapi->ebuild_options()->ebuild_module_suffixes())
        .setenv("PALUDIS_EBUILD_PHASE_VAR",
                eapi->ebuild_environment_variables()->env_ebuild_phase())
        .setenv("PALUDIS_EBUILD_PHASE_FUNC_VAR",
                eapi->ebuild_environment_variables()->env_ebuild_phase_func())
        .setenv("PALUDIS_PIPE_COMMANDS_SUPPORTED", "yes")
        .setenv("PALUDIS_PIPE_COMMANDS_STATUS_SUPPORTED", "yes")
        .pipe_command_handler("PALUDIS_PIPE_COMMAND",
                              std::bind(&pipe_command_handler,
                                        params.environment(),
                                        package_id,
                                        nullptr,
                                        nullptr,
                                        nullptr,
                                        false, _1,
                                        params.maybe_output_manager()));

    if (! eapi->ebuild_metadata_variables()->scm_revision()->name().empty())
        if (package_id->scm_revision_key())
            process.setenv(eapi->ebuild_metadata_variables()->scm_revision()->name(),
                    package_id->scm_revision_key()->parse_value());;

    if (0 != process.run().wait())
        throw ActionFailedError("Write binary command failed");
}

std::string
EbuildBadOptionsCommand::commands() const
{
    return params.commands();
}

bool
EbuildBadOptionsCommand::failure()
{
    return false;
}

void
EbuildBadOptionsCommand::extend_command(Process & process)
{
    const auto & package_id = params.package_id();
    const auto & eapi = package_id->eapi()->supported();
    const auto & environment_variables = eapi->ebuild_environment_variables();

    process
        .setenv("PALUDIS_EBUILD_QUIET", "yes")
        .setenv("PALUDIS_PROFILE_DIR", stringify(*bad_options_params.profiles()->begin()))
        .setenv("PALUDIS_PROFILE_DIRS", join(bad_options_params.profiles()->begin(),
                    bad_options_params.profiles()->end(), " "))
        .setenv("PALUDIS_PROFILES_DIRS", join(bad_options_params.profiles_with_parents()->begin(),
                    bad_options_params.profiles_with_parents()->end(), " "))
        .setenv("EX_UNMET_REQUIREMENTS", join(bad_options_params.unmet_requirements()->begin(),
                    bad_options_params.unmet_requirements()->end(), "\n"));

    if (! environment_variables->env_use().empty())
        process.setenv(environment_variables->env_use(), bad_options_params.use());
    if (! environment_variables->env_use_expand().empty())
        process.setenv(environment_variables->env_use_expand(), bad_options_params.use_expand());
    if (! environment_variables->env_use_expand_hidden().empty())
        process.setenv(environment_variables->env_use_expand_hidden(), bad_options_params.use_expand_hidden());

    for (const auto & kv : *bad_options_params.expand_vars())
        process.setenv(kv.first, kv.second);

    process.setuid_setgid(params.environment()->reduced_uid(), params.environment()->reduced_gid());
}

EbuildBadOptionsCommand::EbuildBadOptionsCommand(const EbuildCommandParams & p,
        const EbuildBadOptionsCommandParams & f) :
    EbuildCommand(p),
    bad_options_params(f)
{
}

std::string
EbuildFetchExtraCommand::commands() const
{
    return params.commands();
}

bool
EbuildFetchExtraCommand::failure()
{
    throw ActionFailedError("Extra fetch failed for '" + stringify(*params.package_id()) + "'");
}

void
EbuildFetchExtraCommand::extend_command(Process & process)
{
    const auto & package_id = params.package_id();
    const auto & eapi = package_id->eapi()->supported();
    const auto & environment_variables = eapi->ebuild_environment_variables();

    process
        .setenv("PALUDIS_LOADSAVEENV_DIR", stringify(fetch_extra_params.loadsaveenv_dir()))
        .setenv("PALUDIS_PROFILE_DIR", stringify(*fetch_extra_params.profiles()->begin()))
        .setenv("PALUDIS_PROFILE_DIRS", join(fetch_extra_params.profiles()->begin(),
                    fetch_extra_params.profiles()->end(), " "))
        .setenv("PALUDIS_PROFILES_DIRS", join(fetch_extra_params.profiles_with_parents()->begin(),
                    fetch_extra_params.profiles_with_parents()->end(), " "))
        .setenv("PALUDIS_ARCHIVES_VAR", environment_variables->env_a())
        .setenv("SLOT", stringify(fetch_extra_params.slot()));

    if (! environment_variables->env_a().empty())
        process.setenv(environment_variables->env_a(), fetch_extra_params.a());
    if (! environment_variables->env_aa().empty())
        process.setenv(environment_variables->env_aa(), fetch_extra_params.aa());
    if (! environment_variables->env_use().empty())
        process.setenv(environment_variables->env_use(), fetch_extra_params.use());
    if (! environment_variables->env_use_expand().empty())
        process.setenv(environment_variables->env_use_expand(), fetch_extra_params.use_expand());
    if (! environment_variables->env_use_expand_hidden().empty())
        process.setenv(environment_variables->env_use_expand_hidden(), fetch_extra_params.use_expand_hidden());

    for (const auto & kv : *fetch_extra_params.expand_vars())
        process.setenv(kv.first, kv.second);
}

EbuildFetchExtraCommand::EbuildFetchExtraCommand(const EbuildCommandParams & p,
        const EbuildFetchExtraCommandParams & f) :
    EbuildCommand(p),
    fetch_extra_params(f)
{
}

