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

#include "config.h"

#include <paludis/util/elf.hh>
#include <paludis/util/elf_dynamic_section.hh>
#include <paludis/util/elf_relocation_section.hh>
#include <paludis/util/elf_symbol_section.hh>
#include <paludis/util/elf_types.hh>

#include <paludis/util/byte_swap.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/indirect_iterator-impl.hh>

#include <string>
#include <exception>
#include <stdexcept>
#include <istream>
#include <vector>
#include <algorithm>

using namespace paludis;

namespace paludis
{
    template <typename ElfType_>
    struct Imp<ElfObject<ElfType_> >
    {
        std::vector<std::shared_ptr<Section<ElfType_> > > sections;
    };

    template <typename ElfType_>
    struct WrappedForwardIteratorTraits<ElfObjectSectionIteratorTag<ElfType_> >
    {
        typedef IndirectIterator<typename std::vector<std::shared_ptr<Section<ElfType_> > >::const_iterator> UnderlyingIterator;
    };
}

enum {
    native_byte_order =
#ifdef PALUDIS_BIG_ENDIAN
    ELFDATA2MSB
#else
    ELFDATA2LSB
#endif
};

namespace
{
    template <typename ElfType_>
    struct ByteSwapElfHeader
    {
        static void swap_in_place(typename ElfType_::Header & hdr)
        {
            hdr.e_type      = byte_swap(hdr.e_type);
            hdr.e_machine   = byte_swap(hdr.e_machine);
            hdr.e_version   = byte_swap(hdr.e_version);
            hdr.e_entry     = byte_swap(hdr.e_entry);
            hdr.e_phoff     = byte_swap(hdr.e_phoff);
            hdr.e_shoff     = byte_swap(hdr.e_shoff);
            hdr.e_flags     = byte_swap(hdr.e_flags);
            hdr.e_ehsize    = byte_swap(hdr.e_ehsize);
            hdr.e_phentsize = byte_swap(hdr.e_phentsize);
            hdr.e_phnum     = byte_swap(hdr.e_phnum);
            hdr.e_shentsize = byte_swap(hdr.e_shentsize);
            hdr.e_shnum     = byte_swap(hdr.e_shnum);
            hdr.e_shstrndx  = byte_swap(hdr.e_shstrndx);
        }
    };

    template <typename ElfType_>
    struct ByteSwapSectionHeader
    {
        static void swap_in_place(typename ElfType_::SectionHeader &);
    };

    template <typename ElfType_>
    void
    ByteSwapSectionHeader<ElfType_>::swap_in_place(typename ElfType_::SectionHeader & shdr)
    {
        shdr.sh_name      = byte_swap(shdr.sh_name);
        shdr.sh_type      = byte_swap(shdr.sh_type);
        shdr.sh_flags     = byte_swap(shdr.sh_flags);
        shdr.sh_addr      = byte_swap(shdr.sh_addr);
        shdr.sh_offset    = byte_swap(shdr.sh_offset);
        shdr.sh_size      = byte_swap(shdr.sh_size);
        shdr.sh_link      = byte_swap(shdr.sh_link);
        shdr.sh_info      = byte_swap(shdr.sh_info);
        shdr.sh_addralign = byte_swap(shdr.sh_addralign);
        shdr.sh_entsize   = byte_swap(shdr.sh_entsize);
    }

    class StreamExceptions
    {
        private:
            std::istream & _stream;
            std::ios_base::iostate _old;

        public:
            StreamExceptions(std::istream & stream, std::ios_base::iostate flags) :
                _stream(stream),
                _old(stream.exceptions())
            {
                stream.exceptions(flags);
            }

            ~StreamExceptions()
            {
                try
                {
                    _stream.exceptions(_old);
                }
                catch (const std::ios_base::failure &)
                {
                }
            }
    };

    template <typename ElfType_>
    class StringResolvingVisitor
    {
        private:
            ElfObject<ElfType_> *_elf_object;

        public:
            StringResolvingVisitor(ElfObject<ElfType_> * elf_object) :
                _elf_object(elf_object)
            {
            }

            void visit(Section<ElfType_> &)
            {
            }

            void visit(SymbolSection<ElfType_> & section)
            {
                typename ElfObject<ElfType_>::SectionIterator sec(_elf_object->get_section_by_index(section.get_link_index()));
                if (_elf_object->section_end() == sec)
                    throw InvalidElfFileError(
                        section.description() + " references non-existent section " +
                        stringify(section.get_link_index()) + " in sh_link");
                section.resolve_symbols(*sec);
            }

            void visit(DynamicSection<ElfType_> & section)
            {
                typename ElfObject<ElfType_>::SectionIterator sec(_elf_object->get_section_by_index(section.get_link_index()));
                if (_elf_object->section_end() == sec)
                    throw InvalidElfFileError(
                        section.description() + " references non-existent section " +
                        stringify(section.get_link_index()) + " in sh_link");
                section.resolve_entry_names(*sec);
            }
    };
}

namespace paludis
{
    namespace littlelf_internals
    {
        template <typename ElfType_>
        class SectionNameResolvingVisitor
        {
            private:
                typename ElfObject<ElfType_>::SectionIterator _begin, _end;

            public:
                SectionNameResolvingVisitor(typename ElfObject<ElfType_>::SectionIterator begin, typename ElfObject<ElfType_>::SectionIterator end) :
                    _begin(begin),
                    _end(end)
                {
                }

                void visit(const Section<ElfType_> &)
                {
                }

                void visit(const StringSection<ElfType_> & section)
                {
                    for (typename ElfObject<ElfType_>::SectionIterator i = _begin; i != _end; ++i)
                        try
                        {
                            i->resolve_section_name(section.get_string(i->get_name_index()));
                        }
                        catch (std::out_of_range &)
                        {
                            throw InvalidElfFileError(
                                i->description() + " has out-of-range name index " +
                                stringify(i->get_name_index()) + " for " + section.description() +
                                " (max " + stringify(section.get_max_string()) + ")");
                        }
                }
        };
    }
}

InvalidElfFileError::InvalidElfFileError(const InvalidElfFileError & other) :
    Exception(other)
{
}

InvalidElfFileError::InvalidElfFileError(const std::string & s) noexcept:
    Exception(s)
{
}

template <typename ElfType_>
bool
ElfObject<ElfType_>::is_valid_elf(std::istream & stream)
{
    try
    {
        StreamExceptions exns(stream, std::ios::eofbit | std::ios::failbit | std::ios::badbit);

        stream.seekg(0, std::ios::beg);

        std::vector<char> ident(EI_NIDENT,0);
        stream.read(&ident.front(), EI_NIDENT);

        // Check the magic \177ELF bytes
        if ( ! (    (   ident[EI_MAG0] == ELFMAG0)
                    && (ident[EI_MAG1] == ELFMAG1)
                    && (ident[EI_MAG2] == ELFMAG2)
                    && (ident[EI_MAG3] == ELFMAG3)
                    ) )
            return false;

        // Check the ELF file version
        if (ident[EI_VERSION] != EV_CURRENT)
            return false;

        // Check whether the endianness is valid
        if ((ident[EI_DATA] != ELFDATA2LSB) && (ident[EI_DATA] != ELFDATA2MSB))
            return false;

        return (ident[EI_CLASS] == ElfType_::elf_class);
    }
    catch (const std::ios_base::failure &)
    {
        stream.clear();
        return false;
    }
}

template <typename ElfType_>
ElfObject<ElfType_>::ElfObject(std::istream & stream) :
    _imp()
{
    try
    {
        StreamExceptions exns(stream, std::ios::eofbit | std::ios::failbit | std::ios::badbit);

        stream.seekg(0, std::ios::beg);
        stream.read(reinterpret_cast<char *>(&_hdr), sizeof(typename ElfType_::Header));
        bool need_byte_swap(_hdr.e_ident[EI_DATA] != native_byte_order);
        if (need_byte_swap)
            ByteSwapElfHeader<ElfType_>::swap_in_place(_hdr);

        std::vector<typename ElfType_::SectionHeader> shdrs;
        if (_hdr.e_shoff)
        {
            if (sizeof(typename ElfType_::SectionHeader) != _hdr.e_shentsize)
                throw InvalidElfFileError(
                    "bad e_shentsize: got " + stringify(_hdr.e_shentsize) + ", expected " +
                    stringify(sizeof(typename ElfType_::SectionHeader)));

            stream.seekg(0, std::ios::end);
            typename ElfType_::Word max_shdrs(
                (typename ElfType_::Word(stream.tellg()) - _hdr.e_shoff) / sizeof(typename ElfType_::SectionHeader));
            stream.seekg(_hdr.e_shoff, std::ios::beg);

            if (_hdr.e_shnum)
            {
                if (_hdr.e_shnum > max_shdrs)
                    throw InvalidElfFileError(
                        "file claims to contain " + stringify(_hdr.e_shnum) +
                        " section headers, but is only big enough to contain " + stringify(max_shdrs));
                std::vector<typename ElfType_::SectionHeader> my_shdrs(_hdr.e_shnum);

                stream.read(reinterpret_cast<char *>(&my_shdrs.front()), sizeof(typename ElfType_::SectionHeader) * _hdr.e_shnum);
                if (need_byte_swap)
                    std::for_each(my_shdrs.begin(), my_shdrs.end(),
                                  &ByteSwapSectionHeader<ElfType_>::swap_in_place);
                shdrs.swap(my_shdrs);
            }
            else
            {
                typename ElfType_::SectionHeader first_shdr;
                stream.read(reinterpret_cast<char *>(&first_shdr), sizeof(typename ElfType_::SectionHeader));
                if (need_byte_swap)
                    ByteSwapSectionHeader<ElfType_>::swap_in_place(first_shdr);
                if (0 == first_shdr.sh_size)
                    throw InvalidElfFileError("got non-zero e_shoff and zero e_shnum, but sh_size of the first section is zero");

                if (first_shdr.sh_size > max_shdrs)
                    throw InvalidElfFileError(
                        "file claims to contain " + stringify(first_shdr.sh_size) +
                        " section headers, but is only big enough to contain " + stringify(max_shdrs));
                std::vector<typename ElfType_::SectionHeader> my_shdrs(first_shdr.sh_size);

                my_shdrs[0] = first_shdr;
                stream.read(reinterpret_cast<char *>(&my_shdrs[1]), sizeof(typename ElfType_::SectionHeader) * (first_shdr.sh_size - 1));
                if (need_byte_swap)
                    std::for_each(next(my_shdrs.begin()), my_shdrs.end(),
                                  &ByteSwapSectionHeader<ElfType_>::swap_in_place);
                shdrs.swap(my_shdrs);
            }
        }

        for (typename std::vector<typename ElfType_::SectionHeader>::iterator i = shdrs.begin(); i != shdrs.end(); ++i)
        {
            if (i->sh_type == SHT_STRTAB)
                _imp->sections.push_back(std::make_shared<StringSection<ElfType_> >(_imp->sections.size(), *i, stream, need_byte_swap));
            else if ( (i->sh_type == SHT_SYMTAB) || (i->sh_type == SHT_DYNSYM) )
                _imp->sections.push_back(std::make_shared<SymbolSection<ElfType_> >(_imp->sections.size(), *i, stream, need_byte_swap));
            else if (i->sh_type == SHT_DYNAMIC)
                _imp->sections.push_back(std::make_shared<DynamicSection<ElfType_> >(_imp->sections.size(), *i, stream, need_byte_swap));
            else if (i->sh_type == SHT_REL)
                _imp->sections.push_back(std::make_shared<RelocationSection<ElfType_, Relocation<ElfType_> > >(_imp->sections.size(), *i, stream, need_byte_swap));
            else if (i->sh_type == SHT_RELA)
                _imp->sections.push_back(std::make_shared<RelocationSection<ElfType_, RelocationA<ElfType_> > >(_imp->sections.size(), *i, stream, need_byte_swap));
            else
                _imp->sections.push_back(std::make_shared<GenericSection<ElfType_> >(_imp->sections.size(), *i));
        }

        if (! _hdr.e_shstrndx)
            return;
        typename ElfType_::Half shstrndx(SHN_XINDEX == _hdr.e_shstrndx ? shdrs[0].sh_link : _hdr.e_shstrndx);
        if (_imp->sections.size() <= shstrndx)
            throw InvalidElfFileError(
                "section name table has index " + stringify(shstrndx) +
                ", but only found " + stringify(_imp->sections.size()) + " sections");

        littlelf_internals::SectionNameResolvingVisitor<ElfType_> res(section_begin(), section_end());
        _imp->sections[shstrndx]->accept(res);
    }
    catch (const std::ios_base::failure &)
    {
        throw InvalidElfFileError("file is truncated, or an offset points past the end of the file");
    }
}

template <typename ElfType_>
ElfObject<ElfType_>::~ElfObject()
{
}

template <typename ElfType_>
void
ElfObject<ElfType_>::resolve_all_strings()
{
    StringResolvingVisitor<ElfType_> v(this);
    for (SectionIterator i = section_begin(); i != section_end(); ++i)
        i->accept(v);
}

template <typename ElfType_>
typename ElfObject<ElfType_>::SectionIterator
ElfObject<ElfType_>::section_begin() const
{
    return SectionIterator(indirect_iterator(_imp->sections.begin()));
}

template <typename ElfType_>
typename ElfObject<ElfType_>::SectionIterator
ElfObject<ElfType_>::section_end() const
{
    return SectionIterator(indirect_iterator(_imp->sections.end()));
}

template <typename ElfType_>
typename ElfObject<ElfType_>::SectionIterator
ElfObject<ElfType_>::get_section_by_index(unsigned int index) const
{
    if (index >= _imp->sections.size())
        return SectionIterator(indirect_iterator(_imp->sections.end()));
    return SectionIterator(indirect_iterator(_imp->sections.begin() + index));
}

namespace paludis
{
    template class PALUDIS_VISIBLE ElfObject<Elf32Type>;
    template class PALUDIS_VISIBLE ElfObject<Elf64Type>;

    template class PALUDIS_VISIBLE WrappedForwardIterator<ElfObject<Elf32Type>::SectionIteratorTag, Section<Elf32Type> >;
    template class PALUDIS_VISIBLE WrappedForwardIterator<ElfObject<Elf64Type>::SectionIteratorTag, Section<Elf64Type> >;
}

