/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <sstream>
#include <list>

using namespace paludis;
using namespace paludis::resolver;

std::ostream &
paludis::resolver::operator<< (std::ostream & s, const Constraint & c)
{
    std::stringstream ss;
    ss << "Constraint(base spec: " << c.base_spec();
    if (c.is_blocker())
        ss << "; blocker";
    ss
        << "; use_installed: " << stringify(c.use_installed())
        << "; reason: " << (c.reason() ? stringify(*c.reason()) : "none")
        << ")";
    s << ss.str();

    return s;
}

namespace paludis
{
    template <>
    struct Implementation<Constraints>
    {
        UseInstalled strictest_use_installed;
        std::list<std::tr1::shared_ptr<const Constraint> > constraints;

        Implementation() :
            strictest_use_installed(ui_if_possible)
        {
        }
    };
}

Constraints::Constraints() :
    PrivateImplementationPattern<Constraints>(new Implementation<Constraints>)
{
}

Constraints::~Constraints()
{
}

Constraints::ConstIterator
Constraints::begin() const
{
    return ConstIterator(_imp->constraints.begin());
}

Constraints::ConstIterator
Constraints::end() const
{
    return ConstIterator(_imp->constraints.end());
}

void
Constraints::add(const std::tr1::shared_ptr<const Constraint> & c)
{
    _imp->constraints.push_back(c);
    _imp->strictest_use_installed = std::min(_imp->strictest_use_installed, c->use_installed());
}

bool
Constraints::empty() const
{
    return _imp->constraints.empty();
}

template class PrivateImplementationPattern<Constraints>;
template class WrappedForwardIterator<Constraints::ConstIteratorTag, const std::tr1::shared_ptr<const Constraint> >;

