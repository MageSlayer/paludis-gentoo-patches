/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_DEFAULT_CONFIG_HH
#define PALUDIS_GUARD_PALUDIS_DEFAULT_CONFIG_HH 1

#include <paludis/dep_atom.hh>
#include <paludis/name.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/smart_record.hh>

#include <map>
#include <vector>
#include <string>

namespace paludis
{
    /**
     * A DefaultConfigError is thrown if a configuration error is encountered
     * by DefaultConfig.
     *
     * \ingroup Exception
     */
    class DefaultConfigError : public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            DefaultConfigError(const std::string & msg) throw ();
    };

    /**
     * Keys for RepositoryConfigEntry.
     */
    enum RepositoryConfigEntryKeys
    {
        rce_format,         ///< Our format
        rce_importance,     ///< Our importance, higher being more important
        rce_keys,           ///< Our key/value data
        last_rce            ///< Number of entries
    };

    /**
     * Tag for RepositoryConfigEntry.
     */
    struct RepositoryConfigEntryTag :
        SmartRecordTag<comparison_mode::FullComparisonTag,
            comparison_method::SmartRecordCompareByKeyTag<rce_importance> >,
        SmartRecordKeys<RepositoryConfigEntryKeys, last_rce>,
        SmartRecordKey<rce_format, std::string>,
        SmartRecordKey<rce_importance, unsigned>,
        SmartRecordKey<rce_keys, std::map<std::string, std::string> >
    {
    };

    /**
     * Holds an entry in a DefaultConfig's repository configuration data.
     */
    typedef MakeSmartRecord<RepositoryConfigEntryTag>::Type RepositoryConfigEntry;

    /**
     * Keys for UseConfigEntry.
     */
    enum UseConfigEntryKeys
    {
        uce_dep_atom,       ///< A dependency atom
        uce_flag_name,      ///< The use flag name
        uce_flag_state      ///< The use flag state
    };

    /**
     * Tag for UseConfigEntry.
     */
    struct UseConfigEntryTag :
        SmartRecordTag<comparison_mode::NoComparisonTag, void>,
        SmartRecordKeys<UseConfigEntryKeys, 3>,
        SmartRecordKey<uce_dep_atom, PackageDepAtom::ConstPointer>,
        SmartRecordKey<uce_flag_name, UseFlagName>,
        SmartRecordKey<uce_flag_state, UseFlagState>
    {
    };

    /**
     *  An entry in a DefaultConfig's use configuration data.
     */
    typedef MakeSmartRecord<UseConfigEntryTag>::Type UseConfigEntry;

    /**
     * DefaultConfig is used by DefaultEnvironment to access the user's
     * configuration settings from on-disk configuration files.
     */
    class DefaultConfig :
        public InstantiationPolicy<DefaultConfig, instantiation_method::SingletonAsNeededTag>
    {
        friend class InstantiationPolicy<DefaultConfig, instantiation_method::SingletonAsNeededTag>;

        private:
            static std::string _config_suffix;
            static bool _config_suffix_can_be_set;

            DefaultConfig();

            ~DefaultConfig();

            std::list<RepositoryConfigEntry> _repos;

            std::map<QualifiedPackageName, std::vector<
                std::pair<PackageDepAtom::ConstPointer, KeywordName> > > _keywords;

            const std::vector<std::pair<PackageDepAtom::ConstPointer, KeywordName> > _empty_keywords;

            std::vector<KeywordName> _default_keywords;

            std::map<QualifiedPackageName, std::vector<PackageDepAtom::ConstPointer> > _user_masks;

            std::map<QualifiedPackageName, std::vector<PackageDepAtom::ConstPointer> > _user_unmasks;

            std::vector<PackageDepAtom::ConstPointer> _empty_masks;

            std::map<QualifiedPackageName, std::vector<UseConfigEntry> > _use;

            std::vector<UseConfigEntry> _empty_use;

            std::vector<std::pair<UseFlagName, UseFlagState> > _default_use;

        public:
            /**
             * Set config suffix. Must be called before we do anything, or not
             * at all.
             */
            static void set_config_suffix(const std::string &);

            /**
             * An iterator for our repositories.
             */
            typedef std::list<RepositoryConfigEntry>::const_iterator RepositoryIterator;

            /**
             * Iterator to the start of our repositories.
             */
            RepositoryIterator begin_repositories() const
            {
                return _repos.begin();
            }

            /**
             * Iterator to past the end of our repositories.
             */
            RepositoryIterator end_repositories() const
            {
                return _repos.end();
            }

            /**
             * Iterate over our package.keywords entries.
             */
            typedef std::vector<std::pair<PackageDepAtom::ConstPointer, KeywordName> >::const_iterator
                PackageKeywordsIterator;

            /**
             * Iterator to the start of the package.keywords entries for a
             * particular package.
             */
            PackageKeywordsIterator begin_package_keywords(const QualifiedPackageName & d) const
            {
                std::map<QualifiedPackageName, std::vector<
                    std::pair<PackageDepAtom::ConstPointer, KeywordName> > >::const_iterator r;
                if (_keywords.end() != ((r = _keywords.find(d))))
                    return r->second.begin();
                else
                    return _empty_keywords.begin();
            }

            /**
             * Iterator to past the end of the package.keywords entries for a
             * particular file.
             */
            PackageKeywordsIterator end_package_keywords(const QualifiedPackageName & d) const
            {
                std::map<QualifiedPackageName, std::vector<
                    std::pair<PackageDepAtom::ConstPointer, KeywordName> > >::const_iterator r;
                if (_keywords.end() != ((r = _keywords.find(d))))
                    return r->second.end();
                else
                    return _empty_keywords.end();
            }

            /**
             * Iterator over the default keywords entries.
             */
            typedef std::vector<KeywordName>::const_iterator DefaultKeywordsIterator;

            /**
             * Iterator to the start of our default keywords entries.
             */
            DefaultKeywordsIterator begin_default_keywords() const
            {
                return _default_keywords.begin();
            }

            /**
             * Iterator to past the end of our default keywords entries.
             */
            DefaultKeywordsIterator end_default_keywords() const
            {
                return _default_keywords.end();
            }

            /**
             * Iterator over user package masks.
             */
            typedef IndirectIterator<std::vector<PackageDepAtom::ConstPointer>::const_iterator,
                    const PackageDepAtom> UserMasksIterator;

            /**
             * Iterator to the start of the user package masks.
             */
            UserMasksIterator begin_user_masks(const QualifiedPackageName & d) const
            {
                std::map<QualifiedPackageName, std::vector<PackageDepAtom::ConstPointer> >::const_iterator r;
                if (_user_masks.end() != ((r = _user_masks.find(d))))
                    return r->second.begin();
                else
                    return _empty_masks.begin();
            }

            /**
             * Iterator to past the end of the user package masks.
             */
            UserMasksIterator end_user_masks(const QualifiedPackageName & d) const
            {
                std::map<QualifiedPackageName, std::vector<PackageDepAtom::ConstPointer> >::const_iterator r;
                if (_user_masks.end() != ((r = _user_masks.find(d))))
                    return r->second.end();
                else
                    return _empty_masks.end();
            }

            /**
             * Iterator over the user package unmasks.
             */
            typedef IndirectIterator<std::vector<PackageDepAtom::ConstPointer>::const_iterator,
                    const PackageDepAtom> UserUnmasksIterator;

            /**
             * Iterator to the start of the user package unmasks.
             */
            UserUnmasksIterator begin_user_unmasks(const QualifiedPackageName & d) const
            {
                std::map<QualifiedPackageName, std::vector<PackageDepAtom::ConstPointer> >::const_iterator r;
                if (_user_unmasks.end() != ((r = _user_unmasks.find(d))))
                    return r->second.begin();
                else
                    return _empty_masks.begin();
            }

            /**
             * Iterator to past the end of the user package unmasks.
             */
            UserUnmasksIterator end_user_unmasks(const QualifiedPackageName & d) const
            {
                std::map<QualifiedPackageName, std::vector<PackageDepAtom::ConstPointer> >::const_iterator r;
                if (_user_unmasks.end() != ((r = _user_unmasks.find(d))))
                    return r->second.end();
                else
                    return _empty_masks.end();
            }

            /**
             * Iterator to the start of the use configuration.
             */
            typedef std::vector<UseConfigEntry>::const_iterator UseConfigIterator;

            /**
             * Iterator to the start of the use configuration for a particular
             * package.
             */
            UseConfigIterator begin_use_config(const QualifiedPackageName & q) const
            {
                std::map<QualifiedPackageName, std::vector<UseConfigEntry> >::const_iterator r;
                if (_use.end() != ((r = _use.find(q))))
                    return r->second.begin();
                else
                    return _empty_use.begin();
            }

            /**
             * Iterator to past the end of the use configuration for a
             * particular package.
             */
            UseConfigIterator end_use_config(const QualifiedPackageName & q) const
            {
                std::map<QualifiedPackageName, std::vector<UseConfigEntry> >::const_iterator r;
                if (_use.end() != ((r = _use.find(q))))
                    return r->second.end();
                else
                    return _empty_use.end();
            }

            /**
             * Iterator over the default use settings.
             */
            typedef std::vector<std::pair<UseFlagName, UseFlagState> >::const_iterator DefaultUseIterator;

            /**
             * Iterator to the start of the default use settings.
             */
            DefaultUseIterator begin_default_use() const
            {
                return _default_use.begin();
            }

            /**
             * Iterator to past the end of the default use settings.
             */
            DefaultUseIterator end_default_use() const
            {
                return _default_use.end();
            }

            /**
             * Our bashrc files.
             */
            std::string bashrc_files() const;
    };
}

#endif
