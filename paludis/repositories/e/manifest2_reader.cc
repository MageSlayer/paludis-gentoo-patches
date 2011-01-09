/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Mike Kelly
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

#include <paludis/repositories/e/manifest2_reader.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/options.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_stat.hh>

#include <list>
#include <map>

using namespace paludis;
using namespace paludis::erepository;

typedef std::map<std::pair<std::string, std::string>, Manifest2Entry> Entries;

namespace paludis
{
    template <>
    struct Imp<Manifest2Reader>
    {
        FSPath manifest;
        Entries entries;

        Imp(const FSPath & f) :
            manifest(f)
        {
        }
    };

    template <>
    struct WrappedForwardIteratorTraits<Manifest2Reader::ConstIteratorTag>
    {
        typedef SecondIteratorTypes<Entries::const_iterator>::Type UnderlyingIterator;
    };
}

Manifest2Error::Manifest2Error(const std::string & msg) throw () :
    Exception("Manifest2 Error: " + msg)
{
}

Manifest2Reader::Manifest2Reader(const FSPath & f) :
    _imp(f)
{
    if (! f.stat().exists())
        return;

    LineConfigFile lines(_imp->manifest, { });

    for (LineConfigFile::ConstIterator l(lines.begin()), l_end(lines.end()) ;
        l != l_end ; ++l)
    {
        std::list<std::string> tokens;
        tokenise_whitespace((*l), create_inserter<std::string>(std::back_inserter(tokens)));
        std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end());

        std::string type, name, sha1, sha256, rmd160, md5;
        off_t size;

        if (t_end == t)
            continue;
        type = (*t);

        if ("EBUILD" != type
                && "EBIN" != type
                && "EXHERES" != type
                && "MISC" != type
                && "AUX" != type
                && "DIST" != type)
            continue;

        ++t;
        if (t_end == t)
            throw Manifest2Error("no file name found");
        name = (*t);

        ++t;
        if (t_end == t)
            throw Manifest2Error("no file size found");
        size = destringify<off_t>(*t);

        ++t;
        for ( ; t!= t_end ; ++t)
        {
            std::string checksum_type(*t);
            ++t;

            if (t_end == t)
                throw Manifest2Error("no checksum for: " + checksum_type);

            if ("SHA1" == checksum_type)
                sha1 = (*t);
            else if ("SHA256" == checksum_type)
                sha256 = (*t);
            else if ("RMD160" == checksum_type)
                rmd160 = (*t);
            else if ("MD5" == checksum_type)
                md5 = (*t);
            else
                Log::get_instance()->message("e.manifest.unknown_checksum", ll_debug, lc_no_context)
                    << "Skipping unknown checksum type " << checksum_type;
        }

        _imp->entries.insert(std::make_pair(std::make_pair(type,name), make_named_values<Manifest2Entry>(
                        n::md5() = md5,
                        n::name() = name,
                        n::rmd160() = rmd160,
                        n::sha1() = sha1,
                        n::sha256() = sha256,
                        n::size() = size,
                        n::type() = type
                        )));
    }
}

Manifest2Reader::~Manifest2Reader()
{
}

Manifest2Reader::ConstIterator
Manifest2Reader::begin() const
{
    return ConstIterator(second_iterator(_imp->entries.begin()));
}

Manifest2Reader::ConstIterator
Manifest2Reader::end() const
{
    return ConstIterator(second_iterator(_imp->entries.end()));
}

Manifest2Reader::ConstIterator
Manifest2Reader::find(const std::string & type, const std::string & name) const
{
    return ConstIterator(second_iterator(_imp->entries.find(std::make_pair(type,name))));
}

Manifest2Reader::ConstIterator
Manifest2Reader::find(const std::pair<const std::string, const std::string> & p) const
{
    return ConstIterator(second_iterator(_imp->entries.find(p)));
}

template class WrappedForwardIterator<Manifest2Reader::ConstIteratorTag, const Manifest2Entry>;

