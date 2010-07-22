/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/output_manager-fwd.hh>
#include <paludis/dep_spec.hh>
#include <paludis/spec_tree-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/spec_tree.hh>
#include <memory>

namespace paludis
{
    namespace erepository
    {
        typedef std::function<std::shared_ptr<const MirrorsSequence> (const std::string &)> GetMirrorsFunction;

        class PALUDIS_VISIBLE FetchVisitor :
            private PrivateImplementationPattern<FetchVisitor>
        {
            public:
                FetchVisitor(
                        const Environment * const,
                        const std::shared_ptr<const PackageID> &,
                        const EAPI & eapi,
                        const FSEntry & distdir,
                        const bool fetch_unneeded,
                        const bool userpriv,
                        const std::string & mirrors_name,
                        const std::shared_ptr<const URILabel> & initial_label,
                        const bool safe_resume,
                        const std::shared_ptr<OutputManager> &,
                        const GetMirrorsFunction &);

                ~FetchVisitor();

                void visit(const FetchableURISpecTree::NodeType<FetchableURIDepSpec>::Type & node);
                void visit(const FetchableURISpecTree::NodeType<URILabelsDepSpec>::Type & node);
                void visit(const FetchableURISpecTree::NodeType<AllDepSpec>::Type & node);
                void visit(const FetchableURISpecTree::NodeType<ConditionalDepSpec>::Type & node);
        };
    }
}

#endif
