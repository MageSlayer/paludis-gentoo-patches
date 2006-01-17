/* vim: set sw=4 sts=4 et foldmethod=syntax : */

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
