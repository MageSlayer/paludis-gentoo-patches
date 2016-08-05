/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2009 David Leverton
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
#include <paludis/util/sequence.hh>
#include <paludis/util/map-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/make_shared_copy.hh>

#include <paludis/user_dep_spec.hh>
#include <paludis/repository_factory.hh>

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
    struct ResolverAnyTestCase : ResolverTestCase
    {
        std::shared_ptr<ResolverTestData> data;

        void SetUp() override
        {
            data = std::make_shared<ResolverTestData>("any", "exheres-0", "exheres");
            data->get_use_existing_nothing_helper.set_use_existing_for_dependencies(ue_never);
        }

        void TearDown() override
        {
            data.reset();
        }
    };
}

TEST_F(ResolverAnyTestCase, EmptyAlternative)
{
    std::shared_ptr<const Resolved> resolved(data->get_resolved("test/target"));

    this->check_resolved(resolved,
            n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("test/target"))
                .finished()),
            n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unorderable_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished())
            );
}

TEST_F(ResolverAnyTestCase, EmptyAlternativeWithUpdate)
{
    data->install("test", "dep", "2");
    std::shared_ptr<const Resolved> resolved(data->get_resolved("test/target"));

    this->check_resolved(resolved,
            n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("test/target"))
                .finished()),
            n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unorderable_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished())
            );
}

TEST_F(ResolverAnyTestCase, EmptyAlternativeWithUntakenUpgrade)
{
    data->install("test", "dep", "1");
    std::shared_ptr<const Resolved> resolved(data->get_resolved("test/target"));

    this->check_resolved(resolved,
            n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("test/target"))
                .finished()),
            n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unorderable_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished())
            );
}

namespace
{
    enum TriboolValue { tri_true, tri_false, tri_indeterminate };

    template <TriboolValue a_, TriboolValue b_>
    struct ResolverAnyTestEmptyPreferences :
        ResolverAnyTestCase
    {
        void common_test_code()
        {
            Tribool a(a_ == tri_true ? true : a_ == tri_false ? false : Tribool(indeterminate));
            Tribool b(b_ == tri_true ? true : b_ == tri_false ? false : Tribool(indeterminate));

            if (a.is_true())
                data->prefer_or_avoid_helper.add_prefer_name(QualifiedPackageName("preferences/dep-a"));
            else if (a.is_false())
                data->prefer_or_avoid_helper.add_avoid_name(QualifiedPackageName("preferences/dep-a"));

            if (b.is_true())
                data->prefer_or_avoid_helper.add_prefer_name(QualifiedPackageName("preferences/dep-b"));
            else if (b.is_false())
                data->prefer_or_avoid_helper.add_avoid_name(QualifiedPackageName("preferences/dep-b"));

            std::shared_ptr<const Resolved> resolved(data->get_resolved("preferences/target"));

            std::shared_ptr<DecisionChecks> checks;

            if (a.is_true())
            {
                checks = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("preferences/dep-a"))
                        .change(QualifiedPackageName("preferences/target"))
                        .finished());
            }
            else if (b.is_true())
            {
                checks = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("preferences/dep-b"))
                        .change(QualifiedPackageName("preferences/target"))
                        .finished());
            }
            else if (a.is_false())
            {
                checks = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("preferences/dep-middle"))
                        .change(QualifiedPackageName("preferences/target"))
                        .finished());
            }
            else
            {
                checks = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("preferences/dep-a"))
                        .change(QualifiedPackageName("preferences/target"))
                        .finished());
            }

            this->check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = checks,
                    n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::taken_unorderable_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished())
                    );
        }
    };
}

typedef ResolverAnyTestEmptyPreferences<tri_true, tri_true> TestEmptyPreferencesTT;
typedef ResolverAnyTestEmptyPreferences<tri_true, tri_false> TestEmptyPreferencesTF;
typedef ResolverAnyTestEmptyPreferences<tri_true, tri_indeterminate> TestEmptyPreferencesTI;
typedef ResolverAnyTestEmptyPreferences<tri_false, tri_true> TestEmptyPreferencesFT;
typedef ResolverAnyTestEmptyPreferences<tri_false, tri_false> TestEmptyPreferencesFF;
typedef ResolverAnyTestEmptyPreferences<tri_false, tri_indeterminate> TestEmptyPreferencesFI;
typedef ResolverAnyTestEmptyPreferences<tri_indeterminate, tri_true> TestEmptyPreferencesIT;
typedef ResolverAnyTestEmptyPreferences<tri_indeterminate, tri_false> TestEmptyPreferencesIF;
typedef ResolverAnyTestEmptyPreferences<tri_indeterminate, tri_indeterminate> TestEmptyPreferencesII;

TEST_F(TestEmptyPreferencesTT, Works) { common_test_code(); }
TEST_F(TestEmptyPreferencesTF, Works) { common_test_code(); }
TEST_F(TestEmptyPreferencesTI, Works) { common_test_code(); }
TEST_F(TestEmptyPreferencesFT, Works) { common_test_code(); }
TEST_F(TestEmptyPreferencesFF, Works) { common_test_code(); }
TEST_F(TestEmptyPreferencesFI, Works) { common_test_code(); }
TEST_F(TestEmptyPreferencesIT, Works) { common_test_code(); }
TEST_F(TestEmptyPreferencesIF, Works) { common_test_code(); }
TEST_F(TestEmptyPreferencesII, Works) { common_test_code(); }

TEST_F(ResolverAnyTestCase, SelfUseFirst)
{
    std::shared_ptr<const Resolved> resolved(data->get_resolved("self-use-first/target"));

    this->check_resolved(resolved,
            n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("self-use-first/dep"))
                .change(QualifiedPackageName("self-use-first/target"))
                .finished()),
            n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unorderable_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished())
            );
}

TEST_F(ResolverAnyTestCase, SelfUseSecond)
{
    std::shared_ptr<const Resolved> resolved(data->get_resolved("self-use-second/target"));

    this->check_resolved(resolved,
            n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("self-use-second/dep"))
                .change(QualifiedPackageName("self-use-second/target"))
                .finished()),
            n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unorderable_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished())
            );
}

TEST_F(ResolverAnyTestCase, SelfUseNeither)
{
    std::shared_ptr<const Resolved> resolved(data->get_resolved("self-use-neither/target"));

    this->check_resolved(resolved,
            n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("self-use-neither/dep"))
                .change(QualifiedPackageName("self-use-neither/target"))
                .finished()),
            n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("self-use-neither/dep"))
                .finished()),
            n::taken_unorderable_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished())
            );
}

TEST_F(ResolverAnyTestCase, SelfOrOther)
{
    data->install("self-or-other", "other", "1");
    data->get_use_existing_nothing_helper.set_use_existing_for_dependencies(ue_if_same_version);

    std::shared_ptr<const Resolved> resolved(data->get_resolved("self-or-other/target"));

    this->check_resolved(resolved,
            n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("self-or-other/target"))
                .finished()),
            n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unorderable_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished())
            );
}

TEST_F(ResolverAnyTestCase, UpgradeOverAny)
{
    data->install("upgrade-over-any", "dep", "1");

    std::shared_ptr<const Resolved> resolved(data->get_resolved("upgrade-over-any/target"));

    this->check_resolved(resolved,
            n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("upgrade-over-any/dep"))
                .change(QualifiedPackageName("upgrade-over-any/target"))
                .finished()),
            n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unorderable_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished())
            );
}

TEST_F(ResolverAnyTestCase, NoActiveLabels)
{
    std::shared_ptr<const Resolved> resolved(data->get_resolved("no-active-labels/target"));

    this->check_resolved(resolved,
            n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("no-active-labels/target"))
                .finished()),
            n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unorderable_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished())
            );
}

