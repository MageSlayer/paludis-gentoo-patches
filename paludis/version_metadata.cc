/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "version_metadata.hh"
#include "create_insert_iterator.hh"
#include "tokeniser.hh"
#include "create_insert_iterator.hh"
#include <vector>

using namespace paludis;

namespace paludis
{
    /**
     * Implementation data for VersionMetadata.
     */
    template <>
    struct Implementation<VersionMetadata> :
        InternalCounted<Implementation<VersionMetadata> >
    {
        /**
         * Our values.
         */
        std::vector<std::string> values;

        /**
         * Cache: IUSE.
         */
        mutable std::set<UseFlagName> iuse;

        /**
         * Cache: KEYWORDS.
         */
        mutable std::set<KeywordName> keywords;

        /**
         * Cache: PROVIDE.
         */
        mutable std::set<QualifiedPackageName> provide;

        /**
         * Constructor.
         */
        Implementation();
    };
}

Implementation<VersionMetadata>::Implementation()
{
    values.resize(static_cast<unsigned>(last_vmk));
}

VersionMetadata::VersionMetadata() :
    PrivateImplementationPattern<VersionMetadata>(new Implementation<VersionMetadata>())
{
}

VersionMetadata::~VersionMetadata()
{
}

const std::string &
VersionMetadata::get(const VersionMetadataKey key) const
{
    /// \bug check key
    return _implementation->values[key];
}

VersionMetadata &
VersionMetadata::set(const VersionMetadataKey key, const std::string & value)
{
    _implementation->values[key] = value;
    return *this;
}

VersionMetadata::IuseIterator
VersionMetadata::begin_iuse() const
{
    if (_implementation->iuse.empty())
    {
        Tokeniser<delim_kind::AnyOfTag, delim_mode::DelimiterTag> tokeniser(" \t\n");
        tokeniser.tokenise(get(vmk_iuse), create_inserter<UseFlagName>(
                    std::inserter(_implementation->iuse, _implementation->iuse.begin())));
    }
    return _implementation->iuse.begin();
}

VersionMetadata::IuseIterator
VersionMetadata::end_iuse() const
{
    return _implementation->iuse.end();
}

VersionMetadata::KeywordIterator
VersionMetadata::begin_keywords() const
{
    if (_implementation->keywords.empty())
    {
        Tokeniser<delim_kind::AnyOfTag, delim_mode::DelimiterTag> tokeniser(" \t\n");
        tokeniser.tokenise(get(vmk_keywords), create_inserter<KeywordName>(
                    std::inserter(_implementation->keywords, _implementation->keywords.begin())));
    }

    return _implementation->keywords.begin();
}

VersionMetadata::KeywordIterator
VersionMetadata::end_keywords() const
{
    return _implementation->keywords.end();
}

VersionMetadata::ProvideIterator
VersionMetadata::begin_provide() const
{
    if (_implementation->provide.empty())
    {
        Tokeniser<delim_kind::AnyOfTag, delim_mode::DelimiterTag> tokeniser(" \t\n");
        tokeniser.tokenise(get(vmk_provide),
                create_inserter<QualifiedPackageName>(
                    std::inserter(_implementation->provide, _implementation->provide.begin())));
    }

    return _implementation->provide.begin();
}

VersionMetadata::ProvideIterator
VersionMetadata::end_provide() const
{
    return _implementation->provide.end();
}

