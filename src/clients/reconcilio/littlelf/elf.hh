#ifndef ELF_HH_
#define ELF_HH_

#include "elf_sections.hh"

#include <paludis/util/exception.hh>
#include <paludis/util/private_implementation_pattern.hh>

#include <libwrapiter/libwrapiter_forward_iterator-fwd.hh>

#include <iosfwd>

#include <elf.h>

class InvalidElfFileError :
    public paludis::Exception
{
    public:
        InvalidElfFileError(const InvalidElfFileError &);
        InvalidElfFileError() throw ();
};

template <typename ElfType_>
class ElfObject :
    private paludis::PrivateImplementationPattern<ElfObject<ElfType_> >
{
    using paludis::PrivateImplementationPattern<ElfObject>::_imp;

    private:
        typename ElfType_::Header _hdr;

    public:
        static bool is_valid_elf(std::istream & stream);
        ElfObject(std::istream & stream);
        ~ElfObject();

        /**
         * Returns e_type from the ELF header
         */
        unsigned int get_type() const
        {
            return _hdr.e_type;
        }

        /**
         * Returns e_machine from the ELF header
         */
        unsigned int get_arch() const
        {
            return _hdr.e_machine;
        }

        /**
         * Returns the OS ABI field from the ident field
         */
        unsigned char get_os_abi() const
        {
            return _hdr.e_ident[EI_OSABI];
        }

        /**
         * Returns the OS ABI Version field from the ident field
         */
        unsigned char get_os_abi_version() const
        {
            return _hdr.e_ident[EI_ABIVERSION];
        }

        /**
         * Returns the processor-specific flags
         */
        unsigned int get_flags() const
        {
            return _hdr.e_flags;
        }

        /**
         * Returns whether this ELF file uses big-endian or little-endian
         * Please note: If you didn't use is_valid_elf(...) to check whether
         * this is an ELF object, this might be wrong
         */
        unsigned int is_big_endian() const
        {
            // We already checked in is_valid_elf_type whether it's valid or not
            return (_hdr.e_ident[EI_DATA] == ELFDATA2MSB);
        }

        unsigned int get_number_of_sections() const
        {
            return _hdr.e_shnum;
        }

        typedef libwrapiter::ForwardIterator<ElfObject, Section<ElfType_> > SectionIterator;
        SectionIterator section_begin() const;
        SectionIterator section_end() const;

        SectionIterator get_section_by_index(unsigned int index) const;

        void resolve_all_strings();
};


#endif /*ELF_HH_*/
