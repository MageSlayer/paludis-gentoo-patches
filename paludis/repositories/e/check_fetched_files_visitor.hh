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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_CHECK_FETCHED_FILES_VISITOR_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_CHECK_FETCHED_FILES_VISITOR_HH 1

#include <paludis/repositories/e/e_repository_params.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/output_manager-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/spec_tree.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/action-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace erepository
    {
        class PALUDIS_VISIBLE CheckFetchedFilesVisitor :
            private PrivateImplementationPattern<CheckFetchedFilesVisitor>
        {
            private:
                bool check_distfile_manifest(const FSEntry & distfile);

            public:
                CheckFetchedFilesVisitor(
                        const Environment * const,
                        const std::tr1::shared_ptr<const PackageID> &,
                        const FSEntry & distdir,
                        const bool check_unneeded,
                        const bool fetch_restrict,
                        const FSEntry & m2,
                        const UseManifest um,
                        const std::tr1::shared_ptr<OutputManager> & output_manager,
                        const bool exclude_unmirrorable,
                        const bool ignore_unfetched,
                        const bool ignore_not_in_manifest);

                ~CheckFetchedFilesVisitor();

                void visit(const FetchableURISpecTree::NodeType<ConditionalDepSpec>::Type & node);
                void visit(const FetchableURISpecTree::NodeType<AllDepSpec>::Type & node);
                void visit(const FetchableURISpecTree::NodeType<URILabelsDepSpec>::Type & node);
                void visit(const FetchableURISpecTree::NodeType<FetchableURIDepSpec>::Type & node);

                const std::tr1::shared_ptr<const Sequence<FetchActionFailure> > failures() const PALUDIS_ATTRIBUTE((warn_unused_result));

                bool need_nofetch() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
