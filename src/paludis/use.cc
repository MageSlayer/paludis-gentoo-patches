/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include "use.hh"
#include "colour.hh"
#include <paludis/util/tokeniser.hh>
#include <sstream>
#include <set>

using namespace paludis;

std::string
make_pretty_use_flags_string(const Environment * const env, const PackageDatabaseEntry & p,
        VersionMetadata::ConstPointer metadata)
{
    std::ostringstream c;

    if (metadata->get_ebuild_interface())
    {
        const RepositoryUseInterface * const use_interface(
                env->package_database()->
                fetch_repository(p.repository)->use_interface);
        std::set<UseFlagName> iuse;
        WhitespaceTokeniser::get_instance()->tokenise(
                metadata->get_ebuild_interface()->iuse,
                create_inserter<UseFlagName>(std::inserter(iuse, iuse.end())));

        /* display normal use flags first */
        for (std::set<UseFlagName>::const_iterator i(iuse.begin()), i_end(iuse.end()) ;
                i != i_end ; ++i)
        {
            if (use_interface->is_expand_flag(*i))
                continue;

            if (env->query_use(*i, &p))
            {
                if (use_interface && use_interface->query_use_force(*i, &p))
                    c << " " << colour(cl_flag_on, "(" + stringify(*i) + ")");
                else
                    c << " " << colour(cl_flag_on, *i);
            }
            else
            {
                if (use_interface && use_interface->query_use_mask(*i, &p))
                    c << " " << colour(cl_flag_off, "(-" + stringify(*i) + ")");
                else
                    c << " " << colour(cl_flag_off, "-" + stringify(*i));
            }
        }

        /* now display expand flags */
        UseFlagName old_expand_name("OFTEN_NOT_BEEN_ON_BOATS");
        for (std::set<UseFlagName>::const_iterator i(iuse.begin()), i_end(iuse.end()) ;
                i != i_end ; ++i)
        {
            if ((! use_interface->is_expand_flag(*i)) ||
                    (use_interface->is_expand_hidden_flag(*i)))
                continue;

            UseFlagName expand_name(use_interface->expand_flag_name(*i)),
                expand_value(use_interface->expand_flag_value(*i));

            if (expand_name != old_expand_name)
            {
                c << " " << expand_name << ":";
                old_expand_name = expand_name;
            }

            if (env->query_use(*i, &p))
            {
                if (use_interface && use_interface->query_use_force(*i, &p))
                    c << " " << colour(cl_flag_on, "(" + stringify(expand_value) + ")");
                else
                    c << " " << colour(cl_flag_on, expand_value);
            }
            else
            {
                if (use_interface && use_interface->query_use_mask(*i, &p))
                    c << " " << colour(cl_flag_off, "(-" + stringify(expand_value) + ")");
                else
                    c << " " << colour(cl_flag_off, "-" + stringify(expand_value));
            }
        }
    }

    return c.str();
}

