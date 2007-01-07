/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006,2007 Danny van Dyk <kugelfang@gentoo.org>
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

#include <paludis/repositories/cran/cran_description.hh>
#include <paludis/repositories/cran/cran_dep_parser.hh>
#include <paludis/config_file.hh>
#include <paludis/util/log.hh>
#include <paludis/util/strip.hh>
#include <string>

using namespace paludis;

CRANDescription::CRANDescription(const std::string & n, const FSEntry & f) :
    name("cran/" + n),
    version("0"),
    metadata(new VersionMetadata::CRAN(CRANDepParser::parse))
{
    Context context("When parsing file '" + stringify(f) + "' for package '" + n + "':");

    if (! f.is_regular_file())
    {
        Log::get_instance()->message(ll_warning, lc_context, "Unexpected irregular file: '" + stringify(f) + "'.");
        metadata->eapi = "INVALID";

        return;
    }

    LineConfigFile file(f);
    LineConfigFile::Iterator l(file.begin()), l_end(file.end());

    // Fill in default values
    metadata->slot = SlotName("0");
    metadata->eapi = "CRAN-0";
    metadata->get_cran_interface()->keywords = std::string("*");

    std::string key;
    for ( ; l != l_end ; ++l)
    {
        Context local_context("When parsing line '" + *l + "':");

        std::string line(strip_leading(strip_trailing(*l, " \t\r\n"), " \t\r\n")), value;
        std::string::size_type pos(line.find(':'));
        if (std::string::npos == pos)
        {
            value = strip_leading(strip_trailing(line, " \t"), " \t");
        }
        else
        {
            key = strip_leading(strip_trailing(line.substr(0, pos), " \t"), " \t");
            value = strip_leading(strip_trailing(line.substr(pos + 1), " \t\r\n"), " \t\r\n");
        }

        if (("Package" == key) || ("Bundle" == key))
        {
            metadata->get_cran_interface()->package = value;
            metadata->homepage = "http://cran.r-project.org/src/contrib/Descriptions/" + value + ".html";
            if ("Package" == key)
            {
                CRANDescription::normalise_name(value);
                if (n != value)
                    Log::get_instance()->message(ll_warning, lc_context, "Inconsistent package name in file '" +
                                stringify(name) + "': '" + n + "', '" + value + "':");
            }
            else
            {
                metadata->get_cran_interface()->is_bundle = true;
            }
        }
        else if ("Version" == key)
        {
            metadata->get_cran_interface()->version = value;
            normalise_version(value);
            version = VersionSpec(value);
        }
        else if ("Title" == key)
        {
            metadata->description = value;
        }
        else if ("Depends" == key)
        {
            if (value.empty())
                value = "R";
            else
                value.append(", R");
            metadata->deps.build_depend_string = value;
            metadata->deps.run_depend_string = value;
        }
    }
}

