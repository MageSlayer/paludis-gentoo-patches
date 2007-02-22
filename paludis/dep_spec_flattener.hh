/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_ATOM_FLATTENER_HH
#define PALUDIS_GUARD_PALUDIS_DEP_ATOM_FLATTENER_HH 1

#include <paludis/dep_spec.hh>
#include <paludis/package_database.hh>
#include <paludis/environment.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/instantiation_policy.hh>

#include <libwrapiter/libwrapiter_forward_iterator.hh>

/** \file
 * Declarations for DepSpecFlattener.
 *
 * \ingroup grpdepspecflattener
 */

namespace paludis
{
    /**
     * Extract the enabled components of a dep heirarchy for a particular
     * package.
     *
     * This is useful for picking out SRC_URI, PROVIDE etc components. It is
     * <b>not</b> suitable for heirarchies that can contain || ( ) blocks.
     *
     * \ingroup grpdepspecflattener
     * \nosubgrouping
     */
    class DepSpecFlattener :
        private InstantiationPolicy<DepSpecFlattener, instantiation_method::NonCopyableTag>,
        protected DepSpecVisitorTypes::ConstVisitor,
        private PrivateImplementationPattern<DepSpecFlattener>
    {
        protected:
            ///\name Visit methods
            ///{
            void visit(const AllDepSpec *);
            void visit(const AnyDepSpec *) PALUDIS_ATTRIBUTE((noreturn));
            void visit(const UseDepSpec *);
            void visit(const PlainTextDepSpec *);
            void visit(const PackageDepSpec *);
            void visit(const BlockDepSpec *);
            ///}

        public:
            ///\name Basic operations
            ///\{

            DepSpecFlattener(const Environment * const,
                    const PackageDatabaseEntry * const,
                    const std::tr1::shared_ptr<const DepSpec>);

            ~DepSpecFlattener();

            ///\}

            ///\name Iterate over our dep specs
            ///{

            typedef libwrapiter::ForwardIterator<DepSpecFlattener, const StringDepSpec *> Iterator;

            Iterator begin();

            Iterator end() const;

            ///\}
    };
}

#endif
