/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Danny van Dyk <kugelfang@gentoo.org>
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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_CONTRARIUS_TARGET_CONFIG_HH
#define PALUDIS_GUARD_SRC_CLIENTS_CONTRARIUS_TARGET_CONFIG_HH 1

#include <paludis/util/exception.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/host_tuple_name.hh>
#include <list>
#include <utility>

using namespace paludis;

class TargetConfigError :
    public Exception
{
    public:
        TargetConfigError(const std::string & m) throw () :
            Exception("Error in contrarius configuration: " + m)
        {
        }
};

class TargetConfig :
    public InstantiationPolicy<TargetConfig, instantiation_method::SingletonTag>
{
    friend class InstantiationPolicy<TargetConfig, instantiation_method::SingletonTag>;

    private:
        typedef std::list<std::pair<std::string, std::string> > SpecEntryList;

        typedef libwrapiter::ForwardIterator<TargetConfig, const std::string> ConstIterator;

        SpecEntryList _binutils_list;

        SpecEntryList _gcc_list;

        SpecEntryList _headers_list;

        SpecEntryList _libc_list;

        SpecEntryList _aux_list;

        HostTupleName _target;

        std::string _binutils;

        std::string _gcc;

        std::string _headers;

        std::string _libc;

        std::string _aux;

        TargetConfig();

        void _parse_defaults();

        std::string _find_match(SpecEntryList & list);

    public:
        std::string binutils() const
        {
            return _binutils;
        }

        std::string gcc() const
        {
            return _gcc;
        }

        std::string headers() const
        {
            return _headers;
        }

        std::string libc() const
        {
            return _libc;
        }

        std::string aux() const
        {
            return _aux;
        }
};

#endif
