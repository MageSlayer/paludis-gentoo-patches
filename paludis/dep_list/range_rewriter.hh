/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_LIST_RANGE_REWRITER_HH
#define PALUDIS_GUARD_PALUDIS_DEP_LIST_RANGE_REWRITER_HH 1

#include <paludis/dep_spec.hh>

namespace paludis
{
    /**
     * Rewrite a DepSpec heirarchy to replace AllDepSpec and AnyDepSpec
     * collections of PackageDepSpec with a single PackageDepSpec using ranged
     * dependencies.
     *
     * \ingroup grpdepresolver
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RangeRewriter :
        public DepSpecVisitorTypes::ConstVisitor
    {
        private:
            std::tr1::shared_ptr<PackageDepSpec> _spec;
            bool _invalid;

        public:
            ///\name Basic operations
            ///\{

            RangeRewriter();
            virtual ~RangeRewriter();

            ///\}

            /**
             * Our rewritten spec, or a zero pointer if we couldn't do any
             * rewriting.
             */
            std::tr1::shared_ptr<const PackageDepSpec> spec() const
            {
                if (_invalid)
                    return std::tr1::shared_ptr<const PackageDepSpec>();

                return _spec;
            }

            ///\name Visit methods
            ///\{

            void visit(const AllDepSpec *);
            void visit(const AnyDepSpec *);
            void visit(const UseDepSpec *);
            void visit(const PlainTextDepSpec *);
            void visit(const PackageDepSpec *);
            void visit(const BlockDepSpec *);

            ///\}
    };
}

#endif
