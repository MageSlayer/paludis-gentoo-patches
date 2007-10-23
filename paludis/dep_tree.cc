/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/dep_tree.hh>
#include <paludis/util/visitor-impl.hh>

using namespace paludis;

template class ConstVisitor<GenericSpecTree>;
template class ConstAcceptInterface<GenericSpecTree>;
template class TreeLeaf<GenericSpecTree, PackageDepSpec>;
template class TreeLeaf<GenericSpecTree, BlockDepSpec>;
template class TreeLeaf<GenericSpecTree, PlainTextDepSpec>;
template class TreeLeaf<GenericSpecTree, SimpleURIDepSpec>;
template class TreeLeaf<GenericSpecTree, FetchableURIDepSpec>;
template class TreeLeaf<GenericSpecTree, LicenseDepSpec>;
template class ConstTreeSequence<GenericSpecTree, AllDepSpec>;
template class ConstTreeSequence<GenericSpecTree, AnyDepSpec>;
template class ConstTreeSequence<GenericSpecTree, UseDepSpec>;

template class ConstVisitor<LicenseSpecTree>;
template class ConstAcceptInterface<LicenseSpecTree>;
template class TreeLeaf<LicenseSpecTree, LicenseDepSpec>;
template class ConstTreeSequence<LicenseSpecTree, AllDepSpec>;
template class ConstTreeSequence<LicenseSpecTree, AnyDepSpec>;
template class ConstTreeSequence<LicenseSpecTree, UseDepSpec>;

template class ConstVisitor<FetchableURISpecTree>;
template class ConstAcceptInterface<FetchableURISpecTree>;
template class TreeLeaf<FetchableURISpecTree, FetchableURIDepSpec>;
template class ConstTreeSequence<FetchableURISpecTree, AllDepSpec>;
template class ConstTreeSequence<FetchableURISpecTree, UseDepSpec>;

template class ConstVisitor<SimpleURISpecTree>;
template class ConstAcceptInterface<SimpleURISpecTree>;
template class TreeLeaf<SimpleURISpecTree, SimpleURIDepSpec>;
template class ConstTreeSequence<SimpleURISpecTree, AllDepSpec>;
template class ConstTreeSequence<SimpleURISpecTree, UseDepSpec>;

template class ConstVisitor<ProvideSpecTree>;
template class ConstAcceptInterface<ProvideSpecTree>;
template class TreeLeaf<ProvideSpecTree, PackageDepSpec>;
template class ConstTreeSequence<ProvideSpecTree, AllDepSpec>;
template class ConstTreeSequence<ProvideSpecTree, UseDepSpec>;

template class ConstVisitor<RestrictSpecTree>;
template class ConstAcceptInterface<RestrictSpecTree>;
template class TreeLeaf<RestrictSpecTree, PlainTextDepSpec>;
template class ConstTreeSequence<RestrictSpecTree, AllDepSpec>;
template class ConstTreeSequence<RestrictSpecTree, UseDepSpec>;

template class ConstVisitor<DependencySpecTree>;
template class TreeLeaf<DependencySpecTree, PackageDepSpec>;
template class TreeLeaf<DependencySpecTree, BlockDepSpec>;
template class ConstTreeSequence<DependencySpecTree, AllDepSpec>;
template class ConstTreeSequence<DependencySpecTree, AnyDepSpec>;
template class ConstTreeSequence<DependencySpecTree, UseDepSpec>;
template class ConstAcceptInterface<DependencySpecTree>;

template class ConstVisitor<SetSpecTree>;
template class ConstAcceptInterface<SetSpecTree>;
template class TreeLeaf<SetSpecTree, PackageDepSpec>;
template class ConstTreeSequence<SetSpecTree, AllDepSpec>;

