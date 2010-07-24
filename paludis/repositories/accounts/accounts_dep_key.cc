/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/dep_spec.hh>
#include <paludis/formatter.hh>
#include <paludis/environment.hh>
#include <paludis/util/pimp-impl.hh>
#include <sstream>
#include <list>

using namespace paludis;
using namespace paludis::accounts_repository;

namespace paludis
{
    template <>
    struct Imp<AccountsDepKey>
    {
        const Environment * const env;
        const std::shared_ptr<std::list<std::shared_ptr<PackageDepSpec> > > specs;
        const std::shared_ptr<DependencySpecTree> tree;
        const std::shared_ptr<DependenciesLabelSequence> initial_labels;

        Imp(const Environment * const e, const std::shared_ptr<const Set<std::string> > & s) :
            env(e),
            specs(std::make_shared<std::list<std::shared_ptr<PackageDepSpec> >>()),
            tree(std::make_shared<DependencySpecTree>(std::make_shared<AllDepSpec>())),
            initial_labels(std::make_shared<DependenciesLabelSequence>())
        {
            for (Set<std::string>::ConstIterator i(s->begin()), i_end(s->end()) ;
                    i != i_end ; ++i)
            {
                std::shared_ptr<PackageDepSpec> spec(std::make_shared<PackageDepSpec>(make_package_dep_spec({ })
                                .package(CategoryNamePart("group") + PackageNamePart(*i))));
                specs->push_back(spec);
                tree->root()->append(spec);
            }

            initial_labels->push_back(std::make_shared<DependenciesBuildLabel>("build", return_literal_function(true)));
            initial_labels->push_back(std::make_shared<DependenciesRunLabel>("run", return_literal_function(true)));
        }
    };
}

AccountsDepKey::AccountsDepKey(const Environment * const e,
        const std::shared_ptr<const Set<std::string> > & s) :
    Pimp<AccountsDepKey>(e, s)
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
AccountsDepKey::value() const
{
    return _imp->tree;
}

const std::shared_ptr<const DependenciesLabelSequence>
AccountsDepKey::initial_labels() const
{
    return _imp->initial_labels;
}

std::string
AccountsDepKey::pretty_print(const DependencySpecTree::ItemFormatter & f) const
{
    return f.indent(0) + pretty_print_flat(f) + "\n";
}

std::string
AccountsDepKey::pretty_print_flat(const DependencySpecTree::ItemFormatter & f) const
{
    std::stringstream s;

    for (std::list<std::shared_ptr<PackageDepSpec> >::const_iterator i(_imp->specs->begin()),
            i_end(_imp->specs->end()) ; i != i_end ; ++i)
    {
        if (! s.str().empty())
            s << ", ";

        if (_imp->env)
        {
            if (! (*_imp->env)[selection::SomeArbitraryVersion(generator::Matches(**i, { }) |
                        filter::InstalledAtRoot(_imp->env->root()))]->empty())
                s << f.format(**i, format::Installed());
            else if (! (*_imp->env)[selection::SomeArbitraryVersion(generator::Matches(**i, { }) |
                        filter::SupportsAction<InstallAction>() | filter::NotMasked())]->empty())
                s << f.format(**i, format::Installable());
            else
                s << f.format(**i, format::Plain());
        }
        else
            s << f.format(**i, format::Plain());
    }

    return s.str();
}

