/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENT_MAKER_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENT_MAKER_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/virtual_constructor.hh>
#include <paludis/environment.hh>


/** \file
 * Declarations for the EnvironmentMaker class.
 *
 * \ingroup g_environment
 *
 * \section Examples
 *
 * - \ref example_environment.cc "example_environment.cc"
 */

namespace paludis
{
    class FSEntry;

    /**
     * Thrown if an environment of the specified type does not exist.
     *
     * \ingroup g_exceptions
     * \ingroup g_environment
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE NoSuchEnvironmentTypeError : public ConfigurationError
    {
        public:
            ///\name Basic operations
            ///\{

            NoSuchEnvironmentTypeError(const std::string & format) throw ();

            ///\}
    };

    /**
     * Thrown if PALUDIS_ENVIRONMENT_SO_DIR is not a directory.
     *
     * \ingroup g_exceptions
     * \ingroup g_environment
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE PaludisEnvironmentSoDirNotADirectoryError :
        public Exception
    {
        public:
            ///\name Basic operations
            ///\{

            PaludisEnvironmentSoDirNotADirectoryError() throw ();

            ///\}
    };

    /**
     * Thrown if an environment .so cannot be used.
     *
     * \ingroup g_exceptions
     * \ingroup g_environment
     */
    class PALUDIS_VISIBLE PaludisEnvironmentSoDirCannotDlopenError :
        public Exception
    {
        private:
            std::string _file, _dlerr;
            mutable std::string _what;

        public:
            ///\name Basic operations
            ///\{

            PaludisEnvironmentSoDirCannotDlopenError(const std::string & file,
                    const std::string & e) throw ();

            ~PaludisEnvironmentSoDirCannotDlopenError() throw ();

            ///\}

            const char * what() const throw ();
    };

    /**
     * If an EnvironmentMaker default call fails with an exception of this type,
     * it is ok to fall back and try another maker.
     *
     * \ingroup g_environment
     * \ingroup g_exceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE FallBackToAnotherMakerError
    {
        protected:
            ///\name Basic operations
            ///\{

            FallBackToAnotherMakerError();

            ///\}
    };

    /**
     * Virtual constructor for environments.
     *
     * \ingroup g_environment
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE EnvironmentMaker :
        public VirtualConstructor<std::string,
            tr1::shared_ptr<Environment> (*) (const std::string &),
            virtual_constructor_not_found::ThrowException<NoSuchEnvironmentTypeError> >,
        public InstantiationPolicy<EnvironmentMaker, instantiation_method::SingletonTag>,
        private PrivateImplementationPattern<EnvironmentMaker>
    {
        friend class InstantiationPolicy<EnvironmentMaker, instantiation_method::SingletonTag>;

        private:
            EnvironmentMaker();

            void load_dir(const FSEntry &);

        public:
            ///\name Basic operations
            ///\{

            ~EnvironmentMaker();

            ///\}

            /**
             * Create an Environment subclass from the specified spec.
             *
             * \param spec The environment spec, which is in the form
             *     env:suffix, where env is the string representing an
             *     Environment's kind (e.g. "paludis", "portage") and
             *     suffix is the information to pass to the constructing
             *     function (for paludis, a config suffix, and for portage,
             *     a location). If env is not specified, it defaults to
             *     trying paludis then portage. If suffix is not specified,
             *     it defaults to an empty string. If no colon is present,
             *     the supplied string is taken as env (this includes an
             *     empty string).
             *
             * \throw NoSuchEnvironmentTypeError if an invalid environment type
             *     is specified.
             *
             * \see Environment
             * \ingroup g_environment
             */
            tr1::shared_ptr<Environment> make_from_spec(const std::string & spec) const;
    };
}

#endif
