/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolver_functions.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/suggest_restart.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/make_named_values.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/string_list_stream.hh>

#include <paludis/user_dep_spec.hh>
#include <paludis/repository_factory.hh>
#include <paludis/serialise.hh>

#include <paludis/resolver/resolver_test.hh>

#include <list>
#include <functional>
#include <algorithm>
#include <map>

using namespace paludis;
using namespace paludis::resolver;
using namespace paludis::resolver::resolver_test;

namespace
{
    struct ResolverSerialisationTestCase : ResolverTestCase
    {
        std::shared_ptr<ResolverTestData> data;

        void SetUp()
        {
            data = std::make_shared<ResolverTestData>("serialisation", "exheres-0", "exheres");
        }

        void TearDown()
        {
            data.reset();
        }
    };
}

TEST_F(ResolverSerialisationTestCase, Serialisation)
{
    std::shared_ptr<const Resolved> resolved;
    {
        std::shared_ptr<const Resolved> orig_resolved(data->get_resolved("serialisation/target"));
        StringListStream str;
        Serialiser ser(str);
        orig_resolved->serialise(ser);
        str.nothing_more_to_write();

        Deserialiser deser(&data->env, str);
        Deserialisation desern("ResolverLists", deser);
        resolved = std::make_shared<Resolved>(Resolved::deserialise(desern));
    }

    this->check_resolved(resolved,
            n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("serialisation/dep"))
                .change(QualifiedPackageName("serialisation/target"))
                .finished()),
            n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .unable(QualifiedPackageName("serialisation/error"))
                .finished()),
            n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unorderable_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("serialisation/suggestion"))
                .finished()),
            n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished())
            );
}

