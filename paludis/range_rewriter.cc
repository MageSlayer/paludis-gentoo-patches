/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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
#include <paludis/util/sequence.hh>
#include <paludis/version_requirements.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/dep_spec.hh>

using namespace paludis;

RangeRewriter::RangeRewriter() :
    _invalid(false)
{
}

RangeRewriter::~RangeRewriter()
{
}

void
RangeRewriter::visit_sequence(const AllDepSpec &,
        DependencySpecTree::ConstSequenceIterator cur,
        DependencySpecTree::ConstSequenceIterator end)
{
    if (cur != end)
        _invalid = true;
}

void
RangeRewriter::visit_sequence(const AnyDepSpec &,
        DependencySpecTree::ConstSequenceIterator cur,
        DependencySpecTree::ConstSequenceIterator end)
{
    if (cur != end)
        _invalid = true;
}

void
RangeRewriter::visit_sequence(const UseDepSpec &,
        DependencySpecTree::ConstSequenceIterator,
        DependencySpecTree::ConstSequenceIterator)
{
    _invalid = true;
}

void
RangeRewriter::visit_leaf(const PackageDepSpec & a)
{
    if (_invalid)
        return;

    if (a.use_requirements_ptr() || a.slot_ptr() || a.use_requirements_ptr() || ! a.version_requirements_ptr())
    {
        _invalid = true;
        return;
    }

    if (a.version_requirements_mode() != vr_or && 1 != std::distance(a.version_requirements_ptr()->begin(),
                a.version_requirements_ptr()->end()))
    {
        _invalid = true;
        return;
    }

    if (_spec)
    {
        if ((! a.package_ptr()) || (! _spec->package_ptr()) || (*a.package_ptr() != *_spec->package_ptr()))
        {
            _invalid = true;
            return;
        }

        for (VersionRequirements::ConstIterator v(a.version_requirements_ptr()->begin()),
                v_end(a.version_requirements_ptr()->end()) ; v != v_end ; ++v)
            _spec->version_requirements_ptr()->push_back(*v);
    }
    else
    {
        _spec.reset(new PackageDepSpec(a));
        _spec->set_version_requirements_mode(vr_or);
    }
}

void
RangeRewriter::visit_leaf(const BlockDepSpec &)
{
    _invalid = true;
}

void
RangeRewriter::visit_leaf(const DependencyLabelsDepSpec &)
{
    _invalid = true;
}

void
RangeRewriter::visit_leaf(const NamedSetDepSpec &)
{
    _invalid = true;
}

