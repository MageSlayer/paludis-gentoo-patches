/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011, 2012 Ciaran McCreesh
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

#include <paludis/util/buffer_output_stream.hh>
#include <paludis/util/pimp-impl.hh>
#include <list>
#include <string>
#include <mutex>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<BufferOutputStreamBuf>
    {
        mutable std::mutex mutex;

        std::list<std::string> complete_strings;
        std::string active_string;

        Imp()
        {
        }
    };
}

BufferOutputStreamBuf::BufferOutputStreamBuf() :
    _imp()
{
    setg(nullptr, nullptr, nullptr);
}

BufferOutputStreamBuf::~BufferOutputStreamBuf() = default;

BufferOutputStreamBuf::int_type
BufferOutputStreamBuf::overflow(int_type c)
{
    std::unique_lock<std::mutex> lock(_imp->mutex);

    if (c != traits_type::eof())
    {
        _imp->active_string.append(std::string(1, c));
        if (c == '\n' || c == '\r')
        {
            _imp->complete_strings.push_back(_imp->active_string);
            _imp->active_string.clear();
        }
    }

    return c;
}

std::streamsize
BufferOutputStreamBuf::xsputn(const char * s, std::streamsize num)
{
    std::unique_lock<std::mutex> lock(_imp->mutex);

    _imp->active_string.append(std::string(s, num));
    if (std::string::npos != _imp->active_string.find_first_of("\r\n", _imp->active_string.length() - num))
    {
        _imp->complete_strings.push_back(_imp->active_string);
        _imp->active_string.clear();
    }

    return num;
}

void
BufferOutputStreamBuf::unbuffer(std::ostream & stream)
{
    std::list<std::string> c;

    {
        std::unique_lock<std::mutex> lock(_imp->mutex);
        _imp->complete_strings.swap(c);
    }

    for (const auto & s : c)
        stream << s;

    stream << std::flush;
}

bool
BufferOutputStreamBuf::anything_to_unbuffer() const
{
    std::unique_lock<std::mutex> lock(_imp->mutex);
    return ! _imp->complete_strings.empty();
}

BufferOutputStreamBase::BufferOutputStreamBase()
{
}

BufferOutputStream::BufferOutputStream() :
    std::ostream(&buf)
{
}

void
BufferOutputStream::unbuffer(std::ostream & s)
{
    flush();
    buf.unbuffer(s);
}

bool
BufferOutputStream::anything_to_unbuffer() const
{
    return buf.anything_to_unbuffer();
}

namespace paludis
{
    template class Pimp<BufferOutputStreamBuf>;
}
