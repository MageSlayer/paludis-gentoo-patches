/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/environment_factory.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/sequence-fwd.hh>
#include <paludis/util/map-fwd.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/fs_entry-fwd.hh>

#include <string>

/** \file
 * Declarations for the PaludisConfig class and related utilities.
 *
 * \ingroup grppaludisconfig
 */

namespace paludis
{
    struct PaludisEnvironment;

    namespace paludis_environment
    {
        struct KeywordsConf;
        struct UseConf;
        struct LicensesConf;
        struct PackageMaskConf;
        struct MirrorsConf;
        struct OutputConf;
        struct World;

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
            public FallBackToAnotherFormatError
        {
            public:
                ///\name Basic operations
                ///\{

                PaludisConfigNoDirectoryError(const std::string & msg) throw ();

                ///\}
        };

        /**
         * PaludisConfig is used by PaludisEnvironment to access the user's
         * configuration settings from on-disk configuration files.
         *
         * \ingroup grppaludisconfig
         * \nosubgrouping
         */
        class PaludisConfig :
            private Pimp<PaludisConfig>
        {
            public:
                ///\name Basic operations
                ///\{

                PaludisConfig(PaludisEnvironment * const, const std::string & suffix);

                ~PaludisConfig();

                ///\}

                ///\name Config files
                ///\{

                std::shared_ptr<const KeywordsConf> keywords_conf() const;
                std::shared_ptr<const UseConf> use_conf() const;
                std::shared_ptr<const LicensesConf> licenses_conf() const;
                std::shared_ptr<const PackageMaskConf> package_mask_conf() const;
                std::shared_ptr<const PackageMaskConf> package_unmask_conf() const;
                std::shared_ptr<const MirrorsConf> mirrors_conf() const;
                std::shared_ptr<const World> world() const;
                std::shared_ptr<const OutputConf> output_conf() const;

                ///\}

                ///\name Iterate over our repositories
                ///\{

                struct RepositoryConstIteratorTag;
                typedef WrappedForwardIterator<RepositoryConstIteratorTag, const std::function<std::string (const std::string &)> >
                    RepositoryConstIterator;

                RepositoryConstIterator begin_repositories() const;

                RepositoryConstIterator end_repositories() const;

                const std::function<std::string (const std::string &)> repo_func_from_file(const FSEntry &);

                ///\}

                /**
                 * Our bashrc files.
                 */
                std::shared_ptr<const FSEntrySequence> bashrc_files() const;

                /**
                 * The ROOT.
                 */
                std::string root() const;

                /**
                 * Whether it's ok to unmask things that break Portage, regardless of the reason.
                 */
                bool accept_all_breaks_portage() const;

                /**
                 * Specific reasons why a package might break Portage that it's ok to ignore.
                 */
                const Set<std::string> & accept_breaks_portage() const;

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

                /**
                 * The distribution.
                 */
                std::string distribution() const;
        };
    }
}

#endif
