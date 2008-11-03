/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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
#include <paludis/util/visitor.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/attributes.hh>
#include <paludis/dep_spec.hh>
#include <paludis/dep_tree.hh>
#include <tr1/memory>

namespace paludis
{
    namespace erepository
    {
        class PALUDIS_VISIBLE MyOptionsRequirementsVerifier :
            private PrivateImplementationPattern<MyOptionsRequirementsVerifier>,
            public ConstVisitor<PlainTextSpecTree>
        {
            public:
                MyOptionsRequirementsVerifier(const std::tr1::shared_ptr<const ERepositoryID> &);
                ~MyOptionsRequirementsVerifier();

                const std::tr1::shared_ptr<const Sequence<std::string> > unmet_requirements() const PALUDIS_ATTRIBUTE((warn_unused_result));

                void visit_leaf(const PlainTextLabelDepSpec &);

                void visit_leaf(const PlainTextDepSpec &);

                void visit_sequence(const ConditionalDepSpec &,
                        PlainTextSpecTree::ConstSequenceIterator,
                        PlainTextSpecTree::ConstSequenceIterator
                        );

                void visit_sequence(const AllDepSpec &,
                        PlainTextSpecTree::ConstSequenceIterator,
                        PlainTextSpecTree::ConstSequenceIterator
                        );
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<erepository::MyOptionsRequirementsVerifier>;
#endif
}

#endif
