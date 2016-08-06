/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/options.hh>
#include <paludis/choice.hh>
#include <unordered_map>

using namespace paludis;

typedef std::unordered_map<std::pair<ChoicePrefixName, UnprefixedChoiceName>, std::string,
        Hash<std::pair<ChoicePrefixName, UnprefixedChoiceName> > > UseDescs;

namespace paludis
{
    template<>
    struct Imp<UseDesc>
    {
        std::unordered_map<QualifiedPackageName, UseDescs, Hash<QualifiedPackageName> > local_descs;
        UseDescs global_descs;

        void add(const FSPath & f, const ChoicePrefixName & prefix)
        {
            if (f.stat().is_regular_file_or_symlink_to_regular_file())
            {
                LineConfigFile ff(f, { lcfo_disallow_continuations });
                for (LineConfigFile::ConstIterator line(ff.begin()), line_end(ff.end()) ;
                        line != line_end ; ++line)
                {
                    std::string::size_type p(line->find(" - "));
                    if (std::string::npos == p)
                        continue;
                    std::string lhs(line->substr(0, p)), rhs(line->substr(p + 3));

                    std::string::size_type q(lhs.find(':'));
                    if (std::string::npos == q)
                        global_descs.insert(std::make_pair(std::make_pair(ChoicePrefixName(prefix), UnprefixedChoiceName(lhs)), rhs));
                    else
                        local_descs[QualifiedPackageName(lhs.substr(0, q))].insert(
                                std::make_pair(std::make_pair(ChoicePrefixName(prefix), UnprefixedChoiceName(lhs.substr(q + 1))), rhs));
                }
            }
        }

        Imp(const std::shared_ptr<const UseDescFileInfoSequence> & f)
        {
            for (UseDescFileInfoSequence::ConstIterator ff(f->begin()), ff_end(f->end()) ;
                    ff != ff_end ; ++ff)
                add(ff->first, ff->second);
        }
    };
}

UseDesc::UseDesc(const std::shared_ptr<const UseDescFileInfoSequence> & f) :
    _imp(f)
{
}

UseDesc::~UseDesc() = default;

const std::string
UseDesc::describe(
        const QualifiedPackageName & id,
        const ChoicePrefixName & prefix,
        const UnprefixedChoiceName & flag
        ) const
{
    std::unordered_map<QualifiedPackageName, UseDescs, Hash<QualifiedPackageName> >::const_iterator i(_imp->local_descs.find(id));
    if (i != _imp->local_descs.end())
    {
        UseDescs::const_iterator j(i->second.find(std::make_pair(prefix, flag)));
        if (j != i->second.end())
            return j->second;
    }

    UseDescs::const_iterator j(_imp->global_descs.find(std::make_pair(prefix, flag)));
    if (j != _imp->global_descs.end())
        return j->second;

    return "";
}

namespace paludis
{
    template class Sequence<UseDescFileInfo>;
}
