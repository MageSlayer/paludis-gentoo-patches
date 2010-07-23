/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2010 Ciaran McCreesh
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

#include "description_file.hh"
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/stringify.hh>
#include <sstream>
#include <istream>
#include <map>

using namespace paludis;
using namespace paludis::cranrepository;

namespace paludis
{
    template <>
    struct Imp<DescriptionFile>
    {
        std::map<std::string, std::string> values;
    };
}

DescriptionFile::DescriptionFile(const Source & sr) :
    Pimp<DescriptionFile>()
{
    Context c("When parsing CRAN description file '" + sr.filename() + "':");

    std::istringstream s(sr.text());
    if (! s)
        throw ConfigFileError(sr.filename(), "Cannot read input");

    std::string line, line_full;
    while (std::getline(s, line))
    {
        /* cran needs to be taken out and shot in the nuts for this... */
        line = strip_trailing(line, "\r");

        if ((! line.empty()) && (std::string::npos != std::string(" \t").find_first_of(line.at(0))))
        {
            if (line_full.empty())
                throw ConfigFileError(sr.filename(), "Unexpected continuation");

            line_full += " ";
            line_full += strip_leading(strip_trailing(line, " \t"), " \t");
        }
        else
        {
            if (! line_full.empty())
                _line(line_full);

            line_full = strip_leading(strip_trailing(line, " \t"), " \t");
        }
    }

    if (! line_full.empty())
        _line(line_full);
}

DescriptionFile::~DescriptionFile()
{
}

void
DescriptionFile::_line(const std::string & l)
{
    std::string::size_type p(l.find(':'));
    if (std::string::npos == p)
        Log::get_instance()->message("cran.description.malformed", ll_warning, lc_context) << "No colon on line '" << l << "'";
    else
    {
        std::string key(strip_leading(strip_trailing(l.substr(0, p), " \t"), " \t")),
            value(strip_leading(strip_trailing(l.substr(p + 1), " \t"), " \t"));

        if (key.empty())
            ;
        else if (value.empty())
            ;
        else
        {
            std::pair<std::map<std::string, std::string>::iterator, bool> r(_imp->values.insert(std::make_pair(key, value)));
            if ((! r.second) && (r.first->second != value))
                Log::get_instance()->message("cran.description.duplicate", ll_qa, lc_context) << "Key '" << key << "' already set to '"
                    << r.first->second << "', ignoring duplicate key value '" << value << "'";
        }
    }
}

std::string
DescriptionFile::get(const std::string & k) const
{
    std::map<std::string, std::string>::const_iterator i(_imp->values.find(k));
    if (i == _imp->values.end())
        return "";
    return i->second;
}

