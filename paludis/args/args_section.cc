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

#include <paludis/args/args_section.hh>
#include <paludis/args/args_handler.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <list>

using namespace paludis;
using namespace paludis::args;

namespace paludis
{
    template <>
    struct Imp<ArgsSection>
    {
        ArgsHandler * const handler;
        const std::string name;
        std::list<ArgsGroup *> groups;

        Imp(ArgsHandler * const h, const std::string & s) :
            handler(h),
            name(s)
        {
        }
    };

    template <>
    struct WrappedForwardIteratorTraits<ArgsSection::GroupsConstIteratorTag>
    {
        typedef IndirectIterator<std::list<ArgsGroup *>::const_iterator> UnderlyingIterator;
    };
}

ArgsSection::ArgsSection(ArgsHandler * const h, const std::string & s) :
    _imp(h, s)
{
    h->add(this);
}

ArgsSection::~ArgsSection()
{
}

ArgsSection::GroupsConstIterator
ArgsSection::begin() const
{
    return GroupsConstIterator(indirect_iterator(_imp->groups.begin()));
}

ArgsSection::GroupsConstIterator
ArgsSection::end() const
{
    return GroupsConstIterator(indirect_iterator(_imp->groups.end()));
}

void
ArgsSection::add(ArgsGroup * const g)
{
    _imp->groups.push_back(g);
}

void
ArgsSection::remove(ArgsGroup * const g)
{
    _imp->groups.remove(g);
}

ArgsHandler *
ArgsSection::handler() const
{
    return _imp->handler;
}

const std::string
ArgsSection::name() const
{
    return _imp->name;
}

namespace paludis
{
    template class Pimp<ArgsSection>;
    template class WrappedForwardIterator<args::ArgsSection::GroupsConstIteratorTag, const args::ArgsGroup>;
}
