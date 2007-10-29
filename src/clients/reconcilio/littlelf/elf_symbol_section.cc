
#include "elf_symbol_section.hh"
#include "elf_types.hh"
#include "elf.hh"

#include <src/clients/reconcilio/util/byte_swap.hh>

#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/visitor-impl.hh>

#include <libwrapiter/libwrapiter_forward_iterator-impl.hh>

#include <istream>
#include <vector>
#include <stdexcept>

using namespace paludis;

namespace paludis
{
    template <typename ElfType_>
    struct Implementation<SymbolSection<ElfType_> >
    {
        std::vector<Symbol<ElfType_> > symbols;
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

namespace littlelf_internals
{
    template <typename ElfType_>
    class SymbolStringResolvingVisitor :
        public SectionVisitor<ElfType_>
    {
        using SectionVisitor<ElfType_>::visit;

        private:
            typename std::vector<Symbol<ElfType_> >::iterator _begin, _end;

        public:
            SymbolStringResolvingVisitor(typename std::vector<Symbol<ElfType_> >::iterator begin, typename std::vector<Symbol<ElfType_> >::iterator end) :
                _begin(begin),
                _end(end)
            {
            }

            virtual void visit(StringSection<ElfType_> & section)
            {
                try
                {
                    for (typename std::vector<Symbol<ElfType_> >::iterator i = _begin; i != _end; ++i)
                        i->resolve_symbol(section.get_string(i->get_symbol_index()));
                }
                catch (std::out_of_range &)
                {
                    throw InvalidElfFileError();
                }
            }
    };
}

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
        case STB_NUM:
            _binding = "num";
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
SymbolSection<ElfType_>::SymbolSection(const typename ElfType_::SectionHeader & shdr, std::istream & stream, bool need_byte_swap) :
    Section<ElfType_>(shdr),
    PrivateImplementationPattern<SymbolSection>(new Implementation<SymbolSection>),
    _type("invalid")
{
    if (shdr.sh_type == SHT_DYNSYM)
        _type = "DYNSYM";
    else if (shdr.sh_type == SHT_SYMTAB)
        _type = "SYMTAB";

    if (sizeof(typename ElfType_::Symbol) != shdr.sh_entsize)
        throw InvalidElfFileError();
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
    littlelf_internals::SymbolStringResolvingVisitor<ElfType_> v(_imp->symbols.begin(), _imp->symbols.end());
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

