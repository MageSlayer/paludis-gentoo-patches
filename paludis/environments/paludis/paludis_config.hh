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

#include <paludis/dep_spec.hh>
#include <paludis/name.hh>
#include <paludis/util/collection.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/sr.hh>
#include <paludis/environments/environment_maker.hh>

#include <string>

/** \file
 * Declarations for the PaludisConfig class and related utilities.
 *
 * \ingroup grppaludisconfig
 */

namespace paludis
{
    struct EnvironmentMirrorIteratorTag;
    struct PaludisEnvironment;

    /**
     * Iterate over environment mirrors.
     *
     * \see Environment
     * \ingroup grpenvironment
     */
    typedef libwrapiter::ForwardIterator<EnvironmentMirrorIteratorTag,
            const std::pair<const std::string, std::string> > EnvironmentMirrorIterator;

    /**
     * A PaludisConfigError is thrown if a configuration error is encountered
     * by PaludisConfig.
     *
     * \ingroup grpexceptions
     * \ingroup grppaludisconfig
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE PaludisConfigError : public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            PaludisConfigError(const std::string & msg) throw ();
    };

    /**
     * Thrown if the config directory cannot be found by PaludisConfig.
     *
     * \ingroup grpexceptions
     * \ingroup grppaludisconfig
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE PaludisConfigNoDirectoryError :
        public PaludisConfigError,
        public FallBackToAnotherMakerError
    {
        public:
            ///\name Basic operations
            ///\{

            PaludisConfigNoDirectoryError(const std::string & msg) throw ();

            ///\}
    };

#include <paludis/environments/paludis/use_config_entry-sr.hh>
#include <paludis/environments/paludis/repository_config_entry-sr.hh>

    /**
     * PaludisConfig is used by PaludisEnvironment to access the user's
     * configuration settings from on-disk configuration files.
     *
     * \ingroup grppaludisconfig
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE PaludisConfig :
        private PrivateImplementationPattern<PaludisConfig>
    {
        public:
            ///\name Basic operations
            ///\{

            PaludisConfig(PaludisEnvironment * const, const std::string & suffix);

            ~PaludisConfig();

            ///\}

            ///\name Iterate over our repositories
            ///\{

            typedef libwrapiter::ForwardIterator<PaludisConfig, const RepositoryConfigEntry> RepositoryIterator;

            RepositoryIterator begin_repositories() const;

            RepositoryIterator end_repositories() const;

            ///\}

            ///\name Iterate over our default, set and per-package keywords
            ///\{

            typedef libwrapiter::ForwardIterator<PaludisConfig,
                    const std::pair<std::tr1::shared_ptr<const PackageDepSpec>, KeywordName> > PackageKeywordsIterator;

            PackageKeywordsIterator begin_package_keywords(const QualifiedPackageName & d) const;

            PackageKeywordsIterator end_package_keywords(const QualifiedPackageName & d) const;

            typedef libwrapiter::ForwardIterator<PaludisConfig, const KeywordName> DefaultKeywordsIterator;

            DefaultKeywordsIterator begin_default_keywords() const;

            DefaultKeywordsIterator end_default_keywords() const;

            typedef libwrapiter::ForwardIterator<PaludisConfig, SetKeywordConfigEntry> SetKeywordsIterator;

            SetKeywordsIterator begin_set_keywords() const;
            SetKeywordsIterator end_set_keywords() const;

            ///\}

            ///\name Iterate over our default, set and per-package licenses
            ///\{

            typedef libwrapiter::ForwardIterator<PaludisConfig,
                    const std::pair<std::tr1::shared_ptr<const PackageDepSpec>, std::string> > PackageLicensesIterator;

            PackageLicensesIterator begin_package_licenses(const QualifiedPackageName & d) const;

            PackageLicensesIterator end_package_licenses(const QualifiedPackageName & d) const;

            typedef libwrapiter::ForwardIterator<PaludisConfig, const std::string> DefaultLicensesIterator;

            DefaultLicensesIterator begin_default_licenses() const;

            DefaultLicensesIterator end_default_licenses() const;

            typedef libwrapiter::ForwardIterator<PaludisConfig, SetLicenseConfigEntry> SetLicensesIterator;

            SetLicensesIterator begin_set_licenses() const;
            SetLicensesIterator end_set_licenses() const;

            ///\}

            ///\name Iterate over our masks and unmasks
            ///\{

            typedef libwrapiter::ForwardIterator<PaludisConfig, const PackageDepSpec> UserMasksIterator;

            UserMasksIterator begin_user_masks(const QualifiedPackageName & d) const;

            UserMasksIterator end_user_masks(const QualifiedPackageName & d) const;

            typedef libwrapiter::ForwardIterator<PaludisConfig, const PackageDepSpec> UserUnmasksIterator;

            UserUnmasksIterator begin_user_unmasks(const QualifiedPackageName & d) const;

            UserUnmasksIterator end_user_unmasks(const QualifiedPackageName & d) const;

            typedef libwrapiter::ForwardIterator<PaludisConfig, SetMaskConfigEntry> UserMasksSetsIterator;

            UserMasksSetsIterator begin_user_masks_sets() const;
            UserMasksSetsIterator end_user_masks_sets() const;
            UserMasksSetsIterator begin_user_unmasks_sets() const;
            UserMasksSetsIterator end_user_unmasks_sets() const;

            ///\}

            ///\name Iterate over our default and per-package and per-set use flags
            ///\{

            typedef libwrapiter::ForwardIterator<PaludisConfig, const UseConfigEntry> UseConfigIterator;

            UseConfigIterator begin_use_config(const QualifiedPackageName & q) const;

            UseConfigIterator end_use_config(const QualifiedPackageName & q) const;

            typedef libwrapiter::ForwardIterator<PaludisConfig,
                    const std::pair<UseFlagName, UseFlagState> > DefaultUseIterator;

            DefaultUseIterator begin_default_use() const;

            DefaultUseIterator end_default_use() const;

            typedef libwrapiter::ForwardIterator<PaludisConfig, const std::string> UseMinusStarIterator;

            UseMinusStarIterator begin_use_prefixes_with_minus_star() const;
            UseMinusStarIterator end_use_prefixes_with_minus_star() const;

            typedef libwrapiter::ForwardIterator<PaludisConfig,
                    const std::pair<std::tr1::shared_ptr<const PackageDepSpec>, std::string> > PackageUseMinusStarIterator;

            PackageUseMinusStarIterator begin_package_use_prefixes_with_minus_star(const QualifiedPackageName &) const;
            PackageUseMinusStarIterator end_package_use_prefixes_with_minus_star(const QualifiedPackageName &) const;

            typedef libwrapiter::ForwardIterator<PaludisConfig, const SetUseConfigEntry> SetUseConfigIterator;

            SetUseConfigIterator begin_set_use_config() const;
            SetUseConfigIterator end_set_use_config() const;

            typedef libwrapiter::ForwardIterator<PaludisConfig, const SetUseConfigMinusStarEntry> SetUseMinusStarIterator;

            SetUseMinusStarIterator begin_set_use_prefixes_with_minus_star() const;
            SetUseMinusStarIterator end_set_use_prefixes_with_minus_star() const;

            ///\}

            /**
             * Our bashrc files.
             */
            std::string bashrc_files() const;

            /**
             * The ROOT.
             */
            std::string root() const;

            /**
             * Whether it's ok to unmask things that break Portage.
             */
            bool accept_breaks_portage() const;

            ///\name Userpriv
            ///\{

            uid_t reduced_uid() const;
            gid_t reduced_gid() const;
            std::string reduced_username() const;

            ///\}

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
