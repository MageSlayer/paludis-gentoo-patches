/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 David Leverton
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

#include <paludis/util/digest_registry.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/singleton-impl.hh>
#include <map>

using namespace paludis;

namespace
{
    typedef std::map<std::string, DigestRegistry::Function> FunctionMap;
}

namespace paludis
{
    template <>
    struct Imp<DigestRegistry>
    {
        FunctionMap functions;
    };
}

DigestRegistry::DigestRegistry()
{
}

DigestRegistry::~DigestRegistry()
{
}

DigestRegistry::Function
DigestRegistry::get(const std::string & algo) const
{
    FunctionMap::const_iterator it(_imp->functions.find(algo));
    if (_imp->functions.end() == it)
        return Function();
    return it->second;
}

void
DigestRegistry::register_function(const std::string & algo, const Function & func)
{
    _imp->functions.insert(std::make_pair(algo, func));
}

namespace paludis
{
    template class Pimp<DigestRegistry>;
    template class Singleton<DigestRegistry>;
}


