/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009 Ciaran McCreesh
 * Copyright (c) 2006 Mark Loeser
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

#include <paludis/util/exception.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>

#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "config.h"

/** \file
 * Implementation of paludis::FSEntry.
 *
 * \ingroup grpfilesystem
 */

using namespace paludis;

FSError::FSError(const std::string & our_message) throw () :
    Exception(our_message)
{
}

namespace paludis
{
    enum CheckedInfo
    {
        ifse_none,
        ifse_exists,
        ifse_type,
        ifse_full,
        ifse_max
    };

    template <>
    struct Implementation<FSEntry>
    {
        std::string path;

        mutable Mutex mutex;
        mutable std::tr1::shared_ptr<struct ::stat> stat_info;
        mutable bool exists;
        mutable CheckedInfo checked;

        Implementation(const std::string & p) :
            path(p),
            exists(false),
            checked(ifse_none)
        {
        }

        Implementation(const std::string & p, unsigned char d_type) :
            path(p),
            exists(true),
            checked(ifse_exists)
        {
#ifdef HAVE_DIRENT_DTYPE
            if (DT_UNKNOWN != d_type)
            {
                stat_info.reset(new struct ::stat);
                stat_info->st_mode = DTTOIF(d_type);
                checked = ifse_type;
            }
#endif
        }

    };
}

FSEntry::FSEntry(const std::string & path) :
    PrivateImplementationPattern<FSEntry>(new Implementation<FSEntry>(path))
{
    _normalise();
}

FSEntry::FSEntry(const FSEntry & other) :
    PrivateImplementationPattern<FSEntry>(new Implementation<FSEntry>(other._imp->path))
{
    Lock l(other._imp->mutex);
    _imp->stat_info = other._imp->stat_info;
    _imp->exists = other._imp->exists;
    _imp->checked = other._imp->checked;
}

FSEntry::FSEntry(const std::string & path, unsigned char d_type) :
    PrivateImplementationPattern<FSEntry>(new Implementation<FSEntry>(path, d_type))
{
    _normalise();
}

FSEntry::~FSEntry()
{
}

const FSEntry &
FSEntry::operator= (const FSEntry & other)
{
    Lock l(other._imp->mutex);
    _imp->path = other._imp->path;
    _imp->stat_info = other._imp->stat_info;
    _imp->exists = other._imp->exists;
    _imp->checked = other._imp->checked;

    return *this;
}

const FSEntry &
FSEntry::operator/= (const FSEntry & rhs)
{
    if (rhs._imp->path == "/")
        return *this;

    if (_imp->path.empty() || '/' != _imp->path.at(_imp->path.length() - 1))
        _imp->path.append("/");

    if (! rhs._imp->path.empty())
    {
        if ('/' == rhs._imp->path.at(0))
            _imp->path.append(rhs._imp->path.substr(1));
        else
            _imp->path.append(rhs._imp->path);
    }

    _imp->checked = ifse_none;
    _imp->exists = false;
    _imp->stat_info.reset();

    return *this;
}

FSEntry
FSEntry::operator/ (const std::string & rhs) const
{
    return *this / FSEntry(rhs);
}

bool
FSEntry::operator< (const FSEntry & other) const
{
    return _imp->path < other._imp->path;
}

bool
FSEntry::operator== (const FSEntry & other) const
{
    return _imp->path == other._imp->path;
}

bool
FSEntry::exists() const
{
    if (_imp->checked < ifse_exists)
        _stat();

    return _imp->exists;
}

bool
FSEntry::is_directory() const
{
    if (_imp->checked < ifse_type)
        _stat();

    if (_imp->exists)
        return S_ISDIR((*_imp->stat_info).st_mode);

    return false;
}

bool
FSEntry::is_directory_or_symlink_to_directory() const
{
    if (_imp->checked < ifse_type)
        _stat();

    if (_imp->exists)
        return S_ISDIR((*_imp->stat_info).st_mode) ||
            (is_symbolic_link() && realpath_if_exists().is_directory());

    return false;
}

bool
FSEntry::is_fifo() const
{
    if (_imp->checked < ifse_type)
        _stat();

    if (_imp->exists)
        return S_ISFIFO((*_imp->stat_info).st_mode);

    return false;
}

bool
FSEntry::is_device() const
{
    if (_imp->checked < ifse_type)
        _stat();

    if (_imp->exists)
        return S_ISBLK((*_imp->stat_info).st_mode) || S_ISCHR((*_imp->stat_info).st_mode);

    return false;
}

bool
FSEntry::is_regular_file() const
{
    if (_imp->checked < ifse_type)
        _stat();

    if (_imp->exists)
        return S_ISREG((*_imp->stat_info).st_mode);

    return false;
}

bool
FSEntry::is_regular_file_or_symlink_to_regular_file() const
{
    if (_imp->checked < ifse_type)
        _stat();

    if (_imp->exists)
        return S_ISREG((*_imp->stat_info).st_mode) ||
            (is_symbolic_link() && realpath_if_exists().is_regular_file());

    return false;
}

bool
FSEntry::is_symbolic_link() const
{
    if (_imp->checked < ifse_type)
        _stat();

    if (_imp->exists)
        return S_ISLNK((*_imp->stat_info).st_mode);

    return false;
}


bool
FSEntry::has_permission(const FSUserGroup & user_group, const FSPermission & fs_perm) const
{
    Context context("When checking permissions on '" + stringify(_imp->path) + "':");

    _stat();

    if (! _imp->exists)
        throw FSError("Filesystem entry '" + _imp->path + "' does not exist");

    switch (user_group)
    {
        case fs_ug_owner:
            {
                switch (fs_perm)
                {
                    case fs_perm_read:
                        return (*_imp->stat_info).st_mode & S_IRUSR;
                    case fs_perm_write:
                        return (*_imp->stat_info).st_mode & S_IWUSR;
                    case fs_perm_execute:
                        return (*_imp->stat_info).st_mode & S_IXUSR;
                }
                throw InternalError(PALUDIS_HERE, "Unhandled FSPermission");
            }
        case fs_ug_group:
            {
                switch (fs_perm)
                {
                    case fs_perm_read:
                        return (*_imp->stat_info).st_mode & S_IRGRP;
                    case fs_perm_write:
                        return (*_imp->stat_info).st_mode & S_IWGRP;
                    case fs_perm_execute:
                        return (*_imp->stat_info).st_mode & S_IXGRP;
                }
                throw InternalError(PALUDIS_HERE, "Unhandled FSPermission");
            }
        case fs_ug_others:
            {
                switch (fs_perm)
                {
                    case fs_perm_read:
                        return (*_imp->stat_info).st_mode & S_IROTH;
                    case fs_perm_write:
                        return (*_imp->stat_info).st_mode & S_IWOTH;
                    case fs_perm_execute:
                        return (*_imp->stat_info).st_mode & S_IXOTH;
                }
                throw InternalError(PALUDIS_HERE, "Unhandled FSPermission");
            }
    }

    throw InternalError(PALUDIS_HERE, "Unhandled FSUserGroup");
}

mode_t
FSEntry::permissions() const
{
    Context context("When fetching permissions for '" + stringify(_imp->path) + "':");

    _stat();

    if (! _imp->exists)
        throw FSError("Filesystem entry '" + _imp->path + "' does not exist");

    return _imp->stat_info->st_mode;
}

void
FSEntry::_normalise()
{
    try
    {
        if (std::string::npos != _imp->path.find("//"))
        {
            std::string new_path;
            std::string::size_type p(0);
            while (p < _imp->path.length())
            {
                if ('/' == _imp->path[p])
                {
                    new_path += '/';
                    while (++p < _imp->path.length())
                        if ('/' != _imp->path[p])
                            break;
                }
                else
                    new_path += _imp->path[p++];
            }
            _imp->path = new_path;
        }

        if (! _imp->path.empty())
            if ('/' == _imp->path.at(_imp->path.length() - 1))
                _imp->path.erase(_imp->path.length() - 1);
        if (_imp->path.empty())
            _imp->path = "/";
    }
    catch (const std::exception & e)
    {
        Context c("When normalising FSEntry path '" + _imp->path + "':");
        throw InternalError(PALUDIS_HERE,
                "caught std::exception '" + stringify(e.what()) + "'");
    }
}

void
FSEntry::_stat() const
{
    Lock l(_imp->mutex);
    if (_imp->checked == ifse_full)
        return;

    Context context("When calling stat() on '" + stringify(_imp->path) + "':");

    _imp->stat_info.reset(new struct stat);
    if (0 != lstat(_imp->path.c_str(), _imp->stat_info.get()))
    {
        if (errno != ENOENT && errno != ENOTDIR)
            throw FSError("Error running stat() on '" + stringify(_imp->path) + "': "
                    + strerror(errno));

        _imp->exists = false;
        _imp->stat_info.reset();
    }
    else
        _imp->exists = true;

    _imp->checked = ifse_full;
}

std::string
FSEntry::basename() const
{
    if (_imp->path == "/")
        return _imp->path;

    return _imp->path.substr(_imp->path.rfind('/') + 1);
}

FSEntry
FSEntry::strip_leading(const FSEntry & f) const
{
    std::string root(stringify(f));

    if (root == "/")
        root.clear();
    if (0 != _imp->path.compare(0, root.length(), root))
        throw FSError("Can't strip leading '" + root + "' from FSEntry '" + _imp->path + "'");
    return FSEntry(_imp->path.substr(root.length()));
}

FSEntry
FSEntry::dirname() const
{
    if (_imp->path == "/")
        return FSEntry(_imp->path);

    return FSEntry(_imp->path.substr(0, _imp->path.rfind('/')));
}

FSEntry
FSEntry::realpath() const
{
    Context context("When fetching realpath of '" + stringify(_imp->path) + "':");

#ifdef HAVE_CANONICALIZE_FILE_NAME
    char * r(canonicalize_file_name(_imp->path.c_str()));
    if (! r)
        throw FSError("Could not resolve path '" + _imp->path + "'");
    FSEntry result(r);
    std::free(r);
    return result;
#else
    char r[PATH_MAX + 1];
    std::memset(r, 0, PATH_MAX + 1);
    if (! exists())
        throw FSError("Could not resolve path '" + _imp->path + "'");
    if (! ::realpath(_imp->path.c_str(), r))
        throw FSError("Could not resolve path '" + _imp->path + "'");
    FSEntry result(r);
    if (! result.exists())
        throw FSError("Could not resolve path '" + _imp->path + "'");
    return result;
#endif
}

FSEntry
FSEntry::realpath_if_exists() const
{
    Context context("When fetching realpath of '" + stringify(_imp->path) + "', if it exists:");

#ifdef HAVE_CANONICALIZE_FILE_NAME
    char * r(canonicalize_file_name(_imp->path.c_str()));
    if (! r)
        return *this;
    FSEntry result(r);
    std::free(r);
    return result;
#else
    char r[PATH_MAX + 1];
    std::memset(r, 0, PATH_MAX + 1);
    if (! exists())
        return *this;
    if (! ::realpath(_imp->path.c_str(), r))
        return *this;
    FSEntry result(r);
    if (! result.exists())
        return *this;
    return result;
#endif
}

FSEntry
FSEntry::cwd()
{
    char r[PATH_MAX + 1];
    std::memset(r, 0, PATH_MAX + 1);
    if (! ::getcwd(r, PATH_MAX))
        throw FSError("Could not get current working directory");
    return FSEntry(r);
}

std::ostream &
paludis::operator<< (std::ostream & s, const FSEntry & f)
{
    s << f._imp->path;
    return s;
}

time_t
FSEntry::ctime() const
{
    _stat();

    if (! _imp->exists)
        throw FSError("Filesystem entry '" + _imp->path + "' does not exist");

    return (*_imp->stat_info).st_ctime;
}

time_t
FSEntry::mtime() const
{
    _stat();

    if (! _imp->exists)
        throw FSError("Filesystem entry '" + _imp->path + "' does not exist");

    return (*_imp->stat_info).st_mtime;
}

off_t
FSEntry::file_size() const
{
    _stat();

    if (! _imp->exists)
        throw FSError("Filesystem entry '" + _imp->path + "' does not exist");

    if (! is_regular_file())
        throw FSError("file_size called on non-regular file '" + _imp->path + "'");

    return _imp->stat_info->st_size;
}

bool
FSEntry::mkdir(mode_t mode)
{
    if (0 == ::mkdir(_imp->path.c_str(), mode))
        return true;

    int e(errno);
    if (e == EEXIST)
    {
        if (is_directory())
            return false;
        throw FSError("mkdir '" + _imp->path + "' failed: target exists and is not a directory");
    }
    else
        throw FSError("mkdir '" + _imp->path + "' failed: " + ::strerror(e));
}

bool
FSEntry::symlink(const std::string & target)
{
    if (0 == ::symlink(target.c_str(), _imp->path.c_str()))
        return true;

    int e(errno);
    if (e == EEXIST)
    {
        if (is_symbolic_link() && target == readlink())
            return false;
        throw FSError("symlink '" + _imp->path + "' to '" + target + "' failed: target exists");
    }
    else
        throw FSError("symlink '" + _imp->path + "' to '" + target + "' failed: " + ::strerror(e));
}

bool
FSEntry::unlink()
{
#ifdef HAVE_LCHFLAGS
    if (0 != ::lchflags(_imp->path.c_str(), 0))
    {
        int e(errno);
        if (e != ENOENT)
            throw FSError("lchflags for unlink '" + _imp->path + "' failed: " + ::strerror(e));
    }
#endif

    if (0 == ::unlink(_imp->path.c_str()))
        return true;

    int e(errno);
    if (e == ENOENT)
        return false;
    else
        throw FSError("unlink '" + _imp->path + "' failed: " + ::strerror(e));
}

bool
FSEntry::rmdir()
{
    if (0 == ::rmdir(_imp->path.c_str()))
        return true;

    int e(errno);
    if (e == ENOENT)
        return false;
    else
        throw FSError("rmdir '" + _imp->path + "' failed: " + ::strerror(e));
}

bool
FSEntry::utime(const struct ::utimbuf * buf)
{
    if (0 == ::utime(_imp->path.c_str(), buf))
        return true;

    int e(errno);
    if (e == ENOENT)
        return false;
    else
        throw FSError("utime '" + _imp->path + "' failed: " + ::strerror(e));
}

std::string
FSEntry::readlink() const
{
    char buf[PATH_MAX + 1];
    std::memset(buf, 0, PATH_MAX + 1);
    if (-1 == ::readlink(_imp->path.c_str(), buf, PATH_MAX))
        throw FSError("readlink '" + _imp->path + "' failed: " + ::strerror(errno));
    return buf;
}

void
FSEntry::chown(const uid_t new_owner, const gid_t new_group)
{
    if (0 != ::chown(_imp->path.c_str(), new_owner, new_group))
        throw FSError("chown '" + _imp->path + "' to '" + stringify(new_owner) + "', '"
                + stringify(new_group) + "' failed: " + ::strerror(errno));
}

void
FSEntry::lchown(const uid_t new_owner, const gid_t new_group)
{
    if (0 != ::lchown(_imp->path.c_str(), new_owner, new_group))
        throw FSError("lchown '" + _imp->path + "' to '" + stringify(new_owner) + "', '"
                + stringify(new_group) + "' failed: " + ::strerror(errno));
}

void
FSEntry::chmod(const mode_t mode)
{
    if (0 != ::chmod(_imp->path.c_str(), mode))
        throw FSError("chmod '" + _imp->path + "' failed: " + ::strerror(errno));
}

uid_t
FSEntry::owner() const
{
    _stat();

    if (! _imp->exists)
        throw FSError("Filesystem entry '" + _imp->path + "' does not exist");

    return _imp->stat_info->st_uid;
}

gid_t
FSEntry::group() const
{
    _stat();

    if (! _imp->exists)
        throw FSError("Filesystem entry '" + _imp->path + "' does not exist");

    return _imp->stat_info->st_gid;
}

void
FSEntry::rename(const FSEntry & new_name)
{
    if (0 != std::rename(_imp->path.c_str(), new_name._imp->path.c_str()))
        throw FSError("rename('" + stringify(_imp->path) + "', '" + stringify(new_name._imp->path) + "') failed: " +
                ::strerror(errno));
}

std::pair<dev_t, ino_t>
FSEntry::lowlevel_id() const
{
    _stat();

    if (! _imp->exists)
        throw FSError("Filesystem entry '" + _imp->path + "' does not exist");

    return std::make_pair(_imp->stat_info->st_dev, _imp->stat_info->st_ino);
}

template class Sequence<FSEntry>;
template class WrappedForwardIterator<Sequence<FSEntry>::ConstIteratorTag, const FSEntry>;
template class WrappedForwardIterator<Sequence<FSEntry>::ReverseConstIteratorTag, const FSEntry>;
template class WrappedOutputIterator<Sequence<FSEntry>::InserterTag, FSEntry>;

template class Set<FSEntry>;
template class WrappedForwardIterator<Set<FSEntry>::ConstIteratorTag, const FSEntry>;

