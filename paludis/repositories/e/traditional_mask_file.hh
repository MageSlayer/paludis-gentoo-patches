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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_TRADITIONAL_MASK_FILE_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_TRADITIONAL_MASK_FILE_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

#include <paludis/mask-fwd.hh>

#include <paludis/repositories/e/mask_info.hh>

namespace paludis
{
    namespace erepository
    {
        struct EAPI;

        class PALUDIS_VISIBLE TraditionalMaskFile
        {
            private:
                Pimp<TraditionalMaskFile> _imp;

            public:
                TraditionalMaskFile(const FSPath &, const LineConfigFileOptions &, const EAPI &);
                ~TraditionalMaskFile();

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag,
                        const std::pair<const std::string, std::shared_ptr<const MaskInfo> > > ConstIterator;
                ConstIterator begin() const;
                ConstIterator end() const;
        };
    }

    extern template class WrappedForwardIterator<erepository::TraditionalMaskFile::ConstIteratorTag,
             const std::pair<const std::string, std::shared_ptr<const erepository::MaskInfo> > >;
}

#endif

