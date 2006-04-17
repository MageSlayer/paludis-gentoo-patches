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

#include <istream>
#include <list>
#include <map>
#include <paludis/util/exception.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/instantiation_policy.hh>
#include <string>

/** \file
 * Declarations for the ConfigFile classes.
 *
 * \ingroup ConfigFile
 * \ingroup Exception
 */

namespace paludis
{
    /**
     * Thrown if an error occurs when reading a ConfigFile.
     *
     * \ingroup Exception
     * \ingroup ConfigFile
     */
    class ConfigFileError : public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            ConfigFileError(const std::string & message) throw ();
    };

    /**
     * A ConfigFile is a file containing one entry per line, with lines
     * starting with a # being ignored and leading and trailing whitespace
     * being discarded.
     *
     * \ingroup ConfigFile
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

            /**
             * Our filename, or blank if unknown.
             */
            std::string filename() const
            {
                return _filename;
            }

        public:
            /**
             * Destructor.
             */
            virtual ~ConfigFile();
    };

    /**
     * A LineConfigFile is a ConfigFile that provides access to the
     * normalised lines. Do not subclass.
     *
     * \ingroup ConfigFile
     */
    class LineConfigFile : protected ConfigFile
    {
        private:
            mutable std::list<std::string> _lines;

        protected:
            void accept_line(const std::string &) const;

        public:
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

            /**
             * Iterator over our lines.
             */
            typedef std::list<std::string>::const_iterator Iterator;

            /**
             * Iterator to the start of our lines.
             */
            Iterator begin() const
            {
                return _lines.begin();
            }

            /**
             * Iterator to past the end of our lines.
             */
            Iterator end() const
            {
                return _lines.end();
            }
    };

    /**
     * A KeyValueConfigFileError is thrown if bad data is encountered in
     * a ConfigFile.
     *
     * \ingroup ConfigFile
     * \ingroup Exception
     */
    class KeyValueConfigFileError : public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            KeyValueConfigFileError(const std::string & message,
                    const std::string & filename = "") throw ();
    };

    /**
     * A KeyValueConfigFile is a ConfigFile that provides access to the
     * normalised lines. Do not subclass.
     *
     * \ingroup ConfigFile
     */
    class KeyValueConfigFile : protected ConfigFile
    {
        private:
            mutable std::map<std::string, std::string> _entries;

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
                    const std::map<std::string, std::string> &);

            /**
             * Constructor, from a filename, with defaults.
             */
            KeyValueConfigFile(const std::string & filename,
                    const std::map<std::string, std::string> &);

            /**
             * Constructor, from a filename, with defaults.
             */
            KeyValueConfigFile(const FSEntry & filename,
                    const std::map<std::string, std::string> &);

            /**
             * Destructor.
             */
            ~KeyValueConfigFile();

            /**
             * Iterator over our key/values.
             */
            typedef std::map<std::string, std::string>::const_iterator Iterator;

            /**
             * Start of our key/values.
             */
            Iterator begin() const
            {
                return _entries.begin();
            }

            /**
             * Past the end of our key/values.
             */
            Iterator end() const
            {
                return _entries.end();
            }

            /**
             * Fetch the specified key, or a blank string.
             */
            std::string get(const std::string & key) const
            {
                return _entries[key];
            }
    };

    /**
     * An AdvisoryFileError is thrown if bad data is encountered in
     * a ConfigFile.
     *
     * \ingroup ConfigFile
     * \ingroup Exception
     */
    class AdvisoryFileError : public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            AdvisoryFileError(const std::string & message,
                    const std::string & filename = "") throw ();
    };

    /**
     * An AdvisoryFile is a file containing all necessary information to
     * update one or more packages in order to avoid a security problem.
     *
     * It uses a textformat with RFC 822 style headers, an empty line denotes
     * the beginning of a multi-line description of the security problem.
     *
     * Valid header items are:
     *  Affected, Bug-Url, Commited-By, Id, Reviewed-By, Unaffected, Url
     *
     * \ingroup ConfigFile
     */

    class AdvisoryFile : protected ConfigFile
    {
        private:
            mutable std::map<std::string, std::string> _entries;
            mutable bool _end_of_header;
        protected:
            void accept_line(const std::string &) const;
            void sanitise();

        public:
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

            /**
             * Constructor, from a stream, with defaults.
             */
            AdvisoryFile(std::istream * const,
                    const std::map<std::string, std::string> &);

            /**
             * Constructor, from a filename, with defaults.
             */
            AdvisoryFile(const std::string & filename,
                    const std::map<std::string, std::string> &);

            /**
             * Constructor, from a filename, with defaults.
             */
            AdvisoryFile(const FSEntry & filename,
                    const std::map<std::string, std::string> &);

            /**
             * Destructor.
             */
            ~AdvisoryFile();

            /**
             * Iterator over our lines.
             */
            typedef std::map<std::string, std::string>::const_iterator Iterator;

            /**
             * Iterator to the start of our lines.
             */
            Iterator begin() const
            {
                return _entries.begin();
            }

            /**
             * Iterator to past the end of our lines.
             */
            Iterator end() const
            {
                return _entries.end();
            }

            /**
             * Fetch the specified key, or a blank string.
             */
            std::string get(const std::string & key) const
            {
                return _entries[key];
            }
    };

}

#endif
