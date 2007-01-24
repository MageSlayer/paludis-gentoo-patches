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

#include "range_rewriter.hh"

using namespace paludis;

RangeRewriter::RangeRewriter() :
    _atom(0),
    _invalid(false)
{
}

RangeRewriter::~RangeRewriter()
{
}

void
RangeRewriter::visit(const AllDepAtom * a)
{
    if (a->begin() != a->end())
        _invalid = true;
}

void
RangeRewriter::visit(const AnyDepAtom * a)
{
    if (a->begin() != a->end())
        _invalid = true;
}

void
RangeRewriter::visit(const UseDepAtom *)
{
    _invalid = true;
}

void
RangeRewriter::visit(const PlainTextDepAtom *)
{
    _invalid = true;
}

void
RangeRewriter::visit(const PackageDepAtom * a)
{
    if (_invalid)
        return;

    if (a->use_requirements_ptr() || a->slot_ptr() || a->use_requirements_ptr() || ! a->version_requirements_ptr())
    {
        _invalid = true;
        return;
    }

    if (a->version_requirements_mode() != vr_or && 1 != std::distance(a->version_requirements_ptr()->begin(),
                a->version_requirements_ptr()->end()))
    {
        _invalid = true;
        return;
    }

    if (_atom)
    {
        if (a->package() != _atom->package())
        {
            _invalid = true;
            return;
        }

        for (VersionRequirements::Iterator v(a->version_requirements_ptr()->begin()),
                v_end(a->version_requirements_ptr()->end()) ; v != v_end ; ++v)
            _atom->version_requirements_ptr()->push_back(*v);
    }
    else
    {
        _atom.assign(new PackageDepAtom(*a));
        _atom->set_version_requirements_mode(vr_or);
    }
}

void
RangeRewriter::visit(const BlockDepAtom *)
{
    _invalid = true;
}

