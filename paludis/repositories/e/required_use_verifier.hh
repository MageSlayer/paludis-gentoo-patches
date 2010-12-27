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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_REQUIRED_USE_VERIFIER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_REQUIRED_USE_VERIFIER_HH 1

#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/attributes.hh>
#include <paludis/dep_spec.hh>
#include <paludis/spec_tree.hh>
#include <memory>

namespace paludis
{
    namespace erepository
    {
        class PALUDIS_VISIBLE RequiredUseVerifier :
            private Pimp<RequiredUseVerifier>
        {
            private:
                bool matches(const std::string &);

            public:
                RequiredUseVerifier(
                        const Environment * const,
                        const std::shared_ptr<const ERepositoryID> &);
                ~RequiredUseVerifier();

                const std::shared_ptr<const Sequence<std::string> > unmet_requirements() const PALUDIS_ATTRIBUTE((warn_unused_result));

                void visit(const RequiredUseSpecTree::NodeType<PlainTextDepSpec>::Type & node);
                void visit(const RequiredUseSpecTree::NodeType<AllDepSpec>::Type & node);
                void visit(const RequiredUseSpecTree::NodeType<AnyDepSpec>::Type & node);
                void visit(const RequiredUseSpecTree::NodeType<ExactlyOneDepSpec>::Type & node);
                void visit(const RequiredUseSpecTree::NodeType<ConditionalDepSpec>::Type & node);
        };
    }

    extern template class Pimp<erepository::RequiredUseVerifier>;
}

#endif
