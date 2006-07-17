/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <libwrapiter/libwrapiter_forward_iterator.hh>

#include <iosfwd>
#include <string>

/** \file
 * Declarations for the ConfigFile classes.
 *
 * \ingroup grpconfigfile
 */

namespace paludis
{
    class FSEntry;

    /**
     * Thrown if an error occurs when reading a ConfigFile.
     *
     * \ingroup grpexceptions
     * \ingroup grpconfigfile
     */
    class ConfigFileError : public ConfigurationError
    {
        public:
            ///\name Basic operations
            ///\{

            ConfigFileError(const std::string & message) throw ();

            ///\}
    };

    /**
     * A ConfigFile is a file containing one entry per line, with lines
     * starting with a # being ignored and leading and trailing whitespace
     * being discarded.
     *
     * \ingroup grpconfigfile
     */
    class ConfigFile :
        paludis::InstantiationPolicy<ConfigFile, instantiation_method::NonCopyableTag>
    {
        private:
            std::istream * const _stream;

            mutable bool _has_lines;

            std::string _filename;

            bool _destroy_stream;

            static std::istream * _make_stream(const std::string & filename);

        protected:
            /**
             * In-place normalise a line. By default, trims leading and
             * trailing whitespace. Child classes may override.
             */
            virtual void normalise_line(std::string &) const;

            /**
             * Return whether to skip a line. By default, skips on blank
             * lines and lines starting with a #. Child classes may
             * override. This is called on a normalised line, not a raw
             * string.
             */
            virtual bool skip_line(const std::string &) const;

            /**
             * Accept a normalised line that is not to be skipped.
             */
            virtual void accept_line(const std::string &) const = 0;

            /**
             * If we have not done so already, read in our lines.
             */
            void need_lines() const;

            ///\name Basic operations
            ///\{

            /**
             * Constructor.
             */
            ConfigFile(std::istream * const stream);

            /**
             * Constructor, from a file.
             */
            ConfigFile(const std::string & filename);

            /**
             * Constructor, from a file.
             */
            ConfigFile(const FSEntry & filename);

            ///\}

            /**
             * Our filename, or blank if unknown.
             */
            std::string filename() const
            {
                return _filename;
            }

        public:
            ///\name Basic operations
            ///\{

            virtual ~ConfigFile();

            ///\}
    };

    /**
     * A LineConfigFile is a ConfigFile that provides access to the
     * normalised lines. Do not subclass.
     *
     * \ingroup grplineconfigfile
     */
    class LineConfigFile :
        protected ConfigFile,
        private PrivateImplementationPattern<LineConfigFile>
    {
        protected:
            void accept_line(const std::string &) const;

        public:
            ///\name Basic operations
            ///\{

            /**
             * Constructor, from a stream.
             */
            LineConfigFile(std::istream * const);

            /**
             * Constructor, from a filename.
             */
            LineConfigFile(const std::string & filename);

            /**
             * Constructor, from a filename.
             */
            LineConfigFile(const FSEntry & filename);

            ~LineConfigFile();

            ///\}

            ///\name Iterate over our lines
            ///\{

            typedef libwrapiter::ForwardIterator<LineConfigFile, const std::string> Iterator;

            Iterator begin() const;

            Iterator end() const;

            ///\}
    };

    /**
     * A KeyValueConfigFileError is thrown if bad data is encountered in
     * a ConfigFile.
     *
     * \ingroup grpkvconfigfile
     * \ingroup grpexceptions
     */
    class KeyValueConfigFileError : public ConfigurationError
    {
        public:
            ///\name Basic operations
            ///\{

            KeyValueConfigFileError(const std::string & message,
                    const std::string & filename = "") throw ();

            ///\}
    };

    /**
     * A KeyValueConfigFile is a ConfigFile that provides access to the
     * normalised lines. Do not subclass.
     *
     * \ingroup grpkvconfigfile
     */
    class KeyValueConfigFile :
        protected ConfigFile,
        private PrivateImplementationPattern<KeyValueConfigFile>
    {
        protected:
            void accept_line(const std::string &) const;

            /**
             * Handle variable replacement.
             */
            std::string replace_variables(const std::string &) const;

            /**
             * Handle quote removal.
             */
            std::string strip_quotes(const std::string &) const;

        public:
            ///\name Basic operations
            ///\{

            /**
             * Constructor, from a stream.
             */
            KeyValueConfigFile(std::istream * const);

            /**
             * Constructor, from a filename.
             */
            KeyValueConfigFile(const std::string & filename);

            /**
             * Constructor, from a filename.
             */
            KeyValueConfigFile(const FSEntry & filename);

            /**
             * Constructor, from a stream, with defaults.
             */
            KeyValueConfigFile(std::istream * const,
                    AssociativeCollection<std::string, std::string>::ConstPointer);

            /**
             * Constructor, from a filename, with defaults.
             */
            KeyValueConfigFile(const std::string & filename,
                    AssociativeCollection<std::string, std::string>::ConstPointer);

            /**
             * Constructor, from a filename, with defaults.
             */
            KeyValueConfigFile(const FSEntry & filename,
                    AssociativeCollection<std::string, std::string>::ConstPointer);

            ~KeyValueConfigFile();

            ///\}

            ///\name Iterate over our key/values
            ///\{

            typedef libwrapiter::ForwardIterator<KeyValueConfigFile,
                    std::pair<const std::string, std::string> > Iterator;

            Iterator begin() const;

            Iterator end() const;

            ///\}

            /**
             * Fetch the specified key, or a blank string.
             */
            std::string get(const std::string & key) const;
    };

    /**
     * An AdvisoryFileError is thrown if bad data is encountered in
     * a ConfigFile.
     *
     * \ingroup grpadvisoryconfigfile
     * \ingroup grpexceptions
     */
    class AdvisoryFileError : public ConfigurationError
    {
        public:
            ///\name Basic operations
            ///\{

            AdvisoryFileError(const std::string & message,
                    const std::string & filename = "") throw ();

            ///\}
    };

    /**
     * An AdvisoryFile is a file containing all necessary information to
     * update one or more packages in order to avoid a security problem.
     *
     * It uses a textformat with RFC 822 style headers, an empty line denotes
     * the beginning of a multi-line description of the security problem.
     *
     * Valid header items are:
     *  Affected, Bug-Url, Committed-By, Id, Reviewed-By, Unaffected, Url
     *
     * \ingroup grpadvisoryconfigfile
     */

    class AdvisoryFile :
        protected ConfigFile,
        private PrivateImplementationPattern<AdvisoryFile>
    {
        protected:
            void accept_line(const std::string &) const;

            /**
             * Ensure that the AdvisoryFile contains all mandatory items.
             */
            void sanitise();

        public:
            ///\name Basic operations
            ///\{

            /**
             * Constructor, from a stream.
             */
            AdvisoryFile(std::istream * const);

            /**
             * Constructor, from a filename.
             */
            AdvisoryFile(const std::string & filename);

            /**
             * Constructor, from a filename.
             */
            AdvisoryFile(const FSEntry & filename);

#if 0
            /**
             * Constructor, from a stream, with defaults.
             */
            AdvisoryFile(std::istream * const,
                    AssociativeCollection<std::string, std::string>::ConstPointer);

            /**
             * Constructor, from a filename, with defaults.
             */
            AdvisoryFile(const std::string & filename,
                    AssociativeCollection<std::string, std::string>::ConstPointer);

            /**
             * Constructor, from a filename, with defaults.
             */
            AdvisoryFile(const FSEntry & filename,
                    AssociativeCollection<std::string, std::string>::ConstPointer);
#endif

            ~AdvisoryFile();

            ///\}

            ///\name Iterate over our entries
            ///\{

            typedef libwrapiter::ForwardIterator<AdvisoryFile,
                    std::pair<const std::string, std::string> > EntriesIterator;

            EntriesIterator begin() const;

            EntriesIterator end() const;

            ///\}

            ///\name Iterate over our Affected: and Unaffected: lines.
            ///\{

            typedef libwrapiter::ForwardIterator<AdvisoryFile, std::string> LineIterator;

            LineIterator begin_affected() const;

            LineIterator end_affected() const;

            LineIterator begin_unaffected() const;

            LineIterator end_unaffected() const;

            ///\}

            /**
             * Fetch the specified key, or a blank string.
             */
            std::string get(const std::string & key) const;

    };

    /**
     * A NewsFile represents a GLEP 42 news file.
     *
     * \ingroup grpnewconfigfile
     */
    class NewsFile :
        protected ConfigFile,
        private PrivateImplementationPattern<NewsFile>
    {
        protected:
            void accept_line(const std::string &) const;

        public:
            ///\name Basic operations
            ///\{

            /**
             * Constructor, from a filename.
             */
            NewsFile(const FSEntry & filename);

            ~NewsFile();

            ///\}

            ///\name Iterate over our Display-If-Installed headers
            ///\{

            /// Tag for DisplayIfInstalledIterator.
            struct DisplayIfInstalledIteratorTag;

            typedef libwrapiter::ForwardIterator<DisplayIfInstalledIteratorTag,
                    const std::string> DisplayIfInstalledIterator;

            DisplayIfInstalledIterator begin_display_if_installed() const;

            DisplayIfInstalledIterator end_display_if_installed() const;

            ///\}

            ///\name Iterate over our Display-If-Keyword headers
            ///\{

            /// Tag for DisplayIfKeywordIterator.
            struct DisplayIfKeywordIteratorTag;

            typedef libwrapiter::ForwardIterator<DisplayIfKeywordIteratorTag,
                    const std::string> DisplayIfKeywordIterator;

            DisplayIfKeywordIterator begin_display_if_keyword() const;

            DisplayIfKeywordIterator end_display_if_keyword() const;

            ///\}

            ///\name Iterate over our Display-If-Profile headers
            ///\{

            /// Tag for DisplayIfProfileIterator.
            struct DisplayIfProfileIteratorTag;

            typedef libwrapiter::ForwardIterator<DisplayIfProfileIteratorTag,
                    const std::string> DisplayIfProfileIterator;

            DisplayIfProfileIterator begin_display_if_profile() const;

            DisplayIfProfileIterator end_display_if_profile() const;

            ///\}
    };
}

#endif
