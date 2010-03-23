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

#include <paludis/util/elf_sections.hh>
#include <paludis/util/elf_types.hh>
#include <paludis/util/stringify.hh>

#include <istream>
#include <algorithm>

using namespace paludis;

template <typename ElfType_>
Section<ElfType_>::Section(typename ElfType_::Word index, const typename ElfType_::SectionHeader & shdr) :
    _index(index),
    _shdr(shdr),
    _name("")
{
}

template <typename ElfType_>
Section<ElfType_>::~Section()
{
}

template <typename ElfType_>
std::string
Section<ElfType_>::description() const
{
    return get_type() + " section " + (get_name().empty() ? "[name not yet resolved]" : get_name())
        + " " + stringify(get_index());
}

template <typename ElfType_>
GenericSection<ElfType_>::GenericSection(typename ElfType_::Word index, const typename ElfType_::SectionHeader & shdr) :
    Section<ElfType_>(index, shdr)
{
}

template <typename ElfType_>
GenericSection<ElfType_>::~GenericSection()
{
}

template <typename ElfType_>
std::string
GenericSection<ElfType_>::get_type() const
{
    static std::string type("generic");
    return type;
}

template <typename ElfType_>
StringSection<ElfType_>::StringSection(typename ElfType_::Word index, const typename ElfType_::SectionHeader & shdr, std::istream & stream, bool) :
    Section<ElfType_>(index, shdr),
    _stringTable(shdr.sh_size, ' ')
{
    std::string tmp_table(shdr.sh_size, '\0');
    stream.seekg(shdr.sh_offset, std::ios::beg);
    stream.read(&tmp_table[0], shdr.sh_size);
    std::copy(tmp_table.begin(), tmp_table.end(), _stringTable.begin());
}

template <typename ElfType_>
StringSection<ElfType_>::~StringSection()
{
}

template <typename ElfType_>
std::string
StringSection<ElfType_>::get_string(typename ElfType_::Word index) const
{
    typename ElfType_::Word end(_stringTable.find_first_of('\0', index));
    return _stringTable.substr(index, end-index);
}

template <typename ElfType_>
std::string
StringSection<ElfType_>::get_type() const
{
    static std::string type("STRTAB");
    return type;
}

template class Section<Elf32Type>;
template class Section<Elf64Type>;
template class GenericSection<Elf32Type>;
template class GenericSection<Elf64Type>;
template class StringSection<Elf32Type>;
template class StringSection<Elf64Type>;

