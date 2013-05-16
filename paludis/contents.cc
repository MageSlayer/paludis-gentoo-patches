/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/contents.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/literal_metadata_key.hh>
#include <list>

using namespace paludis;

typedef std::list<std::shared_ptr<const ContentsEntry> > Entries;

namespace paludis
{
    template <>
    struct Imp<ContentsEntry>
    {
        const std::shared_ptr<const MetadataValueKey<FSPath> > location_key;

        Imp(const FSPath & n) :
            location_key(std::make_shared<LiteralMetadataValueKey<FSPath>>("location", "location", mkt_significant, n))
        {
        }
    };
}

ContentsEntry::ContentsEntry(const FSPath & n) :
    _imp(n)
{
    add_metadata_key(_imp->location_key);
}

ContentsEntry::~ContentsEntry()
{
}

void
ContentsEntry::need_keys_added() const
{
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
ContentsEntry::location_key() const
{
    return _imp->location_key;
}

namespace paludis
{
    template <>
    struct Imp<ContentsFileEntry>
    {
        const std::shared_ptr<const MetadataValueKey<std::string> > part_key;

        Imp(const std::string & part) :
            part_key(part.empty()
                        ? nullptr
                        : std::make_shared<LiteralMetadataValueKey<std::string>>("part", "part", mkt_normal, part))
        {
        }
    };
}

ContentsFileEntry::ContentsFileEntry(const FSPath & our_name,
                                     const std::string & our_part) :
    ContentsEntry(our_name), _imp(our_part)
{
    if (_imp->part_key)
        add_metadata_key(_imp->part_key);
}

ContentsFileEntry::~ContentsFileEntry()
{
}

const std::shared_ptr<const MetadataValueKey<std::string> >
ContentsFileEntry::part_key() const
{
    return _imp->part_key;
}

ContentsDirEntry::ContentsDirEntry(const FSPath & our_name) :
    ContentsEntry(our_name)
{
}

ContentsOtherEntry::ContentsOtherEntry(const FSPath & our_name) :
    ContentsEntry(our_name)
{
}

namespace paludis
{
    template <>
    struct Imp<ContentsSymEntry>
    {
        const std::shared_ptr<const MetadataValueKey<std::string> > target_key;
        const std::shared_ptr<const MetadataValueKey<std::string> > part_key;

        Imp(const std::string & target, const std::string & part) :
            target_key(std::make_shared<LiteralMetadataValueKey<std::string>>("target", "target", mkt_normal, target)),
            part_key(part.empty()
                        ? nullptr
                        : std::make_shared<LiteralMetadataValueKey<std::string>>("part", "part", mkt_normal, part))
        {
        }
    };
}

ContentsSymEntry::ContentsSymEntry(const FSPath & our_name,
                                   const std::string & our_target,
                                   const std::string & our_part) :
    ContentsEntry(our_name), _imp(our_target, our_part)
{
    add_metadata_key(_imp->target_key);
    if (_imp->part_key)
        add_metadata_key(_imp->part_key);
}

ContentsSymEntry::~ContentsSymEntry()
{
}

const std::shared_ptr<const MetadataValueKey<std::string> >
ContentsSymEntry::target_key() const
{
    return _imp->target_key;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
ContentsSymEntry::part_key() const
{
    return _imp->part_key;
}

namespace paludis
{
    template<>
    struct Imp<Contents>
    {
        Entries c;
    };

    template <>
    struct WrappedForwardIteratorTraits<Contents::ConstIteratorTag>
    {
        typedef Entries::const_iterator UnderlyingIterator;
    };
}

Contents::Contents() :
    _imp()
{
}

Contents::~Contents()
{
}

void
Contents::add(const std::shared_ptr<const ContentsEntry> & c)
{
    _imp->c.push_back(c);
}

Contents::ConstIterator
Contents::begin() const
{
    return ConstIterator(_imp->c.begin());
}

Contents::ConstIterator
Contents::end() const
{
    return ConstIterator(_imp->c.end());
}

namespace paludis
{
    template class Pimp<Contents>;
    template class Pimp<ContentsEntry>;
    template class Pimp<ContentsSymEntry>;
    template class Pimp<ContentsFileEntry>;

    template class WrappedForwardIterator<Contents::ConstIteratorTag, const std::shared_ptr<const ContentsEntry> >;
}
