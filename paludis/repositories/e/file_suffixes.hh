/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_FILE_SUFFIXES_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_FILE_SUFFIXES_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/util/singleton.hh>
#include <paludis/util/fs_path-fwd.hh>
#include <paludis/name-fwd.hh>
#include <string>

namespace paludis
{
    namespace erepository
    {
        class FileSuffixes :
            public Singleton<FileSuffixes>
        {
            friend class Singleton<FileSuffixes>;

            private:
                Pimp<FileSuffixes> _imp;

                FileSuffixes();
                ~FileSuffixes();

            public:
                bool is_known_suffix(const std::string & s) const;

                const std::string manifest_key(const std::string & s) const;

                const std::string get_package_file_manifest_key(const FSPath & e, const QualifiedPackageName & q) const;

                bool is_package_file(const QualifiedPackageName & n, const FSPath & e) const;

                const std::string guess_eapi_without_hint(const std::string & s) const;

                const std::string guess_eapi_from_filename(const QualifiedPackageName &, const FSPath & e) const;
        };
    }

    extern template class Singleton<erepository::FileSuffixes>;
}

#endif
