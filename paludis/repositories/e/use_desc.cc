/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2010 Ciaran McCreesh
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
#include <paludis/util/sequence-impl.hh>
#include <paludis/choice.hh>
#include <tr1/unordered_map>

using namespace paludis;

template class Sequence<UseDescFileInfo>;
typedef std::tr1::unordered_map<std::pair<ChoicePrefixName, UnprefixedChoiceName>, std::string,
        Hash<std::pair<ChoicePrefixName, UnprefixedChoiceName> > > UseDescs;

namespace paludis
{
    template<>
    struct Implementation<UseDesc>
    {
        std::tr1::unordered_map<QualifiedPackageName, UseDescs, Hash<QualifiedPackageName> > local_descs;
        UseDescs global_descs;

        void add(const FSEntry & f, const ChoicePrefixName & prefix)
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
                    std::string lhs(line->substr(0, p)), rhs(line->substr(p + 3));

                    std::string::size_type q(lhs.find(':'));
                    if (std::string::npos == q)
                        global_descs.insert(make_pair(make_pair(prefix, lhs), rhs));
                    else
                        local_descs[QualifiedPackageName(lhs.substr(0, q))].insert(make_pair(make_pair(prefix, lhs.substr(q + 1)), rhs));
                }
            }
        }

        Implementation(const std::tr1::shared_ptr<const UseDescFileInfoSequence> & f)
        {
            for (UseDescFileInfoSequence::ConstIterator ff(f->begin()), ff_end(f->end()) ;
                    ff != ff_end ; ++ff)
                add(ff->first, ff->second);
        }
    };
}

UseDesc::UseDesc(const std::tr1::shared_ptr<const UseDescFileInfoSequence> & f) :
    PrivateImplementationPattern<UseDesc>(new Implementation<UseDesc>(f))
{
}

UseDesc::~UseDesc()
{
}

const std::string
UseDesc::describe(
        const QualifiedPackageName & id,
        const ChoicePrefixName & prefix,
        const UnprefixedChoiceName & flag
        ) const
{
    std::tr1::unordered_map<QualifiedPackageName, UseDescs, Hash<QualifiedPackageName> >::const_iterator i(_imp->local_descs.find(id));
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

