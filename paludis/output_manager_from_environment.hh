/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_OUTPUT_MANAGER_FROM_ENVIRONMENT_HH
#define PALUDIS_GUARD_PALUDIS_OUTPUT_MANAGER_FROM_ENVIRONMENT_HH 1

#include <paludis/output_manager_from_environment-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/pimp.hh>
#include <paludis/output_manager-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/action-fwd.hh>
#include <paludis/create_output_manager_info-fwd.hh>

namespace paludis
{
    class PALUDIS_VISIBLE OutputManagerFromEnvironment
    {
        private:
            Pimp<OutputManagerFromEnvironment> _imp;

        public:
            OutputManagerFromEnvironment(
                    const Environment * const,
                    const std::shared_ptr<const PackageID> &,
                    const OutputExclusivity,
                    const ClientOutputFeatures &);

            ~OutputManagerFromEnvironment();

            const std::shared_ptr<OutputManager> operator() (const Action &);

            const std::shared_ptr<OutputManager> output_manager_if_constructed();

            void construct_standard_if_unconstructed();
    };

    extern template class Pimp<OutputManagerFromEnvironment>;
}

#endif
