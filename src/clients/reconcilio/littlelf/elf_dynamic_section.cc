
#include "elf_dynamic_section.hh"
#include "elf_types.hh"
#include "elf.hh"

#include <src/clients/reconcilio/util/byte_swap.hh>

#include <paludis/util/clone-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/visitor-impl.hh>

#include <libwrapiter/libwrapiter_forward_iterator-impl.hh>

#include <istream>
#include <map>
#include <vector>
#include <stdexcept>

using namespace paludis;

namespace paludis
{
    template <typename ElfType_>
    struct Implementation<DynamicEntries<ElfType_> >
    {
        std::map<typename ElfType_::DynamicTag, tr1::shared_ptr<DynamicEntry<ElfType_> > > available_types;
    };

    template <typename ElfType_>
    struct Implementation<DynamicSection<ElfType_> >
    {
        std::vector<paludis::tr1::shared_ptr<DynamicEntry<ElfType_> > > dynamic_entries;
    };
}

namespace littlelf_internals
{
    template <typename ElfType_>
    class DynEntriesStringResolvingVisitor :
        public DynamicEntriesVisitor<ElfType_>
    {
        using DynamicEntriesVisitor<ElfType_>::visit;

        private:
            const StringSection<ElfType_> & _string_section;

        public:
            DynEntriesStringResolvingVisitor(const StringSection<ElfType_> & string_section) :
                _string_section(string_section)
            {
            }

            virtual void visit(DynamicEntryString<ElfType_> & entry)
            {
                try
                {
                    entry.resolve_string(_string_section.get_string(entry.get_string_index()));
                }
                catch (std::out_of_range &)
                {
                    throw InvalidElfFileError();
                }
            }
    };
}

namespace
{
    template <typename ElfType_>
    struct ByteSwapDynamicEntry
    {
        static void swap_in_place(typename ElfType_::DynamicEntry & entry)
        {
            entry.d_tag      = byte_swap(entry.d_tag);
            entry.d_un.d_val = byte_swap(entry.d_un.d_val);
        }
    };

    template <typename ElfType_>
    class DynamicSectionStringResolvingVisitor :
        public SectionVisitor<ElfType_>
    {
        using SectionVisitor<ElfType_>::visit;

        private:
            typename std::vector<tr1::shared_ptr<DynamicEntry<ElfType_> > >::iterator _begin, _end;

        public:
            DynamicSectionStringResolvingVisitor(
                typename std::vector<tr1::shared_ptr<DynamicEntry<ElfType_> > >::iterator begin,
                typename std::vector<tr1::shared_ptr<DynamicEntry<ElfType_> > >::iterator end) :
                _begin(begin),
                _end(end)
            {
            }

            virtual void visit(StringSection<ElfType_> & section)
            {
                littlelf_internals::DynEntriesStringResolvingVisitor<ElfType_> v(section);
                for(typename std::vector<tr1::shared_ptr<DynamicEntry<ElfType_> > >::iterator i = _begin; i != _end; ++i)
                    (*i)->accept(v);
            }
    };
}

template <typename ElfType_>
DynamicEntry<ElfType_>::DynamicEntry(const std::string & my_tag_name) :
    _tag_name(my_tag_name)
{
}

template <typename ElfType_>
DynamicEntry<ElfType_>::~DynamicEntry()
{
}

template <typename ElfType_>
DynamicEntryUnknown<ElfType_>::DynamicEntryUnknown() :
    DynamicEntry<ElfType_>("unknown")
{
}

template <typename ElfType_>
DynamicEntryUnknown<ElfType_>::~DynamicEntryUnknown()
{
}

template <typename ElfType_>
void
DynamicEntryUnknown<ElfType_>::initialize(const typename ElfType_::DynamicEntry &)
{
}

template <typename ElfType_>
DynamicEntryFlag<ElfType_>::DynamicEntryFlag(const std::string & name) :
    DynamicEntry<ElfType_>(name)
{
}

template <typename ElfType_>
DynamicEntryFlag<ElfType_>::~DynamicEntryFlag()
{
}

template <typename ElfType_>
void
DynamicEntryFlag<ElfType_>::initialize(const typename ElfType_::DynamicEntry &)
{
}

template <typename ElfType_>
DynamicEntryValue<ElfType_>::DynamicEntryValue(const std::string & name) :
    DynamicEntry<ElfType_>(name)
{
}

template <typename ElfType_>
DynamicEntryValue<ElfType_>::~DynamicEntryValue()
{
}

template <typename ElfType_>
void
DynamicEntryValue<ElfType_>::initialize(const typename ElfType_::DynamicEntry & entry)
{
    _value = entry.d_un.d_val;
}

template <typename ElfType_>
DynamicEntryPointer<ElfType_>::DynamicEntryPointer(const std::string & name) :
    DynamicEntry<ElfType_>(name)
{
}

template <typename ElfType_>
DynamicEntryPointer<ElfType_>::~DynamicEntryPointer()
{
}

template <typename ElfType_>
void
DynamicEntryPointer<ElfType_>::initialize(const typename ElfType_::DynamicEntry & entry)
{
    _pointer = entry.d_un.d_ptr;
}

template <typename ElfType_>
DynamicEntryString<ElfType_>::DynamicEntryString(const std::string & name) :
    DynamicEntry<ElfType_>(name),
    _str("")
{
}

template <typename ElfType_>
DynamicEntryString<ElfType_>::~DynamicEntryString()
{
}

template <typename ElfType_>
void
DynamicEntryString<ElfType_>::initialize(const typename ElfType_::DynamicEntry & entry)
{
    _value = entry.d_un.d_val;
}

template <typename ElfType_>
DynamicEntries<ElfType_>::DynamicEntries() :
    PrivateImplementationPattern<DynamicEntries>(new Implementation<DynamicEntries>)
{
    register_type(DT_NEEDED,  make_shared_ptr(new DynamicEntryString<ElfType_>("NEEDED")));
    register_type(DT_RPATH,   make_shared_ptr(new DynamicEntryString<ElfType_>("RPATH")));
    register_type(DT_RUNPATH, make_shared_ptr(new DynamicEntryString<ElfType_>("RUNPATH")));
    register_type(DT_SONAME,  make_shared_ptr(new DynamicEntryString<ElfType_>("SONAME")));
    register_type(DT_TEXTREL, make_shared_ptr(new DynamicEntryFlag<ElfType_>("TEXTREL")));
    register_type(DT_NULL,    make_shared_ptr(new DynamicEntryFlag<ElfType_>("NULL")));
    register_type(DT_SYMTAB,  make_shared_ptr(new DynamicEntryPointer<ElfType_>("SYMTAB")));
    register_type(DT_STRTAB,  make_shared_ptr(new DynamicEntryPointer<ElfType_>("STRTAB")));
}

template <typename ElfType_>
DynamicEntries<ElfType_>::~DynamicEntries()
{
}

template <typename ElfType_>
void
DynamicEntries<ElfType_>::register_type(typename ElfType_::DynamicTag identifier, tr1::shared_ptr<DynamicEntry<ElfType_> > entry)
{
    _imp->available_types[identifier] = entry;
}

template <typename ElfType_>
tr1::shared_ptr<DynamicEntry<ElfType_> >
DynamicEntries<ElfType_>::get_entry(typename ElfType_::DynamicTag tag) const
{
    typename std::map<typename ElfType_::DynamicTag, tr1::shared_ptr<DynamicEntry<ElfType_> > >::const_iterator i;
    if (( i = _imp->available_types.find(tag)) != _imp->available_types.end())
        return i->second->clone();
    return make_shared_ptr(new DynamicEntryUnknown<ElfType_>());
}

template <typename ElfType_>
bool
DynamicEntries<ElfType_>::has_entry(typename ElfType_::DynamicTag identifier) const
{
    return ( _imp->available_types.find(identifier) != _imp->available_types.end() );
}

template <typename ElfType_>
DynamicSection<ElfType_>::DynamicSection(const typename ElfType_::SectionHeader & shdr, std::istream & stream, bool need_byte_swap) :
    Section<ElfType_>(shdr),
    PrivateImplementationPattern<DynamicSection>(new Implementation<DynamicSection>)
{
    if (sizeof(typename ElfType_::DynamicEntry) != shdr.sh_entsize)
        throw InvalidElfFileError();

    stream.seekg(shdr.sh_offset, std::ios::beg);
    std::vector<typename ElfType_::DynamicEntry> tmp_entries(shdr.sh_size / sizeof(typename ElfType_::DynamicEntry));
    stream.read( reinterpret_cast<char *>(&tmp_entries.front()), shdr.sh_size );
    if (need_byte_swap)
        std::for_each(tmp_entries.begin(), tmp_entries.end(),
                      &ByteSwapDynamicEntry<ElfType_>::swap_in_place);

    for (typename std::vector<typename ElfType_::DynamicEntry>::iterator i = tmp_entries.begin(); i != tmp_entries.end(); ++i)
    {
        paludis::tr1::shared_ptr<DynamicEntry<ElfType_> > instance(DynamicEntries<ElfType_>::get_instance()->get_entry(i->d_tag));
        instance->initialize(*i);
        _imp->dynamic_entries.push_back(instance);
    }
}

template <typename ElfType_>
DynamicSection<ElfType_>::~DynamicSection()
{
}

template <typename ElfType_>
std::string
DynamicSection<ElfType_>::get_type() const
{
    static std::string type("DYNAMIC");
    return type;
}

template <typename ElfType_>
void
DynamicSection<ElfType_>::resolve_entry_names(Section<ElfType_> & string_section)
{
    DynamicSectionStringResolvingVisitor<ElfType_> v(_imp->dynamic_entries.begin(), _imp->dynamic_entries.end());
    string_section.accept(v);
}

template <typename ElfType_>
typename DynamicSection<ElfType_>::EntryIterator
DynamicSection<ElfType_>::entry_begin() const
{
    return EntryIterator(indirect_iterator(_imp->dynamic_entries.begin()));
}

template <typename ElfType_>
typename DynamicSection<ElfType_>::EntryIterator
DynamicSection<ElfType_>::entry_end() const
{
    return EntryIterator(indirect_iterator(_imp->dynamic_entries.end()));
}

template class DynamicSection<Elf32Type>;
template class DynamicSection<Elf64Type>;

