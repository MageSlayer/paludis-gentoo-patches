/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENT_ENVIRONMENT_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENT_ENVIRONMENT_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/virtual_constructor.hh>
#include <paludis/environment.hh>

namespace paludis
{
    class FSEntry;

    /**
     * Thrown if an environment of the specified type does not exist.
     *
     * \ingroup grpexceptions
     * \ingroup grpenvironment
     * \nosubgrouping
     */
    class NoSuchEnvironmentTypeError : public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            NoSuchEnvironmentTypeError(const std::string & format) throw ();
    };

    /**
     * Thrown if PALUDIS_ENVIRONMENT_SO_DIR is not a directory.
     *
     * \ingroup grpexceptions
     * \ingroup grprepository
     */
    class PALUDIS_VISIBLE PaludisEnvironmentSoDirNotADirectoryError :
        public Exception
    {
        public:
            PaludisEnvironmentSoDirNotADirectoryError() throw ();
    };

    /**
     * Thrown if an environment .so cannot be used.
     *
     * \ingroup grpexceptions
     * \ingroup grprepository
     */
    class PaludisEnvironmentSoDirCannotDlopenError :
        public Exception
    {
        private:
            std::string _file, _dlerr;
            mutable std::string _what;

        public:
            PaludisEnvironmentSoDirCannotDlopenError(const std::string & file,
                    const std::string & e) throw ();

            ~PaludisEnvironmentSoDirCannotDlopenError() throw ();

            const char * what() const throw ();
    };

    /**
     * Virtual constructor for environments.
     *
     * \ingroup grprepository
     */
    class PALUDIS_VISIBLE EnvironmentMaker :
        public VirtualConstructor<std::string,
            std::tr1::shared_ptr<Environment> (*) (const std::string &),
            virtual_constructor_not_found::ThrowException<NoSuchEnvironmentTypeError> >,
        public InstantiationPolicy<EnvironmentMaker, instantiation_method::SingletonTag>,
        private PrivateImplementationPattern<EnvironmentMaker>
    {
        friend class InstantiationPolicy<EnvironmentMaker, instantiation_method::SingletonTag>;

        private:
            EnvironmentMaker();

            void load_dir(const FSEntry &);

        public:
            ~EnvironmentMaker();

            std::tr1::shared_ptr<Environment> make_from_spec(const std::string &) const;
    };
}

#endif
