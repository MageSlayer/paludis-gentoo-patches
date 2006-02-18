/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "nest_atom.hh"

using namespace paludis;

NestAtom::NestAtom()
{
}

NestAtom::~NestAtom()
{
}

const UseNestAtom *
NestAtom::as_use_nest_atom() const
{
    return 0;
}

CompositeNestAtom::CompositeNestAtom()
{
}

void
CompositeNestAtom::add_child(NestAtom::ConstPointer c)
{
    _children.push_back(c);
}

AllNestAtom::AllNestAtom()
{
}

UseNestAtom::UseNestAtom(const UseFlagName & flag, bool inverse) :
    _flag(flag),
    _inverse(inverse)
{
}

const UseNestAtom *
UseNestAtom::as_use_nest_atom() const
{
    return this;
}

TextNestAtom::TextNestAtom(const std::string & text) :
    _text(text)
{
}

TextNestAtom::~TextNestAtom()
{
}

