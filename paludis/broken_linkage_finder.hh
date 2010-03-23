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

#ifndef PALUDIS_GUARD_PALUDIS_BROKEN_LINKAGE_FINDER_HH
#define PALUDIS_GUARD_PALUDIS_BROKEN_LINKAGE_FINDER_HH

#include <paludis/util/attributes.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>

#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>

#include <tr1/memory>

namespace paludis
{
    class PALUDIS_VISIBLE BrokenLinkageFinder :
        private paludis::PrivateImplementationPattern<BrokenLinkageFinder>,
        private paludis::InstantiationPolicy<BrokenLinkageFinder, paludis::instantiation_method::NonCopyableTag>
    {
        public:
            BrokenLinkageFinder(const paludis::Environment *, const std::string &);
            ~BrokenLinkageFinder();

            struct BrokenPackageConstIteratorTag;
            typedef paludis::WrappedForwardIterator<BrokenPackageConstIteratorTag,
                    const std::tr1::shared_ptr<const paludis::PackageID>
                        > BrokenPackageConstIterator;
            BrokenPackageConstIterator begin_broken_packages() const PALUDIS_ATTRIBUTE((warn_unused_result));
            BrokenPackageConstIterator end_broken_packages() const PALUDIS_ATTRIBUTE((warn_unused_result));

            struct BrokenFileConstIteratorTag;
            typedef paludis::WrappedForwardIterator<BrokenFileConstIteratorTag, const paludis::FSEntry> BrokenFileConstIterator;
            BrokenFileConstIterator begin_broken_files(const std::tr1::shared_ptr<const paludis::PackageID> &)
                const PALUDIS_ATTRIBUTE((warn_unused_result));
            BrokenFileConstIterator end_broken_files(const std::tr1::shared_ptr<const paludis::PackageID> &)
                const PALUDIS_ATTRIBUTE((warn_unused_result));

            struct MissingRequirementConstIteratorTag;
            typedef paludis::WrappedForwardIterator<MissingRequirementConstIteratorTag, const std::string> MissingRequirementConstIterator;
            MissingRequirementConstIterator begin_missing_requirements(
                const std::tr1::shared_ptr<const paludis::PackageID> &, const paludis::FSEntry &)
                const PALUDIS_ATTRIBUTE((warn_unused_result));
            MissingRequirementConstIterator end_missing_requirements(
                const std::tr1::shared_ptr<const paludis::PackageID> &, const paludis::FSEntry &)
                const PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif

