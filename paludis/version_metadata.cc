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

#include <paludis/util/iterator.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/version_metadata.hh>
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
    if (key < 0 || key >= static_cast<int>(_imp->values.size()))
        throw InternalError(PALUDIS_HERE, "Bad value for key");
    return _imp->values[key];
}

VersionMetadata &
VersionMetadata::set(const VersionMetadataKey key, const std::string & value)
{
    if (key < 0 || key >= static_cast<int>(_imp->values.size()))
        throw InternalError(PALUDIS_HERE, "Bad value for key");

    _imp->values[key] = value;

    switch (key)
    {
        case vmk_iuse:
            _imp->iuse.clear();
            break;

        case vmk_keywords:
            _imp->keywords.clear();
            break;

        case vmk_provide:
            _imp->provide.clear();
            break;

        default:
            break;
    }

    return *this;
}

VersionMetadata::IuseIterator
VersionMetadata::begin_iuse() const
{
    if (_imp->iuse.empty())
    {
        Tokeniser<delim_kind::AnyOfTag, delim_mode::DelimiterTag> tokeniser(" \t\n");
        tokeniser.tokenise(get(vmk_iuse), create_inserter<UseFlagName>(
                    std::inserter(_imp->iuse, _imp->iuse.begin())));
    }
    return _imp->iuse.begin();
}

VersionMetadata::IuseIterator
VersionMetadata::end_iuse() const
{
    return _imp->iuse.end();
}

VersionMetadata::KeywordIterator
VersionMetadata::begin_keywords() const
{
    if (_imp->keywords.empty())
    {
        Tokeniser<delim_kind::AnyOfTag, delim_mode::DelimiterTag> tokeniser(" \t\n");
        tokeniser.tokenise(get(vmk_keywords), create_inserter<KeywordName>(
                    std::inserter(_imp->keywords, _imp->keywords.begin())));
    }

    return _imp->keywords.begin();
}

VersionMetadata::KeywordIterator
VersionMetadata::end_keywords() const
{
    return _imp->keywords.end();
}

