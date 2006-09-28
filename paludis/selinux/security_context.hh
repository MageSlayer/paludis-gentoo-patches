/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Stephen Bennett <spb@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_SECURITY_CONTEXT_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_SECURITY_CONTEXT_HH 1

#include <string>
#include <paludis/util/exception.hh>
#include <paludis/util/counted_ptr.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>

/** \file
 * Declarations for SecurityContext and associated classes.
 *
 * \ingroup grplibpaludisselinux
 */

namespace paludis
{
    class FSCreateCon;
    class MatchPathCon;

    /**
     * Error class for SELinux-related functions
     *
     * \ingroup grplibpaludisselinux
     * \ingroup grpexceptions
     */
    class PALUDIS_VISIBLE SELinuxException :
        public Exception
    {
        public:
            /// Constructor.
            SELinuxException(const std::string & our_message)
                : Exception(our_message)
            {
            }
    };

    /**
     * Security context class. Wraps security_context_t.
     *
     * \ingroup grplibpaludisselinux
     */
    class PALUDIS_VISIBLE SecurityContext :
        private PrivateImplementationPattern<SecurityContext>,
        private InstantiationPolicy<SecurityContext, instantiation_method::NonCopyableTag>,
        public InternalCounted<SecurityContext>
    {
        public:
            /**
             * Constructor
             */
            SecurityContext();

            /**
             * Can be constructed from a string.
             */
            SecurityContext(const std::string &);

            /**
             * Destructor
             */
            ~SecurityContext();

            friend std::ostream& paludis::operator<<(std::ostream&, const SecurityContext &);
            friend class paludis::FSCreateCon;
            friend class paludis::MatchPathCon;

            /**
             * Returns a SecurityContext referring to the current process's context
             */
            static SecurityContext::ConstPointer current_context();

            /**
             * Returns a SecurityContext referring to the current filesystem creation context
             */
            static SecurityContext::ConstPointer fs_create_context();
    };

    /**
     * A SecurityContext can be written to a stream.
     *
     * \ingroup grplibpaludisselinux
     */
    std::ostream& operator<<(std::ostream&, const SecurityContext &) PALUDIS_VISIBLE;

    /**
     * RAII-style wrapper for setfscreatecon().
     *
     * Create an FSCreateCon object to set the security context of newly created file objects.
     * When destroyed, it will revert to the previous creation context.
     *
     * Note that this operation is not thread-safe. Any multi-threaded code calling it must use a
     * critical section to ensure the desired behaviour.
     *
     * \ingroup grplibpaludisselinux
     */
    class PALUDIS_VISIBLE FSCreateCon
    {
        private:
            SecurityContext::ConstPointer _context;
            SecurityContext::ConstPointer _prev_context;

        public:
            /**
             * Constructor
             */
            FSCreateCon(SecurityContext::ConstPointer);

            /**
             * Destructor
             */
            ~FSCreateCon();
    };

    /**
     * Wrapper class around matchpathcon().
     *
     * \ingroup grplibpaludisselinux
     */
    class PALUDIS_VISIBLE MatchPathCon :
        public InstantiationPolicy<MatchPathCon, instantiation_method::SingletonAsNeededTag>
    {
        private:
            bool _good;

        public:
            /**
             * Constructor. Optional parameter is the path to the file_contexts to use.
             */
            MatchPathCon();

            /**
             * Destructor
             */
            ~MatchPathCon();

            /**
             * Retrieve the default context for a given pathname
             */
            SecurityContext::ConstPointer match(const std::string &, mode_t = 0) const;

            /**
             * Did the initialisation succeed?
             */
            bool good() const;
    };

}

#endif
