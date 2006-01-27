/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
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

#include <paludis/instantiation_policy.hh>
#include <paludis/exception.hh>
#include <istream>
#include <list>
#include <string>

/** \file
 * Declarations for the ConfigFile class.
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
}

#endif
