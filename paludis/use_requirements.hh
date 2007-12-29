/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_USE_REQUIREMENTS_HH
#define PALUDIS_GUARD_PALUDIS_USE_REQUIREMENTS_HH 1

#include <paludis/use_requirements-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/name.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/wrapped_output_iterator.hh>

namespace paludis
{
    /**
     * A selection of USE flag requirements.
     *
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE UseRequirements :
        private PrivateImplementationPattern<UseRequirements>
    {
        public:
            ///\name Basic operations
            ///\{

            UseRequirements();
            UseRequirements(const UseRequirements &);
            ~UseRequirements();

            ///\}

            ///\name Iterate over our USE requirements
            ///\{

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag, const tr1::shared_ptr<const UseRequirement> > ConstIterator;

            ConstIterator begin() const;
            ConstIterator end() const;

            ///\}

            /// Find the requirement for a particular USE flag.
            ConstIterator find(const UseFlagName & u) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /// Insert a new requirement.
            void insert(const tr1::shared_ptr<const UseRequirement> &);

            /// Are we empty?
            bool empty() const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * Visitor types for a UseRequirement.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    struct UseRequirementVisitorTypes :
        VisitorTypes<
            UseRequirementVisitorTypes,
            UseRequirement,
            EnabledUseRequirement,
            DisabledUseRequirement,
            EqualUseRequirement,
            NotEqualUseRequirement
        >
    {
    };

    /**
     * A requirement for a use flag.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     */
    class PALUDIS_VISIBLE UseRequirement :
        public virtual ConstAcceptInterface<UseRequirementVisitorTypes>
    {
        public:
            virtual ~UseRequirement() = 0;

            /// Our use flag.
            virtual const UseFlagName flag() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * An enabled requirement for a use flag.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     */
    class PALUDIS_VISIBLE EnabledUseRequirement :
        public UseRequirement,
        public ConstAcceptInterfaceVisitsThis<UseRequirementVisitorTypes, EnabledUseRequirement>
    {
        private:
            const UseFlagName _name;

        public:
            ///\name Basic operations
            ///\{

            EnabledUseRequirement(const UseFlagName &);
            ~EnabledUseRequirement();

            ///\}

            virtual const UseFlagName flag() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A disabled requirement for a use flag.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     */
    class PALUDIS_VISIBLE DisabledUseRequirement :
        public UseRequirement,
        public ConstAcceptInterfaceVisitsThis<UseRequirementVisitorTypes, DisabledUseRequirement>
    {
        private:
            const UseFlagName _name;

        public:
            ///\name Basic operations
            ///\{

            DisabledUseRequirement(const UseFlagName &);
            ~DisabledUseRequirement();

            ///\}

            const UseFlagName flag() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * An equal requirement for a use flag.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     */
    class PALUDIS_VISIBLE EqualUseRequirement :
        public UseRequirement,
        public ConstAcceptInterfaceVisitsThis<UseRequirementVisitorTypes, EqualUseRequirement>
    {
        private:
            const UseFlagName _name;
            const tr1::shared_ptr<const PackageID> _id;

        public:
            ///\name Basic operations
            ///\{

            EqualUseRequirement(const UseFlagName &, const tr1::shared_ptr<const PackageID> &);
            ~EqualUseRequirement();

            ///\}

            const UseFlagName flag() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /// Our package.
            const tr1::shared_ptr<const PackageID> package_id() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A not equal requirement for a use flag.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     */
    class PALUDIS_VISIBLE NotEqualUseRequirement :
        public UseRequirement,
        public ConstAcceptInterfaceVisitsThis<UseRequirementVisitorTypes, NotEqualUseRequirement>
    {
        private:
            const UseFlagName _name;
            const tr1::shared_ptr<const PackageID> _id;

        public:
            ///\name Basic operations
            ///\{

            NotEqualUseRequirement(const UseFlagName &, const tr1::shared_ptr<const PackageID> &);
            ~NotEqualUseRequirement();

            ///\}

            const UseFlagName flag() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /// Our package.
            const tr1::shared_ptr<const PackageID> package_id() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
