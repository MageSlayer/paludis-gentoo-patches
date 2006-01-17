/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
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

#include "pstream_in_buf.hh"
#include "pstream_error.hh"
#include "stringify.hh"
#include <cstring>
#include <errno.h>
#include <exception>

using namespace paludis;

PStreamInBuf::int_type
PStreamInBuf::underflow()
{
    if (0 == fd)
        return EOF;

    if (gptr() < egptr())
        return *gptr();

    int num_putback = gptr() - eback();
    if (num_putback > putback_size)
        num_putback = putback_size;
    std::memmove(buffer + putback_size - num_putback,
            gptr() - num_putback, num_putback);

    size_t n = fread(buffer + putback_size, 1, buffer_size - putback_size, fd);
    if (n <= 0)
        return EOF;

    setg(buffer + putback_size - num_putback, buffer + putback_size,
            buffer + putback_size + n);

    return *gptr();
}

PStreamInBuf::PStreamInBuf(const std::string & command) :
    _command(command),
    fd(popen(command.c_str(), "r"))
{
    if (0 == fd)
        throw PStreamError("popen('" + _command + "', 'r') failed: " +
                strerror(errno));

    setg(buffer + putback_size, buffer + putback_size, buffer + putback_size);
}

PStreamInBuf::~PStreamInBuf()
{
    if (0 != fd)
        pclose(fd);
}

int
PStreamInBuf::exit_status()
{
    if (0 != fd)
    {
        _exit_status = pclose(fd);
        fd = 0;
    }
    return _exit_status;
}
