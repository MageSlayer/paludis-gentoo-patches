/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNAVAILABLE_UNAVAILABLE_REPOSITORY_FILE_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNAVAILABLE_UNAVAILABLE_REPOSITORY_FILE_HH 1

#include <paludis/repositories/unavailable/unavailable_repository_file-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/fs_entry.hh>

namespace paludis
{
    namespace unavailable_repository
    {
        class PALUDIS_VISIBLE UnavailableRepositoryFile :
            private Pimp<UnavailableRepositoryFile>
        {
            private:
                void _load(const FSEntry &);

            public:
                UnavailableRepositoryFile(const FSEntry &);
                ~UnavailableRepositoryFile();

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag, const UnavailableRepositoryFileEntry> ConstIterator;
                ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

                std::string repo_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                std::string homepage() const PALUDIS_ATTRIBUTE((warn_unused_result));
                std::string description() const PALUDIS_ATTRIBUTE((warn_unused_result));
                std::string sync() const PALUDIS_ATTRIBUTE((warn_unused_result));
                std::string repo_format() const PALUDIS_ATTRIBUTE((warn_unused_result));
                std::string dependencies() const PALUDIS_ATTRIBUTE((warn_unused_result));
                bool autoconfigurable() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

    extern template class Pimp<unavailable_repository::UnavailableRepositoryFile>;
}

#endif
