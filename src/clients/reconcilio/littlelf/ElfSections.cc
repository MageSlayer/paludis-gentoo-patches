
#include "ElfSections.hh"
#include "ElfTypes.hh"

#include <paludis/util/visitor-impl.hh>

#include <istream>
#include <algorithm>

using namespace paludis;

template <typename ElfType_>
Section<ElfType_>::Section(const typename ElfType_::SectionHeader & shdr) :
    _shdr(shdr),
    _name("")
{
}

template <typename ElfType_>
Section<ElfType_>::~Section()
{
}

template <typename ElfType_>
GenericSection<ElfType_>::GenericSection(const typename ElfType_::SectionHeader & shdr) :
    Section<ElfType_>(shdr)
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
StringSection<ElfType_>::StringSection(const typename ElfType_::SectionHeader & shdr, std::istream & stream) :
    Section<ElfType_>(shdr),
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

