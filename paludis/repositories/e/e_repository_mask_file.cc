/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton <levertond@googlemail.com>
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

#include "e_repository_mask_file.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/strip.hh>
#include <paludis/config_file.hh>
#include <paludis/mask.hh>

#include <libwrapiter/libwrapiter_forward_iterator-impl.hh>
#include <libwrapiter/libwrapiter_output_iterator-impl.hh>

#include <list>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Implementation<MaskFile>
    {
        std::list<std::pair<const std::string, tr1::shared_ptr<const RepositoryMaskInfo> > > lines;
    };
}

MaskFile::MaskFile(const FSEntry & f, const LineConfigFileOptions & opts) :
    PrivateImplementationPattern<MaskFile>(new Implementation<MaskFile>)
{
    LineConfigFileOptions myopts(opts);
    myopts += lcfo_disallow_comments;
    myopts += lcfo_no_skip_blank_lines;

    LineConfigFile file(f, myopts);
    tr1::shared_ptr<Sequence<std::string> > comment(new Sequence<std::string>);
    bool comment_used(false);
    for (LineConfigFile::Iterator it(file.begin()), it_end(file.end()); it_end != it; ++it)
    {
        if (it->empty())
        {
            comment.reset(new Sequence<std::string>);
            comment_used = false;
            continue;
        }

        if ('#' == it->at(0))
        {
            if (comment_used)
            {
                tr1::shared_ptr<Sequence<std::string> > cpy(new Sequence<std::string>);
                std::copy(comment->begin(), comment->end(), cpy->back_inserter());
                comment = cpy;
                comment_used = false;
            }
            comment->push_back(strip_leading(it->substr(1), " \t\r\n"));
            continue;
        }

        _imp->lines.push_back(std::make_pair(*it, tr1::shared_ptr<RepositoryMaskInfo>(new RepositoryMaskInfo(f, comment))));
        comment_used = true;
    }
}

MaskFile::Iterator
MaskFile::begin() const
{
    return Iterator(_imp->lines.begin());
}

MaskFile::Iterator
MaskFile::end() const
{
    return Iterator(_imp->lines.end());
}

MaskFile::~MaskFile()
{
}

