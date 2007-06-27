/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENT_IMPLEMENTATION_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENT_IMPLEMENTATION_HH 1

#include <paludis/environment.hh>
#include <paludis/eapi-fwd.hh>
#include <paludis/package_id-fwd.hh>

namespace paludis
{
    /**
     * Simplifies implementing the Environment interface.
     *
     * \ingroup grpenvironment
     * \see Environment
     */
    class PALUDIS_VISIBLE EnvironmentImplementation :
        public Environment
    {
        protected:
            ///\name Mask reasons

            /**
             * Do we accept a particular EAPI?
             *
             * Default behaviour: recognised EAPIs accepted.
             */
            virtual bool accept_eapi(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Accept packages with versions, EAPIs etc that will break Portage?
             *
             * Default behaviour: true.
             */
            virtual bool accept_breaks_portage(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Is a package masked by user settings?
             *
             * Default behaviour: false.
             */
            virtual bool masked_by_user(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Is a package unmasked by user settings?
             *
             * Default behaviour: false.
             */
            virtual bool unmasked_by_user(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Does a package break Portage?
             */
            virtual bool breaks_portage(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            ///\name Package sets
            ///\{

            /**
             * Return the environment-specific named set, or a zero pointer if no such set is available.
             */
            virtual tr1::shared_ptr<SetSpecTree::ConstItem> local_set(const SetName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

        public:
            ///\name Basic operations
            ///\{

            virtual ~EnvironmentImplementation() = 0;

            ///\}

            virtual bool query_use(const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const UseFlagNameCollection> known_use_expand_names(
                    const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual MaskReasons mask_reasons(const PackageID &,
                    const MaskReasonsOptions & = MaskReasonsOptions()) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool accept_license(const std::string &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool accept_keywords(tr1::shared_ptr<const KeywordNameCollection>, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const FSEntryCollection> bashrc_files() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const FSEntryCollection> syncers_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const FSEntryCollection> fetchers_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const FSEntryCollection> hook_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const FSEntry root() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual uid_t reduced_uid() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual gid_t reduced_gid() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const MirrorsCollection> mirrors(const std::string &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const SetNameCollection> set_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<SetSpecTree::ConstItem> set(const SetName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const DestinationsCollection> default_destinations() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual HookResult perform_hook(const Hook &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string default_distribution() const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
