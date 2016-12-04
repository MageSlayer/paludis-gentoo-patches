/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_ARGS_ARGS_HANDLER_HH
#define PALUDIS_GUARD_ARGS_ARGS_HANDLER_HH 1

#include <paludis/args/args_section.hh>
#include <paludis/args/args_group.hh>
#include <paludis/util/iterator_range.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/options.hh>
#include <paludis/util/sequence.hh>
#include <memory>
#include <iosfwd>
#include <string>

/** \file
 * Declarations for ArgsHandler.
 *
 * \ingroup g_args
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    namespace args
    {
        class ArgsOption;

#include <paludis/args/args_handler-se.hh>

        typedef Options<ArgsHandlerOption> ArgsHandlerOptions;

        /**
         * Handles command line arguments.
         *
         * \ingroup g_args
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE ArgsHandler
        {
            friend class ArgsSection;
            friend std::ostream & operator<< (std::ostream &, const ArgsHandler &);

            private:
                Pimp<ArgsHandler> _imp;

            protected:
                /**
                 * Add a new usage line.
                 */
                void add_usage_line(const std::string & l);

                /**
                 * Add a new environment line.
                 */
                void add_environment_variable(const std::string & e, const std::string & desc);

                /**
                 * Add a new example.
                 */
                void add_example(const std::string & e, const std::string & desc);

                /**
                 * Add a new note.
                 */
                void add_note(const std::string &);

                /**
                 * Add a new description.
                 */
                void add_description_line(const std::string & l);

                /**
                 * Add a 'see also' item.
                 *
                 * \since 0.48.2
                 */
                void add_see_also(const std::string &, int section);

                /**
                 * Add an new ArgsSection (called by the ArgsSection constructor).
                 */
                void add(ArgsSection * const);

                /**
                 * Dump, for --help output (called by operator<<).
                 */
                void dump_to_stream(std::ostream & s) const;

                /**
                 * Called after run(), for convenience. Does nothing.
                 */
                virtual void post_run();

            public:
                ///\name Basic operations
                ///\{

                ArgsHandler();

                virtual ~ArgsHandler();

                ArgsHandler(const ArgsHandler &) = delete;

                ArgsHandler & operator= (const ArgsHandler &) = delete;

                ///\}

                ///\name Iterate over our parameters (non - and -- switches and their values)
                ///\{

                struct ParametersConstIteratorTag;
                typedef WrappedForwardIterator<ParametersConstIteratorTag, const std::string> ParametersConstIterator;

                ParametersConstIterator begin_parameters() const;
                ParametersConstIterator end_parameters() const;

                IteratorRange<ParametersConstIterator> parameters() const {
                    return make_range(begin_parameters(), end_parameters());
                }

                bool empty() const;

                /**
                 * If aho_separate_after_dashes, everything after a -- goes
                 * here.
                 *
                 * \since 0.47
                 */
                const std::shared_ptr<const Sequence<std::string> > separate_after_dashes_args() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                ///\}

                /**
                 * Add an ArgsOption instance.
                 */
                void add_option(ArgsOption * const, const std::string & long_name,
                        const char short_name = '\0');

                /**
                 * Remove an ArgsOption instance.
                 */
                void remove_option(const std::string & long_name, const char short_name = '\0');

                ///\name About our application (for documentation)
                ///\{

                /**
                 * What is our application name?
                 */
                virtual std::string app_name() const = 0;

                /**
                 * What is our application's Unix manual section?
                 */
                virtual std::string man_section() const
                {
                    return "1";
                }

                /**
                 * One line synopsis of what our application is.
                 */
                virtual std::string app_synopsis() const = 0;

                /**
                 * Long description of what our application is.
                 */
                virtual std::string app_description() const = 0;

                ///\}

                ///\name Iterate over our usage lines (for documentation)
                ///\{

                struct UsageLineConstIteratorTag;
                typedef WrappedForwardIterator<UsageLineConstIteratorTag, const std::string> UsageLineConstIterator;

                UsageLineConstIterator begin_usage_lines() const;
                UsageLineConstIterator end_usage_lines() const;

                IteratorRange<UsageLineConstIterator> usage_lines() const {
                    return make_range(begin_usage_lines(), end_usage_lines());
                }

                ///\}

                ///\name Iterate over our environment lines (for documentation)
                ///\{

                struct EnvironmentLineConstIteratorTag;
                typedef WrappedForwardIterator<EnvironmentLineConstIteratorTag,
                        const std::pair<std::string, std::string> > EnvironmentLineConstIterator;

                EnvironmentLineConstIterator begin_environment_lines() const;
                EnvironmentLineConstIterator end_environment_lines() const;

                IteratorRange<EnvironmentLineConstIterator> environment_lines() const {
                    return make_range(begin_environment_lines(), end_environment_lines());
                }

                ///\}

                ///\name Iterate over our examples (for documentation)
                ///\{

                struct ExamplesConstIteratorTag;
                typedef WrappedForwardIterator<ExamplesConstIteratorTag,
                        const std::pair<std::string, std::string> > ExamplesConstIterator;

                ExamplesConstIterator begin_examples() const;
                ExamplesConstIterator end_examples() const;

                IteratorRange<ExamplesConstIterator> examples() const {
                    return make_range(begin_examples(), end_examples());
                }

                ///\}

                ///\name Iterate over our sections
                ///\{

                struct ArgsSectionsConstIteratorTag;
                typedef WrappedForwardIterator<ArgsSectionsConstIteratorTag, const ArgsSection> ArgsSectionsConstIterator;

                ArgsSectionsConstIterator begin_args_sections() const;
                ArgsSectionsConstIterator end_args_sections() const;

                IteratorRange<ArgsSectionsConstIterator> args_sections() const {
                    return make_range(begin_args_sections(), end_args_sections());
                }

                /**
                 * The 'Options' section.
                 *
                 * Created if it does not exist.
                 */
                ArgsSection * main_options_section() PALUDIS_ATTRIBUTE((warn_unused_result));

                ///\}

                ///\name Iterate over our notes
                ///\{

                struct NotesIteratorTag;
                typedef WrappedForwardIterator<NotesIteratorTag, const std::string > NotesIterator;

                NotesIterator begin_notes() const;
                NotesIterator end_notes() const;

                IteratorRange<NotesIterator> notes() const {
                    return make_range(begin_notes(), end_notes());
                }

                ///\}

                ///\name Iterate over our extra description lines (for documentation)
                ///\{

                struct DescriptionLineConstIteratorTag;
                typedef WrappedForwardIterator<DescriptionLineConstIteratorTag, const std::string> DescriptionLineConstIterator;

                DescriptionLineConstIterator begin_description_lines() const;
                DescriptionLineConstIterator end_description_lines() const;

                IteratorRange<DescriptionLineConstIterator> description_lines() const {
                    return make_range(begin_description_lines(), end_description_lines());
                }

                ///\}

                ///\name Iterate over our 'see also' lines
                ///\since 0.48.2
                ///\{

                struct SeeAlsoConstIteratorTag;
                typedef WrappedForwardIterator<SeeAlsoConstIteratorTag, const std::pair<std::string, int> > SeeAlsoConstIterator;

                SeeAlsoConstIterator begin_see_alsos() const;
                SeeAlsoConstIterator end_see_alsos() const;

                IteratorRange<SeeAlsoConstIterator> see_alsos() const {
                    return make_range(begin_see_alsos(), end_see_alsos());
                }

                ///\}

                ///\name For use by ArgsVisitor
                ///\{

                struct ArgsIteratorTag;
                typedef WrappedForwardIterator<ArgsIteratorTag, std::string> ArgsIterator;

                ///\}

                /**
                 * Parse command line arguments. The third argument is used to
                 * set PALUDIS_CLIENT.  The fourth argument is the name of an
                 * environment variable holding arguments which are prepended
                 * to the command line arguments. The fifth argument is used as
                 * a prefix to export our command line via the environment.
                 */
                void run(
                        const int argc,
                        const char * const * const argv,
                        const std::string & client,
                        const std::string & env_var,
                        const std::string & env_prefix,
                        const ArgsHandlerOptions & options = ArgsHandlerOptions());

                /**
                 * Parse command line arguments. The third argument is used to
                 * set PALUDIS_CLIENT.  The fourth argument is the name of an
                 * environment variable holding arguments which are prepended
                 * to the command line arguments. The fifth argument is used as
                 * a prefix to export our command line via the environment.
                 */
                void run(
                        const std::shared_ptr<const Sequence<std::string> > &,
                        const std::string & client,
                        const std::string & env_var,
                        const std::string & env_prefix,
                        const ArgsHandlerOptions & options = ArgsHandlerOptions());
        };

        /**
         * Output an ArgsHandler to an ostream, for --help output.
         *
         * \ingroup g_args
         */
        std::ostream & operator<< (std::ostream &, const ArgsHandler &) PALUDIS_VISIBLE;
    }

    extern template class WrappedForwardIterator<args::ArgsHandler::ParametersConstIteratorTag, const std::string>;
    extern template class WrappedForwardIterator<args::ArgsHandler::UsageLineConstIteratorTag, const std::string>;
    extern template class WrappedForwardIterator<args::ArgsHandler::EnvironmentLineConstIteratorTag, const std::pair<std::string, std::string>>;
    extern template class WrappedForwardIterator<args::ArgsHandler::ExamplesConstIteratorTag, const std::pair<std::string, std::string>>;
    extern template class WrappedForwardIterator<args::ArgsHandler::ArgsSectionsConstIteratorTag, const args::ArgsSection>;
    extern template class WrappedForwardIterator<args::ArgsHandler::DescriptionLineConstIteratorTag, const std::string>;
    extern template class WrappedForwardIterator<args::ArgsHandler::NotesIteratorTag, const std::string>;
    extern template class WrappedForwardIterator<args::ArgsHandler::ArgsIteratorTag, std::string>;
    extern template class WrappedForwardIterator<args::ArgsHandler::SeeAlsoConstIteratorTag, const std::pair<std::string, int> >;
}

#endif
