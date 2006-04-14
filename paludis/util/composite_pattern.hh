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

#ifndef PALUDIS_GUARD_PALUDIS_COMPOSITE_PATTERN_HH
#define PALUDIS_GUARD_PALUDIS_COMPOSITE_PATTERN_HH 1

/** \file
 * Declarations for the Composite template class.
 *
 * \ingroup Utility
 */

namespace paludis
{
    /**
     * A Composite class represents both a class and a collection of
     * child instances of the same class.
     *
     * \ingroup Utility
     */
    template <typename ChildClass_, typename CompositeClass_ = ChildClass_>
    class Composite
    {
        private:
            mutable CompositeClass_ * _composite;

        protected:
            /**
             * Constructor.
             */
            Composite() :
                _composite(0)
            {
            }

        public:
            /**
             * Fetch a CompositeClass_ representation of ourself, or 0 if we
             * are not composite.
             */
            CompositeClass_ * get_composite() const
            {
                if (0 == _composite)
                    _composite = & dynamic_cast<CompositeClass_>(*this);
                return _composite;
            }
    };
}

#endif
