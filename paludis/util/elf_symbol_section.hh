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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_ELF_SYMBOL_SECTION_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_ELF_SYMBOL_SECTION_HH 1

#include <paludis/util/elf_sections.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>

#include <iosfwd>

namespace paludis
{
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
    struct SymbolSectionSymbolIteratorTag;

    template <typename ElfType_>
    class SymbolSection :
        public Section<ElfType_>,
        public paludis::ImplementAcceptMethods<Section<ElfType_>, SymbolSection<ElfType_> >
    {
        private:
            Pimp<SymbolSection> _imp;
            std::string _type;

        public:
            SymbolSection(typename ElfType_::Word, const typename ElfType_::SectionHeader &, std::istream &, bool);
            virtual ~SymbolSection();

            virtual std::string get_type() const
            {
                return _type;
            }

            void resolve_symbols(Section<ElfType_> &);

            typedef SymbolSectionSymbolIteratorTag<ElfType_> SymbolIteratorTag;
            typedef paludis::WrappedForwardIterator<SymbolIteratorTag, const Symbol<ElfType_ > > SymbolIterator;
            SymbolIterator symbol_begin() const;
            SymbolIterator symbol_end() const;
    };
}

#endif
