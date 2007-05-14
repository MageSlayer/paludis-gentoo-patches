/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_SPEC_FWD_HH
#define PALUDIS_GUARD_PALUDIS_DEP_SPEC_FWD_HH 1

#include <iosfwd>
#include <string>
#include <paludis/util/attributes.hh>
#include <paludis/util/visitor-fwd.hh>

namespace paludis
{
    class DepSpec;
    class CompositeDepSpec;
    class PackageDepSpec;
    class PlainTextDepSpec;
    class AllDepSpec;
    class AnyDepSpec;
    class UseDepSpec;
    class SetDepSpec;
    class BlockDepSpec;

#include <paludis/dep_spec-se.hh>

    /**
     * Visitor types for a visitor that can visit a DepSpec heirarchy.
     *
     * \ingroup grpdepspecs
     */
    typedef VisitorTypes<PackageDepSpec *, PlainTextDepSpec *, AllDepSpec *, AnyDepSpec *,
            UseDepSpec *, BlockDepSpec *> DepSpecVisitorTypes;

    /**
     * A PlainTextDepSpec can be written to an ostream.
     *
     * \ingroup grpdepspecs
     */
    std::ostream & operator<< (std::ostream &, const PlainTextDepSpec &) PALUDIS_VISIBLE;

    class PackageDepSpecError;

    /**
     * A PackageDepSpec can be written to an ostream.
     *
     * \ingroup grpdepspecs
     */
    std::ostream & operator<< (std::ostream &, const PackageDepSpec &) PALUDIS_VISIBLE;
}


#endif
