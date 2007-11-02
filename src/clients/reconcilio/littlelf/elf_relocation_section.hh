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
class RelocationSection :
    public Section<ElfType_>,
    public paludis::AcceptInterfaceVisitsThis<SectionVisitorTypes<ElfType_> , RelocationSection<ElfType_, Relocation_> >,
    private paludis::PrivateImplementationPattern<RelocationSection<ElfType_, Relocation_> >
{
    using paludis::PrivateImplementationPattern<RelocationSection>::_imp;

    public:
        RelocationSection(const typename ElfType_::SectionHeader &, std::istream &, bool);
        virtual ~RelocationSection();

        virtual std::string get_type() const
        {
            return Relocation_::type_name;
        }

        typedef paludis::WrappedForwardIterator<enum RelocationIteratorTag { }, const typename Relocation_::Entry> RelocationIterator;
        RelocationIterator relocation_begin() const;
        RelocationIterator relocation_end() const;
};

#endif /*ELFRELOCATIONSECTION_HH_*/
