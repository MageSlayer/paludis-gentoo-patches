
#include "elf.hh"
#include "elf_dynamic_section.hh"
#include "elf_relocation_section.hh"
#include "elf_symbol_section.hh"
#include "elf_types.hh"

#include <paludis/util/iterator.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/visitor-impl.hh>

#include <libwrapiter/libwrapiter_forward_iterator-impl.hh>

#include <string>
#include <exception>
#include <istream>
#include <vector>

using namespace paludis;

namespace paludis
{
    template <typename ElfType_>
    struct Implementation<ElfObject<ElfType_> >
    {
        std::vector<paludis::tr1::shared_ptr<Section<ElfType_> > > sections;
    };
}

namespace
{
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
                _stream.exceptions(_old);
            }
    };

    template <typename ElfType_>
    class StringResolvingVisitor :
        public SectionVisitor<ElfType_>
    {
        using SectionVisitor<ElfType_>::visit;

        private:
            ElfObject<ElfType_> *_elf_object;

        public:
            StringResolvingVisitor(ElfObject<ElfType_> * elf_object) :
                _elf_object(elf_object)
            {
            }

            virtual void visit(SymbolSection<ElfType_> & section)
            {
                section.resolve_symbols(*_elf_object->get_section_by_index(section.get_link_index()));
            }

            virtual void visit(DynamicSection<ElfType_> & section)
            {
                section.resolve_entry_names(*_elf_object->get_section_by_index(section.get_link_index()));
            }
    };
}

namespace littlelf_internals
{
    template <typename ElfType_>
    class SectionNameResolvingVisitor :
        public ConstSectionVisitor<ElfType_>
    {
        using ConstSectionVisitor<ElfType_>::visit;

        private:
            typename ElfObject<ElfType_>::SectionIterator _begin, _end;

        public:
            SectionNameResolvingVisitor(typename ElfObject<ElfType_>::SectionIterator begin, typename ElfObject<ElfType_>::SectionIterator end) :
                _begin(begin),
                _end(end)
            {
            }

            virtual void visit(const StringSection<ElfType_> & section)
            {
                for (typename ElfObject<ElfType_>::SectionIterator i = _begin; i != _end; ++i)
                    i->resolve_section_name(section.get_string(i->get_name_index()));
            }
    };
}

InvalidElfFileError::InvalidElfFileError(const InvalidElfFileError & other) :
    Exception(other)
{
}

InvalidElfFileError::InvalidElfFileError() throw ():
    Exception("Invalid ELF file")
{
}

template <typename ElfType_>
bool
ElfObject<ElfType_>::is_valid_elf(std::istream & stream)
{
    StreamExceptions exns(stream, std::ios::eofbit | std::ios::failbit | std::ios::badbit);

    try
    {
        stream.seekg(0, std::ios::beg);
        if (stream.fail())
            return false;

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
        return false;
    }
}

template <typename ElfType_>
ElfObject<ElfType_>::ElfObject(std::istream & stream) :
    PrivateImplementationPattern<ElfObject>(new Implementation<ElfObject>)
{
    StreamExceptions exns(stream, std::ios::eofbit | std::ios::failbit | std::ios::badbit);

    try
    {
        stream.seekg(0, std::ios::beg);
        stream.read(reinterpret_cast<char *>(&_hdr), sizeof(typename ElfType_::Header));
        stream.seekg(_hdr.e_shoff, std::ios::beg);
        // The standard guarantees that there's at least one section
        std::vector<typename ElfType_::SectionHeader> shdrs(_hdr.e_shnum);
        stream.read(reinterpret_cast<char *>(&shdrs.front()), sizeof(typename ElfType_::SectionHeader) * _hdr.e_shnum);

        for (typename std::vector<typename ElfType_::SectionHeader>::iterator i = shdrs.begin(); i != shdrs.end(); ++i)
        {
            if (i->sh_type == SHT_STRTAB)
                _imp->sections.push_back(make_shared_ptr(new StringSection<ElfType_>(*i, stream)));
            else if ( (i->sh_type == SHT_SYMTAB) || (i->sh_type == SHT_DYNSYM) )
                _imp->sections.push_back(make_shared_ptr(new SymbolSection<ElfType_>(*i, stream)));
            else if (i->sh_type == SHT_DYNAMIC)
                _imp->sections.push_back(make_shared_ptr(new DynamicSection<ElfType_>(*i, stream)));
            else if (i->sh_type == SHT_REL)
                _imp->sections.push_back(make_shared_ptr(new RelocationSection<ElfType_, Relocation<ElfType_> >(*i, stream)));
            else if (i->sh_type == SHT_RELA)
                _imp->sections.push_back(make_shared_ptr(new RelocationSection<ElfType_, RelocationA<ElfType_> >(*i, stream)));
            else
                _imp->sections.push_back(make_shared_ptr(new GenericSection<ElfType_>(*i)));
        }

        if (! _hdr.e_shstrndx)
            return;

        littlelf_internals::SectionNameResolvingVisitor<ElfType_> res(section_begin(), section_end());
        _imp->sections[_hdr.e_shstrndx]->accept(res);
    }
    catch (const std::ios_base::failure &)
    {
        throw InvalidElfFileError();
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

template class ElfObject<Elf32Type>;
template class ElfObject<Elf64Type>;

