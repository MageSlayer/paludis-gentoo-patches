/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_CONFIG_FILE_HH
#define PALUDIS_GUARD_PALUDIS_CONFIG_FILE_HH 1

#include <paludis/util/config_file-fwd.hh>
#include <paludis/util/map-fwd.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/options-fwd.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/fs_path-fwd.hh>

#include <iosfwd>
#include <string>
#include <memory>
#include <functional>

/** \file
 * Declarations for the ConfigFile classes.
 *
 * \ingroup g_config_file
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Raised if an error is encountered parsing a configuration file.
     *
     * \ingroup g_config_file
     * \ingroup g_exceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ConfigFileError :
        public ConfigurationError
    {
        public:
            ///\name Basic operations
            ///\{

            /**
             * Constructor, where the filename is known.
             *
             * \param filename The filename in which the error occurred. May be a blank string, if
             *   the filename is not necessarily known.
             * \param message A description of the error.
             */
            ConfigFileError(const std::string & filename, const std::string & message) noexcept;

            /**
             * Constructor, where the filename is not known.
             *
             * \param message A description of the error.
             */
            ConfigFileError(const std::string & message) noexcept;

            ///\}
    };

    /**
     * Base class for configuration files.
     *
     * Data is read in from a ConfigFile::Source instance, which can be created from
     * an FSPath, a std::istream or a string.
     *
     * \see KeyValueConfigFile
     * \see LineConfigFile
     *
     * \ingroup g_config_file
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ConfigFile
    {
        public:
            /**
             * A source (for example, a file or a string) usable by ConfigFile.
             *
             * \see ConfigFile
             * \ingroup g_config_file
             * \nosubgrouping
             */
            class PALUDIS_VISIBLE Source
            {
                private:
                    Pimp<Source> _imp;

                public:
                    ///\name Basic operations
                    ///\{

                    Source(const FSPath &);
                    Source(const std::string &);
                    Source(std::istream &);

                    Source(const Source &);
                    const Source & operator= (const Source &);

                    ~Source();

                    ///\}

                    /**
                     * Our text, for use by ConfigFile.
                     */
                    const std::string & text() const PALUDIS_ATTRIBUTE((warn_unused_result));

                    /**
                     * Our filename (may be empty), for use by ConfigFile.
                     */
                    const std::string & filename() const PALUDIS_ATTRIBUTE((warn_unused_result));
            };

            ///\name Basic operations
            ///\{

            virtual ~ConfigFile() = 0;

            ///\}
    };

    /**
     * A line-based configuration file.
     *
     * Various syntax options are available, and are controlled by LineConfigFileOptions:
     *
     * - Unless lcfo_disallow_continuations, lines ending in a backslash are continuations
     * - Unless lcfo_disallow_comments, lines starting with a \# are comments
     * - Unless lcfo_preserve_whitespace, leading and trailing whitespace is stripped
     * - Unless lcfo_no_skip_blank_lines, blank lines are skipped.
     *
     * \ingroup g_config_file
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE LineConfigFile :
        public ConfigFile
    {
        private:
            Pimp<LineConfigFile> _imp;

        public:
            ///\name Basic operations
            ///\{

            LineConfigFile(const Source &, const LineConfigFileOptions &);

            ~LineConfigFile();

            ///\}

            ///\name Iterate over our lines
            ///\{

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag, const std::string> ConstIterator;

            ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
            ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}
    };

    /**
     * A key=value configuration file.
     *
     * Various syntax options are available, and are controlled by KeyValueConfigFileOptions:
     *
     * - Unless kvcfo_disallow_continuations, line continuations via backslashes are allowed.
     * - Unless kvcfo_disallow_comments, comments start with a \#.
     * - Unless kvcfo_disallow_space_around_equals, the equals sign can have surrounding whitespace.
     * - Unless kvcfo_disallow_space_inside_unquoted_values, foo = bar baz is legal.
     * - Unless kvcfo_disallow_single_quoted_strings, single quoted strings are legal.
     * - Unless kvcfo_disallow_double_quoted_strings, double quoted strings are legal.
     * - Unless kvcfo_disallow_unquoted_values, unquoted values are legal.
     * - Unless kvcfo_disallow_variables, variables using $foo and ${foo} are expanded.
     * - Unless kvcfo_disallow_source, source path is legal.
     * - Unless kvcfo_preserve_whitespace, leading and trailing whitespace on values is stripped.
     * - If kvcfo_allow_sections, sections in the form "[foo]" and "[foo bar]" are allowed. A key
     *       'baz' in section "[foo]" will be treated as "foo/baz", and "[foo bar]" as "foo/bar/baz".
     *
     * \ingroup g_config_file
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE KeyValueConfigFile :
        public ConfigFile
    {
        private:
            Pimp<KeyValueConfigFile> _imp;

        public:
            typedef std::function<std::string (const KeyValueConfigFile &, const std::string &)> DefaultFunction;
            typedef std::function<std::string (const KeyValueConfigFile &,
                    const std::string & var, const std::string & old_value, const std::string & new_value)> TransformationFunction;

            static std::string no_defaults(const KeyValueConfigFile &, const std::string &);
            static std::string no_transformation(const KeyValueConfigFile &, const std::string &, const std::string &, const std::string &);

            ///\name Basic operations
            ///\{

            /**
             * Constructor.
             *
             * \since 0.28
             */
            KeyValueConfigFile(
                    const Source &,
                    const KeyValueConfigFileOptions &,
                    const DefaultFunction &,
                    const TransformationFunction &
                    );

            ~KeyValueConfigFile();

            ///\}

            ///\name Iterate over our keys
            ///\{

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag,
                    const std::pair<const std::string, std::string> > ConstIterator;
            ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
            ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            /**
             * Fetch the value for a particular key.
             */
            std::string get(const std::string &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            const KeyValueConfigFileOptions & options() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const DefaultFunction & default_function() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const TransformationFunction & transformation_function() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    extern template class Pimp<ConfigFile::Source>;
    extern template class Pimp<LineConfigFile>;
    extern template class Pimp<KeyValueConfigFile>;

    extern template class PALUDIS_VISIBLE WrappedForwardIterator<LineConfigFile::ConstIteratorTag, const std::string>;
    extern template class PALUDIS_VISIBLE WrappedForwardIterator<KeyValueConfigFile::ConstIteratorTag, const std::pair<const std::string, std::string> >;
}

#endif
