/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PALUDIS_GUARD_SRC_CONTRARIUS_COMMAND_LINE_HH
#define PALUDIS_GUARD_SRC_CONTRARIUS_COMMAND_LINE_HH 1

#include <paludis/args/args.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/args/log_level_arg.hh>

class CommandLine :
    public paludis::args::ArgsHandler,
    public paludis::InstantiationPolicy<CommandLine, paludis::instantiation_method::SingletonTag>
{
    friend class paludis::InstantiationPolicy<CommandLine, paludis::instantiation_method::SingletonTag>;

    private:
        /// Constructor.
        CommandLine();

        /// Destructor.
        ~CommandLine();

    public:
        ///\name Program information
        ///\{

        virtual std::string app_name() const;
        virtual std::string app_synopsis() const;
        virtual std::string app_description() const;

        ///\}

        ///\name Action arguments
        ///\{

        /// Action arguments.
        paludis::args::ArgsGroup action_args;

        /// --version
        paludis::args::SwitchArg a_version;

        /// --help
        paludis::args::SwitchArg a_help;

        ///\}

        ///\name Build arguments
        ///\{

        /// Build arguments.
        paludis::args::ArgsGroup build_args;

        /// --fetch
        paludis::args::SwitchArg a_fetch;

        /// --pretend
        paludis::args::SwitchArg a_pretend;

        /// --environment
        paludis::args::StringArg a_environment;

        /// --show-reasons
        paludis::args::EnumArg a_show_reasons;

        /// --stage
        paludis::args::EnumArg a_stage;

        /// --target
        paludis::args::StringArg a_target;

        /// --headers \todo Find a better name!
        paludis::args::SwitchArg a_headers;

        /// --always-rebuild
        paludis::args::SwitchArg a_always_rebuild;

        /// --debug-build
        paludis::args::EnumArg a_debug_build;

        ///\}

        ///\name Output options
        ///\{

        /// Output options.
        paludis::args::ArgsGroup output_options;

        /// --verbose
        paludis::args::SwitchArg a_verbose;

        /// --log-level
        paludis::args::LogLevelArg a_log_level;

        /// --no-colour
        paludis::args::SwitchArg a_no_colour;

        /// --no-color
        paludis::args::AliasArg a_no_color;

        /// --resume-command-template
        paludis::args::StringArg a_resume_command_template;
        ///\}
};

/**
 * Show the help message.
 */
struct DoHelp
{
    const std::string message;

    DoHelp(const std::string & m = "") :
        message(m)
    {
    }
};

#endif
