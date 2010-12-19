/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_MYOPTIONS_REQUIREMENTS_VERIFIER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_MYOPTIONS_REQUIREMENTS_VERIFIER_HH 1

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
        class PALUDIS_VISIBLE MyOptionsRequirementsVerifier :
            private Pimp<MyOptionsRequirementsVerifier>
        {
            private:
                void verify_one(const ChoicePrefixName &, const std::string &,
                        const std::shared_ptr<const DepSpecAnnotations> &);

            public:
                MyOptionsRequirementsVerifier(const std::shared_ptr<const ERepositoryID> &);
                ~MyOptionsRequirementsVerifier();

                const std::shared_ptr<const Sequence<std::string> > unmet_requirements() const PALUDIS_ATTRIBUTE((warn_unused_result));

                void visit(const PlainTextSpecTree::NodeType<PlainTextLabelDepSpec>::Type & node);
                void visit(const PlainTextSpecTree::NodeType<PlainTextDepSpec>::Type & node);
                void visit(const PlainTextSpecTree::NodeType<AllDepSpec>::Type & node);
                void visit(const PlainTextSpecTree::NodeType<ConditionalDepSpec>::Type & node);
        };
    }

    extern template class Pimp<erepository::MyOptionsRequirementsVerifier>;
}

#endif
