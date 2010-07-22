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

#include <paludis/util/elf_relocation_section.hh>
#include <paludis/util/elf_types.hh>
#include <paludis/util/elf.hh>
#include <paludis/util/byte_swap.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/stringify.hh>
#include <algorithm>
#include <istream>
#include <vector>

using namespace paludis;

namespace paludis
{
    template <typename ElfType_, typename Relocation_>
    struct Implementation<RelocationSection<ElfType_, Relocation_> >
    {
        std::vector<typename Relocation_::Entry> relocations;
    };

    template <typename ElfType_, typename Relocation_>
    struct WrappedForwardIteratorTraits<RelocationSectionRelocationIteratorTag<ElfType_, Relocation_> >
    {
        typedef typename std::vector<typename Relocation_::Entry>::const_iterator UnderlyingIterator;
    };
}

namespace
{
    template <typename, typename> struct ByteSwapRelocation;

    template <typename ElfType_>
    struct ByteSwapRelocation<ElfType_, Relocation<ElfType_> >
    {
        static void swap_in_place(typename ElfType_::Relocation & rel)
        {
            rel.r_offset = byte_swap(rel.r_offset);
            rel.r_info   = byte_swap(rel.r_info);
        }
    };

    template <typename ElfType_>
    struct ByteSwapRelocation<ElfType_, RelocationA<ElfType_> >
    {
        static void swap_in_place(typename ElfType_::RelocationA & rela)
        {
            rela.r_offset = byte_swap(rela.r_offset);
            rela.r_info   = byte_swap(rela.r_info);
            rela.r_addend = byte_swap(rela.r_addend);
        }
    };
}

template <typename ElfType_>
RelocationEntry<ElfType_>::RelocationEntry(const typename ElfType_::Relocation & my_relocation) :
    _my_relocation(my_relocation)
{
}

template <typename ElfType_>
RelocationEntry<ElfType_>::~RelocationEntry()
{
}

template <typename ElfType_>
RelocationAEntry<ElfType_>::RelocationAEntry(const typename ElfType_::RelocationA & my_relocation) :
    _my_relocation(my_relocation)
{
}

template <typename ElfType_>
RelocationAEntry<ElfType_>::~RelocationAEntry()
{
}

template <typename ElfType_> const std::string Relocation<ElfType_>::type_name = "REL";
template <typename ElfType_> const std::string RelocationA<ElfType_>::type_name = "RELA";

template <typename ElfType_, typename Relocation_>
RelocationSection<ElfType_, Relocation_>::RelocationSection(
    typename ElfType_::Word index, const typename ElfType_::SectionHeader & shdr, std::istream & stream, bool need_byte_swap) :
    Section<ElfType_>(index, shdr),
    PrivateImplementationPattern<RelocationSection>()
{
    if (sizeof(typename Relocation_::Type) != shdr.sh_entsize)
        throw InvalidElfFileError(
            "bad sh_entsize for " + this->description() + ": got " + stringify(shdr.sh_entsize) + ", expected " +
            stringify(sizeof(typename Relocation_::Type)));

    std::vector<typename Relocation_::Type> relocations(shdr.sh_size / sizeof(typename Relocation_::Type));
    stream.seekg(shdr.sh_offset, std::ios::beg);
    stream.read(reinterpret_cast<char *>(&relocations.front()), shdr.sh_size);
    if (need_byte_swap)
        std::for_each(relocations.begin(), relocations.end(),
                      &ByteSwapRelocation<ElfType_, Relocation_>::swap_in_place);

    for (typename std::vector<typename Relocation_::Type>::iterator i = relocations.begin(); i != relocations.end(); ++i)
        _imp->relocations.push_back(typename Relocation_::Entry(*i));
}

template <typename ElfType_, typename Relocation_>
RelocationSection<ElfType_, Relocation_>::~RelocationSection()
{
}

template <typename ElfType_, typename Relocation_>
typename RelocationSection<ElfType_, Relocation_>::RelocationIterator
RelocationSection<ElfType_, Relocation_>::relocation_begin() const
{
    return RelocationIterator(_imp->relocations.begin());
}

template <typename ElfType_, typename Relocation_>
typename RelocationSection<ElfType_, Relocation_>::RelocationIterator
RelocationSection<ElfType_, Relocation_>::relocation_end() const
{
    return RelocationIterator(_imp->relocations.end());
}

template class RelocationSection<Elf32Type, Relocation<Elf32Type> >;
template class WrappedForwardIterator<RelocationSection<Elf32Type, Relocation<Elf32Type> >::RelocationIteratorTag,
         const Relocation<Elf32Type>::Entry>;

template class RelocationSection<Elf32Type, RelocationA<Elf32Type> >;
template class WrappedForwardIterator<RelocationSection<Elf32Type, RelocationA<Elf32Type> >::RelocationIteratorTag,
         const RelocationA<Elf32Type>::Entry>;

template class RelocationSection<Elf64Type, Relocation<Elf64Type> >;
template class WrappedForwardIterator<RelocationSection<Elf64Type, Relocation<Elf64Type> >::RelocationIteratorTag,
         const Relocation<Elf64Type>::Entry>;

template class RelocationSection<Elf64Type, RelocationA<Elf64Type> >;
template class WrappedForwardIterator<RelocationSection<Elf64Type, RelocationA<Elf64Type> >::RelocationIteratorTag,
         const RelocationA<Elf64Type>::Entry>;

