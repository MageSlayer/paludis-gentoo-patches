/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PALUDIS_GUARD_SRC_QUALUDIS_QUALUDIS_COMMAND_LINE_HH
#define PALUDIS_GUARD_SRC_QUALUDIS_QUALUDIS_COMMAND_LINE_HH 1

#include <paludis/args/args.hh>
#include <paludis/qa-fwd.hh>
#include <paludis/util/instantiation_policy.hh>
#include <src/common_args/log_level_arg.hh>

class QualudisCommandLine :
    public paludis::args::ArgsHandler,
    public paludis::InstantiationPolicy<QualudisCommandLine, paludis::instantiation_method::SingletonTag>
{
    friend class paludis::InstantiationPolicy<QualudisCommandLine, paludis::instantiation_method::SingletonTag>;

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

        /// --log-level
        paludis::args::LogLevelArg a_log_level;

        /// --message-level
        paludis::args::EnumArg a_message_level;

        paludis::QAMessageLevel message_level;

        ///\}

        ///\name Configuration options
        ///\{

        paludis::args::ArgsGroup configuration_options;

        paludis::args::StringArg a_write_cache_dir;
        paludis::args::StringArg a_master_repository_dir;

        ///\}
};


#endif
