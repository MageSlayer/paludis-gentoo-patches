/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Tiziano Müller
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

#ifndef ELFRELOCATIONSECTION_HH_
#define ELFRELOCATIONSECTION_HH_

#include "elf_sections.hh"

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>

#include <iosfwd>

template <typename ElfType_>
class RelocationEntry
{
    private:
        typename ElfType_::Relocation _my_relocation;

    public:
        RelocationEntry(const typename ElfType_::Relocation &);
        ~RelocationEntry();

        typename ElfType_::RelocationOffset get_offset() const
        {
            return _my_relocation.r_offset;
        }

        typename ElfType_::RelocationInfo get_info() const
        {
            return _my_relocation.r_info;
        }
};

template <typename ElfType_>
class RelocationAEntry
{
    private:
        typename ElfType_::RelocationA _my_relocation;

    public:
        RelocationAEntry(const typename ElfType_::RelocationA &);
        ~RelocationAEntry();

        typename ElfType_::RelocationOffset get_offset() const
        {
            return _my_relocation.r_offset;
        }

        typename ElfType_::RelocationInfo get_info() const
        {
            return _my_relocation.r_info;
        }

        typename ElfType_::RelocationAddend get_addend() const
        {
            return _my_relocation.r_addend;
        }
};

template <typename ElfType_>
struct Relocation
{
    typedef typename ElfType_::Relocation Type;
    typedef RelocationEntry<ElfType_> Entry;
    const static std::string type_name;
};

template <typename ElfType_>
struct RelocationA
{
    typedef typename ElfType_::RelocationA Type;
    typedef RelocationAEntry<ElfType_> Entry;
    const static std::string type_name;
};

template <typename ElfType_, typename Relocation_>
struct RelocationSectionRelocationIteratorTag;

template <typename ElfType_, typename Relocation_>
class RelocationSection :
    public Section<ElfType_>,
    public paludis::ImplementAcceptMethods<Section<ElfType_>, RelocationSection<ElfType_, Relocation_> >,
    private paludis::PrivateImplementationPattern<RelocationSection<ElfType_, Relocation_> >
{
    using paludis::PrivateImplementationPattern<RelocationSection>::_imp;

    public:
        RelocationSection(typename ElfType_::Word, const typename ElfType_::SectionHeader &, std::istream &, bool);
        virtual ~RelocationSection();

        virtual std::string get_type() const
        {
            return Relocation_::type_name;
        }

        typedef RelocationSectionRelocationIteratorTag<ElfType_, Relocation_> RelocationIteratorTag;
        typedef paludis::WrappedForwardIterator<RelocationIteratorTag, const typename Relocation_::Entry> RelocationIterator;
        RelocationIterator relocation_begin() const;
        RelocationIterator relocation_end() const;
};

#endif /*ELFRELOCATIONSECTION_HH_*/
