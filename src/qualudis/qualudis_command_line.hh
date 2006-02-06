/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PALUDIS_GUARD_SRC_QUALUDIS_QUALUDIS_COMMAND_LINE_HH
#define PALUDIS_GUARD_SRC_QUALUDIS_QUALUDIS_COMMAND_LINE_HH 1

#include <paludis/args/args.hh>
#include <paludis/instantiation_policy.hh>

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
        /// \name Action arguments
        ///{

        /// Action arguments.
        paludis::args::ArgsGroup action_args;

        /// --check
        paludis::args::SwitchArg a_check;

        /// --version
        paludis::args::SwitchArg a_version;

        /// --help
        paludis::args::SwitchArg a_help;

        ///}
};


#endif
