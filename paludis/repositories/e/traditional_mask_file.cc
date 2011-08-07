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

#include <paludis/repositories/e/traditional_mask_file.hh>
#include <paludis/repositories/e/eapi.hh>

#include <paludis/util/pimp-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/log.hh>
#include <paludis/util/join.hh>

#include <paludis/mask.hh>

#include <vector>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

typedef std::list<std::pair<const std::string, std::shared_ptr<const erepository::MaskInfo> > > MaskFileLines;

namespace paludis
{
    template <>
    struct Imp<TraditionalMaskFile>
    {
        MaskFileLines lines;
    };

    template <>
    struct WrappedForwardIteratorTraits<TraditionalMaskFile::ConstIteratorTag>
    {
        typedef MaskFileLines::const_iterator UnderlyingIterator;
    };
}

TraditionalMaskFile::TraditionalMaskFile(const FSPath & f, const LineConfigFileOptions & opts, const EAPI & eapi) :
    _imp()
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

        std::vector<std::string> tokens;
        tokenise_whitespace(*it, std::back_inserter(tokens));

        if (tokens.size() == 0)
            continue;
        else if (tokens.size() == 1)
        {
        }
        else if (tokens.size() == 2 && ! eapi.supported()->allow_tokens_in_mask_files())
        {
            Log::get_instance()->message("e.mask.malformed", ll_qa, lc_context)
                << "Line '" << *it << "' in '" << f << "' contains tokens after the spec; ignoring";
            continue;
        }
        else
        {
            Log::get_instance()->message("e.mask.malformed", ll_qa, lc_context)
                << "Line '" << *it << "' in '" << f << "' contains multiple tokens; ignoring";
            continue;
        }

        _imp->lines.push_back(std::make_pair(tokens.at(0), std::make_shared<erepository::MaskInfo>(
                        make_named_values<erepository::MaskInfo>(
                            n::comment() = join(comment->begin(), comment->end(), " "),
                            n::mask_file() = f,
                            n::token() = tokens.size() < 2 ? "" : tokens.at(1)
                            ))));
        comment_used = true;
    }
}

TraditionalMaskFile::ConstIterator
TraditionalMaskFile::begin() const
{
    return ConstIterator(_imp->lines.begin());
}

TraditionalMaskFile::ConstIterator
TraditionalMaskFile::end() const
{
    return ConstIterator(_imp->lines.end());
}

TraditionalMaskFile::~TraditionalMaskFile()
{
}

namespace paludis
{
    template class WrappedForwardIterator<TraditionalMaskFile::ConstIteratorTag,
             const std::pair<const std::string, std::shared_ptr<const MaskInfo> > >;
}
