/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <dirent.h>
#include <errno.h>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/stringify.hh>
#include <sys/types.h>

/** \file
 * Implementation of paludis::DirIterator.
 *
 * \ingroup Filesystem
 */

using namespace paludis;

DirOpenError::DirOpenError(const FSEntry & location, const int errno_value) throw () :
    FSError("Error opening directory '" + stringify(location) + "': " + strerror(errno_value))
{
}

DirIterator::DirIterator(const FSEntry & base, bool ignore_dotfiles) :
    _base(base),
    _ignore_dotfiles(ignore_dotfiles),
    _items(new std::set<FSEntry>)
{
    DIR * d(opendir(stringify(base).c_str()));
    if (0 == d)
        throw DirOpenError(base, errno);

    struct dirent * de;
    while (0 != ((de = readdir(d))))
        if (ignore_dotfiles)
        {
            if ('.' != de->d_name[0])
                _items->insert(_base / std::string(de->d_name));
        }
        else if (! (de->d_name[0] == '.' &&
                    (de->d_name[1] == '\0' || (de->d_name[1] == '.' && de->d_name[2] == '\0'))))
            _items->insert(_base / std::string(de->d_name));

    _iter = _items->begin();

    closedir(d);
}

DirIterator::DirIterator(const DirIterator & other) :
    _base(other._base),
    _ignore_dotfiles(other._ignore_dotfiles),
    _items(other._items),
    _iter(other._iter)
{
}

DirIterator::DirIterator() :
    _base(""),
    _items(new std::set<FSEntry>),
    _iter(_items->end())
{
}

DirIterator::~DirIterator()
{
}

const DirIterator &
DirIterator::operator= (const DirIterator & other)
{
    if (this != &other)
    {
        _base = other._base;
        _items = other._items;
        _iter = other._iter;
    }
    return *this;
}

const FSEntry &
DirIterator::operator* () const
{
    return *_iter;
}

const FSEntry *
DirIterator::operator-> () const
{
    return &*_iter;
}

DirIterator &
DirIterator::operator++ ()
{
    ++_iter;
    return *this;
}

DirIterator
DirIterator::operator++ (int)
{
    DirIterator c(*this);
    _iter++;
    return c;
}

bool
DirIterator::operator== (const DirIterator & other) const
{
    if (other._iter == other._items->end())
        return _iter == _items->end();

    if (_iter == _items->end())
        return other._iter == other._items->end();

    if (other._items != _items)
        throw InternalError(PALUDIS_HERE,
                "comparing two different DirIterators.");

    return other._iter == _iter;
}

bool
DirIterator::operator!= (const DirIterator & other) const
{
    return ! operator== (other);
}

