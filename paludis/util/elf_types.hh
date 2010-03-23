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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_ELF_TYPES_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_ELF_TYPES_HH 1

#include <elf.h>

namespace paludis
{
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

        typedef Elf64_Xword  DynamicValue;
        typedef Elf64_Addr   DynamicPointer;
        typedef Elf64_Sxword DynamicTag;

        typedef Elf64_Addr   RelocationOffset;
        typedef Elf64_Xword  RelocationInfo;
        typedef Elf64_Sxword RelocationAddend;

        typedef Elf64_Xword SectionSize;
    };
}

#endif

