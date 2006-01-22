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

#ifndef PALUDIS_GUARD_PALUDIS_KEY_VALUE_CONFIG_FILE_HH
#define PALUDIS_GUARD_PALUDIS_KEY_VALUE_CONFIG_FILE_HH 1

#include <paludis/config_file.hh>
#include <map>

/** \file
 * Declarations for the KeyValueConfigFile class.
 *
 * \ingroup ConfigFile
 */


namespace paludis
{
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

            std::string replace_variables(const std::string &) const;

            std::string strip_quotes(const std::string &) const;

        public:
            KeyValueConfigFile(std::istream * const);

            KeyValueConfigFile(const std::string & filename);

            ~KeyValueConfigFile();

            typedef std::map<std::string, std::string>::const_iterator Iterator;

            Iterator begin() const
            {
                return _entries.begin();
            }

            Iterator end() const
            {
                return _entries.end();
            }

            std::string get(const std::string & key) const
            {
                return _entries[key];
            }
    };

}

#endif
