/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "contents.hh"
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <list>

using namespace paludis;

template class ConstVisitor<ContentsVisitorTypes>;
template class ConstAcceptInterface<ContentsVisitorTypes>;

template class ConstAcceptInterfaceVisitsThis<ContentsVisitorTypes, ContentsFileEntry>;
template class ConstAcceptInterfaceVisitsThis<ContentsVisitorTypes, ContentsDirEntry>;
template class ConstAcceptInterfaceVisitsThis<ContentsVisitorTypes, ContentsSymEntry>;
template class ConstAcceptInterfaceVisitsThis<ContentsVisitorTypes, ContentsFifoEntry>;
template class ConstAcceptInterfaceVisitsThis<ContentsVisitorTypes, ContentsDevEntry>;
template class ConstAcceptInterfaceVisitsThis<ContentsVisitorTypes, ContentsMiscEntry>;

template class Visits<const ContentsFileEntry>;
template class Visits<const ContentsDirEntry>;
template class Visits<const ContentsSymEntry>;
template class Visits<const ContentsFifoEntry>;
template class Visits<const ContentsDevEntry>;
template class Visits<const ContentsMiscEntry>;

ContentsEntry::ContentsEntry(const std::string & n) :
    _name(n)
{
}

ContentsEntry::~ContentsEntry()
{
}

std::string
ContentsEntry::name() const
{
    return _name;
}

ContentsFileEntry::ContentsFileEntry(const std::string & our_name) :
    ContentsEntry(our_name)
{
}

ContentsDirEntry::ContentsDirEntry(const std::string & our_name) :
    ContentsEntry(our_name)
{
}

ContentsMiscEntry::ContentsMiscEntry(const std::string & our_name) :
    ContentsEntry(our_name)
{
}

ContentsFifoEntry::ContentsFifoEntry(const std::string & our_name) :
    ContentsEntry(our_name)
{
}

ContentsDevEntry::ContentsDevEntry(const std::string & our_name) :
    ContentsEntry(our_name)
{
}

ContentsSymEntry::ContentsSymEntry(const std::string & our_name, const std::string & our_target) :
    ContentsEntry(our_name),
    _target(our_target)
{
}

std::string
ContentsSymEntry::target() const
{
    return _target;
}

namespace paludis
{
    /**
     * Implementation data for Contents.
     *
     * \ingroup grpcontents
     */
    template<>
    struct Implementation<Contents>
    {
        std::list<tr1::shared_ptr<const ContentsEntry> > c;
    };
}

Contents::Contents() :
    PrivateImplementationPattern<Contents>(new Implementation<Contents>())
{
}

Contents::~Contents()
{
}

void
Contents::add(tr1::shared_ptr<const ContentsEntry> c)
{
    _imp->c.push_back(c);
}

Contents::ConstIterator
Contents::begin() const
{
    return ConstIterator(_imp->c.begin());
}

Contents::ConstIterator
Contents::end() const
{
    return ConstIterator(_imp->c.end());
}

std::ostream &
paludis::operator<< (std::ostream & s, const ContentsSymEntry & e)
{
    s << e.name() << " -> " << e.target();
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const ContentsEntry & e)
{
    s << e.name();
    return s;
}
