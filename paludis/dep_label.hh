/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_LABEL_HH
#define PALUDIS_GUARD_PALUDIS_DEP_LABEL_HH 1

#include <paludis/dep_label-fwd.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/attributes.hh>

/** \file
 * Declarations for dependency label-related classes.
 *
 * \ingroup g_dep_spec
 *
 * \section Examples
 *
 * - \ref example_dep_label.cc "example_dep_label.cc"
 */

namespace paludis
{
    /**
     * Types for visiting a URI label.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    struct URILabelVisitorTypes :
        VisitorTypes<
            URILabelVisitorTypes,
            URILabel,
            URIMirrorsThenListedLabel,
            URIMirrorsOnlyLabel,
            URIListedOnlyLabel,
            URIListedThenMirrorsLabel,
            URILocalMirrorsOnlyLabel,
            URIManualOnlyLabel
        >
    {
    };

    /**
     * Types for visiting a dependency label.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    struct DependencyLabelVisitorTypes :
        VisitorTypes<
            DependencyLabelVisitorTypes,
            DependencyLabel,
            DependencySystemLabel,
            DependencyTypeLabel,
            DependencySuggestLabel,
            DependencyABIsLabel
        >
    {
    };

    /**
     * Types for visiting a dependency system label.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    struct DependencySystemLabelVisitorTypes :
        VisitorTypes<
            DependencySystemLabelVisitorTypes,
            DependencySystemLabel,
            DependencyHostLabel,
            DependencyTargetLabel
        >
    {
    };

    /**
     * Types for visiting a dependency type label.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    struct DependencyTypeLabelVisitorTypes :
        VisitorTypes<
            DependencyTypeLabelVisitorTypes,
            DependencyTypeLabel,
            DependencyBuildLabel,
            DependencyRunLabel,
            DependencyInstallLabel,
            DependencyCompileLabel
        >
    {
    };

    /**
     * Types for visiting a dependency suggests label.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    struct DependencySuggestLabelVisitorTypes :
        VisitorTypes<
            DependencySuggestLabelVisitorTypes,
            DependencySuggestLabel,
            DependencySuggestedLabel,
            DependencyRecommendedLabel,
            DependencyRequiredLabel
        >
    {
    };

    /**
     * Types for visiting a dependency abi label.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    struct DependencyABIsLabelVisitorTypes :
        VisitorTypes<
            DependencyABIsLabelVisitorTypes,
            DependencyABIsLabel,
            DependencyAnyLabel,
            DependencyMineLabel,
            DependencyPrimaryLabel,
            DependencyABILabel
        >
    {
    };

    /**
     * URI label base class.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE URILabel :
        private InstantiationPolicy<URILabel, instantiation_method::NonCopyableTag>,
        public virtual ConstAcceptInterface<URILabelVisitorTypes>
    {
        public:
            ///\name Basic operations
            ///\{

            virtual ~URILabel() = 0;

            ///\}

            /// Our text.
            virtual const std::string text() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A concrete URI label class.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    template <typename T_>
    class PALUDIS_VISIBLE ConcreteURILabel :
        public URILabel,
        public ConstAcceptInterfaceVisitsThis<URILabelVisitorTypes, ConcreteURILabel<T_> >,
        private PrivateImplementationPattern<ConcreteURILabel<T_> >
    {
        private:
            using PrivateImplementationPattern<ConcreteURILabel<T_> >::_imp;

        public:
            ///\name Basic operations
            ///\{

            ConcreteURILabel(const std::string &);
            ~ConcreteURILabel();

            ///\}

            virtual const std::string text() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /// Convenience typedef alias to obtain our tag.
            typedef T_ Tag;
    };

    /**
     * Dependency label base class.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DependencyLabel :
        private InstantiationPolicy<DependencyLabel, instantiation_method::NonCopyableTag>,
        public virtual ConstAcceptInterface<DependencyLabelVisitorTypes>
    {
        public:
            ///\name Basic operations
            ///\{

            virtual ~DependencyLabel() = 0;

            ///\}

            /// Our text.
            virtual const std::string text() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * System dependency label base class.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    struct PALUDIS_VISIBLE DependencySystemLabel :
        public DependencyLabel,
        public ConstAcceptInterfaceVisitsThis<DependencyLabelVisitorTypes, DependencySystemLabel>,
        public virtual ConstAcceptInterface<DependencySystemLabelVisitorTypes>
    {
        /// Convenience alias for our visitor types.
        typedef DependencySystemLabelVisitorTypes VisitorTypes;
    };

    /**
     * Type dependency label base class.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    struct PALUDIS_VISIBLE DependencyTypeLabel :
        public DependencyLabel,
        public ConstAcceptInterfaceVisitsThis<DependencyLabelVisitorTypes, DependencyTypeLabel>,
        public virtual ConstAcceptInterface<DependencyTypeLabelVisitorTypes>
    {
        /// Convenience alias for our visitor types.
        typedef DependencyTypeLabelVisitorTypes VisitorTypes;
    };

    /**
     * Suggest dependency label base class.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    struct PALUDIS_VISIBLE DependencySuggestLabel :
        public DependencyLabel,
        public ConstAcceptInterfaceVisitsThis<DependencyLabelVisitorTypes, DependencySuggestLabel>,
        public virtual ConstAcceptInterface<DependencySuggestLabelVisitorTypes>
    {
        /// Convenience alias for our visitor types.
        typedef DependencySuggestLabelVisitorTypes VisitorTypes;
    };

    /**
     * ABI dependency label base class.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    struct PALUDIS_VISIBLE DependencyABIsLabel :
        public DependencyLabel,
        public ConstAcceptInterfaceVisitsThis<DependencyLabelVisitorTypes, DependencyABIsLabel>,
        public virtual ConstAcceptInterface<DependencyABIsLabelVisitorTypes>
    {
        /// Convenience alias for our visitor types.
        typedef DependencyABIsLabelVisitorTypes VisitorTypes;
    };

    /**
     * A concrete dependency label class.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    template <typename T_, typename C_>
    class PALUDIS_VISIBLE ConcreteDependencyLabel :
        public C_,
        public ConstAcceptInterfaceVisitsThis<typename C_::VisitorTypes, ConcreteDependencyLabel<T_, C_> >,
        private PrivateImplementationPattern<ConcreteDependencyLabel<T_, C_> >
    {
        private:
            using PrivateImplementationPattern<ConcreteDependencyLabel<T_, C_> >::_imp;

        public:
            ///\name Basic operations
            ///\{

            ConcreteDependencyLabel(const std::string &);
            ~ConcreteDependencyLabel();

            ///\}

            virtual const std::string text() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /// Convenience typedef alias to obtain our tag.
            typedef T_ Tag;
    };
}

#endif
