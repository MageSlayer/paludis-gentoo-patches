/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Mike Kelly <pioto@pioto.org>
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
#include <paludis/util/fs_entry.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/log.hh>
#include <paludis/util/options.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

#include <list>

#include <paludis/repositories/e/manifest2_entry-sr.cc>


/** \file
 * Implementation of manifest2_reader.hh
 *
 * \ingroup grpmanifest2reader
 */

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template<>
    struct Implementation<Manifest2Reader>
    {
        FSEntry manifest;
        std::list<Manifest2Entry> entries;

        Implementation(const FSEntry & f) :
            manifest(f)
        {
        }
    };
}

Manifest2Error::Manifest2Error(const std::string & msg) throw () :
    ActionError("Manifest2 Error: " + msg)
{
}

Manifest2Reader::Manifest2Reader(const FSEntry & f) :
    PrivateImplementationPattern<Manifest2Reader>(new Implementation<Manifest2Reader>(f))
{
    if (! f.exists())
        return;

    LineConfigFile lines(_imp->manifest, LineConfigFileOptions());

    for (LineConfigFile::ConstIterator l(lines.begin()), l_end(lines.end()) ;
        l != l_end ; ++l)
    {
        std::list<std::string> tokens;
        WhitespaceTokeniser::tokenise((*l),
            create_inserter<std::string>(std::back_inserter(tokens)));
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
                Log::get_instance()->message(ll_debug, lc_no_context)
                    << "Skipping unknown checksum type " << checksum_type;
        }

        _imp->entries.push_back(Manifest2Entry::create()
            .type(type)
            .size(size)
            .name(name)
            .sha1(sha1)
            .sha256(sha256)
            .rmd160(rmd160)
            .md5(md5)
            );
    }
}

Manifest2Reader::~Manifest2Reader()
{
}

Manifest2Reader::ConstIterator
Manifest2Reader::begin() const
{
    return ConstIterator(_imp->entries.begin());
}

Manifest2Reader::ConstIterator
Manifest2Reader::end() const
{
    return ConstIterator(_imp->entries.end());
}
