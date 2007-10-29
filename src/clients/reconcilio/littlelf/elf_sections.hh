#ifndef ELFSECTIONS_HH_
#define ELFSECTIONS_HH_

#include <string>
#include <iosfwd>

#include <paludis/util/visitor.hh>

template <typename ElfType_> class ElfObject;
template <typename ElfType_> class Section;
template <typename ElfType_> class StringSection;
template <typename ElfType_> class DynamicSection;
template <typename ElfType_> class SymbolSection;
template <typename ElfType_> class GenericSection;
template <typename ElfType_, class RelocationType_> class RelocationSection;

template <typename ElfType_> class Relocation;
template <typename ElfType_> class RelocationA;

template <typename ElfType_>
struct SectionVisitorTypes :
    paludis::VisitorTypes<
        SectionVisitorTypes<ElfType_>,
        Section<ElfType_>,
        StringSection<ElfType_>,
        DynamicSection<ElfType_>,
        SymbolSection<ElfType_>,
        GenericSection<ElfType_>,
        RelocationSection<ElfType_, Relocation<ElfType_> >,
        RelocationSection<ElfType_, RelocationA<ElfType_> >
        >
{
};

template <typename ElfType_>
class ConstSectionVisitor :
    public paludis::ConstVisitor<SectionVisitorTypes<ElfType_> >
{
    public:
        virtual void visit(const StringSection<ElfType_> &)  {}
        virtual void visit(const SymbolSection<ElfType_> &)  {}
        virtual void visit(const DynamicSection<ElfType_> &) {}
        virtual void visit(const GenericSection<ElfType_> &) {}
        virtual void visit(const RelocationSection<ElfType_, Relocation<ElfType_> > &)  {}
        virtual void visit(const RelocationSection<ElfType_, RelocationA<ElfType_> > &) {}
};

template <typename ElfType_>
class SectionVisitor :
    public paludis::Visitor<SectionVisitorTypes<ElfType_> >
{
    public:
        virtual void visit(StringSection<ElfType_> &)  {}
        virtual void visit(SymbolSection<ElfType_> &)  {}
        virtual void visit(DynamicSection<ElfType_> &) {}
        virtual void visit(GenericSection<ElfType_> &) {}
        virtual void visit(RelocationSection<ElfType_, Relocation<ElfType_> > &)  {}
        virtual void visit(RelocationSection<ElfType_, RelocationA<ElfType_> > &) {}
};

namespace littlelf_internals
{
    template <typename ElfType_> class SectionNameResolvingVisitor;
}

template <typename ElfType_>
class Section :
    public virtual paludis::AcceptInterface<SectionVisitorTypes<ElfType_> >
{
    friend class littlelf_internals::SectionNameResolvingVisitor<ElfType_>;

    private:
        typename ElfType_::SectionHeader _shdr;
        std::string _name;

    protected:
        void resolve_section_name(std::string name)
        {
            _name = name;
        }

        typename ElfType_::Word get_name_index() const
        {
            return _shdr.sh_name;
        }

    public:
        Section(const typename ElfType_::SectionHeader &);
        virtual ~Section();

        virtual typename ElfType_::Offset get_data_offset() const
        {
            return _shdr.sh_offset;
        }

        virtual std::string get_name() const
        {
            return _name;
        }
        virtual std::string get_type() const = 0;

        typename ElfType_::Word get_link_index() const
        {
            return _shdr.sh_link;
        }

        typename ElfType_::Address get_virtual_address() const
        {
            return _shdr.sh_addr;
        }

        typename ElfType_::SectionSize get_size() const
        {
            return _shdr.sh_size;
        }
};


template <typename ElfType_>
class GenericSection :
    public Section<ElfType_>,
    public paludis::AcceptInterfaceVisitsThis<SectionVisitorTypes<ElfType_> , GenericSection<ElfType_> >
{
    public:
        GenericSection(const typename ElfType_::SectionHeader &);
        virtual ~GenericSection();
        virtual std::string get_type() const;
};

template <typename ElfType_>
class StringSection :
    public Section<ElfType_>,
    public paludis::AcceptInterfaceVisitsThis<SectionVisitorTypes<ElfType_> , StringSection<ElfType_> >
{
    private:
        std::string _stringTable;

    public:
        StringSection(const typename ElfType_::SectionHeader &, std::istream &, bool);
        virtual ~StringSection();

        std::string get_string(typename ElfType_::Word) const;

        virtual std::string get_type() const;
};

#endif /*ELFSECTIONS_HH_*/
