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

#include <libwrapiter/libwrapiter_forward_iterator.hh>

#include <iosfwd>
#include <string>
#include <tr1/memory>

/** \file
 * Declarations for the ConfigFile classes.
 *
 * \ingroup grpconfigfile
 */

namespace paludis
{
    class FSEntry;

    class ConfigFileError :
        public ConfigurationError
    {
        public:
            ConfigFileError(const std::string & filename, const std::string & message) throw ();
            ConfigFileError(const std::string & message) throw ();
    };

    class ConfigFile :
        private InstantiationPolicy<ConfigFile, instantiation_method::NonCopyableTag>
    {
        public:
            class Source :
                private PrivateImplementationPattern<Source>
            {
                public:
                    Source(const FSEntry &);
                    Source(const std::string &);
                    Source(std::istream &);

                    Source(const Source &);
                    const Source & operator= (const Source &);

                    ~Source();

                    std::istream & stream() const;
                    std::string filename() const;
            };

            virtual ~ConfigFile();

        protected:
            ConfigFile(const Source &);
    };

    class LineConfigFile :
        public ConfigFile,
        private PrivateImplementationPattern<LineConfigFile>
    {
        public:
            LineConfigFile(const Source &);
            ~LineConfigFile();

            typedef libwrapiter::ForwardIterator<LineConfigFile, const std::string> Iterator;

            Iterator begin() const;
            Iterator end() const;
    };

    class KeyValueConfigFile :
        public ConfigFile,
        private PrivateImplementationPattern<KeyValueConfigFile>
    {
        public:
            class Defaults :
                private PrivateImplementationPattern<Defaults>
            {
                public:
                    template <typename T_>
                    Defaults(T_)
                    {
                        T_::WrongTypeForDefaults;
                    }

                    Defaults(std::string (*)(const std::string &, const std::string &));

                    Defaults();

                    Defaults(const Defaults &);
                    const Defaults & operator= (const Defaults &);

                    ~Defaults();

                    std::string get(const std::string &) const;

                    typedef libwrapiter::ForwardIterator<Defaults, const std::pair<const std::string, std::string> > Iterator;
                    Iterator begin() const;
                    Iterator end() const;
            };

            KeyValueConfigFile(const Source &, const Defaults & = Defaults(),
                    bool (* is_incremental) (const std::string &, const KeyValueConfigFile &) = 0);
            ~KeyValueConfigFile();

            typedef libwrapiter::ForwardIterator<KeyValueConfigFile, const std::pair<const std::string, std::string> > Iterator;
            Iterator begin() const;
            Iterator end() const;

            std::string get(const std::string &) const;
    };

    template<>
    KeyValueConfigFile::Defaults::Defaults(std::tr1::shared_ptr<const KeyValueConfigFile>);

    template<>
    KeyValueConfigFile::Defaults::Defaults(std::tr1::shared_ptr<const AssociativeCollection<std::string, std::string> >);

    template<>
    KeyValueConfigFile::Defaults::Defaults(std::tr1::shared_ptr<KeyValueConfigFile>);

    template<>
    KeyValueConfigFile::Defaults::Defaults(std::tr1::shared_ptr<AssociativeCollection<std::string, std::string> >);
}

#endif
