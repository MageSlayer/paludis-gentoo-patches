/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/can_skip_phase.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/metadata_key.hh>
#include <paludis/dep_spec.hh>
#include <paludis/spec_tree.hh>
#include <set>
#include <algorithm>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    struct FindAnyFetchesFinder
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> package_id;
        bool result;

        FindAnyFetchesFinder(const Environment * const e, const std::shared_ptr<const PackageID> & id) :
            env(e),
            package_id(id),
            result(true)
        {
        }

        void visit(const FetchableURISpecTree::NodeType<AllDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const FetchableURISpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            if (node.spec()->condition_met(env, package_id))
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const FetchableURISpecTree::NodeType<FetchableURIDepSpec>::Type &)
        {
            result = false;
        }

        void visit(const FetchableURISpecTree::NodeType<URILabelsDepSpec>::Type &)
        {
        }
    };
}

bool
paludis::erepository::can_skip_phase(
        const Environment * const env,
        const std::shared_ptr<const ERepositoryID> & id,
        const EAPIPhase & phase)
{
    if (! id->defined_phases_key())
        return false;

    std::string skipifno(phase.equal_option("skipifno"));
    if (skipifno.empty())
        return false;

    std::set<std::string> skip_if_no_values;
    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(skipifno, ",", "",
            inserter(skip_if_no_values, skip_if_no_values.begin()));

    for (std::set<std::string>::const_iterator i(skip_if_no_values.begin()), i_end(skip_if_no_values.end()) ;
            i != i_end ; ++i)
    {
        if (*i == "*sources")
        {
            if (id->fetches_key())
            {
                FindAnyFetchesFinder f(env, id);
                id->fetches_key()->parse_value()->top()->accept(f);
                if (! f.result)
                    return false;
            }
        }
        else
        {
            auto d(id->defined_phases_key()->parse_value());
            if (d->end() != d->find(*i))
                return false;
        }
    }

    return true;
}

