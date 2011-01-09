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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_ELF_DYNAMIC_SECTION_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_ELF_DYNAMIC_SECTION_HH 1

#include <paludis/util/elf_sections.hh>
#include <paludis/util/clone.hh>
#include <paludis/util/singleton.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <paludis/util/simple_visitor.hh>
#include <paludis/util/type_list.hh>
#include <memory>
#include <string>
#include <iosfwd>

namespace paludis
{
    template <typename ElfType_> class DynamicEntry;
    template <typename ElfType_> class DynamicEntryUnknown;
    template <typename ElfType_> class DynamicEntryValue;
    template <typename ElfType_> class DynamicEntryPointer;
    template <typename ElfType_> class DynamicEntryString;
    template <typename ElfType_> class DynamicEntryFlag;

    template <typename ElfType_>
    class DynamicEntry :
        public virtual DeclareAbstractAcceptMethods<DynamicEntry<ElfType_>, typename MakeTypeList<
            DynamicEntryUnknown<ElfType_>,
            DynamicEntryValue<ElfType_>,
            DynamicEntryPointer<ElfType_>,
            DynamicEntryString<ElfType_>,
            DynamicEntryFlag<ElfType_>
        >::Type>,
        public virtual Cloneable<DynamicEntry<ElfType_> >
    {
        private:
            std::string _tag_name;
            typename ElfType_::Word _index;

        public:
            DynamicEntry(const std::string &);
            ~DynamicEntry();
            virtual void initialize(typename ElfType_::Word, const typename ElfType_::DynamicEntry & entry);

            std::string tag_name() const
            {
                return _tag_name;
            }

            std::string description() const;
    };

    template <typename ElfType_>
    class DynamicEntryUnknown :
        public virtual DynamicEntry<ElfType_>,
        public ImplementAcceptMethods<DynamicEntry<ElfType_>, DynamicEntryUnknown<ElfType_> >,
        public CloneUsingThis<DynamicEntry<ElfType_>, DynamicEntryUnknown<ElfType_> >
    {
        public:
            DynamicEntryUnknown();
            virtual ~DynamicEntryUnknown();
    };

    template <typename ElfType_>
    class DynamicEntryFlag :
        public virtual DynamicEntry<ElfType_>,
        public ImplementAcceptMethods<DynamicEntry<ElfType_>, DynamicEntryFlag<ElfType_> >,
        public CloneUsingThis<DynamicEntry<ElfType_>, DynamicEntryFlag<ElfType_> >
    {
        public:
            DynamicEntryFlag(const std::string &);
            ~DynamicEntryFlag();
    };

    template <typename ElfType_>
    class DynamicEntryValue :
        public virtual DynamicEntry<ElfType_>,
        public ImplementAcceptMethods<DynamicEntry<ElfType_>, DynamicEntryValue<ElfType_> >,
        public CloneUsingThis<DynamicEntry<ElfType_>, DynamicEntryValue<ElfType_> >
    {
        private:
            typename ElfType_::DynamicValue _value;

        public:
            DynamicEntryValue(const std::string &);
            virtual ~DynamicEntryValue();
            virtual void initialize(typename ElfType_::Word, const typename ElfType_::DynamicEntry & entry);

            typename ElfType_::DynamicValue operator() () const
            {
                return _value;
            }
    };

    template <typename ElfType_>
    class DynamicEntryPointer :
        public virtual DynamicEntry<ElfType_>,
        public ImplementAcceptMethods<DynamicEntry<ElfType_>, DynamicEntryPointer<ElfType_> >,
        public CloneUsingThis<DynamicEntry<ElfType_>, DynamicEntryPointer<ElfType_> >
    {
        private:
            typename ElfType_::DynamicPointer _pointer;

        public:
            DynamicEntryPointer(const std::string &);
            virtual ~DynamicEntryPointer();
            virtual void initialize(typename ElfType_::Word, const typename ElfType_::DynamicEntry &);

            typename ElfType_::DynamicPointer operator() () const
            {
                return _pointer;
            }
    };

    namespace littlelf_internals
    {
        template <typename ElfType_> class DynEntriesStringResolvingVisitor;
    }

    template <typename ElfType_>
    class DynamicEntryString :
        public virtual DynamicEntry<ElfType_>,
        public ImplementAcceptMethods<DynamicEntry<ElfType_>, DynamicEntryString<ElfType_> >,
        public CloneUsingThis<DynamicEntry<ElfType_>, DynamicEntryString<ElfType_> >
    {
        friend class littlelf_internals::DynEntriesStringResolvingVisitor<ElfType_>;

        private:
            typename ElfType_::DynamicValue _value;
            std::string _str;

        public:
            DynamicEntryString(const std::string &);
            virtual ~DynamicEntryString();
            virtual void initialize(typename ElfType_::Word, const typename ElfType_::DynamicEntry &);

            std::string operator() () const
            {
                return _str;
            }

        private:
            void resolve_string(std::string str)
            {
                _str = str;
            }

            typename ElfType_::DynamicValue get_string_index() const
            {
                return _value;
            }
    };

    template <typename ElfType_>
    class DynamicEntries :
        public Singleton<DynamicEntries<ElfType_> >
    {
        friend class Singleton<DynamicEntries>;

        private:
            Pimp<DynamicEntries> _imp;

        public:
            void register_type(typename ElfType_::DynamicTag, std::shared_ptr<DynamicEntry<ElfType_> >);

            std::shared_ptr<DynamicEntry<ElfType_> > get_entry(typename ElfType_::DynamicTag) const;
            bool has_entry(typename ElfType_::DynamicTag) const;

        private:
            DynamicEntries();
            ~DynamicEntries();
    };

    template <typename ElfType_>
    struct DynamicSectionEntryIteratorTag;

    template <typename ElfType_>
    class PALUDIS_VISIBLE DynamicSection :
        public Section<ElfType_>,
        public ImplementAcceptMethods<Section<ElfType_>, DynamicSection<ElfType_> >
    {
        private:
            Pimp<DynamicSection> _imp;

        public:
            DynamicSection(typename ElfType_::Word, const typename ElfType_::SectionHeader &, std::istream &, bool);
            virtual ~DynamicSection();

            virtual std::string get_type() const;

            void resolve_entry_names(Section<ElfType_> &);

            typedef DynamicSectionEntryIteratorTag<ElfType_> EntryIteratorTag;
            typedef WrappedForwardIterator<EntryIteratorTag, DynamicEntry<ElfType_> > EntryIterator;
            EntryIterator entry_begin() const;
            EntryIterator entry_end() const;
    };
}

#endif
