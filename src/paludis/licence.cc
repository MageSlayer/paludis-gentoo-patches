/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include "licence.hh"
#include "colour.hh"
#include <ostream>

void
LicenceDisplayer::visit(const paludis::AllDepAtom * atom)
{
    stream << "( ";
    std::for_each(atom->begin(), atom->end(), paludis::accept_visitor(this));
    stream << ") ";
}

void
LicenceDisplayer::visit(const paludis::AnyDepAtom * atom)
{
    stream << "|| ( ";
    std::for_each(atom->begin(), atom->end(), paludis::accept_visitor(this));
    stream << ") ";
}

void
LicenceDisplayer::visit(const paludis::UseDepAtom * atom)
{
    stream << atom->flag() << "? ( ";
    std::for_each(atom->begin(), atom->end(), paludis::accept_visitor(this));
    stream << ") ";
}

void
LicenceDisplayer::visit(const paludis::PlainTextDepAtom * atom)
{
    if (env->accept_license(atom->text(), db_entry))
        stream << colour(cl_not_masked, atom->text());
    else
        stream << colour(cl_masked, "(" + atom->text() + ")!");
    stream << " ";
}

LicenceDisplayer::LicenceDisplayer(
        std::ostream & s,
        const paludis::Environment * const e,
        const paludis::PackageDatabaseEntry * const d) :
    stream(s),
    env(e),
    db_entry(d)
{
}
