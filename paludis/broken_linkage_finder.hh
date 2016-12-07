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
#include <paludis/util/fs_path-fwd.hh>
#include <paludis/util/iterator_range.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/sequence-fwd.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>

#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>

#include <memory>

namespace paludis
{
    class PALUDIS_VISIBLE BrokenLinkageFinder
    {
        private:
            Pimp<BrokenLinkageFinder> _imp;

        public:
            BrokenLinkageFinder(const Environment *, const std::shared_ptr<const Sequence<std::string>> &);
            ~BrokenLinkageFinder();

            BrokenLinkageFinder(const BrokenLinkageFinder &) = delete;
            BrokenLinkageFinder & operator= (const BrokenLinkageFinder &) = delete;

            struct BrokenPackageConstIteratorTag;
            typedef WrappedForwardIterator<BrokenPackageConstIteratorTag, const std::shared_ptr<const PackageID>> BrokenPackageConstIterator;
            BrokenPackageConstIterator begin_broken_packages() const PALUDIS_ATTRIBUTE((warn_unused_result));
            BrokenPackageConstIterator end_broken_packages() const PALUDIS_ATTRIBUTE((warn_unused_result));
            IteratorRange<BrokenPackageConstIterator> broken_packages() const;

            struct BrokenFileConstIteratorTag;
            typedef WrappedForwardIterator<BrokenFileConstIteratorTag, const FSPath> BrokenFileConstIterator;
            BrokenFileConstIterator begin_broken_files(const std::shared_ptr<const PackageID> &)
                const PALUDIS_ATTRIBUTE((warn_unused_result));
            BrokenFileConstIterator end_broken_files(const std::shared_ptr<const PackageID> &)
                const PALUDIS_ATTRIBUTE((warn_unused_result));
            IteratorRange<BrokenFileConstIterator> broken_files(const std::shared_ptr<const PackageID> &) const;

            struct MissingRequirementConstIteratorTag;
            typedef WrappedForwardIterator<MissingRequirementConstIteratorTag, const std::string> MissingRequirementConstIterator;
            MissingRequirementConstIterator begin_missing_requirements(const std::shared_ptr<const PackageID> &, const FSPath &)
                const PALUDIS_ATTRIBUTE((warn_unused_result));
            MissingRequirementConstIterator end_missing_requirements(const std::shared_ptr<const PackageID> &, const FSPath &)
                const PALUDIS_ATTRIBUTE((warn_unused_result));
            IteratorRange<MissingRequirementConstIterator> missing_requirements(const std::shared_ptr<const PackageID> &, const FSPath &) const;
    };
}

#endif

