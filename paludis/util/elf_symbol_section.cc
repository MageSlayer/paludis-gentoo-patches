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

#include <paludis/util/elf_symbol_section.hh>
#include <paludis/util/elf_sections.hh>
#include <paludis/util/elf_types.hh>
#include <paludis/util/elf_relocation_section.hh>
#include <paludis/util/elf_dynamic_section.hh>
#include <paludis/util/elf.hh>

#include <paludis/util/byte_swap.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/stringify.hh>

#include <istream>
#include <vector>
#include <stdexcept>
#include <algorithm>

using namespace paludis;

namespace paludis
{
    template <typename ElfType_>
    struct Implementation<SymbolSection<ElfType_> >
    {
        std::vector<Symbol<ElfType_> > symbols;
    };

    template <typename ElfType_>
    struct WrappedForwardIteratorTraits<SymbolSectionSymbolIteratorTag<ElfType_> >
    {
        typedef typename std::vector<Symbol<ElfType_> >::const_iterator UnderlyingIterator;
    };
}

namespace
{
    template <typename ElfType_>
    struct ByteSwapSymbol
    {
        static void swap_in_place(typename ElfType_::Symbol & sym)
        {
            sym.st_name  = byte_swap(sym.st_name);
            sym.st_value = byte_swap(sym.st_value);
            sym.st_size  = byte_swap(sym.st_size);
            sym.st_info  = byte_swap(sym.st_info);
            sym.st_other = byte_swap(sym.st_other);
            sym.st_shndx = byte_swap(sym.st_shndx);
        }
    };
}

template <typename ElfType_>
class littlelf_internals::SymbolStringResolvingVisitor
{
    private:
        const SymbolSection<ElfType_> & _sym_section;
        typename std::vector<Symbol<ElfType_> >::iterator _begin, _end;

    public:
        SymbolStringResolvingVisitor(const SymbolSection<ElfType_> & sym_section,
            typename std::vector<Symbol<ElfType_> >::iterator begin,
            typename std::vector<Symbol<ElfType_> >::iterator end) :
            _sym_section(sym_section),
            _begin(begin),
            _end(end)
        {
        }

        void visit(Section<ElfType_> &)
        {
        }

        void visit(StringSection<ElfType_> & string_section)
        {
            for (typename std::vector<Symbol<ElfType_> >::iterator i = _begin; i != _end; ++i)
                try
                {
                    i->resolve_symbol(string_section.get_string(i->get_symbol_index()));
                }
                catch (std::out_of_range &)
                {
                    throw InvalidElfFileError(
                        "symbol " + stringify(i - _begin) + " in " + _sym_section.description() + " has out-of-range string index " +
                        stringify(i->get_symbol_index()) + " for " + string_section.description() +
                        " (max " + stringify(string_section.get_max_string()) + ")");
                }
        }
};

template <typename ElfType_>
Symbol<ElfType_>::Symbol(const typename ElfType_::Symbol & my_symbol) :
    _my_symbol(my_symbol),
    _symbol_name("(unresolved)"),
    _binding("invalid"),
    _visibility("invalid")
{
    switch (ELF64_ST_BIND(my_symbol.st_info))
    {
        case STB_LOCAL:
            _binding = "local";
            break;
        case STB_GLOBAL:
            _binding = "global";
            break;
        case STB_WEAK:
            _binding = "weak";
            break;
        case STB_LOOS:
            _binding = "loos";
            break;
        case STB_HIOS:
            _binding = "hios";
            break;
        case STB_LOPROC:
            _binding = "loproc";
            break;
        case STB_HIPROC:
            _binding = "hiproc";
            break;
    }

    switch (ELF64_ST_VISIBILITY(my_symbol.st_other))
    {
        case STV_DEFAULT:
            _visibility = "default";
            break;
        case STV_INTERNAL:
            _visibility = "internal";
            break;
        case STV_HIDDEN:
            _visibility = "hidden";
            break;
        case STV_PROTECTED:
            _visibility = "protected";
            break;
    }
}

template <typename ElfType_>
Symbol<ElfType_>::~Symbol()
{
}

template <typename ElfType_>
SymbolSection<ElfType_>::SymbolSection(typename ElfType_::Word index, const typename ElfType_::SectionHeader & shdr, std::istream & stream, bool need_byte_swap) :
    Section<ElfType_>(index, shdr),
    PrivateImplementationPattern<SymbolSection>(new Implementation<SymbolSection>),
    _type("invalid")
{
    if (shdr.sh_type == SHT_DYNSYM)
        _type = "DYNSYM";
    else if (shdr.sh_type == SHT_SYMTAB)
        _type = "SYMTAB";

    if (sizeof(typename ElfType_::Symbol) != shdr.sh_entsize)
        throw InvalidElfFileError(
            "bad sh_entsize for " + this->description() + ": got " + stringify(shdr.sh_entsize) + ", expected " +
            stringify(sizeof(typename ElfType_::Symbol)));

    std::vector<typename ElfType_::Symbol> symbols(shdr.sh_size / sizeof(typename ElfType_::Symbol));
    stream.seekg(shdr.sh_offset, std::ios::beg);
    stream.read( reinterpret_cast<char *>(&symbols.front()), shdr.sh_size );
    if (need_byte_swap)
        std::for_each(symbols.begin(), symbols.end(),
                      &ByteSwapSymbol<ElfType_>::swap_in_place);

    for (typename std::vector<typename ElfType_::Symbol>::iterator i = symbols.begin(); i != symbols.end(); ++i)
        _imp->symbols.push_back(Symbol<ElfType_>(*i));
}

template <typename ElfType_>
SymbolSection<ElfType_>::~SymbolSection()
{
}

template <typename ElfType_>
void
SymbolSection<ElfType_>::resolve_symbols(Section<ElfType_> & string_section)
{
    littlelf_internals::SymbolStringResolvingVisitor<ElfType_> v(
        *this, _imp->symbols.begin(), _imp->symbols.end());
    string_section.accept(v);
}

template <typename ElfType_>
typename SymbolSection<ElfType_>::SymbolIterator
SymbolSection<ElfType_>::symbol_begin() const
{
    return SymbolIterator(_imp->symbols.begin());
}

template <typename ElfType_>
typename SymbolSection<ElfType_>::SymbolIterator
SymbolSection<ElfType_>::symbol_end() const
{
    return SymbolIterator(_imp->symbols.end());
}

template class SymbolSection<Elf32Type>;
template class SymbolSection<Elf64Type>;

template class WrappedForwardIterator<SymbolSection<Elf32Type>::SymbolIteratorTag, const Symbol<Elf32Type> >;
template class WrappedForwardIterator<SymbolSection<Elf64Type>::SymbolIteratorTag, const Symbol<Elf64Type> >;

