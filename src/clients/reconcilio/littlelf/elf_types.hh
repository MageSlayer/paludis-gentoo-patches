#ifndef ELFTYPES_HH_
#define ELFTYPES_HH_

#include <elf.h>

struct Elf32Type
{
    enum {
        elf_class = ELFCLASS32
    };

    typedef Elf32_Ehdr Header;
    typedef Elf32_Shdr SectionHeader;
    typedef Elf32_Sym  Symbol;
    typedef Elf32_Dyn  DynamicEntry;
    typedef Elf32_Rel  Relocation;
    typedef Elf32_Rela RelocationA;

    typedef Elf32_Off    Offset;
    typedef Elf32_Half   Half;
    typedef Elf32_Word   Word;
    typedef Elf32_Xword  Xword;
    typedef Elf32_Sxword Sxword;
    typedef Elf32_Addr   Address;

    typedef Elf32_Word  DynamicValue;
    typedef Elf32_Addr  DynamicPointer;
    typedef Elf32_Sword DynamicTag;

    typedef Elf32_Addr  RelocationOffset;
    typedef Elf32_Word  RelocationInfo;
    typedef Elf32_Sword RelocationAddend;

    typedef Elf32_Word SectionSize;
};

struct Elf64Type
{
    enum {
        elf_class = ELFCLASS64
    };

    typedef Elf64_Ehdr Header;
    typedef Elf64_Shdr SectionHeader;
    typedef Elf64_Sym  Symbol;
    typedef Elf64_Dyn  DynamicEntry;
    typedef Elf64_Rel  Relocation;
    typedef Elf64_Rela RelocationA;

    typedef Elf64_Off    Offset;
    typedef Elf64_Half   Half;
    typedef Elf64_Word   Word;
    typedef Elf64_Xword  Xword;
    typedef Elf64_Sxword Sxword;
    typedef Elf64_Addr   Address;

    typedef Elf32_Xword  DynamicValue;
    typedef Elf64_Addr   DynamicPointer;
    typedef Elf64_Sxword DynamicTag;

    typedef Elf64_Addr   RelocationOffset;
    typedef Elf64_Xword  RelocationInfo;
    typedef Elf64_Sxword RelocationAddend;

    typedef Elf64_Xword SectionSize;
};

#endif /*ELFTYPES_HH_*/
