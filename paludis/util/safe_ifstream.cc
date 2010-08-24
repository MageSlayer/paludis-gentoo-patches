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

#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/stringify.hh>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <cstring>
#include <errno.h>

using namespace paludis;

SafeIFStreamBuf::SafeIFStreamBuf(const int f) :
    fd(f)
{
    setg(buffer + lookbehind_size, buffer + lookbehind_size, buffer + lookbehind_size);
    underflow();
}

SafeIFStreamBuf::int_type
SafeIFStreamBuf::underflow()
{
    if (gptr() < egptr())
        return traits_type::to_int_type(*gptr());

    int n_putback = gptr() - eback();
    if (n_putback > lookbehind_size)
        n_putback = lookbehind_size;

    std::memmove(buffer + (lookbehind_size - n_putback), gptr() - n_putback, n_putback);

    int n_read(read(fd, buffer + lookbehind_size, buffer_size - lookbehind_size));
    if (-1 == n_read)
        throw SafeIFStreamError("Error reading from fd " + stringify(fd) + ": " + strerror(errno));
    else if (0 == n_read)
        return traits_type::eof();

    setg(buffer + (lookbehind_size - n_putback), buffer + lookbehind_size, buffer + lookbehind_size + n_read);
    return traits_type::to_int_type(*gptr());
}

SafeIFStreamBuf::pos_type
SafeIFStreamBuf::seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode)
{
    off_t result;
    if (dir == std::ios_base::beg)
        result = lseek(fd, off, SEEK_SET);
    else if (dir == std::ios_base::cur)
        result = lseek(fd, off, SEEK_CUR);
    else if (dir == std::ios_base::end)
        result = lseek(fd, off, SEEK_END);
    else
        throw SafeIFStreamError("Got bad std::ios_base::seekdir");

    if (-1 == result)
        throw SafeIFStreamError("Error seeking to offset with fd " + stringify(fd) + ": " + strerror(errno));

    setg(buffer + lookbehind_size, buffer + lookbehind_size, buffer + lookbehind_size);
    return result;
}

SafeIFStreamBuf::pos_type
SafeIFStreamBuf::seekpos(pos_type p, std::ios_base::openmode)
{
    off_t result(lseek(fd, p, SEEK_SET));
    if (-1 == result)
        throw SafeIFStreamError("Error seeking with fd " + stringify(fd) + ": " + strerror(errno));

    setg(buffer + lookbehind_size, buffer + lookbehind_size, buffer + lookbehind_size);
    return p;
}

SafeIFStreamBase::SafeIFStreamBase(const int f) :
    buf(f)
{
}

SafeIFStream::SafeIFStream(const int f) :
    SafeIFStreamBase(f),
    std::istream(&buf),
    _close(false)
{
}

namespace
{
    int open_path(const FSPath & e)
    {
        Context context("When opening '" + stringify(e) + "' for read:");

        int result(open(stringify(e).c_str(), O_RDONLY | O_CLOEXEC));
        if (-1 == result)
            throw SafeIFStreamError("Could not open '" + stringify(e) + "': " + strerror(errno));

        return result;
    }
}

SafeIFStream::SafeIFStream(const FSPath & e) :
    SafeIFStreamBase(open_path(e)),
    std::istream(&buf),
    _close(true)
{
}

SafeIFStream::~SafeIFStream()
{
    if (_close)
        ::close(buf.fd);
}

SafeIFStreamError::SafeIFStreamError(const std::string & s) throw () :
    Exception(s)
{
}

