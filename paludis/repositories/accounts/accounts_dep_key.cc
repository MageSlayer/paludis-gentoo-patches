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

#include <paludis/repositories/accounts/accounts_dep_key.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/always_enabled_dependency_label.hh>
#include <paludis/pretty_printer.hh>
#include <sstream>
#include <list>

using namespace paludis;
using namespace paludis::accounts_repository;

namespace
{
    struct AccountsDepKeyData :
        Singleton<AccountsDepKeyData>
    {
        const std::shared_ptr<DependenciesLabelSequence> initial_labels;

        AccountsDepKeyData() :
            initial_labels(std::make_shared<DependenciesLabelSequence>())
        {
            initial_labels->push_back(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesBuildLabelTag> >("build"));
            initial_labels->push_back(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesRunLabelTag> >("run"));
        }
    };
}

namespace paludis
{
    template <>
    struct Imp<AccountsDepKey>
    {
        const Environment * const env;
        const std::shared_ptr<std::list<std::shared_ptr<PackageDepSpec> > > specs;
        const std::shared_ptr<DependencySpecTree> tree;

        Imp(const Environment * const e, const std::shared_ptr<const Set<std::string> > & s) :
            env(e),
            specs(std::make_shared<std::list<std::shared_ptr<PackageDepSpec> >>()),
            tree(std::make_shared<DependencySpecTree>(std::make_shared<AllDepSpec>()))
        {
            for (const auto & i : *s)
            {
                std::shared_ptr<PackageDepSpec> spec(std::make_shared<PackageDepSpec>(make_package_dep_spec({ })
                                .package(CategoryNamePart("group") + PackageNamePart(i))));
                specs->push_back(spec);
                tree->top()->append(spec);
            }
        }
    };
}

AccountsDepKey::AccountsDepKey(const Environment * const e,
        const std::shared_ptr<const Set<std::string> > & s) :
    _imp(e, s)
{
}

AccountsDepKey::~AccountsDepKey()
{
}

const std::string
AccountsDepKey::raw_name() const
{
    return "dependencies";
}

const std::string
AccountsDepKey::human_name() const
{
    return "Dependencies";
}

MetadataKeyType
AccountsDepKey::type() const
{
    return mkt_dependencies;
}

const std::shared_ptr<const DependencySpecTree>
AccountsDepKey::parse_value() const
{
    return _imp->tree;
}

const std::shared_ptr<const DependenciesLabelSequence>
AccountsDepKey::initial_labels() const
{
    return AccountsDepKeyData::get_instance()->initial_labels;
}

const std::string
AccountsDepKey::pretty_print_value(
        const PrettyPrinter & pretty_printer,
        const PrettyPrintOptions &) const
{
    std::stringstream s;

    for (const auto & i : *_imp->specs)
    {
        if (! s.str().empty())
            s << ", ";

        s << pretty_printer.prettify(*i);
    }

    return s.str();
}

