/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_PROFILE_FILE_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_PROFILE_FILE_HH 1

#include <paludis/repositories/e/eapi-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <paludis/mask-fwd.hh>
#include <type_traits>
#include <functional>

namespace paludis
{
    struct ERepository;

    namespace erepository
    {
        template <typename F_>
        class PALUDIS_VISIBLE ProfileFile
        {
            private:
                Pimp<ProfileFile<F_> > _imp;

            public:
                ///\name Basic operations
                ///\{

                explicit ProfileFile(const EAPIForFileFunction &);
                explicit ProfileFile(const ERepository * const);

                ~ProfileFile();

                ///\}

                /**
                 * Add a file.
                 */
                void add_file(const FSPath &);

                ///\name Iterate over our profile lines.
                ///\{

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag, const std::pair<
                    std::shared_ptr<const erepository::EAPI>,
                          const typename std::remove_reference<
                              typename F_::ConstIterator::value_type>::type> > ConstIterator;
                ConstIterator begin() const;
                ConstIterator end() const;

                ///\}
        };
    }
}

#endif
