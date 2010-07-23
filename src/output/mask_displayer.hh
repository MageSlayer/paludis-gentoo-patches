/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_SRC_OUTPUT_MASK_DISPLAYER_HH
#define PALUDIS_GUARD_PALUDIS_SRC_OUTPUT_MASK_DISPLAYER_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/mask.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>

namespace paludis
{
    class MaskDisplayer :
        private Pimp<MaskDisplayer>
    {
        public:
            MaskDisplayer(const Environment * const, const std::shared_ptr<const PackageID> &,
                    const bool want_description);
            ~MaskDisplayer();

            std::string result() const PALUDIS_ATTRIBUTE((warn_unused_result));

            void visit(const UnacceptedMask & m);

            void visit(const RepositoryMask & m);

            void visit(const UserMask & m);

            void visit(const UnsupportedMask & m);

            void visit(const AssociationMask & m);
    };
}

#endif
