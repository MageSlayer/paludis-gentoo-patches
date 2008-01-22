/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh
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
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <sys/types.h>
#include <set>
#include <cstring>
#include <cerrno>

using namespace paludis;

namespace paludis
{
    /**
     * Implementation data for DirIterator.
     *
     * \ingroup grpfilesystem
     */
    template<>
    struct Implementation<DirIterator>
    {
        FSEntry base;
        bool ignore_dotfiles;
        tr1::shared_ptr<std::set<FSEntry> > items;
        std::set<FSEntry>::iterator iter;

        Implementation(const FSEntry & b, bool i, tr1::shared_ptr<std::set<FSEntry> > ii) :
            base(b),
            ignore_dotfiles(i),
            items(ii)
        {
        }
    };
}

DirOpenError::DirOpenError(const FSEntry & location, const int errno_value) throw () :
    FSError("Error opening directory '" + stringify(location) + "': " + std::strerror(errno_value))
{
}

DirIterator::DirIterator(const FSEntry & base, bool ignore_dotfiles) :
    PrivateImplementationPattern<DirIterator>(new Implementation<DirIterator>(
                base, ignore_dotfiles, tr1::shared_ptr<std::set<FSEntry> >(new std::set<FSEntry>)))
{
    DIR * d(opendir(stringify(_imp->base).c_str()));
    if (0 == d)
        throw DirOpenError(_imp->base, errno);

    struct dirent * de;
    while (0 != ((de = readdir(d))))
        if (_imp->ignore_dotfiles)
        {
            if ('.' != de->d_name[0])
                _imp->items->insert(_imp->base / std::string(de->d_name));
        }
        else if (! (de->d_name[0] == '.' &&
                    (de->d_name[1] == '\0' || (de->d_name[1] == '.' && de->d_name[2] == '\0'))))
            _imp->items->insert(_imp->base / std::string(de->d_name));

    _imp->iter = _imp->items->begin();

    closedir(d);
}

DirIterator::DirIterator(const DirIterator & other) :
    PrivateImplementationPattern<DirIterator>(new Implementation<DirIterator>(
                other._imp->base, other._imp->ignore_dotfiles, other._imp->items))
{
    _imp->iter = other._imp->iter;
}

DirIterator::DirIterator() :
    PrivateImplementationPattern<DirIterator>(new Implementation<DirIterator>(
                FSEntry(""), true, tr1::shared_ptr<std::set<FSEntry> >(new std::set<FSEntry>)))
{
    _imp->iter = _imp->items->end();
}

DirIterator::~DirIterator()
{
}

DirIterator &
DirIterator::operator= (const DirIterator & other)
{
    if (this != &other)
    {
        _imp->base = other._imp->base;
        _imp->items = other._imp->items;
        _imp->iter = other._imp->iter;
        _imp->ignore_dotfiles = other._imp->ignore_dotfiles;
    }
    return *this;
}

const FSEntry &
DirIterator::operator* () const
{
    return *_imp->iter;
}

const FSEntry *
DirIterator::operator-> () const
{
    return &*_imp->iter;
}

DirIterator &
DirIterator::operator++ ()
{
    ++_imp->iter;
    return *this;
}

DirIterator
DirIterator::operator++ (int)
{
    DirIterator c(*this);
    _imp->iter++;
    return c;
}

bool
DirIterator::operator== (const DirIterator & other) const
{
    if (other._imp->iter == other._imp->items->end())
        return _imp->iter == _imp->items->end();

    if (_imp->iter == _imp->items->end())
        return other._imp->iter == other._imp->items->end();

    if (other._imp->items != _imp->items)
        throw InternalError(PALUDIS_HERE,
                "comparing two different DirIterators.");

    return other._imp->iter == _imp->iter;
}

bool
DirIterator::operator!= (const DirIterator & other) const
{
    return ! operator== (other);
}

