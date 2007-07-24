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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_FETCH_VISITOR_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_FETCH_VISITOR_HH 1

#include <paludis/repositories/e/eapi-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/visitor-fwd.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/tr1_memory.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/environment-fwd.hh>

namespace paludis
{
    namespace erepository
    {
        class PALUDIS_VISIBLE FetchVisitor :
            private PrivateImplementationPattern<FetchVisitor>,
            public ConstVisitor<URISpecTree>
        {
            public:
                FetchVisitor(
                        const Environment * const,
                        const tr1::shared_ptr<const PackageID> &,
                        const EAPI & eapi,
                        const FSEntry & distdir,
                        const bool fetch_unneeded,
                        const bool userpriv,
                        const std::string & mirrors_name,
                        const bool fetch_restrict);

                ~FetchVisitor();

                void visit_sequence(const AllDepSpec &,
                        URISpecTree::ConstSequenceIterator,
                        URISpecTree::ConstSequenceIterator);

                void visit_sequence(const UseDepSpec &,
                        URISpecTree::ConstSequenceIterator,
                        URISpecTree::ConstSequenceIterator);

                void visit_leaf(const LabelsDepSpec<URILabelVisitorTypes> &);

                void visit_leaf(const URIDepSpec &);
        };
    }
}

#endif
