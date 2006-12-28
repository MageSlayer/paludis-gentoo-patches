/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_SRC_LICENCE_HH
#define PALUDIS_GUARD_SRC_LICENCE_HH 1

#include <paludis/paludis.hh>
#include <iosfwd>

/**
 * Display licences.
 */
struct LicenceDisplayer :
    paludis::DepAtomVisitorTypes::ConstVisitor
{
    /// Our stream.
    std::ostream & stream;

    /// Our environment.
    const paludis::Environment * const env;

    /// Our db entry.
    const paludis::PackageDatabaseEntry * const db_entry;

    /// Constructor.
    LicenceDisplayer(
            std::ostream & stream,
            const paludis::Environment * const e,
            const paludis::PackageDatabaseEntry * const d);

    ///\name Visit methods
    ///{
    void visit(const paludis::AllDepAtom * atom);

    void visit(const paludis::AnyDepAtom * atom);

    void visit(const paludis::UseDepAtom * atom);

    void visit(const paludis::PlainTextDepAtom * atom);

    void visit(const paludis::PackageDepAtom *)
    {
    }

    void visit(const paludis::BlockDepAtom *)
    {
    }
    ///}
};

#endif
