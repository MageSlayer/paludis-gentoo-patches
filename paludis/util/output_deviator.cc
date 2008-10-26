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
#include <paludis/util/fd_output_stream.hh>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctime>
#include <unistd.h>
#include <iostream>

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
        int descriptor;
        std::tr1::shared_ptr<FDOutputStream> stream;

        Implementation(const FSEntry & f) :
            file_name(f),
            descriptor(open(stringify(f).c_str(), O_CREAT | O_WRONLY, 0644))
        {
            if (-1 == descriptor)
                Log::get_instance()->message("output_deviator.open_failed", ll_warning, lc_context) << "Cannot open '"
                    << f << + "' for write, sending output to stdout and stderr instead";
            else
                stream.reset(new FDOutputStream(descriptor));
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
OutputDeviator::make_output_deviant(const std::string & n)
{
    return make_shared_ptr(new OutputDeviant(_imp->log_dir / (n + "." + stringify(std::time(0)) + ".log")));
}

OutputDeviant::OutputDeviant(const FSEntry & f) :
    PrivateImplementationPattern<OutputDeviant>(new Implementation<OutputDeviant>(f))
{
}

OutputDeviant::~OutputDeviant()
{
    if (_imp->descriptor != 1)
    {
        if (0 != ::close(_imp->descriptor))
            Log::get_instance()->message("output_deviant.close_failed", ll_warning, lc_context)
                << "Cannot close '" << _imp->file_name << "'";
    }
}

std::ostream *
OutputDeviant::stdout_stream() const
{
    if (_imp->stream)
        return _imp->stream.get();
    else
        return &std::cout;
}

std::ostream *
OutputDeviant::stderr_stream() const
{
    if (_imp->stream)
        return _imp->stream.get();
    else
        return &std::cerr;
}

void
OutputDeviant::discard_log()
{
    if (-1 != _imp->descriptor)
        if (-1 == ::unlink(stringify(_imp->file_name).c_str()))
            Log::get_instance()->message("output_deviant.unlink_failed", ll_warning, lc_context)
                << "Cannot unlink '" << _imp->file_name << "'";
}

const FSEntry
OutputDeviant::log_file_name() const
{
    return _imp->file_name;
}

template class PrivateImplementationPattern<OutputDeviator>;
template class PrivateImplementationPattern<OutputDeviant>;

