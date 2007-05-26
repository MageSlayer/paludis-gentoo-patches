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

#include "licence.hh"
#include "colour.hh"
#include <ostream>
#include <algorithm>

using namespace paludis;

void
LicenceDisplayer::visit_sequence(const AllDepSpec &,
        LicenseSpecTree::ConstSequenceIterator begin,
        LicenseSpecTree::ConstSequenceIterator end)
{
    stream << "( ";
    std::for_each(begin, end, accept_visitor(*this));
    stream << ") ";
}

void
LicenceDisplayer::visit_sequence(const AnyDepSpec &,
        LicenseSpecTree::ConstSequenceIterator begin,
        LicenseSpecTree::ConstSequenceIterator end)
{
    stream << "|| ( ";
    std::for_each(begin, end, accept_visitor(*this));
    stream << ") ";
}

void
LicenceDisplayer::visit_sequence(const UseDepSpec & spec,
        LicenseSpecTree::ConstSequenceIterator begin,
        LicenseSpecTree::ConstSequenceIterator end)
{
    if (spec.inverse())
        stream << "!";

    stream << spec.flag() << "? ( ";
    std::for_each(begin, end, accept_visitor(*this));
    stream << ") ";
}

void
LicenceDisplayer::visit_leaf(const PlainTextDepSpec & spec)
{
    if (env->accept_license(spec.text(), db_entry))
        stream << colour(cl_not_masked, spec.text());
    else
        stream << colour(cl_masked, "(" + spec.text() + ")!");
    stream << " ";
}

LicenceDisplayer::LicenceDisplayer(
        std::ostream & s,
        const Environment * const e,
        const PackageDatabaseEntry & d) :
    stream(s),
    env(e),
    db_entry(d)
{
}

