/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/util/fs_stat.hh>
#include <paludis/util/fs_error.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/timestamp.hh>

#include <string>
#include <cerrno>
#include <cstring>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<FSStat>
    {
        FSPath path;
        bool exists;
        struct stat st;

        Imp(const FSPath & p, bool x, const struct stat & s) :
            path(p),
            exists(x),
            st(s)
        {
        }

        Imp(const FSPath & p) :
            path(p),
            exists(false)
        {
            if (0 != lstat(stringify(p).c_str(), &st))
            {
                if (errno != ENOENT && errno != ENOTDIR)
                    throw FSError("Error running stat() on '" + stringify(p) + "': " + strerror(errno));
            }
            else
                exists = true;
        }
    };
}


FSStat::FSStat(const FSPath & p) :
    _imp(p)
{
}

FSStat::FSStat(const FSStat & p) :
    _imp(p._imp->path, p._imp->exists, p._imp->st)
{
}

FSStat &
FSStat::operator= (const FSStat & p)
{
    if (&p != this)
    {
        _imp->exists = p._imp->exists;
        _imp->st = p._imp->st;
    }
    return *this;
}

FSStat::~FSStat() = default;

bool
FSStat::exists() const
{
    return _imp->exists;
}

bool
FSStat::is_regular_file() const
{
    return _imp->exists && S_ISREG(_imp->st.st_mode);
}

bool
FSStat::is_regular_file_or_symlink_to_regular_file() const
{
    return _imp->exists && (S_ISREG(_imp->st.st_mode) || (is_symlink() && _imp->path.realpath_if_exists().stat().is_regular_file()));
}

bool
FSStat::is_directory() const
{
    return _imp->exists && S_ISDIR(_imp->st.st_mode);
}

bool
FSStat::is_directory_or_symlink_to_directory() const
{
    return _imp->exists && (S_ISDIR(_imp->st.st_mode) || (is_symlink() && _imp->path.realpath_if_exists().stat().is_directory()));
}

bool
FSStat::is_symlink() const
{
    return _imp->exists && (S_ISLNK(_imp->st.st_mode));
}

uid_t
FSStat::owner() const
{
    if (! _imp->exists)
        throw FSError("Filesystem entry '" + stringify(_imp->path) + "' does not exist");
    return _imp->st.st_uid;
}

gid_t
FSStat::group() const
{
    if (! _imp->exists)
        throw FSError("Filesystem entry '" + stringify(_imp->path) + "' does not exist");
    return _imp->st.st_gid;
}

Timestamp
FSStat::ctim() const
{
    if (! _imp->exists)
    {
        Context context("When fetching ctime of '" + stringify(_imp->path) + "':");
        throw FSError("Filesystem entry '" + stringify(_imp->path) + "' does not exist");
    }

    return Timestamp(_imp->st.st_ctim);
}

Timestamp
FSStat::mtim() const
{
    if (! _imp->exists)
    {
        Context context("When fetching mtime of '" + stringify(_imp->path) + "':");
        throw FSError("Filesystem entry '" + stringify(_imp->path) + "' does not exist");
    }

    return Timestamp(_imp->st.st_mtim);
}

mode_t
FSStat::permissions() const
{
    Context context("When fetching permissions for '" + stringify(_imp->path) + "':");

    if (! _imp->exists)
        throw FSError("Filesystem entry '" + stringify(_imp->path) + "' does not exist");

    return _imp->st.st_mode;
}

off_t
FSStat::file_size() const
{
    if (! _imp->exists)
        throw FSError("Filesystem entry '" + stringify(_imp->path) + "' does not exist");

    if (! is_regular_file())
        throw FSError("file_size called on non-regular file '" + stringify(_imp->path) + "'");

    return _imp->st.st_size;
}

std::pair<dev_t, ino_t>
FSStat::lowlevel_id() const
{
    if (! _imp->exists)
        throw FSError("Filesystem entry '" + stringify(_imp->path) + "' does not exist");

    return std::make_pair(_imp->st.st_dev, _imp->st.st_ino);
}

template class Pimp<FSStat>;

