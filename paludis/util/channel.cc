/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
 * Copyright (c) 2009 David Leverton
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

#include "channel.hh"
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <unistd.h>
#include <cstring>
#include <errno.h>

using namespace paludis;

Channel::~Channel()
{
    Context context("When destroying channel FDs '" + stringify(read_fd()) + "', '" + stringify(write_fd()) + "'");
    if (-1 != _fds[0])
        if (-1 == close(_fds[0]))
            Log::get_instance()->message("util.channel.close", ll_warning, lc_context) << "close(" << _fds[0] << ") -> " << strerror(errno);
    if (-1 != _fds[1])
        if (-1 == close(_fds[1]))
            Log::get_instance()->message("util.channel.close", ll_warning, lc_context) << "close(" << _fds[1] << ") -> " << strerror(errno);
}

