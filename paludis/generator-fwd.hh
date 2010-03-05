/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_GENERATOR_FWD_HH
#define PALUDIS_GUARD_PALUDIS_GENERATOR_FWD_HH 1

#include <paludis/util/attributes.hh>
#include <iosfwd>

/** \file
 * Forward declarations for paludis/generator.hh .
 *
 * \ingroup g_selections
 */

namespace paludis
{
    class Generator;

    namespace generator
    {
        class All;
        class Matches;
        class Package;
        class FromRepository;
        class InRepository;
        class Category;
        class Intersection;
        class Union;
        template <typename> class SomeIDsMightSupportAction;
    }

    /**
     * Two Generator instances can be and-ed together, to obtain only PackageID
     * instances that satisfy the constraints of Generator instances.
     *
     * \ingroup g_selections
     */
    Generator operator& (const Generator &, const Generator &)
        PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;

    /**
     * Two Generator instances can be added, to obtain PackageID instances that
     * satisfy the constraints of either Generator instance.
     *
     * \ingroup g_selections
     */
    Generator operator+ (const Generator &, const Generator &)
        PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;

    /**
     * A FilteredGenerator can be represented as a string. Generally this is
     * used only for stringifying a Selection.
     *
     * \ingroup g_selections
     */
    std::ostream & operator<< (std::ostream &, const Generator &)
        PALUDIS_VISIBLE;
}

#endif
