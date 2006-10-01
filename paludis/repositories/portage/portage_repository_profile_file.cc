/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "portage_repository_profile_file.hh"
#include <paludis/util/log.hh>
#include <paludis/config_file.hh>
#include <set>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<ProfileFile> :
        InternalCounted<Implementation<ProfileFile> >
    {
        std::set<std::string> lines;
    };
}

void
ProfileFile::add_file(const FSEntry & f)
{
    Context context("When adding profile configuration file '" + stringify(f) + "':");

    if (! f.exists())
        return;

    LineConfigFile file(f);
    for (LineConfigFile::Iterator line(file.begin()), line_end(file.end()) ; line != line_end ; ++line)
    {
        if (0 == line->compare(0, 1, "-", 0, 1))
        {
            std::set<std::string>::iterator i(_imp->lines.find(line->substr(1)));
            if (_imp->lines.end() == i)
                Log::get_instance()->message(ll_qa, lc_context, "No match for '" + *line + "'");
            else
                _imp->lines.erase(i);
        }
        else
            _imp->lines.insert(*line);
    }
}

ProfileFile::ProfileFile() :
    PrivateImplementationPattern<ProfileFile>(new Implementation<ProfileFile>)
{
}

ProfileFile::~ProfileFile()
{
}

ProfileFile::Iterator
ProfileFile::begin() const
{
    return Iterator(_imp->lines.begin());
}

ProfileFile::Iterator
ProfileFile::end() const
{
    return Iterator(_imp->lines.end());
}

