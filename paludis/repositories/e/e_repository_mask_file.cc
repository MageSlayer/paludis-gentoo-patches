/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton
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
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/mask.hh>

#include <list>

using namespace paludis;
using namespace paludis::erepository;

typedef std::list<std::pair<const std::string, std::shared_ptr<const RepositoryMaskInfo> > > MaskFileLines;

namespace paludis
{
    template <>
    struct Imp<MaskFile>
    {
        MaskFileLines lines;
    };

    template <>
    struct WrappedForwardIteratorTraits<MaskFile::ConstIteratorTag>
    {
        typedef MaskFileLines::const_iterator UnderlyingIterator;
    };
}

MaskFile::MaskFile(const FSEntry & f, const LineConfigFileOptions & opts) :
    Pimp<MaskFile>()
{
    LineConfigFileOptions myopts(opts);
    myopts += lcfo_disallow_comments;
    myopts += lcfo_no_skip_blank_lines;

    LineConfigFile file(f, myopts);
    std::shared_ptr<Sequence<std::string> > comment(std::make_shared<Sequence<std::string>>());
    bool comment_used(false);
    for (LineConfigFile::ConstIterator it(file.begin()), it_end(file.end()); it_end != it; ++it)
    {
        if (it->empty())
        {
            comment = std::make_shared<Sequence<std::string>>();
            comment_used = false;
            continue;
        }

        if ('#' == it->at(0))
        {
            if (comment_used)
            {
                std::shared_ptr<Sequence<std::string> > cpy(std::make_shared<Sequence<std::string>>());
                std::copy(comment->begin(), comment->end(), cpy->back_inserter());
                comment = cpy;
                comment_used = false;
            }
            comment->push_back(strip_leading(it->substr(1), " \t\r\n"));
            continue;
        }

        _imp->lines.push_back(std::make_pair(*it, std::shared_ptr<RepositoryMaskInfo>(std::make_shared<RepositoryMaskInfo>(
                            make_named_values<RepositoryMaskInfo>(n::comment() = comment, n::mask_file() = f)))));
        comment_used = true;
    }
}

MaskFile::ConstIterator
MaskFile::begin() const
{
    return ConstIterator(_imp->lines.begin());
}

MaskFile::ConstIterator
MaskFile::end() const
{
    return ConstIterator(_imp->lines.end());
}

MaskFile::~MaskFile()
{
}

template class WrappedForwardIterator<MaskFile::ConstIteratorTag,
         const std::pair<const std::string, std::shared_ptr<const RepositoryMaskInfo> > >;

