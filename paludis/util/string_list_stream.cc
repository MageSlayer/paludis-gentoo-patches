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

#include <paludis/util/string_list_stream.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/condition_variable.hh>
#include <list>
#include <string>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<StringListStreamBuf>
    {
        Mutex mutex;
        ConditionVariable condition;

        std::string active_string;
        std::list<std::string> future_strings;

        bool nothing_more_to_write;

        Imp() :
            nothing_more_to_write(false)
        {
        }
    };
}

StringListStreamBuf::StringListStreamBuf() :
    _imp()
{
    setg(0, 0, 0);
}

StringListStreamBuf::~StringListStreamBuf()
{
}

StringListStreamBuf::int_type
StringListStreamBuf::overflow(int_type c)
{
    Lock lock(_imp->mutex);
    _imp->condition.signal();

    if (c != traits_type::eof())
    {
        if ((_imp->future_strings.empty()) || (_imp->future_strings.back().length() > 1024))
            _imp->future_strings.push_back(std::string(1, c));
        else
            _imp->future_strings.back().append(std::string(1, c));
    }

    return c;
}

std::streamsize
StringListStreamBuf::xsputn(const char * s, std::streamsize num)
{
    Lock lock(_imp->mutex);
    _imp->condition.signal();

    if ((_imp->future_strings.empty()) || (_imp->future_strings.back().length() + num > 1024))
        _imp->future_strings.push_back(std::string(s, num));
    else
        _imp->future_strings.back().append(std::string(s, num));
    return num;
}

void
StringListStreamBuf::nothing_more_to_write()
{
    Lock lock(_imp->mutex);
    _imp->nothing_more_to_write = true;
    _imp->condition.signal();
}

StringListStreamBuf::int_type
StringListStreamBuf::underflow()
{
    Lock lock(_imp->mutex);

    if (gptr() < egptr())
        return traits_type::to_int_type(*gptr());

    while (_imp->future_strings.empty())
    {
        if (_imp->nothing_more_to_write)
            return traits_type::eof();

        _imp->condition.wait(_imp->mutex);
    }

    _imp->active_string = *_imp->future_strings.begin();
    _imp->future_strings.erase(_imp->future_strings.begin());
    setg(&_imp->active_string[0], &_imp->active_string[0],
            &_imp->active_string[0] + _imp->active_string.length());

    return traits_type::to_int_type(*gptr());
}

StringListStreamBase::StringListStreamBase()
{
}

StringListStreamBase::~StringListStreamBase()
{
}

StringListStream::StringListStream() :
    std::istream(&buf),
    std::ostream(&buf)
{
}

StringListStream::~StringListStream()
{
}

void
StringListStream::nothing_more_to_write()
{
    flush();
    buf.nothing_more_to_write();
}

namespace paludis
{
    template class Pimp<StringListStreamBuf>;
}
