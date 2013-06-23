/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_SINGLETON_HH
#define PALUDIS_GUARD_PALUDIS_SINGLETON_HH 1

#include <paludis/util/attributes.hh>

/** \file
 * Singleton pattern.
 *
 * \ingroup g_oo
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Singletons have a single instance and are created when first used.
     *
     * \ingroup g_oo
     * \nosubgrouping
     */
    template <typename OurType_>
    class PALUDIS_VISIBLE Singleton
    {
        private:
            static OurType_ * * _get_instance_ptr();

            class DeleteOnDestruction;
            friend class DeleteOnDestruction;

            static void _delete(OurType_ * const p);

        public:
            ///\name Basic operations
            ///\{

            Singleton() = default;
            Singleton(const Singleton &) = delete;
            const Singleton & operator= (const Singleton &) = delete;

            ///\}

            ///\name Singleton operations
            ///\{

            /**
             * Fetch our instance.
             */
            static OurType_ * get_instance()
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Destroy our instance.
             */
            static void destroy_instance();

            ///\}
    };
}

#endif
