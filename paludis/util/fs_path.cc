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

#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_error.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/options.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/log.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>

#include <cstdio>
#include <string>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "config.h"

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<FSPath>
    {
        std::string path;

        Imp(const std::string & p) :
            path(p)
        {
        }
    };
}

FSPath::FSPath(const std::string & path) :
    _imp(path)
{
    _normalise();
}

FSPath::FSPath(const FSPath & other) :
    _imp(other._imp->path)
{
}

FSPath::~FSPath() = default;

FSStat
FSPath::stat() const
{
    return FSStat(*this);
}

FSPath &
FSPath::operator= (const FSPath & other)
{
    _imp->path = other._imp->path;
    return *this;
}

FSPath &
FSPath::operator/= (const FSPath & rhs)
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

    return *this;
}

FSPath &
FSPath::operator/= (const std::string & rhs)
{
    return operator/= (FSPath(rhs));
}

FSPath
FSPath::operator/ (const FSPath & rhs) const
{
    FSPath result(*this);
    result /= rhs;
    return result;
}

FSPath
FSPath::operator/ (const std::string & rhs) const
{
    return *this / FSPath(rhs);
}

bool
paludis::operator== (const FSPath & me, const FSPath & other)
{
    return me._imp->path == other._imp->path;
}

bool
paludis::operator!= (const FSPath & me, const FSPath & other)
{
    return ! operator== (me, other);
}

void
FSPath::_normalise()
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
        Context c("When normalising FSPath path '" + _imp->path + "':");
        throw InternalError(PALUDIS_HERE,
                "caught std::exception '" + stringify(e.what()) + "'");
    }
}

const std::string
FSPath::basename() const
{
    if (_imp->path == "/")
        return _imp->path;

    return _imp->path.substr(_imp->path.rfind('/') + 1);
}

FSPath
FSPath::strip_leading(const FSPath & f) const
{
    std::string root(stringify(f));

    if (root == "/")
        root.clear();
    if (0 != _imp->path.compare(0, root.length(), root))
        throw FSError("Can't strip leading '" + root + "' from FSPath '" + _imp->path + "'");
    return FSPath(_imp->path.substr(root.length()));
}

bool
FSPath::starts_with(const FSPath & f) const
{
    std::string root(stringify(f));

    if (root == "/")
        root.clear();
    root.append("/");
    return 0 == (_imp->path + "/").compare(0, root.length(), root);
}

FSPath
FSPath::dirname() const
{
    if (_imp->path == "/")
        return FSPath(_imp->path);

    return FSPath(_imp->path.substr(0, _imp->path.rfind('/')));
}

FSPath
FSPath::realpath() const
{
    Context context("When fetching realpath of '" + stringify(_imp->path) + "':");

#ifdef HAVE_CANONICALIZE_FILE_NAME
    char * r(canonicalize_file_name(_imp->path.c_str()));
    if (! r)
        throw FSError("Could not resolve path '" + _imp->path + "'");
    FSPath result(r);
    std::free(r);
    return result;
#else
    char r[PATH_MAX + 1];
    std::memset(r, 0, PATH_MAX + 1);
    if (! exists())
        throw FSError("Could not resolve path '" + _imp->path + "'");
    if (! ::realpath(_imp->path.c_str(), r))
        throw FSError("Could not resolve path '" + _imp->path + "'");
    FSPath result(r);
    if (! result.exists())
        throw FSError("Could not resolve path '" + _imp->path + "'");
    return result;
#endif
}

FSPath
FSPath::realpath_if_exists() const
{
    Context context("When fetching realpath of '" + stringify(_imp->path) + "', if it exists:");

#ifdef HAVE_CANONICALIZE_FILE_NAME
    char * r(canonicalize_file_name(_imp->path.c_str()));
    if (! r)
        return *this;
    FSPath result(r);
    std::free(r);
    return result;
#else
    char r[PATH_MAX + 1];
    std::memset(r, 0, PATH_MAX + 1);
    if (! exists())
        return *this;
    if (! ::realpath(_imp->path.c_str(), r))
        return *this;
    FSPath result(r);
    if (! result.exists())
        return *this;
    return result;
#endif
}

FSPath
FSPath::cwd()
{
    char r[PATH_MAX + 1];
    std::memset(r, 0, PATH_MAX + 1);
    if (! ::getcwd(r, PATH_MAX))
        throw FSError("Could not get current working directory");
    return FSPath(r);
}

std::ostream &
paludis::operator<< (std::ostream & s, const FSPath & f)
{
    s << f._imp->path;
    return s;
}

bool
FSPath::mkdir(const mode_t mode, const FSPathMkdirOptions & options) const
{
    if (0 == ::mkdir(_imp->path.c_str(), mode))
        return true;

    int e(errno);
    if (e == EEXIST && options[fspmkdo_ok_if_exists])
    {
        if (stat().is_directory())
            return false;
        throw FSError("mkdir '" + _imp->path + "' failed: target exists and is not a directory");
    }
    else
        throw FSError("mkdir '" + _imp->path + "' failed: " + ::strerror(e));
}

bool
FSPath::symlink(const std::string & target) const
{
    if (0 == ::symlink(target.c_str(), _imp->path.c_str()))
        return true;

    int e(errno);
    if (e == EEXIST)
    {
        if (stat().is_symlink() && target == readlink())
            return false;
        throw FSError("symlink '" + _imp->path + "' to '" + target + "' failed: target exists");
    }
    else
        throw FSError("symlink '" + _imp->path + "' to '" + target + "' failed: " + ::strerror(e));
}

bool
FSPath::unlink() const
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
FSPath::rmdir() const
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
FSPath::utime(const Timestamp & t) const
{
    Context context("When setting utime for '" + stringify(_imp->path) + "':");

#ifdef HAVE_UTIMENSAT
    static bool utimensat_works(true);

    if (utimensat_works)
    {
        struct timespec ts[2] = { t.as_timespec(), t.as_timespec() };
        if (0 == ::utimensat(AT_FDCWD, _imp->path.c_str(), ts, 0))
            return true;

        int e(errno);
        if (e == ENOENT)
            return false;
        else if (e == ENOSYS)
        {
            utimensat_works = false;
            Log::get_instance()->message("util.fs_entry.utime.utimensat_unimplemented", ll_debug, lc_context)
                << "utimensat(2) not implemented by this kernel, using utimes(2)";
        }
        else
            throw FSError("utimensat '" + _imp->path + "' failed: " + ::strerror(e));
    }
#endif

    struct timeval tv[2] = { t.as_timeval(), t.as_timeval() };
    if (0 == ::utimes(_imp->path.c_str(), tv))
        return true;

    int e(errno);
    if (e == ENOENT)
        return false;
    else
        throw FSError("utimes '" + _imp->path + "' failed: " + ::strerror(e));
}

std::string
FSPath::readlink() const
{
    char buf[PATH_MAX + 1];
    std::memset(buf, 0, PATH_MAX + 1);
    if (-1 == ::readlink(_imp->path.c_str(), buf, PATH_MAX))
        throw FSError("readlink '" + _imp->path + "' failed: " + ::strerror(errno));
    return buf;
}

void
FSPath::chown(const uid_t new_owner, const gid_t new_group) const
{
    if (0 != ::chown(_imp->path.c_str(), new_owner, new_group))
        throw FSError("chown '" + _imp->path + "' to '" + stringify(new_owner) + "', '"
                + stringify(new_group) + "' failed: " + ::strerror(errno));
}

void
FSPath::lchown(const uid_t new_owner, const gid_t new_group) const
{
    if (0 != ::lchown(_imp->path.c_str(), new_owner, new_group))
        throw FSError("lchown '" + _imp->path + "' to '" + stringify(new_owner) + "', '"
                + stringify(new_group) + "' failed: " + ::strerror(errno));
}

void
FSPath::chmod(const mode_t mode) const
{
    if (0 != ::chmod(_imp->path.c_str(), mode))
        throw FSError("chmod '" + _imp->path + "' failed: " + ::strerror(errno));
}

void
FSPath::rename(const FSPath & new_name) const
{
    if (0 != std::rename(_imp->path.c_str(), new_name._imp->path.c_str()))
        throw FSError("rename('" + stringify(_imp->path) + "', '" + stringify(new_name._imp->path) + "') failed: " +
                ::strerror(errno));
}

bool
FSPathComparator::operator() (const FSPath & a, const FSPath & b) const
{
    return stringify(a) < stringify(b);
}

namespace paludis
{
    template class PALUDIS_VISIBLE Sequence<FSPath>;
    template class PALUDIS_VISIBLE WrappedForwardIterator<Sequence<FSPath>::ConstIteratorTag, const FSPath>;
    template class PALUDIS_VISIBLE WrappedForwardIterator<Sequence<FSPath>::ReverseConstIteratorTag, const FSPath>;
    template class PALUDIS_VISIBLE WrappedOutputIterator<Sequence<FSPath>::InserterTag, FSPath>;

    template class PALUDIS_VISIBLE Set<FSPath, FSPathComparator>;
    template class PALUDIS_VISIBLE WrappedForwardIterator<Set<FSPath, FSPathComparator>::ConstIteratorTag, const FSPath>;

    template class Pimp<FSPath>;
}

