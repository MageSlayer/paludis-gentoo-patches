/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#include <paludis/util/output_deviator.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tee_output_stream.hh>
#include <paludis/util/tail_output_stream.hh>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <fstream>

namespace paludis
{
    template <>
    struct Implementation<OutputDeviator>
    {
        const FSEntry log_dir;

        Implementation(const FSEntry & l) :
            log_dir(l)
        {
        }
    };

    template <>
    struct Implementation<OutputDeviant>
    {
        const FSEntry file_name;
        std::tr1::shared_ptr<TeeOutputStream> tee_stream;
        std::tr1::shared_ptr<std::ofstream> f_stream;
        std::tr1::shared_ptr<TailOutputStream> tail_stream;

        Implementation(const FSEntry & f, const unsigned int number_of_tail_lines) :
            file_name(f),
            f_stream(new std::ofstream(stringify(file_name).c_str()))
        {
            if (! *f_stream)
            {
                Log::get_instance()->message("output_deviator.open_failed", ll_warning, lc_context) << "Cannot open '"
                    << file_name << + "' for write, sending output to stdout and stderr instead";
                f_stream.reset();
            }
            else
            {
                tail_stream.reset(new TailOutputStream(number_of_tail_lines));
                tee_stream.reset(new TeeOutputStream(f_stream.get(), tail_stream.get()));
            }
        }
    };
}

OutputDeviator::OutputDeviator(const FSEntry & l) :
    PrivateImplementationPattern<OutputDeviator>(new Implementation<OutputDeviator>(l))
{
}

OutputDeviator::~OutputDeviator()
{
}

const std::tr1::shared_ptr<OutputDeviant>
OutputDeviator::make_output_deviant(const std::string & n, const unsigned number_of_tail_lines)
{
    return make_shared_ptr(new OutputDeviant(_imp->log_dir / (n + "." + stringify(std::time(0)) + ".log"), number_of_tail_lines));
}

OutputDeviant::OutputDeviant(const FSEntry & f, const unsigned int n) :
    PrivateImplementationPattern<OutputDeviant>(new Implementation<OutputDeviant>(f, n))
{
}

OutputDeviant::~OutputDeviant()
{
}

std::ostream *
OutputDeviant::stdout_stream() const
{
    if (_imp->tee_stream)
        return _imp->tee_stream.get();
    else
        return &std::cout;
}

std::ostream *
OutputDeviant::stderr_stream() const
{
    if (_imp->tee_stream)
        return _imp->tee_stream.get();
    else
        return &std::cerr;
}

void
OutputDeviant::discard_log()
{
    if (_imp->f_stream)
        if (-1 == ::unlink(stringify(_imp->file_name).c_str()))
            Log::get_instance()->message("output_deviant.unlink_failed", ll_warning, lc_context)
                << "Cannot unlink '" << _imp->file_name << "'";
}

const FSEntry
OutputDeviant::log_file_name() const
{
    return _imp->file_name;
}

const std::tr1::shared_ptr<const Sequence<std::string> >
OutputDeviant::tail(const bool clear) const
{
    if (_imp->tail_stream)
        return _imp->tail_stream->tail(clear);
    else
        return make_null_shared_ptr();
}

template class PrivateImplementationPattern<OutputDeviator>;
template class PrivateImplementationPattern<OutputDeviant>;

