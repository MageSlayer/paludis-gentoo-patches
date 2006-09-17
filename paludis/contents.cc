/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <list>

/** \file
 * Implementation for Contents classes.
 *
 * \ingroup grpcontents
 */

using namespace paludis;

ContentsEntry::~ContentsEntry()
{
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

ContentsSymEntry::ContentsSymEntry(const std::string & our_name, const std::string & our_target) :
    ContentsEntry(our_name),
    _target(our_target)
{
}

namespace paludis
{
    /**
     * Implementation data for Contents.
     *
     * \ingroup grpcontents
     */
    template<>
    struct Implementation<Contents> :
        InternalCounted<Implementation<Contents> >
    {
        std::list<ContentsEntry::ConstPointer> c;
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
Contents::add(ContentsEntry::ConstPointer c)
{
    _imp->c.push_back(c);
}

Contents::Iterator
Contents::begin() const
{
    return Iterator(_imp->c.begin());
}

Contents::Iterator
Contents::end() const
{
    return Iterator(_imp->c.end());
}

