/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2011 Ingmar Vanhassel
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

#ifndef PALUDIS_GUARD_PALUDIS_ARGS_MAN_HH
#define PALUDIS_GUARD_PALUDIS_ARGS_MAN_HH 1

#include <paludis/args/args_handler.hh>
#include <iosfwd>

namespace paludis
{
    namespace args
    {
        class DocWriter;

        /**
         * Write docs to an ostream.
         *
         * \ingroup grplibpaludisargs
         */
        void generate_doc(DocWriter & dw, const ArgsHandler * const h) PALUDIS_VISIBLE;

        /**
         * Write docs from args classes in a particular format.
         *
         * \ingroup grplibpaludisargs
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE DocWriter
        {
            public:
                ///\name Basic operations
                ///\{

                virtual ~DocWriter() = 0;

                ///\}

                ///\name Output routines
                ///\{

                virtual void heading(const std::string & name, const std::string & section,
                        const std::string & synopsis) = 0;
                virtual void start_usage_lines() = 0;
                virtual void usage_line(const std::string & name, const std::string & line) = 0;

                virtual void start_description(const std::string & description) = 0;
                virtual void extra_description(const std::string & description) = 0;
                virtual void end_description() = 0;

                virtual void start_options(const std::string &) = 0;
                virtual void start_arg_group(const std::string & name, const std::string & description) = 0;
                virtual void arg_group_item(const char & short_name, const std::string & long_name,
                        const std::string & negated_long_name, const std::string & description) = 0;
                virtual void start_extra_arg() = 0;
                virtual void extra_arg_enum(const AllowedEnumArg &, const std::string & default_arg) = 0;
                virtual void extra_arg_string_set(const std::string & first, const std::string & second) = 0;
                virtual void end_extra_arg() = 0;
                virtual void end_arg_group() = 0;
                virtual void end_options() = 0;

                virtual void start_environment() = 0;
                virtual void environment_line(const std::string & first, const std::string & second) = 0;
                virtual void end_environment() = 0;

                virtual void start_examples() = 0;
                virtual void example(const std::string &, const std::string &) = 0;
                virtual void end_examples() = 0;

                virtual void start_notes() = 0;
                virtual void note(const std::string &) = 0;
                virtual void end_notes() = 0;

                virtual void section(const std::string & title) = 0;
                virtual void subsection(const std::string & title) = 0;
                virtual void paragraph(const std::string & text) = 0;

                virtual void start_see_alsos() = 0;
                virtual void see_also(const std::string &, const int, const bool first) = 0;
                virtual void end_see_alsos() = 0;

                ///\}
        };

        /**
         * Create Asciidoc documentation from args classes.
         *
         * \ingroup grplibpaludisargs
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE AsciidocWriter :
            public DocWriter
        {
            private:
                std::ostream & _os;

            public:
                ///\name Basic operations
                ///\{
                AsciidocWriter(std::ostream & os);
                ~AsciidocWriter() = default;
                ///\}

                void heading(const std::string & name, const std::string & section,
                        const std::string & synopis);
                void start_usage_lines();
                void usage_line(const std::string & name, const std::string & line);

                void start_description(const std::string & description);
                void extra_description(const std::string & description);
                void end_description();

                void start_options(const std::string & s);
                void start_arg_group(const std::string & name, const std::string & description);
                void arg_group_item(const char & short_name, const std::string & long_name,
                        const std::string & negated_long_name, const std::string & description);
                void start_extra_arg();
                void extra_arg_enum(const AllowedEnumArg &, const std::string & default_arg);
                void extra_arg_string_set(const std::string & first, const std::string & second);
                void end_extra_arg();
                void end_arg_group();
                void end_options();

                void start_environment();
                void environment_line(const std::string & first, const std::string & second);
                void end_environment();

                void start_examples();
                void example(const std::string &, const std::string &);
                void end_examples();

                void start_notes();
                void note(const std::string &);
                void end_notes();

                void section(const std::string & title);
                void subsection(const std::string & title);
                void paragraph(const std::string & text);

                void start_see_alsos();
                void see_also(const std::string &, const int, const bool first);
                void end_see_alsos();
        };
    }
}

#endif
