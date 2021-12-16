/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_CREATE_OUTPUT_MANAGER_INFO_HH
#define PALUDIS_GUARD_PALUDIS_CREATE_OUTPUT_MANAGER_INFO_HH 1

#include <paludis/create_output_manager_info-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/type_list.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/serialise-fwd.hh>
#include <paludis/action-fwd.hh>
#include <paludis/name-fwd.hh>

namespace paludis
{
    /**
     * Information for Environment::create_output_manager.
     *
     * \since 0.36
     * \ingroup g_environment
     * \see Environment::create_output_manager
     */
    class PALUDIS_VISIBLE CreateOutputManagerInfo :
        public virtual DeclareAbstractAcceptMethods<CreateOutputManagerInfo, MakeTypeList<
            CreateOutputManagerForPackageIDActionInfo,
            CreateOutputManagerForRepositorySyncInfo
        >::Type>
    {
        public:
            virtual ~CreateOutputManagerInfo() = default;

            static const std::shared_ptr<CreateOutputManagerInfo> deserialise(
                    Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void serialise(Serialiser &) const = 0;
    };

    /**
     * Information for Environment::create_output_manager, if we're performing a
     * PackageID action.
     *
     * \since 0.36
     * \ingroup g_environment
     * \see Environment::create_output_manager
     */
    class PALUDIS_VISIBLE CreateOutputManagerForPackageIDActionInfo :
        public CreateOutputManagerInfo,
        public ImplementAcceptMethods<CreateOutputManagerInfo, CreateOutputManagerForPackageIDActionInfo>
    {
        private:
            Pimp<CreateOutputManagerForPackageIDActionInfo> _imp;

        public:
            /**
             * \since 0.46
             */
            CreateOutputManagerForPackageIDActionInfo(
                    const std::shared_ptr<const PackageID> & id,
                    const std::string & action_name,
                    const std::shared_ptr<const Set<std::string> > & action_flags,
                    const OutputExclusivity output_exclusivity,
                    const ClientOutputFeatures & output_features);

            /**
             * \since 0.46
             */
            CreateOutputManagerForPackageIDActionInfo(
                    const std::shared_ptr<const PackageID> & id,
                    const Action &,
                    const OutputExclusivity output_exclusivity,
                    const ClientOutputFeatures & output_features);

            ~CreateOutputManagerForPackageIDActionInfo() override;

            const std::shared_ptr<const PackageID> package_id() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * \since 0.44
             */
            const std::string action_name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * \since 0.44
             */
            const std::shared_ptr<const Set<std::string> > action_flags() const PALUDIS_ATTRIBUTE((warn_unused_result));

            OutputExclusivity output_exclusivity() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * \since 0.46
             */
            const ClientOutputFeatures client_output_features() const PALUDIS_ATTRIBUTE((warn_unused_result));

            void serialise(Serialiser &) const override;

            static const std::shared_ptr<CreateOutputManagerForPackageIDActionInfo> deserialise(
                    Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * Information for Environment::create_output_manager, if we're performing a
     * Repository sync.
     *
     * \since 0.36
     * \ingroup g_environment
     * \see Environment::create_output_manager
     */
    class PALUDIS_VISIBLE CreateOutputManagerForRepositorySyncInfo :
        public CreateOutputManagerInfo,
        public ImplementAcceptMethods<CreateOutputManagerInfo, CreateOutputManagerForRepositorySyncInfo>
    {
        private:
            Pimp<CreateOutputManagerForRepositorySyncInfo> _imp;

        public:
            /**
             * \since 0.46
             */
            CreateOutputManagerForRepositorySyncInfo(
                    const RepositoryName & repo_name,
                    const OutputExclusivity,
                    const ClientOutputFeatures &);

            ~CreateOutputManagerForRepositorySyncInfo() override;

            /**
             * \since 0.44
             */
            const RepositoryName repository_name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            OutputExclusivity output_exclusivity() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * \since 0.46
             */
            const ClientOutputFeatures client_output_features() const PALUDIS_ATTRIBUTE((warn_unused_result));

            void serialise(Serialiser &) const override;

            static const std::shared_ptr<CreateOutputManagerForRepositorySyncInfo> deserialise(
                    Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
