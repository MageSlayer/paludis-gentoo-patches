/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENT_IMPLEMENTATION_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENT_IMPLEMENTATION_HH 1

#include <paludis/environment.hh>
#include <paludis/package_id-fwd.hh>

/** \file
 * Declarations for the Environment class.
 *
 * \ingroup g_environment
 *
 * \section Examples
 *
 * - None at this time. The EnvironmentImplementation class is of no relevance
 *   to clients.
 */

namespace paludis
{
    /**
     * Simplifies implementing the Environment interface.
     *
     * Most Environment subclasses derive from this class, rather than directly
     * from the abstract base. It provides default implementations for many
     * methods.
     *
     * \ingroup g_environment
     * \see Environment
     */
    class PALUDIS_VISIBLE EnvironmentImplementation :
        public Environment
    {
        protected:
            virtual tr1::shared_ptr<SetSpecTree::ConstItem> local_set(const SetName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

        public:
            ///\name Basic operations
            ///\{

            virtual ~EnvironmentImplementation() = 0;

            ///\}

            virtual bool query_use(const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const FSEntrySequence> bashrc_files() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const FSEntrySequence> syncers_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const FSEntrySequence> fetchers_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const SetNameSet> set_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<SetSpecTree::ConstItem> set(const SetName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const DestinationsSet> default_destinations() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string default_distribution() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool is_paludis_package(const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
