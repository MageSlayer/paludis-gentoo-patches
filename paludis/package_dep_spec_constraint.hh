/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_PACKAGE_DEP_SPEC_CONSTRAINT_HH
#define PALUDIS_GUARD_PALUDIS_PACKAGE_DEP_SPEC_CONSTRAINT_HH 1

#include <paludis/package_dep_spec_constraint-fwd.hh>
#include <paludis/name.hh>

#include <paludis/util/attributes.hh>
#include <paludis/util/pool.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/type_list.hh>

namespace paludis
{
    class PALUDIS_VISIBLE PackageDepSpecConstraint :
        public virtual DeclareAbstractAcceptMethods<PackageDepSpecConstraint, MakeTypeList<
            NameConstraint>::Type>
    {
        public:
            virtual ~PackageDepSpecConstraint() = 0;
    };

    class PALUDIS_VISIBLE NameConstraint :
        public PackageDepSpecConstraint,
        public ImplementAcceptMethods<PackageDepSpecConstraint, NameConstraint>
    {
        friend class Pool<NameConstraint>;

        private:
            QualifiedPackageName _name;

            NameConstraint(const QualifiedPackageName &);

            NameConstraint(const NameConstraint &) = delete;

        public:
            ~NameConstraint();

            const QualifiedPackageName name() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    extern template class Pool<NameConstraint>;
}

#endif
