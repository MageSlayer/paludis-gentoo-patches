/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/smart_record.hh>

#include <map>
#include <vector>
#include <string>

/** \file
 * Declarations for the DefaultConfig class and related utilities.
 *
 * \ingroup grpdefaultconfig
 */

namespace paludis
{
    /**
     * A DefaultConfigError is thrown if a configuration error is encountered
     * by DefaultConfig.
     *
     * \ingroup grpexceptions
     * \ingroup grpdefaultconfig
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
     *
     * \see RepositoryConfigEntry
     *
     * \ingroup grpdefaultconfig
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
     *
     * \see RepositoryConfigEntry
     *
     * \ingroup grpdefaultconfig
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
     *
     * \ingroup grpdefaultconfig
     */
    typedef MakeSmartRecord<RepositoryConfigEntryTag>::Type RepositoryConfigEntry;

    /**
     * Keys for UseConfigEntry.
     *
     * \see UseConfigEntry
     *
     * \ingroup grpdefaultconfig
     */
    enum UseConfigEntryKeys
    {
        uce_dep_atom,       ///< A dependency atom
        uce_flag_name,      ///< The use flag name
        uce_flag_state      ///< The use flag state
    };

    /**
     * Tag for UseConfigEntry.
     *
     * \see UseConfigEntry
     *
     * \ingroup grpdefaultconfig
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
     * An entry in a DefaultConfig's use configuration data.
     *
     * \ingroup grpdefaultconfig
     */
    typedef MakeSmartRecord<UseConfigEntryTag>::Type UseConfigEntry;

    /**
     * DefaultConfig is used by DefaultEnvironment to access the user's
     * configuration settings from on-disk configuration files.
     *
     * \ingroup grpdefaultconfig
     */
    class DefaultConfig :
        public InstantiationPolicy<DefaultConfig, instantiation_method::SingletonAsNeededTag>,
        private PrivateImplementationPattern<DefaultConfig>
    {
        friend class InstantiationPolicy<DefaultConfig, instantiation_method::SingletonAsNeededTag>;

        private:
            DefaultConfig();

            ~DefaultConfig();

        public:
            /**
             * Set config suffix. Must be called before we do anything, or not
             * at all.
             */
            static void set_config_suffix(const std::string &);

            /**
             * Get config suffix.
             */
            static std::string config_suffix();

            ///\name Iterate over our repositories
            ///{

            typedef std::list<RepositoryConfigEntry>::const_iterator RepositoryIterator;

            RepositoryIterator begin_repositories() const;

            RepositoryIterator end_repositories() const;

            ///}

            ///\name Iterate over our default and per-package keywords
            ///{

            typedef std::vector<std::pair<PackageDepAtom::ConstPointer, KeywordName> >::const_iterator
                PackageKeywordsIterator;

            PackageKeywordsIterator begin_package_keywords(const QualifiedPackageName & d) const;

            PackageKeywordsIterator end_package_keywords(const QualifiedPackageName & d) const;

            typedef std::vector<KeywordName>::const_iterator DefaultKeywordsIterator;

            DefaultKeywordsIterator begin_default_keywords() const;

            DefaultKeywordsIterator end_default_keywords() const;

            ///}

            ///\name Iterate over our default and per-package licenses
            ///{

            typedef std::vector<std::pair<PackageDepAtom::ConstPointer, std::string> >::const_iterator
                PackageLicensesIterator;

            PackageLicensesIterator begin_package_licenses(const QualifiedPackageName & d) const;

            PackageLicensesIterator end_package_licenses(const QualifiedPackageName & d) const;

            typedef std::vector<std::string>::const_iterator DefaultLicensesIterator;

            DefaultLicensesIterator begin_default_licenses() const;

            DefaultLicensesIterator end_default_licenses() const;

            ///}

            ///\name Iterate over our masks and unmasks
            ///{

            typedef IndirectIterator<std::vector<PackageDepAtom::ConstPointer>::const_iterator,
                    const PackageDepAtom> UserMasksIterator;

            UserMasksIterator begin_user_masks(const QualifiedPackageName & d) const;

            UserMasksIterator end_user_masks(const QualifiedPackageName & d) const;

            typedef IndirectIterator<std::vector<PackageDepAtom::ConstPointer>::const_iterator,
                    const PackageDepAtom> UserUnmasksIterator;

            UserUnmasksIterator begin_user_unmasks(const QualifiedPackageName & d) const;

            UserUnmasksIterator end_user_unmasks(const QualifiedPackageName & d) const;

            ///}

            ///\name Iterate over our default and per-package use flags
            ///{

            typedef std::vector<UseConfigEntry>::const_iterator UseConfigIterator;

            UseConfigIterator begin_use_config(const QualifiedPackageName & q) const;

            UseConfigIterator end_use_config(const QualifiedPackageName & q) const;

            typedef std::vector<std::pair<UseFlagName, UseFlagState> >::const_iterator DefaultUseIterator;

            DefaultUseIterator begin_default_use() const;

            DefaultUseIterator end_default_use() const;

            ///}

            /**
             * Our bashrc files.
             */
            std::string bashrc_files() const;

            /**
             * The paludis command.
             */
            std::string paludis_command() const;

            /**
             * Set the paludis command.
             */
            void set_paludis_command(const std::string & s);

            /**
             * The ROOT.
             */
            std::string root() const;

            /**
             * The config directory.
             */
            std::string config_dir() const;

            ///\name Iterate over our mirrors
            ///\{

            typedef std::multimap<std::string, std::string>::const_iterator MirrorIterator;

            MirrorIterator begin_mirrors(const std::string & m) const;

            MirrorIterator end_mirrors(const std::string & m) const;

            ///\}
    };
}

#endif
