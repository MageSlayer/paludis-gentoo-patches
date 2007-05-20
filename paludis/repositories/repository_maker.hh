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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORY_MAKER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORY_MAKER_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/virtual_constructor.hh>
#include <paludis/repository.hh>

namespace paludis
{
    class FSEntry;

    /**
     * Thrown if a repository of the specified type does not exist.
     *
     * \ingroup grpexceptions
     * \ingroup grprepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE NoSuchRepositoryTypeError : public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            NoSuchRepositoryTypeError(const std::string & format) throw ();
    };

    /**
     * Thrown if PALUDIS_REPOSITORY_SO_DIR is not a directory.
     *
     * \ingroup grpexceptions
     * \ingroup grprepository
     */
    class PALUDIS_VISIBLE PaludisRepositorySoDirNotADirectoryError :
        public Exception
    {
        public:
            PaludisRepositorySoDirNotADirectoryError() throw ();
    };

    /**
     * Thrown if a repository .so cannot be used.
     *
     * \ingroup grpexceptions
     * \ingroup grprepository
     */
    class PALUDIS_VISIBLE PaludisRepositorySoDirCannotDlopenError :
        public Exception
    {
        private:
            std::string _file, _dlerr;
            mutable std::string _what;

        public:
            ///\name Basic operations
            ///\{

            PaludisRepositorySoDirCannotDlopenError(const std::string & file,
                    const std::string & e) throw ();

            ~PaludisRepositorySoDirCannotDlopenError() throw ();

            ///\}

            const char * what() const throw ();
    };

    /**
     * Virtual constructor for repositories.
     *
     * \ingroup grprepository
     */
    class PALUDIS_VISIBLE RepositoryMaker :
        public VirtualConstructor<std::string,
            tr1::shared_ptr<Repository> (*) (Environment * const,
                    tr1::shared_ptr<const AssociativeCollection<std::string, std::string> >),
            virtual_constructor_not_found::ThrowException<NoSuchRepositoryTypeError> >,
        public InstantiationPolicy<RepositoryMaker, instantiation_method::SingletonTag>,
        private PrivateImplementationPattern<RepositoryMaker>
    {
        friend class InstantiationPolicy<RepositoryMaker, instantiation_method::SingletonTag>;

        private:
            RepositoryMaker();

            void load_dir(const FSEntry &);

        public:
            ~RepositoryMaker();
    };
}

#endif
