/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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
#include <paludis/dep_spec-fwd.hh>
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
 * - \ref example_dep_spec.cc "example_dep_spec.cc" (for specifications)
 * - \ref example_dep_tree.cc "example_dep_tree.cc" (for specification trees)
 * - \ref example_dep_tag.cc "example_dep_tag.cc" (for tags)
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
            DependencyPostLabel,
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

        typedef ConstAcceptInterface<DependencySystemLabelVisitorTypes>::Heirarchy Heirarchy;
        using ConstAcceptInterface<DependencySystemLabelVisitorTypes>::accept;
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

        typedef ConstAcceptInterface<DependencyTypeLabelVisitorTypes>::Heirarchy Heirarchy;
        using ConstAcceptInterface<DependencyTypeLabelVisitorTypes>::accept;
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

        typedef ConstAcceptInterface<DependencySuggestLabelVisitorTypes>::Heirarchy Heirarchy;
        using ConstAcceptInterface<DependencySuggestLabelVisitorTypes>::accept;
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

        typedef ConstAcceptInterface<DependencyABIsLabelVisitorTypes>::Heirarchy Heirarchy;
        using ConstAcceptInterface<DependencyABIsLabelVisitorTypes>::accept;
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

    /**
     * A collection of each dependency label type.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     */
    class PALUDIS_VISIBLE ActiveDependencyLabels :
        private PrivateImplementationPattern<ActiveDependencyLabels>
    {
        public:
            ///\name Basic operations
            ///\{

            ActiveDependencyLabels(const DependencyLabelsDepSpec &);
            ActiveDependencyLabels(const DependencyLabelSequence &);
            ActiveDependencyLabels(const ActiveDependencyLabels &);
            ActiveDependencyLabels(const ActiveDependencyLabels &, const DependencyLabelsDepSpec &);
            ~ActiveDependencyLabels();

            ActiveDependencyLabels & operator= (const ActiveDependencyLabels &);

            ///\}

            ///\name Current label selections
            ///\{

            const std::tr1::shared_ptr<const DependencySystemLabelSequence> system_labels() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::tr1::shared_ptr<const DependencyTypeLabelSequence> type_labels() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::tr1::shared_ptr<const DependencyABIsLabelSequence> abi_labels() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::tr1::shared_ptr<const DependencySuggestLabelSequence> suggest_labels() const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}
    };

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class ConstAcceptInterface<DependencyABIsLabelVisitorTypes>;
    extern template class ConstAcceptInterface<DependencyLabelVisitorTypes>;
    extern template class ConstAcceptInterface<DependencySuggestLabelVisitorTypes>;
    extern template class ConstAcceptInterface<DependencySystemLabelVisitorTypes>;
    extern template class ConstAcceptInterface<DependencyTypeLabelVisitorTypes>;
    extern template class ConstAcceptInterface<URILabelVisitorTypes>;

    extern template class ConstAcceptInterfaceVisitsThis<DependencyLabelVisitorTypes, DependencyABIsLabel>;
    extern template class ConstAcceptInterfaceVisitsThis<DependencyLabelVisitorTypes, DependencySuggestLabel>;
    extern template class ConstAcceptInterfaceVisitsThis<DependencyLabelVisitorTypes, DependencySystemLabel>;
    extern template class ConstAcceptInterfaceVisitsThis<DependencyLabelVisitorTypes, DependencyTypeLabel>;

    extern template class Visits<DependencyABIsLabel>;
    extern template class Visits<DependencySuggestLabel>;
    extern template class Visits<DependencySystemLabel>;
    extern template class Visits<DependencyTypeLabel>;

    extern template class ConstAcceptInterfaceVisitsThis<URILabelVisitorTypes, URIMirrorsThenListedLabel>;
    extern template class ConstAcceptInterfaceVisitsThis<URILabelVisitorTypes, URIMirrorsOnlyLabel>;
    extern template class ConstAcceptInterfaceVisitsThis<URILabelVisitorTypes, URIListedOnlyLabel>;
    extern template class ConstAcceptInterfaceVisitsThis<URILabelVisitorTypes, URIListedThenMirrorsLabel>;
    extern template class ConstAcceptInterfaceVisitsThis<URILabelVisitorTypes, URILocalMirrorsOnlyLabel>;
    extern template class ConstAcceptInterfaceVisitsThis<URILabelVisitorTypes, URIManualOnlyLabel>;

    extern template class Visits<URIMirrorsThenListedLabel>;
    extern template class Visits<URIMirrorsOnlyLabel>;
    extern template class Visits<URIListedOnlyLabel>;
    extern template class Visits<URIListedThenMirrorsLabel>;
    extern template class Visits<URILocalMirrorsOnlyLabel>;
    extern template class Visits<URIManualOnlyLabel>;

    extern template class ConstAcceptInterfaceVisitsThis<DependencySystemLabelVisitorTypes, DependencyHostLabel>;
    extern template class ConstAcceptInterfaceVisitsThis<DependencySystemLabelVisitorTypes, DependencyTargetLabel>;

    extern template class Visits<DependencySystemLabel>;
    extern template class Visits<DependencyHostLabel>;
    extern template class Visits<DependencyTargetLabel>;

    extern template class ConstAcceptInterfaceVisitsThis<DependencyTypeLabelVisitorTypes, DependencyBuildLabel>;
    extern template class ConstAcceptInterfaceVisitsThis<DependencyTypeLabelVisitorTypes, DependencyRunLabel>;
    extern template class ConstAcceptInterfaceVisitsThis<DependencyTypeLabelVisitorTypes, DependencyPostLabel>;
    extern template class ConstAcceptInterfaceVisitsThis<DependencyTypeLabelVisitorTypes, DependencyInstallLabel>;
    extern template class ConstAcceptInterfaceVisitsThis<DependencyTypeLabelVisitorTypes, DependencyCompileLabel>;

    extern template class Visits<DependencyBuildLabel>;
    extern template class Visits<DependencyRunLabel>;
    extern template class Visits<DependencyPostLabel>;
    extern template class Visits<DependencyInstallLabel>;
    extern template class Visits<DependencyCompileLabel>;

    extern template class ConstAcceptInterfaceVisitsThis<DependencySuggestLabelVisitorTypes, DependencySuggestedLabel>;
    extern template class ConstAcceptInterfaceVisitsThis<DependencySuggestLabelVisitorTypes, DependencyRecommendedLabel>;
    extern template class ConstAcceptInterfaceVisitsThis<DependencySuggestLabelVisitorTypes, DependencyRequiredLabel>;

    extern template class Visits<DependencySuggestedLabel>;
    extern template class Visits<DependencyRecommendedLabel>;
    extern template class Visits<DependencyRequiredLabel>;

    extern template class ConstAcceptInterfaceVisitsThis<DependencyABIsLabelVisitorTypes, DependencyAnyLabel>;
    extern template class ConstAcceptInterfaceVisitsThis<DependencyABIsLabelVisitorTypes, DependencyMineLabel>;
    extern template class ConstAcceptInterfaceVisitsThis<DependencyABIsLabelVisitorTypes, DependencyPrimaryLabel>;
    extern template class ConstAcceptInterfaceVisitsThis<DependencyABIsLabelVisitorTypes, DependencyABILabel>;

    extern template class Visits<DependencyAnyLabel>;
    extern template class Visits<DependencyMineLabel>;
    extern template class Visits<DependencyPrimaryLabel>;
    extern template class Visits<DependencyABILabel>;

    extern template class InstantiationPolicy<DependencyLabel, instantiation_method::NonCopyableTag>;
    extern template class InstantiationPolicy<URILabel, instantiation_method::NonCopyableTag>;

    extern template class ConcreteDependencyLabel<DependencyHostLabelTag, DependencySystemLabel>;
    extern template class ConcreteDependencyLabel<DependencyTargetLabelTag, DependencySystemLabel>;
    extern template class ConcreteDependencyLabel<DependencyBuildLabelTag, DependencyTypeLabel>;
    extern template class ConcreteDependencyLabel<DependencyRunLabelTag, DependencyTypeLabel>;
    extern template class ConcreteDependencyLabel<DependencyPostLabelTag, DependencyTypeLabel>;
    extern template class ConcreteDependencyLabel<DependencyInstallLabelTag, DependencyTypeLabel>;
    extern template class ConcreteDependencyLabel<DependencyCompileLabelTag, DependencyTypeLabel>;
    extern template class ConcreteDependencyLabel<DependencySuggestedLabelTag, DependencySuggestLabel>;
    extern template class ConcreteDependencyLabel<DependencyRecommendedLabelTag, DependencySuggestLabel>;
    extern template class ConcreteDependencyLabel<DependencyRequiredLabelTag, DependencySuggestLabel>;
    extern template class ConcreteDependencyLabel<DependencyAnyLabelTag, DependencyABIsLabel>;
    extern template class ConcreteDependencyLabel<DependencyMineLabelTag, DependencyABIsLabel>;
    extern template class ConcreteDependencyLabel<DependencyPrimaryLabelTag, DependencyABIsLabel>;
    extern template class ConcreteDependencyLabel<DependencyABILabelTag, DependencyABIsLabel>;

    extern template class ConcreteURILabel<URIMirrorsThenListedLabelTag>;
    extern template class ConcreteURILabel<URIMirrorsOnlyLabelTag>;
    extern template class ConcreteURILabel<URIListedOnlyLabelTag>;
    extern template class ConcreteURILabel<URIListedThenMirrorsLabelTag>;
    extern template class ConcreteURILabel<URILocalMirrorsOnlyLabelTag>;
    extern template class ConcreteURILabel<URIManualOnlyLabelTag>;

    extern template class PrivateImplementationPattern<ConcreteURILabel<URIMirrorsThenListedLabel> >;
    extern template class PrivateImplementationPattern<ConcreteURILabel<URIMirrorsOnlyLabel> >;
    extern template class PrivateImplementationPattern<ConcreteURILabel<URIListedOnlyLabel> >;
    extern template class PrivateImplementationPattern<ConcreteURILabel<URIListedThenMirrorsLabel> >;
    extern template class PrivateImplementationPattern<ConcreteURILabel<URILocalMirrorsOnlyLabel> >;
    extern template class PrivateImplementationPattern<ConcreteURILabel<URIManualOnlyLabel> >;

    extern template class PrivateImplementationPattern<ConcreteDependencyLabel<DependencyHostLabelTag, DependencySystemLabel> >;
    extern template class PrivateImplementationPattern<ConcreteDependencyLabel<DependencyTargetLabelTag, DependencySystemLabel> >;
    extern template class PrivateImplementationPattern<ConcreteDependencyLabel<DependencyBuildLabelTag, DependencyTypeLabel> >;
    extern template class PrivateImplementationPattern<ConcreteDependencyLabel<DependencyRunLabelTag, DependencyTypeLabel> >;
    extern template class PrivateImplementationPattern<ConcreteDependencyLabel<DependencyPostLabelTag, DependencyTypeLabel> >;
    extern template class PrivateImplementationPattern<ConcreteDependencyLabel<DependencyInstallLabelTag, DependencyTypeLabel> >;
    extern template class PrivateImplementationPattern<ConcreteDependencyLabel<DependencyCompileLabelTag, DependencyTypeLabel> >;
    extern template class PrivateImplementationPattern<ConcreteDependencyLabel<DependencySuggestedLabelTag, DependencySuggestLabel> >;
    extern template class PrivateImplementationPattern<ConcreteDependencyLabel<DependencyRecommendedLabelTag, DependencySuggestLabel> >;
    extern template class PrivateImplementationPattern<ConcreteDependencyLabel<DependencyRequiredLabelTag, DependencySuggestLabel> >;
    extern template class PrivateImplementationPattern<ConcreteDependencyLabel<DependencyAnyLabelTag, DependencyABIsLabel> >;
    extern template class PrivateImplementationPattern<ConcreteDependencyLabel<DependencyMineLabelTag, DependencyABIsLabel> >;
    extern template class PrivateImplementationPattern<ConcreteDependencyLabel<DependencyPrimaryLabelTag, DependencyABIsLabel> >;
    extern template class PrivateImplementationPattern<ConcreteDependencyLabel<DependencyABILabelTag, DependencyABIsLabel> >;

    extern template class PrivateImplementationPattern<ActiveDependencyLabels>;
#endif
}

#endif
