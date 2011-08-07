/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/pimp-impl.hh>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<SafeOFStreamBuf>
    {
        std::shared_ptr<std::string> buffered_text;
    };
}

SafeOFStreamBuf::SafeOFStreamBuf(const int f, const bool buffer) :
    _imp(),
    fd(f)
{
    if (buffer)
        _imp->buffered_text = std::make_shared<std::string>();
}

SafeOFStreamBuf::~SafeOFStreamBuf()
{
}

SafeOFStreamBuf::int_type
SafeOFStreamBuf::overflow(int_type c)
{
    if (_imp->buffered_text)
    {
        _imp->buffered_text->append(1, c);
        if (_imp->buffered_text->length() >= 4096)
            write_buffered();
    }
    else
    {
        if (c != traits_type::eof())
        {
            char z = c;
            if (1 != write(fd, &z, 1))
                return traits_type::eof();
        }
    }

    return c;
}

std::streamsize
SafeOFStreamBuf::xsputn(const char * s, std::streamsize num)
{
    if (_imp->buffered_text)
    {
        _imp->buffered_text->append(s, num);
        if (_imp->buffered_text->length() >= 4096)
            write_buffered();

        return num;
    }
    else
        return write(fd, s, num);
}

void
SafeOFStreamBuf::write_buffered()
{
    if (! _imp->buffered_text)
        return;

    while (! _imp->buffered_text->empty())
    {
        int n(::write(fd, _imp->buffered_text->data(), _imp->buffered_text->length()));
        if (-1 == n)
            throw SafeOFStreamError("Write to fd " + stringify(fd) + " failed");
        _imp->buffered_text->erase(0, n);
    }
}

SafeOFStreamBase::SafeOFStreamBase(const int f, const bool b) :
    buf(f, b)
{
}

SafeOFStream::SafeOFStream(const int f, const bool buffer) :
    SafeOFStreamBase(f, buffer),
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

SafeOFStream::SafeOFStream(const FSPath & p, const int open_flags, const bool b) :
    SafeOFStreamBase(check_open_path(p, open_flags), b),
    std::ostream(&buf),
    _close(true)
{
}

SafeOFStream::~SafeOFStream()
{
    buf.write_buffered();

    if (_close)
        ::close(buf.fd);

    if (! *this)
        throw SafeOFStreamError("Write to fd " + stringify(buf.fd) + " failed");
}

SafeOFStreamError::SafeOFStreamError(const std::string & s) throw () :
    Exception(s)
{
}

namespace paludis
{
    template class Pimp<SafeOFStreamBuf>;
}
