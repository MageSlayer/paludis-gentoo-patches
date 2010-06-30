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

#ifndef PALUDIS_GUARD_PALUDIS_BROKEN_LINKAGE_CONFIGURATION_HH
#define PALUDIS_GUARD_PALUDIS_BROKEN_LINKAGE_CONFIGURATION_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

#include <string>

namespace paludis
{
    class PALUDIS_VISIBLE BrokenLinkageConfiguration :
        private paludis::PrivateImplementationPattern<BrokenLinkageConfiguration>,
        private paludis::InstantiationPolicy<BrokenLinkageConfiguration, paludis::instantiation_method::NonCopyableTag>
    {
        public:
            BrokenLinkageConfiguration(const paludis::FSEntry &);
            ~BrokenLinkageConfiguration();

            struct DirsIteratorTag;
            typedef paludis::WrappedForwardIterator<DirsIteratorTag, const paludis::FSEntry> DirsIterator;
            DirsIterator begin_search_dirs() const PALUDIS_ATTRIBUTE((warn_unused_result));
            DirsIterator end_search_dirs() const PALUDIS_ATTRIBUTE((warn_unused_result));
            DirsIterator begin_ld_so_conf() const PALUDIS_ATTRIBUTE((warn_unused_result));
            DirsIterator end_ld_so_conf() const PALUDIS_ATTRIBUTE((warn_unused_result));

            bool dir_is_masked(const paludis::FSEntry &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            bool lib_is_masked(const std::string &) const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    extern template class WrappedForwardIterator<BrokenLinkageConfiguration::DirsIteratorTag, const FSEntry>;
}

#endif

