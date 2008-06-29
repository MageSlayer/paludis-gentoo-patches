/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#include "use_desc.hh"
#include <paludis/name.hh>
#include <paludis/package_id.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/hashes.hh>
#include <tr1/unordered_map>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<UseDesc>
    {
        std::tr1::unordered_map<std::string, std::string, Hash<std::string> > desc;

        void add(const FSEntry & f, const std::string & prefix)
        {
            if (f.is_regular_file_or_symlink_to_regular_file())
            {
                LineConfigFile ff(f, LineConfigFileOptions() + lcfo_disallow_continuations);
                for (LineConfigFile::ConstIterator line(ff.begin()), line_end(ff.end()) ;
                        line != line_end ; ++line)
                {
                    std::string::size_type p(line->find(" - "));
                    if (std::string::npos == p)
                        continue;

                    desc.insert(std::make_pair(prefix + line->substr(0, p), line->substr(p + 3)));
                }
            }
        }

        Implementation(const FSEntry & f, const std::string & expand_sep)
        {
            add(f / "use.desc", "");
            add(f / "use.local.desc", "");

            if ((f / "desc").is_directory_or_symlink_to_directory())
                for (DirIterator d(f / "desc"), d_end ; d != d_end ; ++d)
                    if (is_file_with_extension(*d, ".desc", IsFileWithOptions()))
                        add(*d, strip_trailing_string(d->basename(), ".desc") + expand_sep);
        }
    };
}

UseDesc::UseDesc(const FSEntry & f, const std::string & expand_sep) :
    PrivateImplementationPattern<UseDesc>(new Implementation<UseDesc>(f, expand_sep))
{
}

UseDesc::~UseDesc()
{
}

std::string
UseDesc::describe(const UseFlagName & f, const PackageID & e) const
{
    std::tr1::unordered_map<std::string, std::string, Hash<std::string> >::const_iterator i(
            _imp->desc.find(stringify(e.name()) + ":" + stringify(f)));
    if (_imp->desc.end() != i)
        return i->second;

    std::tr1::unordered_map<std::string, std::string, Hash<std::string> >::const_iterator j(_imp->desc.find(stringify(f)));
    if (_imp->desc.end() != j)
        return j->second;

    return "";
}

