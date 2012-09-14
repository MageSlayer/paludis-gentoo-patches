/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/fs_path-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/metadata_key-fwd.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/spec_tree-fwd.hh>
#include <paludis/slot-fwd.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_added_by> added_by;
        typedef Name<struct name_bug_ids> bug_ids;
        typedef Name<struct name_comment> comment;
        typedef Name<struct name_commit_id> commit_id;
        typedef Name<struct name_description> description;
        typedef Name<struct name_homepage> homepage;
        typedef Name<struct name_name> name;
        typedef Name<struct name_remote_ids> remote_ids;
        typedef Name<struct name_removed_by> removed_by;
        typedef Name<struct name_removed_from> removed_from;
        typedef Name<struct name_slot> slot;
        typedef Name<struct name_version> version;
    }

    namespace unwritten_repository
    {
        class UnwrittenRepositoryFile;

        struct UnwrittenRepositoryFileEntry
        {
            NamedValue<n::added_by, std::shared_ptr<const MetadataValueKey<std::string> > > added_by;
            NamedValue<n::bug_ids, std::shared_ptr<const MetadataCollectionKey<Sequence<std::string> > > > bug_ids;
            NamedValue<n::comment, std::shared_ptr<const MetadataValueKey<std::string> > > comment;
            NamedValue<n::commit_id, std::shared_ptr<const MetadataValueKey<std::string> > > commit_id;
            NamedValue<n::description, std::shared_ptr<const MetadataValueKey<std::string> > > description;
            NamedValue<n::homepage, std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > > homepage;
            NamedValue<n::name, QualifiedPackageName> name;
            NamedValue<n::remote_ids, std::shared_ptr<const MetadataCollectionKey<Sequence<std::string> > > > remote_ids;
            NamedValue<n::removed_by, std::shared_ptr<const MetadataValueKey<std::string> > > removed_by;
            NamedValue<n::removed_from, std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > > removed_from;
            NamedValue<n::slot, std::shared_ptr<const MetadataValueKey<Slot> > > slot;
            NamedValue<n::version, VersionSpec> version;
        };

        class PALUDIS_VISIBLE UnwrittenRepositoryFile
        {
            private:
                Pimp<UnwrittenRepositoryFile> _imp;

                void _load(const FSPath &);

            public:
                UnwrittenRepositoryFile(const FSPath &);
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
