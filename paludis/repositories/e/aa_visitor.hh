/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Mike Kelly
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

#ifndef PALUDIS_GUARD_PALUDIS_AAVISITOR_HH
#define PALUDIS_GUARD_PALUDIS_AAVISITOR_HH 1

#include <paludis/spec_tree.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>

/** \file
 * Declarations for the AAVisitor class.
 *
 * \ingroup grpaavisitor
 */

namespace paludis
{
    namespace erepository
    {
        /**
         * Get a list of all the URIs in a URIDepSpec, regardless of USE
         * flag settings.
         *
         * \ingroup grpaavisitor
         */
        class PALUDIS_VISIBLE AAVisitor :
            private Pimp<AAVisitor>
        {
            public:
                ///\name Basic operations
                ///\{

                AAVisitor();

                ~AAVisitor();

                ///\}

                /// \name Visit functions
                ///{

                void visit(const FetchableURISpecTree::NodeType<FetchableURIDepSpec>::Type & node);
                void visit(const FetchableURISpecTree::NodeType<URILabelsDepSpec>::Type & node);
                void visit(const FetchableURISpecTree::BasicInnerNode & node);

                ///}

                /// \name Iterator functions
                ///{

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag, const std::string> ConstIterator;

                ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

                ///}
        };
    }
}
#endif
