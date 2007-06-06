/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/util/collection.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/options.hh>

#include <libwrapiter/libwrapiter_forward_iterator-fwd.hh>

#include <iosfwd>
#include <string>
#include <paludis/util/tr1_memory.hh>

/** \file
 * Declarations for the ConfigFile classes.
 *
 * \ingroup grpconfigfile
 */

namespace paludis
{
#include <paludis/config_file-se.hh>

    class FSEntry;

    /**
     * Raised if an error is encountered parsing a configuration file.
     *
     * \ingroup grpconfigfile
     * \ingroup grpexceptions
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
            ConfigFileError(const std::string & filename, const std::string & message) throw ();

            /**
             * Constructor, where the filename is not known.
             *
             * \param message A description of the error.
             */
            ConfigFileError(const std::string & message) throw ();

            ///\}
    };

    /**
     * Base class for configuration files.
     *
     * Data is read in from a ConfigFile::Source instance, which can be created from
     * an FSEntry, a std::istream or a string.
     *
     * \see KeyValueConfigFile
     * \see LineConfigFile
     *
     * \ingroup grpconfigfile
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ConfigFile :
        private InstantiationPolicy<ConfigFile, instantiation_method::NonCopyableTag>
    {
        public:
            /**
             * A source (for example, a file or a string) usable by ConfigFile.
             *
             * \see ConfigFile
             * \ingroup grpconfigfile
             * \nosubgrouping
             */
            class PALUDIS_VISIBLE Source :
                private PrivateImplementationPattern<Source>
            {
                public:
                    ///\name Basic operations
                    ///\{

                    Source(const FSEntry &);
                    Source(const std::string &);
                    Source(std::istream &);

                    Source(const Source &);
                    const Source & operator= (const Source &);

                    ~Source();

                    ///\}

                    /**
                     * Our stream, for use by ConfigFile.
                     */
                    std::istream & stream() const;

                    /**
                     * Our filename (may be empty), for use by ConfigFile.
                     */
                    std::string filename() const PALUDIS_ATTRIBUTE((warn_unused_result));
            };

            ///\name Basic operations
            ///\{

            virtual ~ConfigFile();

            ///\}

        protected:

            ///\name Basic operations
            ///\{

            ConfigFile(const Source &);

            ///\}
    };

    /**
     * Options for a LineConfigFile.
     *
     * \see LineConfigFileOption
     * \see LineConfigFile
     * \ingroup grplineconfigfile
     */
    typedef Options<LineConfigFileOption> LineConfigFileOptions;

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
     * \ingroup grplineconfigfile
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE LineConfigFile :
        public ConfigFile,
        private PrivateImplementationPattern<LineConfigFile>
    {
        private:
            void _parse(const Source &, const LineConfigFileOptions &);

        public:
            ///\name Basic operations
            ///\{

            /**
             * Deprecated constructor.
             *
             * \deprecated Use the two argument form.
             */
            LineConfigFile(const Source &) PALUDIS_ATTRIBUTE((deprecated));

            LineConfigFile(const Source &, const LineConfigFileOptions &);

            ~LineConfigFile();

            ///\}

            ///\name Iterate over our lines
            ///\{

            typedef libwrapiter::ForwardIterator<LineConfigFile, const std::string> Iterator;

            Iterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
            Iterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}
    };

    /**
     * Options for a KeyValueConfigFileOptions.
     *
     * \see KeyValueConfigFileOption
     * \see KeyValueConfigFile
     */
    typedef Options<KeyValueConfigFileOption> KeyValueConfigFileOptions;

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
     *
     * \ingroup grpkvconfigfile
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE KeyValueConfigFile :
        public ConfigFile,
        private PrivateImplementationPattern<KeyValueConfigFile>
    {
        public:
            /**
             * A source of default values for a KeyValueConfigFile.
             *
             * \ingroup grpkvconfigfile
             * \see KeyValueConfigFile
             * \nosubgrouping
             */
            class PALUDIS_VISIBLE Defaults :
                private PrivateImplementationPattern<Defaults>
            {
                public:
                    ///\name Basic operations
                    ///\{

                    /**
                     * Avoid various C++ syntax oddities by providing a default constructor that
                     * can't be used by anything.
                     *
                     * This is specialised for permissible parameters.
                     */
                    template <typename T_>
                    Defaults(T_)
                    {
                        T_::WrongTypeForDefaults;
                    }

                    /**
                     * Defaults, from a function.
                     */
                    Defaults(std::string (*)(const std::string &, const std::string &));

                    /**
                     * Defaults, from an empty string.
                     */
                    Defaults();

                    Defaults(const Defaults &);
                    const Defaults & operator= (const Defaults &);

                    ~Defaults();

                    ///\}

                    /**
                     * Get the default value for a particular key.
                     */
                    std::string get(const std::string &) const PALUDIS_ATTRIBUTE((warn_unused_result));

                    ///\name Iterate over our default keys
                    ///\{

                    typedef libwrapiter::ForwardIterator<Defaults, const std::pair<const std::string, std::string> > Iterator;
                    Iterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                    Iterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

                    ///\}
            };

        private:
            void _parse(const Source &, const KeyValueConfigFileOptions &, const Defaults &);

        public:
            ///\name Basic operations
            ///\{

            /**
             * Constructor.
             *
             * \deprecated Use the form taking KeyValueConfigFileOptions.
             */
            KeyValueConfigFile(const Source &, const Defaults & = Defaults(),
                    bool (* is_incremental) (const std::string &, const KeyValueConfigFile &) = 0) PALUDIS_ATTRIBUTE((deprecated));

            KeyValueConfigFile(const Source &, const KeyValueConfigFileOptions &, const Defaults & = Defaults(),
                    bool (* is_incremental) (const std::string &, const KeyValueConfigFile &) = 0);

            ~KeyValueConfigFile();

            ///\}

            ///\name Iterate over our keys
            ///\{

            typedef libwrapiter::ForwardIterator<KeyValueConfigFile, const std::pair<const std::string, std::string> > Iterator;
            Iterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
            Iterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            /**
             * Fetch the value for a particular key.
             */
            std::string get(const std::string &) const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * Use another KeyValueConfigFile for defaults.
     *
     * \ingroup grpkvconfigfile
     */
    template<>
    KeyValueConfigFile::Defaults::Defaults(tr1::shared_ptr<const KeyValueConfigFile>);

    /**
     * Use a string pair collection for defaults.
     *
     * \ingroup grpkvconfigfile
     */
    template<>
    KeyValueConfigFile::Defaults::Defaults(tr1::shared_ptr<const AssociativeCollection<std::string, std::string> >);

    /**
     * Use another KeyValueConfigFile for defaults (non-const).
     *
     * \ingroup grpkvconfigfile
     */
    template<>
    KeyValueConfigFile::Defaults::Defaults(tr1::shared_ptr<KeyValueConfigFile>);

    /**
     * Use a string pair collection for defaults (non-const).
     *
     * \ingroup grpkvconfigfile
     */
    template<>
    KeyValueConfigFile::Defaults::Defaults(tr1::shared_ptr<AssociativeCollection<std::string, std::string> >);
}

#endif
