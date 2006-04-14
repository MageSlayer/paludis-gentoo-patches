/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_CONTAINER_ENTRY_HH
#define PALUDIS_GUARD_PALUDIS_CONTAINER_ENTRY_HH 1

#include <list>

/** \file
 * Declarations for the ContainerEntry class.
 *
 * \ingroup Utility
 */

namespace paludis
{
    /**
     * Hold an entry in a container for as long as our ContainerEntry instance
     * is in scope (RAII, see \ref EffCpp item 13 or \ref TCppPL section 14.4).
     *
     * \ingroup Utility
     */
    template <typename Container_>
    struct ContainerEntry;

    /**
     * Hold an entry in a container for as long as our ContainerEntry instance
     * is in scope (RAII, see \ref EffCpp item 13 or \ref TCppPL section 14.4;
     * partial specialisation for std::list).
     *
     * \ingroup Utility
     */
    template <typename Item_>
    class ContainerEntry<std::list<Item_> >
    {
        private:
            std::list<Item_> * const _list;

            typename std::list<Item_>::iterator _item;

        public:
            /**
             * Constructor.
             */
            ContainerEntry(std::list<Item_> * const list, const Item_ & item) :
                _list(list),
                _item(list->insert(list->begin(), item))
            {
            }

            /**
             * Destructor.
             */
            ~ContainerEntry()
            {
                _list->erase(_item);
            }
    };
}

#endif
