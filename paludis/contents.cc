/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/contents.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <list>

using namespace paludis;

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
    template<>
    struct Implementation<Contents>
    {
        std::list<std::tr1::shared_ptr<const ContentsEntry> > c;
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
Contents::add(const std::tr1::shared_ptr<const ContentsEntry> & c)
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
paludis::operator<< (std::ostream & s, const ContentsEntry & e)
{
    s << e.as_string();
    return s;
}

const std::string
ContentsEntry::as_string() const
{
    return name();
}

const std::string
ContentsSymEntry::as_string() const
{
    return name() + " -> " + target();
}

template class InstantiationPolicy<ContentsEntry, instantiation_method::NonCopyableTag>;
template class InstantiationPolicy<Contents, instantiation_method::NonCopyableTag>;

template class PrivateImplementationPattern<Contents>;

template class WrappedForwardIterator<Contents::ConstIteratorTag, const std::tr1::shared_ptr<const ContentsEntry> >;

