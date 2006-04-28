/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

/** \file
 * Implementation for Contents classes.
 *
 * \ingroup grpcontents
 */

using namespace paludis;

ContentsEntry::~ContentsEntry()
{
}

ContentsFileEntry::ContentsFileEntry(const std::string & name) :
    ContentsEntry(name)
{
}

ContentsDirEntry::ContentsDirEntry(const std::string & name) :
    ContentsEntry(name)
{
}

ContentsMiscEntry::ContentsMiscEntry(const std::string & name) :
    ContentsEntry(name)
{
}

ContentsSymEntry::ContentsSymEntry(const std::string & name, const std::string & target) :
    ContentsEntry(name),
    _target(target)
{
}

Contents::Contents()
{
}

