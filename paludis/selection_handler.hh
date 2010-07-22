/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010 Ciaran McCreesh
 * Copyright (c) 2008 David Leverton
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

#ifndef PALUDIS_GUARD_PALUDIS_FILTER_HANDLER_HH
#define PALUDIS_GUARD_PALUDIS_FILTER_HANDLER_HH 1

#include <paludis/selection_handler-fwd.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/util/attributes.hh>
#include <memory>

namespace paludis
{
    class SelectionHandler
    {
        protected:
            const FilteredGenerator _fg;

            SelectionHandler(const FilteredGenerator & g) :
                _fg(g)
            {
            }

        public:
            virtual ~SelectionHandler() = 0;

            SelectionHandler(const SelectionHandler &) = delete;
            SelectionHandler & operator= (const SelectionHandler &) = delete;

            virtual std::string as_string() const = 0;

            virtual std::shared_ptr<PackageIDSequence> perform_select(const Environment * const) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };
}

#endif
