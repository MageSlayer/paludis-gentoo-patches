/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNWRITTEN_UNWRITTEN_REPOSITORY_FILE_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNWRITTEN_UNWRITTEN_REPOSITORY_FILE_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/named_value.hh>
#include <paludis/metadata_key-fwd.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/spec_tree-fwd.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct added_by_name> added_by;
        typedef Name<struct bug_ids_name> bug_ids;
        typedef Name<struct comment_name> comment;
        typedef Name<struct description_name> description;
        typedef Name<struct homepage_name> homepage;
        typedef Name<struct name_name> name;
        typedef Name<struct remote_ids_name> remote_ids;
        typedef Name<struct slot_name> slot;
        typedef Name<struct version_name> version;
    }

    namespace unwritten_repository
    {
        class UnwrittenRepositoryFile;

        struct UnwrittenRepositoryFileEntry
        {
            NamedValue<n::added_by, std::shared_ptr<const MetadataValueKey<std::string> > > added_by;
            NamedValue<n::bug_ids, std::shared_ptr<const MetadataCollectionKey<Sequence<std::string> > > > bug_ids;
            NamedValue<n::comment, std::shared_ptr<const MetadataValueKey<std::string> > > comment;
            NamedValue<n::description, std::shared_ptr<const MetadataValueKey<std::string> > > description;
            NamedValue<n::homepage, std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > > homepage;
            NamedValue<n::name, QualifiedPackageName> name;
            NamedValue<n::remote_ids, std::shared_ptr<const MetadataCollectionKey<Sequence<std::string> > > > remote_ids;
            NamedValue<n::slot, std::shared_ptr<const MetadataValueKey<SlotName> > > slot;
            NamedValue<n::version, VersionSpec> version;
        };

        class PALUDIS_VISIBLE UnwrittenRepositoryFile :
            private Pimp<UnwrittenRepositoryFile>
        {
            private:
                void _load(const FSEntry &);

            public:
                UnwrittenRepositoryFile(const FSEntry &);
                ~UnwrittenRepositoryFile();

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag, const UnwrittenRepositoryFileEntry> ConstIterator;
                ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

    extern template class Pimp<unwritten_repository::UnwrittenRepositoryFile>;
}

#endif
