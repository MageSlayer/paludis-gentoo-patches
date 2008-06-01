/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_SELECTION_HH
#define PALUDIS_GUARD_PALUDIS_SELECTION_HH 1

#include <paludis/selection-fwd.hh>
#include <paludis/selection_handler-fwd.hh>
#include <paludis/filtered_generator-fwd.hh>
#include <paludis/filter-fwd.hh>
#include <paludis/generator-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/exception.hh>
#include <paludis/name-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/environment-fwd.hh>

namespace paludis
{
    class DidNotGetExactlyOneError :
        public Exception
    {
        public:
            DidNotGetExactlyOneError(const std::string &,
                    const std::tr1::shared_ptr<const PackageIDSet> &) throw ();
    };

    class PALUDIS_VISIBLE Selection :
        private PrivateImplementationPattern<Selection>
    {
        protected:
            Selection(const std::tr1::shared_ptr<const SelectionHandler> &);

        public:
            Selection(const Selection &);
            ~Selection();
            Selection & operator= (const Selection &);

            std::string as_string() const PALUDIS_ATTRIBUTE((warn_unused_result));

            std::tr1::shared_ptr<PackageIDSequence> perform_select(const Environment * const) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    namespace selection
    {
        class PALUDIS_VISIBLE SomeArbitraryVersion :
            public Selection
        {
            public:
                SomeArbitraryVersion(const FilteredGenerator &);
        };

        class PALUDIS_VISIBLE BestVersionOnly :
            public Selection
        {
            public:
                BestVersionOnly(const FilteredGenerator &);
        };

        class PALUDIS_VISIBLE BestVersionInEachSlot :
            public Selection
        {
            public:
                BestVersionInEachSlot(const FilteredGenerator &);
        };

        class PALUDIS_VISIBLE AllVersionsSorted :
            public Selection
        {
            public:
                AllVersionsSorted(const FilteredGenerator &);
        };

        class PALUDIS_VISIBLE AllVersionsGroupedBySlot :
            public Selection
        {
            public:
                AllVersionsGroupedBySlot(const FilteredGenerator &);
        };

        class PALUDIS_VISIBLE AllVersionsUnsorted :
            public Selection
        {
            public:
                AllVersionsUnsorted(const FilteredGenerator &);
        };

        class PALUDIS_VISIBLE RequireExactlyOne :
            public Selection
        {
            public:
                RequireExactlyOne(const FilteredGenerator &);
        };
    };

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<Selection>;
#endif
}

#endif
