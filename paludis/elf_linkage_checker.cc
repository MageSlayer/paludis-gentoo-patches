/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
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

#include "elf_linkage_checker.hh"

#include <paludis/util/elf.hh>
#include <paludis/util/elf_dynamic_section.hh>
#include <paludis/util/elf_types.hh>
#include <paludis/util/elf_relocation_section.hh>
#include <paludis/util/elf_symbol_section.hh>

#include <paludis/util/realpath.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/join.hh>
#include <paludis/util/log.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/safe_ifstream.hh>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <map>
#include <set>
#include <vector>

using namespace paludis;

namespace
{
    struct ElfArchitecture
    {
        // not in elf.h; from glibc-2.5/sysdeps/s390/s390-32/dl-machine.h
        static const unsigned S390_OLD = 0xA390;
        // not in FreeBSD elf.h
        static const unsigned MIPS_ABI2 = 32;

        unsigned _machine;
        unsigned char _class;
        bool _bigendian, _mips_n32;

        bool operator< (const ElfArchitecture &) const PALUDIS_ATTRIBUTE((warn_unused_result));

        static unsigned normalise_arch(unsigned arch)
        {
            switch (arch)
            {
                case EM_MIPS_RS3_LE:
                    return EM_MIPS;
                case S390_OLD:
                    return EM_S390;
            }
            return arch;
        }

        template <typename ElfType_>
        ElfArchitecture(const ElfObject<ElfType_> & elf) :
            _machine(normalise_arch(elf.get_arch())),
            _class(ElfType_::elf_class),
            _bigendian(elf.is_big_endian()),
            _mips_n32(EM_MIPS == _machine && MIPS_ABI2 & elf.get_flags())
        {
        }
    };

    bool
    ElfArchitecture::operator< (const ElfArchitecture & other) const
    {
        if (_machine != other._machine)
            return _machine < other._machine;
        if (_class != other._class)
            return _class < other._class;
        if (_bigendian != other._bigendian)
            return _bigendian < other._bigendian;
        return _mips_n32 < other._mips_n32;
    }
}

typedef std::multimap<FSEntry, FSEntry> Symlinks;
typedef std::map<ElfArchitecture, std::map<std::string, std::vector<FSEntry> > > Needed;

namespace paludis
{
    template <>
    struct Imp<ElfLinkageChecker>
    {
        FSEntry root;
        std::string library;

        Mutex mutex;

        std::map<FSEntry, ElfArchitecture> seen;
        Symlinks symlinks;

        std::map<ElfArchitecture, std::vector<std::string> > libraries;
        Needed needed;

        std::vector<FSEntry> extra_lib_dirs;

        template <typename> bool check_elf(const FSEntry &, std::istream &);
        void handle_library(const FSEntry &, const ElfArchitecture &);
        template <typename> bool check_extra_elf(const FSEntry &, std::istream &, std::set<ElfArchitecture> &);

        Imp(const FSEntry & the_root, const std::string & the_library) :
            root(the_root),
            library(the_library)
        {
        }
    };
}

ElfLinkageChecker::ElfLinkageChecker(const FSEntry & root, const std::string & library) :
    Pimp<ElfLinkageChecker>(root, library)
{
}

ElfLinkageChecker::~ElfLinkageChecker()
{
}

bool
ElfLinkageChecker::check_file(const FSEntry & file)
{
    std::string basename(file.basename());
    if (! (std::string::npos != basename.find(".so.") ||
           (3 <= basename.length() && ".so" == basename.substr(basename.length() - 3)) ||
           file.has_permission(fs_ug_owner, fs_perm_execute)))
        return false;

    SafeIFStream stream(file);
    return _imp->check_elf<Elf32Type>(file, stream) || _imp->check_elf<Elf64Type>(file, stream);
}

template <typename ElfType_>
bool
Imp<ElfLinkageChecker>::check_elf(const FSEntry & file, std::istream & stream)
{
    if (! ElfObject<ElfType_>::is_valid_elf(stream))
        return false;

    try
    {
        Context ctx("When checking '" + stringify(file) + "' as a " +
                    stringify<int>(ElfType_::elf_class * 32) + "-bit ELF file:");
        ElfObject<ElfType_> elf(stream);
        if (ET_EXEC != elf.get_type() && ET_DYN != elf.get_type())
        {
            Log::get_instance()->message("reconcilio.broken_linkage_finder.not_interesting", ll_debug, lc_context)
                << "File is not an executable or shared library";
            return true;
        }

        ElfArchitecture arch(elf);
        elf.resolve_all_strings();

        Lock l(mutex);

        if (library.empty() && ET_DYN == elf.get_type())
            handle_library(file, arch);

        for (typename ElfObject<ElfType_>::SectionIterator sec_it(elf.section_begin()),
                 sec_it_end(elf.section_end()); sec_it_end != sec_it; ++sec_it)
        {
            const DynamicSection<ElfType_> * dyn_sec(simple_visitor_cast<const DynamicSection<ElfType_> >(*sec_it));

            if (0 != dyn_sec)
                for (typename DynamicSection<ElfType_>::EntryIterator ent_it(dyn_sec->entry_begin()),
                         ent_it_end(dyn_sec->entry_end()); ent_it_end != ent_it; ++ent_it)
                {
                    const DynamicEntryString<ElfType_> * ent_str(simple_visitor_cast<const DynamicEntryString<ElfType_> >(*ent_it));

                    if (0 != ent_str && "NEEDED" == ent_str->tag_name())
                    {
                        const std::string & req((*ent_str)());
                        if (library.empty() || library == req)
                        {
                            Log::get_instance()->message("reconcilio.broken_linkage_finder.depends", ll_debug, lc_context)
                                << "File depends on " << req;
                            needed[arch][req].push_back(file);
                        }
                    }
                }
        }
    }
    catch (const InvalidElfFileError & e)
    {
        Log::get_instance()->message("reconcilio.broken_linkage_finder.invalid", ll_warning, lc_no_context)
            << "'" << file << "' appears to be invalid or corrupted: " << e.message();
    }

    return true;
}

void
Imp<ElfLinkageChecker>::handle_library(const FSEntry & file, const ElfArchitecture & arch)
{
    seen.insert(std::make_pair(file, arch));
    std::pair<Symlinks::const_iterator, Symlinks::const_iterator> range(symlinks.equal_range(file));
    libraries[arch].push_back(file.basename());

    if (range.first != range.second)
    {
        Log::get_instance()->message("reconcilio.broken_linkage_finder.known_symlinks_are",
                ll_debug, lc_context) << "Known symlinks are " <<
            join(second_iterator(range.first), second_iterator(range.second), " ");
        std::transform(second_iterator(range.first), second_iterator(range.second),
                       std::back_inserter(libraries[arch]), std::mem_fn(&FSEntry::basename));
    }
}

void
ElfLinkageChecker::note_symlink(const FSEntry & link, const FSEntry & target)
{
    if (_imp->library.empty())
    {
        Lock l(_imp->mutex);

        std::map<FSEntry, ElfArchitecture>::const_iterator it(_imp->seen.find(target));
        if (_imp->seen.end() != it)
        {
            Log::get_instance()->message("reconcilio.broken_linkage_finder.note_symlink", ll_debug, lc_context)
                << "'" << link << "' is a symlink to known library '" << target << "'";
            _imp->libraries[it->second].push_back(link.basename());
        }
        else
            _imp->symlinks.insert(std::make_pair(target, link));
    }
}

void
ElfLinkageChecker::add_extra_lib_dir(const FSEntry & dir)
{
    _imp->extra_lib_dirs.push_back(dir);
}

void
ElfLinkageChecker::need_breakage_added(
    const std::function<void (const FSEntry &, const std::string &)> & callback)
{
    using namespace std::placeholders;

    typedef std::map<std::string, std::set<ElfArchitecture> > AllMissing;
    AllMissing all_missing;

    for (Needed::iterator arch_it(_imp->needed.begin()),
             arch_it_end(_imp->needed.end()); arch_it_end != arch_it; ++arch_it)
    {
        std::sort(_imp->libraries[arch_it->first].begin(), _imp->libraries[arch_it->first].end());
        _imp->libraries[arch_it->first].erase(
            std::unique(_imp->libraries[arch_it->first].begin(),
                        _imp->libraries[arch_it->first].end()),
            _imp->libraries[arch_it->first].end());

        std::vector<std::string> missing;
        std::set_difference(first_iterator(arch_it->second.begin()),
                            first_iterator(arch_it->second.end()),
                            _imp->libraries[arch_it->first].begin(),
                            _imp->libraries[arch_it->first].end(),
                            std::back_inserter(missing));
        for (std::vector<std::string>::const_iterator it(missing.begin()),
                 it_end(missing.end()); it_end != it; ++it)
            all_missing[*it].insert(arch_it->first);
    }

    for (std::vector<FSEntry>::const_iterator dir_it(_imp->extra_lib_dirs.begin()),
             dir_it_end(_imp->extra_lib_dirs.end()); dir_it_end != dir_it; ++dir_it)
    {
        Context ctx("When searching for missing libraries in '" + stringify(*dir_it) + "':");

        for (AllMissing::iterator missing_it(all_missing.begin()),
                 missing_it_end(all_missing.end()); missing_it_end != missing_it; ++missing_it)
        {
            if (missing_it->second.empty())
                continue;

            FSEntry file(dereference_with_root(*dir_it / missing_it->first, _imp->root));
            if (! file.is_regular_file())
            {
                Log::get_instance()->message("reconcilio.broken_linkage_finder.missing", ll_debug, lc_context)
                    << "'" << file << "' is missing or not a regular file";
                continue;
            }

            try
            {
                SafeIFStream stream(file);

                if (! (_imp->check_extra_elf<Elf32Type>(file, stream, missing_it->second) ||
                       _imp->check_extra_elf<Elf64Type>(file, stream, missing_it->second)))
                    Log::get_instance()->message("reconcilio.broken_linkage_finder.not_an_elf", ll_debug, lc_no_context)
                        << "'" << file << "' is not an ELF file";
            }
            catch (const SafeIFStreamError & e)
            {
                Log::get_instance()->message("reconcilio.broken_linkage_finder.failure", ll_warning, lc_no_context)
                    << "Error opening '" << file << "': '" << e.message() << "' (" << e.what() << ")";
                continue;
            }
        }
    }

    for (AllMissing::const_iterator missing_it(all_missing.begin()),
             missing_it_end(all_missing.end()); missing_it_end != missing_it; ++missing_it)
        for (std::set<ElfArchitecture>::const_iterator arch_it(missing_it->second.begin()),
                 arch_it_end(missing_it->second.end()); arch_it_end != arch_it; ++arch_it)
            std::for_each(_imp->needed[*arch_it][missing_it->first].begin(),
                          _imp->needed[*arch_it][missing_it->first].end(),
                          std::bind(callback, _1, missing_it->first));

}

template <typename ElfType_>
bool
Imp<ElfLinkageChecker>::check_extra_elf(const FSEntry & file, std::istream & stream, std::set<ElfArchitecture> & arches)
{
    if (! ElfObject<ElfType_>::is_valid_elf(stream))
        return false;

    Context ctx("When checking '" + stringify(file) + "' as a " + stringify<int>(ElfType_::elf_class * 32) + "-bit ELF file");

    try
    {
        ElfObject<ElfType_> elf(stream);
        if (ET_DYN == elf.get_type())
        {
            Log::get_instance()->message("reconcilio.broken_linkage_finder.is_library", ll_debug, lc_context)
                << "'" << file << "' is a library";
            arches.erase(ElfArchitecture(elf));
        }
        else
            Log::get_instance()->message("reconcilio.broken_linkage_finder.is_not_library", ll_debug, lc_context)
                << "'" << file << "' is not a library";
    }
    catch (const InvalidElfFileError & e)
    {
        Log::get_instance()->message("reconcilio.broken_linkage_finder.invalid", ll_warning, lc_no_context)
            << "'" << file << "' appears to be invalid or corrupted: " << e.message();
    }

    return true;
}

