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

#include "fs_entry.hh"
#include "exception.hh"
#include "stringify.hh"

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <cstring>
#include <cstdlib>

/** \file
 * Implementation of paludis::FSEntry.
 *
 * \ingroup Filesystem
 */

using namespace paludis;

FSError::FSError(const std::string & message) throw () :
    Exception(message)
{
}

FSEntry::FSEntry(const std::string & path) :
    ComparisonPolicyType(&FSEntry::_path),
    _path(path)
{
    _normalise();
}

FSEntry::FSEntry(const FSEntry & other) :
    ComparisonPolicyType(&FSEntry::_path),
    _path(other._path)
{
}

FSEntry::~FSEntry()
{
}

const FSEntry &
FSEntry::operator= (const FSEntry & other)
{
    _path = other._path;
    return *this;
}

FSEntry::operator std::string() const
{
    return _path;
}

FSEntry
FSEntry::operator/ (const FSEntry & rhs) const
{
    return FSEntry(_path + "/" + rhs._path);
}

FSEntry
FSEntry::operator/ (const std::string & rhs) const
{
    return operator/ (FSEntry(rhs));
}

const FSEntry &
FSEntry::operator/= (const FSEntry & rhs)
{
    _path.append("/");
    _path.append(rhs._path);
    _normalise();
    return *this;
}

bool
FSEntry::exists() const
{
    struct stat s;

    if (0 != stat(_path.c_str(), &s))
    {
        if (errno != ENOENT)
            throw FSError("Error checking whether '" + stringify(_path) + "' exists: "
                    + strerror(errno));
        return false;
    }

    return true;
}

bool
FSEntry::is_directory() const
{
    struct stat s;

    if (0 != stat(_path.c_str(), &s))
    {
        if (errno != ENOENT)
            throw FSError("Error checking whether '" + stringify(_path) + "' is a directory: "
                    + strerror(errno));
        return false;
    }

    return S_ISDIR(s.st_mode);
}

bool
FSEntry::is_regular_file() const
{
    struct stat s;

    if (0 != stat(_path.c_str(), &s))
    {
        if (errno != ENOENT)
            throw FSError("Error checking whether '" + stringify(_path) + "' is a regular file: "
                    + strerror(errno));
        return false;
    }

    return S_ISREG(s.st_mode);
}

void
FSEntry::_normalise()
{
    try
    {
        std::string new_path;
        std::string::size_type p(0);
        while (p < _path.length())
        {
            if ('/' == _path[p])
            {
                new_path += '/';
                while (++p < _path.length())
                    if ('/' != _path[p])
                        break;
            }
            else
                new_path += _path[p++];
        }
        _path = new_path;

        if (! _path.empty())
            if ('/' == _path.at(_path.length() - 1))
                _path.erase(_path.length() - 1);
        if (_path.empty())
            _path = "/";
    }
    catch (const std::exception & e)
    {
        Context c("When normalising FSEntry path '" + _path + "':");
        throw InternalError(PALUDIS_HERE,
                "caught std::exception '" + stringify(e.what()) + "'");
    }
}

std::string
FSEntry::basename() const
{
    return _path.substr(_path.rfind('/') + 1);
}

FSEntry
FSEntry::realpath() const
{
    char r[PATH_MAX + 1];
    std::memset(r, 0, PATH_MAX + 1);
    if (! ::realpath(_path.c_str(), r))
        throw FSError("Could not resolve path '" + _path + "'");
    return FSEntry(r);
}

std::ostream &
paludis::operator<< (std::ostream & s, const FSEntry & f)
{
    s << std::string(f);
    return s;
}


