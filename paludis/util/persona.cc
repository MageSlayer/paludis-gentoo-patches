/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2016 Saleem Abdulrasool
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

#include <paludis/util/persona.hh>

#include <paludis/util/log.hh>

#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>

namespace
{
    template <unsigned Name>
    struct SystemConfigurationParameter
    {
        static const char * const Spelling;
    };

    template <>
    const char * const SystemConfigurationParameter<_SC_GETPW_R_SIZE_MAX>::Spelling = "_SC_GETPW_R_SIZE_MAX";

    template <>
    const char * const SystemConfigurationParameter<_SC_GETGR_R_SIZE_MAX>::Spelling = "_SC_GETGR_R_SIZE_MAX";

    template <unsigned Name, size_t DefaultSize = 1024, long DodgyLimit = 1024 * 128>
    size_t initial_buffer_size(const char *context)
    {
        using namespace paludis;

        long size;

        errno = 0;
        size = ::sysconf(Name);
        if (size == -1 && errno == 0)
            return DefaultSize;

        if (size > DodgyLimit)
          Log::get_instance()->message(context, ll_warning, lc_context)
              << "Got dodgy value " << size << " from "
              << "sysconf(" << SystemConfigurationParameter<Name>::Spelling << ")";

        return std::min(size, DodgyLimit);
    }
}

namespace paludis
{
    int getpwnam_r_s(const char *name, std::vector<char> & buffer,
                     struct passwd & pwd, struct passwd * & result)
    {
        size_t length;
        int rv;

        buffer.clear();

        length =
            initial_buffer_size<_SC_GETPW_R_SIZE_MAX>("accounts.getpw_r_size");

        for (buffer.resize(length);
             (rv = ::getpwnam_r(name, &pwd, buffer.data(), buffer.capacity(),
                                &result)) == ERANGE;
             buffer.resize(length))
          length = length * 2;

        return rv;
    }

    int getgrgid_r_s(gid_t gid, std::vector<char> & buffer,
                     struct group & grp, struct group * & result)
    {
        size_t length;
        int rv;

        buffer.clear();

        length =
            initial_buffer_size<_SC_GETGR_R_SIZE_MAX>("accounts.getgr_r_size");

        for (buffer.resize(length);
             (rv = ::getgrgid_r(gid, &grp, buffer.data(), buffer.capacity(),
                                &result)) == ERANGE;
             buffer.resize(length))
          length = length * 2;

        return rv;
    }

    int getpwuid_r_s(uid_t uid, std::vector<char> & buffer,
                     struct passwd & pwd, struct passwd * & result)
    {
        size_t length;
        int rv;

        buffer.clear();

        length =
            initial_buffer_size<_SC_GETPW_R_SIZE_MAX>("accounts.getpw_r_size");

        for (buffer.resize(length);
             (rv = ::getpwuid_r(uid, &pwd, buffer.data(), buffer.capacity(),
                                &result)) == ERANGE;
             buffer.resize(length))
          length = length * 2;

        return rv;
    }
}

