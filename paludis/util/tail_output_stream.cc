/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010 Ciaran McCreesh
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

#include <paludis/util/tail_output_stream.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/iterator_funcs.hh>
#include <functional>
#include <algorithm>
#include <list>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<TailOutputStreamBuf>
    {
        unsigned int n;
        const unsigned int size;
        std::list<std::string> tail;

        Mutex mutex;

        Imp(const unsigned int nn) :
            n(1),
            size(nn)
        {
            tail.push_back("");
        }
    };
}

TailOutputStreamBuf::TailOutputStreamBuf(const unsigned int n) :
    Pimp<TailOutputStreamBuf>(n)
{
}

TailOutputStreamBuf::~TailOutputStreamBuf()
{
}

TailOutputStreamBuf::int_type
TailOutputStreamBuf::overflow(int_type c)
{
    if (c != traits_type::eof())
        _append(std::string(1, c));
    return c;
}

std::streamsize
TailOutputStreamBuf::xsputn(const char * s, std::streamsize num)
{
    _append(std::string(s, num));
    return num;
}

void
TailOutputStreamBuf::_append(const std::string & s)
{
    Lock lock(_imp->mutex);
    for (std::string::size_type p(0), p_end(s.length()) ; p != p_end ; ++p)
    {
        if ('\n' == s[p])
        {
            if (++_imp->n > _imp->size + 1)
            {
                _imp->tail.pop_front();
                --_imp->n;
            }
            _imp->tail.push_back("");
        }
        else
            _imp->tail.back().append(&s[p], 1);
    }
}

const std::shared_ptr<const Sequence<std::string> >
TailOutputStreamBuf::tail(const bool clear)
{
    std::shared_ptr<Sequence<std::string> > result(new Sequence<std::string>);
    Lock lock(_imp->mutex);
    for (std::list<std::string>::const_iterator i(_imp->tail.begin()), i_end(_imp->tail.end()), i_last(previous(_imp->tail.end())) ;
            i != i_end ; ++i)
    {
        if (i == i_last && i->empty())
            continue;
        result->push_back(*i);
    }

    if (clear)
    {
        _imp->tail.clear();
        _imp->n = 1;
        _imp->tail.push_back("");
    }

    return result;
}

template class Pimp<TailOutputStreamBuf>;

