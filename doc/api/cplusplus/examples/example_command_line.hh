/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PALUDIS_GUARD_DOC_EXAMPLES_EXAMPLE_COMMAND_LINE_HH
#define PALUDIS_GUARD_DOC_EXAMPLES_EXAMPLE_COMMAND_LINE_HH 1

#include <paludis/args/args.hh>
#include <paludis/args/log_level_arg.hh>
#include <paludis/paludis.hh>

/** \file
 *
 * Basic command line handling for most examples.
 */

namespace examples
{
    /**
     * This class provides basic command line handling for most examples.
     *
     * Most Paludis clients should support at least '--help', '--version'
     * and '--log-level'. If paludis::EnvironmentFactory is used to create
     * the environment then '--environment' must also be an option.
     *
     * Clients are free to use whichever command line handling library they
     * prefer, but for convenience all Paludis core clients use a common utility
     * library with a much simpler interface than that provided by overly C-ish
     * getopt derivatives.
     *
     * The command line is a singleton -- that is, only one instance of
     * it exists globally. This avoids the need to pass around lots of
     * parameters.
     */
    class CommandLine :
        public paludis::args::ArgsHandler,
        public paludis::Singleton<CommandLine>
    {
        friend class paludis::Singleton<CommandLine>;

        private:
            CommandLine();
            ~CommandLine() override;

        public:
            virtual void run(const int, const char * const * const,
                    const std::string & client, const std::string & env_var,
                    const std::string & env_prefix);

            ///\name Program information
            ///\{

            std::string app_name() const override;
            std::string app_synopsis() const override;
            std::string app_description() const override;

            ///\}

            ///\name Action arguments
            ///\{

            paludis::args::ArgsGroup action_args;
            paludis::args::SwitchArg a_version;
            paludis::args::SwitchArg a_help;

            ///\}

            ///\name General arguments
            ///{

            paludis::args::ArgsGroup general_args;
            paludis::args::LogLevelArg a_log_level;
            paludis::args::StringArg a_environment;

            ///}
    };

    /**
     * Show a '--help' message, and exit.
     */
    void show_help_and_exit(const char * const argv[]) PALUDIS_ATTRIBUTE((noreturn));

    /**
     * Show a '--version' message, and exit.
     */
    void show_version_and_exit() PALUDIS_ATTRIBUTE((noreturn));
}

#endif
