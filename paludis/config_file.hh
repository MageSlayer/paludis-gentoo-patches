/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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
             * Called when we've read in all our lines. By default, does
             * nothing. Can be used for further validation.
             */
            virtual void done_reading_lines() const;

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
        private:
            bool quotes_are_balanced(const std::string &) const;

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

            virtual void done_reading_lines() const;

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
}

#endif
