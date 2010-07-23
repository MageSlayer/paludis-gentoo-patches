/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2010 Ciaran McCreesh
 * Copyright (c) 2008 Fernando J. Pereda
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
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/options.hh>
#include <sys/types.h>
#include <functional>
#include <set>
#include <cstring>
#include <cerrno>

#include "config.h"

using namespace paludis;

#include <paludis/util/dir_iterator-se.cc>

typedef std::multiset<std::pair<ino_t, FSEntry>, std::function<bool (std::pair<ino_t, FSEntry>, std::pair<ino_t, FSEntry>)> > EntrySet;
template class Pimp<DirIterator>;

namespace paludis
{
    /**
     * Imp data for DirIterator.
     *
     * \ingroup grpfilesystem
     */
    template<>
    struct Imp<DirIterator>
    {
        std::shared_ptr<EntrySet> items;
        EntrySet::iterator iter;

        Imp(const std::shared_ptr<EntrySet> & ii) :
            items(ii)
        {
        }
    };

    bool compare_inode(const std::pair<ino_t, FSEntry> & a, const std::pair<ino_t, FSEntry> & b)
    {
        return a.first < b.first;
    }

    bool compare_name(const std::pair<ino_t, FSEntry> & a, const std::pair<ino_t, FSEntry> & b)
    {
        return a.second < b.second;
    }
}

DirOpenError::DirOpenError(const FSEntry & location, const int errno_value) throw () :
    FSError("Error opening directory '" + stringify(location) + "': " + std::strerror(errno_value))
{
}

DirIterator::DirIterator(const FSEntry & base, const DirIteratorOptions & options) :
    Pimp<DirIterator>(std::shared_ptr<EntrySet>())
{
    using namespace std::placeholders;

    if (options[dio_inode_sort])
        _imp->items = std::make_shared<EntrySet>(&compare_inode);
    else
        _imp->items = std::make_shared<EntrySet>(&compare_name);

    DIR * d(opendir(stringify(base).c_str()));
    if (0 == d)
        throw DirOpenError(base, errno);

    struct dirent * de;
    while (0 != ((de = readdir(d))))
        if (! options[dio_include_dotfiles])
        {
            if ('.' != de->d_name[0])
            {
#ifdef HAVE_DIRENT_DTYPE
                FSEntry f(stringify(base / std::string(de->d_name)), de->d_type);
#else
                FSEntry f(stringify(base / std::string(de->d_name)), 0);
#endif
                _imp->items->insert(std::make_pair(de->d_ino, f));
                if (options[dio_first_only])
                    break;
            }
        }
        else if (! (de->d_name[0] == '.' &&
                    (de->d_name[1] == '\0' || (de->d_name[1] == '.' && de->d_name[2] == '\0'))))
        {
#ifdef HAVE_DIRENT_DTYPE
            FSEntry f(stringify(base / std::string(de->d_name)), de->d_type);
#else
            FSEntry f(stringify(base / std::string(de->d_name)), 0);
#endif
            _imp->items->insert(std::make_pair(de->d_ino, f));
            if (options[dio_first_only])
                break;
        }

    _imp->iter = _imp->items->begin();

    closedir(d);
}

DirIterator::DirIterator(const DirIterator & other) :
    Pimp<DirIterator>(other._imp->items)
{
    _imp->iter = other._imp->iter;
}

DirIterator::DirIterator() :
    Pimp<DirIterator>(std::shared_ptr<EntrySet>(std::make_shared<EntrySet>(&compare_name)))
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
        _imp->items = other._imp->items;
        _imp->iter = other._imp->iter;
    }
    return *this;
}

const FSEntry &
DirIterator::operator* () const
{
    return _imp->iter->second;
}

const FSEntry *
DirIterator::operator-> () const
{
    return &_imp->iter->second;
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

