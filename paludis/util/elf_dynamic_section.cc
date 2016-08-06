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

#include <paludis/util/elf_dynamic_section.hh>
#include <paludis/util/elf_sections.hh>
#include <paludis/util/elf_types.hh>
#include <paludis/util/elf_relocation_section.hh>
#include <paludis/util/elf_symbol_section.hh>
#include <paludis/util/elf.hh>

#include <paludis/util/byte_swap.hh>
#include <paludis/util/clone-impl.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>

#include <istream>
#include <map>
#include <vector>
#include <stdexcept>
#include <algorithm>

using namespace paludis;

namespace paludis
{
    template <typename ElfType_>
    struct Imp<DynamicEntries<ElfType_> >
    {
        std::map<typename ElfType_::DynamicTag, std::shared_ptr<DynamicEntry<ElfType_> > > available_types;
    };

    template <typename ElfType_>
    struct Imp<DynamicSection<ElfType_> >
    {
        std::vector<std::shared_ptr<DynamicEntry<ElfType_> > > dynamic_entries;
    };

    template <typename ElfType_>
    struct WrappedForwardIteratorTraits<DynamicSectionEntryIteratorTag<ElfType_> >
    {
        typedef IndirectIterator<typename std::vector<std::shared_ptr<DynamicEntry<ElfType_> > >::const_iterator> UnderlyingIterator;
    };
}

namespace paludis
{
    namespace littlelf_internals
    {
        template <typename ElfType_>
        class DynEntriesStringResolvingVisitor
        {
            private:
                const DynamicSection<ElfType_> & _dyn_section;
                const StringSection<ElfType_> & _string_section;

            public:
                DynEntriesStringResolvingVisitor(
                    const DynamicSection<ElfType_> & dyn_section,
                    const StringSection<ElfType_> & string_section) :
                    _dyn_section(dyn_section),
                    _string_section(string_section)
                {
                }

                void visit(DynamicEntry<ElfType_> &)
                {
                }

                void visit(DynamicEntryString<ElfType_> & entry)
                {
                    try
                    {
                        entry.resolve_string(_string_section.get_string(entry.get_string_index()));
                    }
                    catch (std::out_of_range &)
                    {
                        throw InvalidElfFileError(
                            entry.description() + " in " + _dyn_section.description() + " has out-of-range string index " +
                            stringify(entry.get_string_index()) + " for " + _string_section.description() +
                            " (max " + stringify(_string_section.get_max_string()) + ")");
                    }
                }
        };
    }
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
    class DynamicSectionStringResolvingVisitor
    {
        private:
            const DynamicSection<ElfType_> & _dyn_section;
            typename std::vector<std::shared_ptr<DynamicEntry<ElfType_> > >::iterator _begin, _end;

        public:
            DynamicSectionStringResolvingVisitor(const DynamicSection<ElfType_> & dyn_section,
                typename std::vector<std::shared_ptr<DynamicEntry<ElfType_> > >::iterator begin,
                typename std::vector<std::shared_ptr<DynamicEntry<ElfType_> > >::iterator end) :
                _dyn_section(dyn_section),
                _begin(begin),
                _end(end)

            {
            }

            void visit(Section<ElfType_> &)
            {
            }

            void visit(StringSection<ElfType_> & section)
            {
                littlelf_internals::DynEntriesStringResolvingVisitor<ElfType_> v(_dyn_section, section);
                for(typename std::vector<std::shared_ptr<DynamicEntry<ElfType_> > >::iterator i = _begin; i != _end; ++i)
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
void
DynamicEntry<ElfType_>::initialize(typename ElfType_::Word index, const typename ElfType_::DynamicEntry &)
{
    _index = index;
}

template <typename ElfType_>
std::string
DynamicEntry<ElfType_>::description() const
{
    return tag_name() + " dynamic entry " + stringify(_index);
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
DynamicEntryFlag<ElfType_>::DynamicEntryFlag(const std::string & name) :
    DynamicEntry<ElfType_>(name)
{
}

template <typename ElfType_>
DynamicEntryFlag<ElfType_>::~DynamicEntryFlag()
{
}

template <typename ElfType_>
DynamicEntryValue<ElfType_>::DynamicEntryValue(const std::string & name) :
    DynamicEntry<ElfType_>(name)
{
}

template <typename ElfType_>
DynamicEntryValue<ElfType_>::~DynamicEntryValue() = default;

template <typename ElfType_>
void
DynamicEntryValue<ElfType_>::initialize(typename ElfType_::Word index, const typename ElfType_::DynamicEntry & entry)
{
    DynamicEntry<ElfType_>::initialize(index, entry);
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
DynamicEntryPointer<ElfType_>::initialize(typename ElfType_::Word index, const typename ElfType_::DynamicEntry & entry)
{
    DynamicEntry<ElfType_>::initialize(index, entry);
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
DynamicEntryString<ElfType_>::initialize(typename ElfType_::Word index, const typename ElfType_::DynamicEntry & entry)
{
    DynamicEntry<ElfType_>::initialize(index, entry);
    _value = entry.d_un.d_val;
}

template <typename ElfType_>
DynamicEntries<ElfType_>::DynamicEntries() :
    _imp()
{
    register_type(DT_NEEDED,  std::make_shared<DynamicEntryString<ElfType_> >("NEEDED"));
    register_type(DT_RPATH,   std::make_shared<DynamicEntryString<ElfType_> >("RPATH"));
    register_type(DT_RUNPATH, std::make_shared<DynamicEntryString<ElfType_> >("RUNPATH"));
    register_type(DT_SONAME,  std::make_shared<DynamicEntryString<ElfType_> >("SONAME"));
    register_type(DT_TEXTREL, std::make_shared<DynamicEntryFlag<ElfType_> >("TEXTREL"));
    register_type(DT_NULL,    std::make_shared<DynamicEntryFlag<ElfType_> >("NULL"));
    register_type(DT_SYMTAB,  std::make_shared<DynamicEntryPointer<ElfType_> >("SYMTAB"));
    register_type(DT_STRTAB,  std::make_shared<DynamicEntryPointer<ElfType_> >("STRTAB"));
}

template <typename ElfType_>
DynamicEntries<ElfType_>::~DynamicEntries()
{
}

template <typename ElfType_>
void
DynamicEntries<ElfType_>::register_type(typename ElfType_::DynamicTag identifier, std::shared_ptr<DynamicEntry<ElfType_> > entry)
{
    _imp->available_types[identifier] = entry;
}

template <typename ElfType_>
std::shared_ptr<DynamicEntry<ElfType_> >
DynamicEntries<ElfType_>::get_entry(typename ElfType_::DynamicTag tag) const
{
    typename std::map<typename ElfType_::DynamicTag, std::shared_ptr<DynamicEntry<ElfType_> > >::const_iterator i;
    if (( i = _imp->available_types.find(tag)) != _imp->available_types.end())
        return i->second->clone();
    return std::make_shared<DynamicEntryUnknown<ElfType_> >();
}

template <typename ElfType_>
bool
DynamicEntries<ElfType_>::has_entry(typename ElfType_::DynamicTag identifier) const
{
    return ( _imp->available_types.find(identifier) != _imp->available_types.end() );
}

template <typename ElfType_>
DynamicSection<ElfType_>::DynamicSection(typename ElfType_::Word index, const typename ElfType_::SectionHeader & shdr, std::istream & stream, bool need_byte_swap) :
    Section<ElfType_>(index, shdr),
    _imp()
{
    if (sizeof(typename ElfType_::DynamicEntry) != shdr.sh_entsize)
        throw InvalidElfFileError(
            "bad sh_entsize for " + this->description() + ": got " + stringify(shdr.sh_entsize) + ", expected " +
            stringify(sizeof(typename ElfType_::DynamicEntry)));

    stream.seekg(shdr.sh_offset, std::ios::beg);
    std::vector<typename ElfType_::DynamicEntry> tmp_entries(shdr.sh_size / sizeof(typename ElfType_::DynamicEntry));
    stream.read( reinterpret_cast<char *>(&tmp_entries.front()), shdr.sh_size );
    if (need_byte_swap)
        std::for_each(tmp_entries.begin(), tmp_entries.end(),
                      &ByteSwapDynamicEntry<ElfType_>::swap_in_place);

    for (typename std::vector<typename ElfType_::DynamicEntry>::iterator i = tmp_entries.begin(); i != tmp_entries.end(); ++i)
    {
        std::shared_ptr<DynamicEntry<ElfType_> > instance(DynamicEntries<ElfType_>::get_instance()->get_entry(i->d_tag));
        instance->initialize(_imp->dynamic_entries.size(), *i);
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
    DynamicSectionStringResolvingVisitor<ElfType_> v(
        *this, _imp->dynamic_entries.begin(), _imp->dynamic_entries.end());
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

namespace paludis
{
    template class PALUDIS_VISIBLE DynamicSection<Elf32Type>;
    template class PALUDIS_VISIBLE DynamicSection<Elf64Type>;

    template class PALUDIS_VISIBLE WrappedForwardIterator<DynamicSection<Elf32Type>::EntryIteratorTag, DynamicEntry<Elf32Type> >;
    template class PALUDIS_VISIBLE WrappedForwardIterator<DynamicSection<Elf64Type>::EntryIteratorTag, DynamicEntry<Elf64Type> >;
}

