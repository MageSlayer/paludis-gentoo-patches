/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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

#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/stringify.hh>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

using namespace paludis;

SafeOFStreamBuf::SafeOFStreamBuf(const int f) :
    fd(f)
{
}

SafeOFStreamBuf::int_type
SafeOFStreamBuf::overflow(int_type c)
{
    if (c != traits_type::eof())
    {
        char z = c;
        if (1 != write(fd, &z, 1))
            return traits_type::eof();
    }
    return c;
}

std::streamsize
SafeOFStreamBuf::xsputn(const char * s, std::streamsize num)
{
    return write(fd, s, num);
}

SafeOFStreamBase::SafeOFStreamBase(const int f) :
    buf(f)
{
}

SafeOFStream::SafeOFStream(const int f) :
    SafeOFStreamBase(f),
    std::ostream(&buf),
    _close(false)
{
}

namespace
{
    int check_open_path(const FSPath & e, int open_flags)
    {
        Context context("When opening '" + stringify(e) + "' for write:");

        if (-1 == open_flags)
            open_flags = O_CREAT | O_TRUNC | O_WRONLY | O_CLOEXEC;

        int result(open(stringify(e).c_str(), open_flags, 0644));
        if (-1 == result)
            throw SafeOFStreamError("Could not open '" + stringify(e) + "': " + strerror(errno));

        return result;
    }
}

SafeOFStream::SafeOFStream(const FSPath & p, const int open_flags) :
    SafeOFStreamBase(check_open_path(p, open_flags)),
    std::ostream(&buf),
    _close(true)
{
}

SafeOFStream::~SafeOFStream()
{
    if (_close)
        ::close(buf.fd);

    if (! *this)
        throw SafeOFStreamError("Write to fd " + stringify(buf.fd) + " failed");
}

SafeOFStreamError::SafeOFStreamError(const std::string & s) throw () :
    Exception(s)
{
}

