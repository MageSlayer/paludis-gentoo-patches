/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2010, 2011 Ciaran McCreesh
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

#include <paludis/util/fs_iterator.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_error.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/util/tribool.hh>

#include <dirent.h>
#include <sys/types.h>
#include <functional>
#include <set>
#include <cstring>
#include <cerrno>

#include "config.h"

using namespace paludis;

#include <paludis/util/fs_iterator-se.cc>

typedef std::multiset<std::pair<ino_t, FSPath>, std::function<bool (std::pair<ino_t, FSPath>, std::pair<ino_t, FSPath>)> > EntrySet;

namespace paludis
{
    template<>
    struct Imp<FSIterator>
    {
        std::shared_ptr<EntrySet> items;
        EntrySet::iterator iter;

        Imp(const std::shared_ptr<EntrySet> & ii) :
            items(ii)
        {
        }
    };

    bool compare_inode(const std::pair<ino_t, FSPath> & a, const std::pair<ino_t, FSPath> & b)
    {
        return a.first < b.first;
    }

    bool compare_name(const std::pair<ino_t, FSPath> & a, const std::pair<ino_t, FSPath> & b)
    {
        return FSPathComparator()(a.second, b.second);
    }
}

FSIterator::FSIterator(const FSPath & base, const FSIteratorOptions & options) :
    _imp(std::shared_ptr<EntrySet>())
{
    using namespace std::placeholders;

    if (options[fsio_inode_sort])
        _imp->items = std::make_shared<EntrySet>(&compare_inode);
    else
        _imp->items = std::make_shared<EntrySet>(&compare_name);

    DIR * d(opendir(stringify(base).c_str()));
    if (0 == d)
        throw FSError("Error opening directory '" + stringify(base) + "'");

    bool have_any_special_wants(options[fsio_want_directories] || options[fsio_want_regular_files]);

    struct dirent * de;
    bool done(false);
    while (0 != ((de = readdir(d))))
    {
        if (done)
            break;

        bool want(false);

        if (! options[fsio_include_dotfiles])
        {
            if ('.' != de->d_name[0])
                want = true;
        }
        else if (! (de->d_name[0] == '.' &&
                    (de->d_name[1] == '\0' || (de->d_name[1] == '.' && de->d_name[2] == '\0'))))
            want = true;

        if (want)
        {
            FSPath f(stringify(base / std::string(de->d_name)));

            if (have_any_special_wants)
            {
                Tribool still_want(indeterminate);

#ifdef HAVE_DIRENT_DTYPE
                if (DT_UNKNOWN != de->d_type)
                {
                    int s(DTTOIF(de->d_type));

                    if (S_ISLNK(s))
                    {
                        if (! options[fsio_deref_symlinks_for_wants])
                            still_want = false;
                    }
                    else if (S_ISREG(s))
                        still_want = options[fsio_want_regular_files];
                    else if (S_ISDIR(s))
                        still_want = options[fsio_want_directories];
                    else
                        still_want = false;
                }
#endif

                if (still_want.is_indeterminate())
                {
                    FSStat f_stat(f);

                    if (f_stat.is_regular_file())
                        still_want = options[fsio_want_regular_files];
                    else if (f_stat.is_directory())
                        still_want = options[fsio_want_directories];
                    else if (f_stat.is_symlink())
                    {
                        if (options[fsio_deref_symlinks_for_wants])
                        {
                            if (f_stat.is_regular_file_or_symlink_to_regular_file())
                                still_want = options[fsio_want_regular_files];
                            else if (f_stat.is_directory_or_symlink_to_directory())
                                still_want = options[fsio_want_directories];
                            else
                                still_want = false;
                        }
                        else
                            still_want = false;
                    }
                    else
                        still_want = false;
                }

                want = still_want.is_true();
            }

            if (want)
            {
                _imp->items->insert(std::make_pair(de->d_ino, f));
                if (options[fsio_first_only])
                    done = true;
            }
        }
    }

    _imp->iter = _imp->items->begin();

    closedir(d);
}

FSIterator::FSIterator(const FSIterator & other) :
    _imp(other._imp->items)
{
    _imp->iter = other._imp->iter;
}

FSIterator::FSIterator() :
    _imp(std::shared_ptr<EntrySet>(std::make_shared<EntrySet>(&compare_name)))
{
    _imp->iter = _imp->items->end();
}

FSIterator::~FSIterator()
{
}

FSIterator &
FSIterator::operator= (const FSIterator & other)
{
    if (this != &other)
    {
        _imp->items = other._imp->items;
        _imp->iter = other._imp->iter;
    }
    return *this;
}

const FSPath &
FSIterator::operator* () const
{
    return _imp->iter->second;
}

const FSPath *
FSIterator::operator-> () const
{
    return &_imp->iter->second;
}

FSIterator &
FSIterator::operator++ ()
{
    ++_imp->iter;
    return *this;
}

FSIterator
FSIterator::operator++ (int)
{
    FSIterator c(*this);
    _imp->iter++;
    return c;
}

bool
paludis::operator== (const FSIterator & me, const FSIterator & other)
{
    if (other._imp->iter == other._imp->items->end())
        return me._imp->iter == me._imp->items->end();

    if (me._imp->iter == me._imp->items->end())
        return other._imp->iter == other._imp->items->end();

    if (other._imp->items != me._imp->items)
        throw InternalError(PALUDIS_HERE, "comparing two different FSIterators.");

    return other._imp->iter == me._imp->iter;
}

bool
paludis::operator!= (const FSIterator & me, const FSIterator & other)
{
    return ! operator== (me, other);
}

template class Pimp<FSIterator>;

