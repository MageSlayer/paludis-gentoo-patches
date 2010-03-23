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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_ELF_SECTIONS_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_ELF_SECTIONS_HH 1

#include <string>
#include <iosfwd>
#include <paludis/util/simple_visitor.hh>
#include <paludis/util/type_list.hh>

namespace paludis
{
    template <typename ElfType_> class ElfObject;
    template <typename ElfType_> class Section;
    template <typename ElfType_> class StringSection;
    template <typename ElfType_> class DynamicSection;
    template <typename ElfType_> class SymbolSection;
    template <typename ElfType_> class GenericSection;
    template <typename ElfType_, class RelocationType_> class RelocationSection;

    template <typename ElfType_> class Relocation;
    template <typename ElfType_> class RelocationA;

    namespace littlelf_internals
    {
        template <typename ElfType_> class SectionNameResolvingVisitor;
    }

    template <typename ElfType_>
    class Section :
        public virtual paludis::DeclareAbstractAcceptMethods<Section<ElfType_>, typename paludis::MakeTypeList<
            StringSection<ElfType_>,
            DynamicSection<ElfType_>,
            SymbolSection<ElfType_>,
            GenericSection<ElfType_>,
            RelocationSection<ElfType_, Relocation<ElfType_> >,
            RelocationSection<ElfType_, RelocationA<ElfType_> >
        >::Type>
    {
        friend class littlelf_internals::SectionNameResolvingVisitor<ElfType_>;

        private:
            typename ElfType_::Word _index;
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
            Section(typename ElfType_::Word, const typename ElfType_::SectionHeader &);
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
            typename ElfType_::Word get_index() const
            {
                return _index;
            }

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

            std::string description() const;
    };


    template <typename ElfType_>
    class GenericSection :
        public Section<ElfType_>,
        public paludis::ImplementAcceptMethods<Section<ElfType_>, GenericSection<ElfType_> >
    {
        public:
            GenericSection(typename ElfType_::Word, const typename ElfType_::SectionHeader &);
            virtual ~GenericSection();
            virtual std::string get_type() const;
    };

    template <typename ElfType_>
    class StringSection :
        public Section<ElfType_>,
        public paludis::ImplementAcceptMethods<Section<ElfType_>, StringSection<ElfType_> >
    {
        private:
            std::string _stringTable;

        public:
            StringSection(typename ElfType_::Word, const typename ElfType_::SectionHeader &, std::istream &, bool);
            virtual ~StringSection();

            std::string get_string(typename ElfType_::Word) const;
            typename ElfType_::Word get_max_string() const
            {
                return _stringTable.length();
            }

            virtual std::string get_type() const;
    };
}

#endif
