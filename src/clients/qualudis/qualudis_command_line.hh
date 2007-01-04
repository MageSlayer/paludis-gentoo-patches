/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PALUDIS_GUARD_SRC_QUALUDIS_QUALUDIS_COMMAND_LINE_HH
#define PALUDIS_GUARD_SRC_QUALUDIS_QUALUDIS_COMMAND_LINE_HH 1

#include <paludis/args/args.hh>
#include <paludis/qa/message.hh>
#include <paludis/util/instantiation_policy.hh>
#include <src/common_args/log_level_arg.hh>

class QualudisCommandLine :
    public paludis::args::ArgsHandler,
    public paludis::InstantiationPolicy<QualudisCommandLine, paludis::instantiation_method::SingletonAsNeededTag>
{
    friend class paludis::InstantiationPolicy<QualudisCommandLine, paludis::instantiation_method::SingletonAsNeededTag>;

    private:
        /// Constructor.
        QualudisCommandLine();

        /// Destructor.
        ~QualudisCommandLine();

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

        /// --describe
        paludis::args::SwitchArg a_describe;

        /// --version
        paludis::args::SwitchArg a_version;

        /// --help
        paludis::args::SwitchArg a_help;

        ///\}

        ///\name Check options
        ///\{

        /// Check options.
        paludis::args::ArgsGroup check_options;

        /// --qa-checks
        paludis::args::StringSetArg a_qa_checks;

        /// --exclude-qa-checks
        paludis::args::StringSetArg a_exclude_qa_checks;

        /// --archs
        paludis::args::StringSetArg a_archs;

        /// --exclude-archs
        paludis::args::StringSetArg a_exclude_archs;

        /// --verbose
        paludis::args::SwitchArg a_verbose;

        /// --quiet
        paludis::args::SwitchArg a_quiet;

        /// --log-level
        paludis::args::LogLevelArg a_log_level;

        /// --message-level
        paludis::args::EnumArg a_message_level;

        paludis::qa::QALevel message_level;

        ///\}

        ///\name Configuration options
        ///\{

        paludis::args::ArgsGroup configuration_options;

        paludis::args::StringArg a_write_cache_dir;

        ///\}
};


#endif
