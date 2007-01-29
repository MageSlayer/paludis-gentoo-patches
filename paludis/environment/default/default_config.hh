/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/util/collection.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/sr.hh>

#include <string>

/** \file
 * Declarations for the DefaultConfig class and related utilities.
 *
 * \ingroup grpdefaultconfig
 */

namespace paludis
{
    struct EnvironmentMirrorIteratorTag;

    /**
     * Iterate over environment mirrors.
     *
     * \see Environment
     * \ingroup grpenvironment
     */
    typedef libwrapiter::ForwardIterator<EnvironmentMirrorIteratorTag,
            const std::pair<const std::string, std::string> > EnvironmentMirrorIterator;

    /**
     * A DefaultConfigError is thrown if a configuration error is encountered
     * by DefaultConfig.
     *
     * \ingroup grpexceptions
     * \ingroup grpdefaultconfig
     */
    class PALUDIS_VISIBLE DefaultConfigError : public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            DefaultConfigError(const std::string & msg) throw ();
    };

#include <paludis/environment/default/use_config_entry-sr.hh>
#include <paludis/environment/default/repository_config_entry-sr.hh>

    /**
     * DefaultConfig is used by DefaultEnvironment to access the user's
     * configuration settings from on-disk configuration files.
     *
     * \ingroup grpdefaultconfig
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DefaultConfig :
        public InstantiationPolicy<DefaultConfig, instantiation_method::SingletonTag>,
        private PrivateImplementationPattern<DefaultConfig>
    {
        friend class InstantiationPolicy<DefaultConfig, instantiation_method::SingletonTag>;

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
            ///\{

            typedef libwrapiter::ForwardIterator<DefaultConfig, const RepositoryConfigEntry> RepositoryIterator;

            RepositoryIterator begin_repositories() const;

            RepositoryIterator end_repositories() const;

            ///\}

            ///\name Iterate over our default, set and per-package keywords
            ///\{

            typedef libwrapiter::ForwardIterator<DefaultConfig,
                    const std::pair<std::tr1::shared_ptr<const PackageDepAtom>, KeywordName> > PackageKeywordsIterator;

            PackageKeywordsIterator begin_package_keywords(const QualifiedPackageName & d) const;

            PackageKeywordsIterator end_package_keywords(const QualifiedPackageName & d) const;

            typedef libwrapiter::ForwardIterator<DefaultConfig, const KeywordName> DefaultKeywordsIterator;

            DefaultKeywordsIterator begin_default_keywords() const;

            DefaultKeywordsIterator end_default_keywords() const;

            typedef libwrapiter::ForwardIterator<DefaultConfig, SetKeywordConfigEntry> SetKeywordsIterator;

            SetKeywordsIterator begin_set_keywords() const;
            SetKeywordsIterator end_set_keywords() const;

            ///\}

            ///\name Iterate over our default, set and per-package licenses
            ///\{

            typedef libwrapiter::ForwardIterator<DefaultConfig,
                    const std::pair<std::tr1::shared_ptr<const PackageDepAtom>, std::string> > PackageLicensesIterator;

            PackageLicensesIterator begin_package_licenses(const QualifiedPackageName & d) const;

            PackageLicensesIterator end_package_licenses(const QualifiedPackageName & d) const;

            typedef libwrapiter::ForwardIterator<DefaultConfig, const std::string> DefaultLicensesIterator;

            DefaultLicensesIterator begin_default_licenses() const;

            DefaultLicensesIterator end_default_licenses() const;

            typedef libwrapiter::ForwardIterator<DefaultConfig, SetLicenseConfigEntry> SetLicensesIterator;

            SetLicensesIterator begin_set_licenses() const;
            SetLicensesIterator end_set_licenses() const;

            ///\}

            ///\name Iterate over our masks and unmasks
            ///\{

            typedef libwrapiter::ForwardIterator<DefaultConfig, const PackageDepAtom> UserMasksIterator;

            UserMasksIterator begin_user_masks(const QualifiedPackageName & d) const;

            UserMasksIterator end_user_masks(const QualifiedPackageName & d) const;

            typedef libwrapiter::ForwardIterator<DefaultConfig, const PackageDepAtom> UserUnmasksIterator;

            UserUnmasksIterator begin_user_unmasks(const QualifiedPackageName & d) const;

            UserUnmasksIterator end_user_unmasks(const QualifiedPackageName & d) const;

            typedef libwrapiter::ForwardIterator<DefaultConfig, SetMaskConfigEntry> UserMasksSetsIterator;

            UserMasksSetsIterator begin_user_masks_sets() const;
            UserMasksSetsIterator end_user_masks_sets() const;
            UserMasksSetsIterator begin_user_unmasks_sets() const;
            UserMasksSetsIterator end_user_unmasks_sets() const;

            ///\}

            ///\name Iterate over our default and per-package and per-set use flags
            ///\{

            typedef libwrapiter::ForwardIterator<DefaultConfig, const UseConfigEntry> UseConfigIterator;

            UseConfigIterator begin_use_config(const QualifiedPackageName & q) const;

            UseConfigIterator end_use_config(const QualifiedPackageName & q) const;

            typedef libwrapiter::ForwardIterator<DefaultConfig,
                    const std::pair<UseFlagName, UseFlagState> > DefaultUseIterator;

            DefaultUseIterator begin_default_use() const;

            DefaultUseIterator end_default_use() const;

            typedef libwrapiter::ForwardIterator<DefaultConfig, const std::string> UseMinusStarIterator;

            UseMinusStarIterator begin_use_prefixes_with_minus_star() const;
            UseMinusStarIterator end_use_prefixes_with_minus_star() const;

            typedef libwrapiter::ForwardIterator<DefaultConfig,
                    const std::pair<std::tr1::shared_ptr<const PackageDepAtom>, std::string> > PackageUseMinusStarIterator;

            PackageUseMinusStarIterator begin_package_use_prefixes_with_minus_star(const QualifiedPackageName &) const;
            PackageUseMinusStarIterator end_package_use_prefixes_with_minus_star(const QualifiedPackageName &) const;

            typedef libwrapiter::ForwardIterator<DefaultConfig, const SetUseConfigEntry> SetUseConfigIterator;

            SetUseConfigIterator begin_set_use_config() const;
            SetUseConfigIterator end_set_use_config() const;

            typedef libwrapiter::ForwardIterator<DefaultConfig, const SetUseConfigMinusStarEntry> SetUseMinusStarIterator;

            SetUseMinusStarIterator begin_set_use_prefixes_with_minus_star() const;
            SetUseMinusStarIterator end_set_use_prefixes_with_minus_star() const;

            ///\}

            /**
             * Add a forced USE flag to this configuration.
             */
            void add_forced_use_config(const UseConfigEntry & e);

            /**
             * Clear all forced USE flags.
             */
            void clear_forced_use_config();

            ///\name Iterate over our forced USE flags
            ///\{
            UseConfigIterator begin_forced_use_config() const;
            UseConfigIterator end_forced_use_config() const;
            ///\}


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

            typedef EnvironmentMirrorIterator MirrorIterator;

            MirrorIterator begin_mirrors(const std::string & m) const;

            MirrorIterator end_mirrors(const std::string & m) const;

            ///\}
    };
}

#endif
