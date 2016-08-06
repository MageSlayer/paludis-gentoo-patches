/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/util/tee_output_stream.hh>
#include <paludis/util/pimp-impl.hh>
#include <list>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<TeeOutputStreamBuf>
    {
        std::list<std::ostream *> streams;
    };
}

TeeOutputStreamBuf::TeeOutputStreamBuf() :
    _imp()
{
}

TeeOutputStreamBuf::~TeeOutputStreamBuf() = default;

TeeOutputStreamBuf::int_type
TeeOutputStreamBuf::overflow(int_type c)
{
    for (auto & stream : _imp->streams)
        stream->put(c);
    return c;
}

int
TeeOutputStreamBuf::sync()
{
    for (auto & stream : _imp->streams)
        *stream << std::flush;
    return 0;
}

std::streamsize
TeeOutputStreamBuf::xsputn(const char * s, std::streamsize num)
{
    for (auto & stream : _imp->streams)
        stream->write(s, num);
    return num;
}

void
TeeOutputStreamBuf::add_stream(std::ostream * const s)
{
    _imp->streams.push_back(s);
}

TeeOutputStream::TeeOutputStream() :
    std::ostream(&buf)
{
}

void
TeeOutputStream::add_stream(std::ostream * const s)
{
    buf.add_stream(s);
}

namespace paludis
{
    template class Pimp<TeeOutputStreamBuf>;
}
