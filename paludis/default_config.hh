/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
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

#ifndef PALUDIS_GUARD_PALUDIS_DEFAULT_CONFIG_HH
#define PALUDIS_GUARD_PALUDIS_DEFAULT_CONFIG_HH 1

#include <paludis/instantiation_policy.hh>
#include <paludis/smart_record.hh>
#include <paludis/fs_entry.hh>
#include <paludis/qualified_package_name.hh>
#include <paludis/package_dep_atom.hh>
#include <paludis/keyword_name.hh>
#include <paludis/indirect_iterator.hh>
#include <map>
#include <vector>

namespace paludis
{
    enum RepositoryConfigEntryKeys
    {
        rce_location,
        rce_profile,
        rce_format
    };

    struct RepositoryConfigEntryTag :
        SmartRecordTag<comparison_mode::FullComparisonTag, comparison_method::SmartRecordCompareByAllTag>,
        SmartRecordKeys<RepositoryConfigEntryKeys, 3>,
        SmartRecordKey<rce_location, FSEntry>,
        SmartRecordKey<rce_profile, FSEntry>,
        SmartRecordKey<rce_format, std::string>
    {
    };

    typedef MakeSmartRecord<RepositoryConfigEntryTag>::Type RepositoryConfigEntry;

    class DefaultConfig :
        public InstantiationPolicy<DefaultConfig, instantiation_method::SingletonAsNeededTag>
    {
        friend class InstantiationPolicy<DefaultConfig, instantiation_method::SingletonAsNeededTag>;

        private:
            DefaultConfig();

            std::list<RepositoryConfigEntry> _repos;

            std::map<QualifiedPackageName, std::vector<
                std::pair<PackageDepAtom::ConstPointer, KeywordName> > > _keywords;

            const std::vector<std::pair<PackageDepAtom::ConstPointer, KeywordName> > _empty_keywords;

            std::vector<KeywordName> _default_keywords;

            std::map<QualifiedPackageName, std::vector<PackageDepAtom::ConstPointer> > _user_masks;

            std::map<QualifiedPackageName, std::vector<PackageDepAtom::ConstPointer> > _user_unmasks;

            std::vector<PackageDepAtom::ConstPointer> _empty_masks;

        public:
            typedef std::list<RepositoryConfigEntry>::const_iterator RepositoryIterator;

            RepositoryIterator begin_repositories() const
            {
                return _repos.begin();
            }

            RepositoryIterator end_repositories() const
            {
                return _repos.end();
            }

            typedef std::vector<std::pair<PackageDepAtom::ConstPointer, KeywordName> >::const_iterator
                PackageKeywordsIterator;

            PackageKeywordsIterator begin_package_keywords(const QualifiedPackageName & d) const
            {
                std::map<QualifiedPackageName, std::vector<
                    std::pair<PackageDepAtom::ConstPointer, KeywordName> > >::const_iterator r;
                if (_keywords.end() != ((r = _keywords.find(d))))
                    return r->second.begin();
                else
                    return _empty_keywords.begin();
            }

            PackageKeywordsIterator end_package_keywords(const QualifiedPackageName & d) const
            {
                std::map<QualifiedPackageName, std::vector<
                    std::pair<PackageDepAtom::ConstPointer, KeywordName> > >::const_iterator r;
                if (_keywords.end() != ((r = _keywords.find(d))))
                    return r->second.end();
                else
                    return _empty_keywords.end();
            }

            typedef std::vector<KeywordName>::const_iterator DefaultKeywordsIterator;

            DefaultKeywordsIterator begin_default_keywords() const
            {
                return _default_keywords.begin();
            }

            DefaultKeywordsIterator end_default_keywords() const
            {
                return _default_keywords.end();
            }

            typedef IndirectIterator<std::vector<PackageDepAtom::ConstPointer>::const_iterator,
                    const PackageDepAtom> UserMasksIterator;

            UserMasksIterator begin_user_masks(const QualifiedPackageName & d) const
            {
                std::map<QualifiedPackageName, std::vector<PackageDepAtom::ConstPointer> >::const_iterator r;
                if (_user_masks.end() != ((r = _user_masks.find(d))))
                    return r->second.begin();
                else
                    return _empty_masks.begin();
            }

            UserMasksIterator end_user_masks(const QualifiedPackageName & d) const
            {
                std::map<QualifiedPackageName, std::vector<PackageDepAtom::ConstPointer> >::const_iterator r;
                if (_user_masks.end() != ((r = _user_masks.find(d))))
                    return r->second.end();
                else
                    return _empty_masks.end();
            }

            typedef IndirectIterator<std::vector<PackageDepAtom::ConstPointer>::const_iterator,
                    const PackageDepAtom> UserUnmasksIterator;

            UserUnmasksIterator begin_user_unmasks(const QualifiedPackageName & d) const
            {
                std::map<QualifiedPackageName, std::vector<PackageDepAtom::ConstPointer> >::const_iterator r;
                if (_user_unmasks.end() != ((r = _user_unmasks.find(d))))
                    return r->second.begin();
                else
                    return _empty_masks.begin();
            }

            UserUnmasksIterator end_user_unmasks(const QualifiedPackageName & d) const
            {
                std::map<QualifiedPackageName, std::vector<PackageDepAtom::ConstPointer> >::const_iterator r;
                if (_user_unmasks.end() != ((r = _user_unmasks.find(d))))
                    return r->second.end();
                else
                    return _empty_masks.end();
            }
    };
}

#endif
