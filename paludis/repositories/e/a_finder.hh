/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_A_FINDER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_A_FINDER_HH 1

#include <paludis/dep_label.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/dep_spec.hh>
#include <paludis/spec_tree.hh>
#include <memory>
#include <list>

namespace paludis
{
    namespace erepository
    {
        class AFinder
        {
            private:
                std::list<std::pair<const FetchableURIDepSpec *, const URILabelsDepSpec *> > _specs;
                std::list<const URILabelsDepSpec *> _labels;

                const Environment * const env;
                const std::shared_ptr<const PackageID> id;

            public:
                AFinder(const Environment * const e, const std::shared_ptr<const PackageID> & i);

                void visit(const FetchableURISpecTree::NodeType<FetchableURIDepSpec>::Type & node);

                void visit(const FetchableURISpecTree::NodeType<URILabelsDepSpec>::Type & node);

                void visit(const FetchableURISpecTree::NodeType<AllDepSpec>::Type & node);

                void visit(const FetchableURISpecTree::NodeType<ConditionalDepSpec>::Type & node);

                typedef std::list<std::pair<const FetchableURIDepSpec *,
                        const URILabelsDepSpec *> >::const_iterator ConstIterator;

                ConstIterator begin() const;

                ConstIterator end() const;
        };
    }
}

#endif
