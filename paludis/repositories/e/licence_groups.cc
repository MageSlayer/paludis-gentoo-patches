/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/licence_groups.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/set.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_output_iterator.hh>

#include <vector>
#include <map>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Imp<LicenceGroups>
    {
        std::map<std::string, std::shared_ptr<Set<std::string> > > store;
    };
}

LicenceGroups::LicenceGroups() :
    _imp()
{
}

LicenceGroups::~LicenceGroups() = default;

void
LicenceGroups::add(const FSPath & p)
{
    try
    {
        LineConfigFile file(p, { });
        for (auto l(file.begin()), l_end(file.end()) ;
                l != l_end ; ++l)
        {
            std::vector<std::string> tokens;
            tokenise_whitespace(*l, std::back_inserter(tokens));

            if (tokens.size() < 2)
                continue;

            auto i(_imp->store.insert(std::make_pair(tokens.at(0), std::make_shared<Set<std::string> >())).first->second);
            std::copy(tokens.begin() + 1, tokens.end(), i->inserter());
        }
    }
    catch (const Exception & e)
    {
        Log::get_instance()->message("e.licence_groups.bad", ll_warning, lc_no_context)
            << "Got error '" << e.message() << "' (" << e.what() << ") when parsing " << p;
    }
}

const std::shared_ptr<const Set<std::string> >
LicenceGroups::maybe_expand_licence_nonrecursively(const std::string & s) const
{
    if (0 == s.compare(0, 1, "@", 0, 1))
    {
        auto i(_imp->store.find(s.substr(1)));
        if (_imp->store.end() == i)
            return nullptr;
        else
            return i->second;
    }
    else
        return nullptr;
}

