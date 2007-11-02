#ifndef ELFSYMBOLSECTION_HH_
#define ELFSYMBOLSECTION_HH_

#include "elf_sections.hh"

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>

#include <iosfwd>

namespace littlelf_internals
{
    template <typename ElfType_> class SymbolStringResolvingVisitor;
}

template <typename ElfType_>
class Symbol
{
    friend class littlelf_internals::SymbolStringResolvingVisitor<ElfType_>;

    private:
        typename ElfType_::Symbol _my_symbol;
        std::string _symbol_name;
        std::string _binding, _visibility;

    protected:
        void resolve_symbol(std::string symbol_name)
        {
            _symbol_name = symbol_name;
        }

        typename ElfType_::Word get_symbol_index() const
        {
            return _my_symbol.st_name;
        }

    public:
        Symbol(const typename ElfType_::Symbol &);
        ~Symbol();

        std::string name() const
        {
            return _symbol_name;
        }

        std::string binding() const
        {
            return _binding;
        }

        std::string visibility() const
        {
            return _visibility;
        }
};

template <typename ElfType_>
class SymbolSection :
    public Section<ElfType_>,
    public paludis::AcceptInterfaceVisitsThis<SectionVisitorTypes<ElfType_> , SymbolSection<ElfType_> >,
    private paludis::PrivateImplementationPattern<SymbolSection<ElfType_> >
{
    using paludis::PrivateImplementationPattern<SymbolSection>::_imp;

    private:
        std::string _type;

    public:
        SymbolSection(const typename ElfType_::SectionHeader &, std::istream &, bool);
        virtual ~SymbolSection();

        virtual std::string get_type() const
        {
            return _type;
        }

        void resolve_symbols(Section<ElfType_> &);

        typedef paludis::WrappedForwardIterator<enum SymbolIteratorTag { }, const Symbol<ElfType_ > > SymbolIterator;
        SymbolIterator symbol_begin() const;
        SymbolIterator symbol_end() const;
};

#endif /*ELFSYMBOLSECTION_HH_*/
