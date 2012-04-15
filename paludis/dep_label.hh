/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/type_list.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

/** \file
 * Declarations for dependency label-related classes.
 *
 * \ingroup g_dep_spec
 *
 * \section Examples
 *
 * - \ref example_dep_label.cc "example_dep_label.cc"
 * - \ref example_dep_spec.cc "example_dep_spec.cc" (for specifications)
 * - \ref example_dep_tag.cc "example_dep_tag.cc" (for tags)
 */

namespace paludis
{
    /**
     * URI label base class.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE URILabel :
        public virtual DeclareAbstractAcceptMethods<URILabel, MakeTypeList<
            URIMirrorsThenListedLabel, URIMirrorsOnlyLabel, URIListedOnlyLabel, URIListedThenMirrorsLabel,
            URILocalMirrorsOnlyLabel, URIManualOnlyLabel>::Type>
    {
        public:
            ///\name Basic operations
            ///\{

            URILabel() = default;
            virtual ~URILabel() = 0;

            URILabel(const URILabel &) = delete;
            URILabel & operator= (const URILabel &) = delete;

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
    class PALUDIS_VISIBLE SpecificURILabel :
        public URILabel,
        public ImplementAcceptMethods<URILabel, SpecificURILabel<T_> >
    {
        private:
            const std::string _text;

        public:
            ///\name Basic operations
            ///\{

            SpecificURILabel(const std::string &);
            ~SpecificURILabel();

            ///\}

            virtual const std::string text() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /// Convenience typedef alias to obtain our tag.
            typedef T_ Tag;
    };

    /**
     * Dependencies label base class.
     *
     * \since 0.42
     * \ingroup g_dep_spec
     */
    class PALUDIS_VISIBLE DependenciesLabel :
        public virtual DeclareAbstractAcceptMethods<DependenciesLabel, MakeTypeList<
            DependenciesBuildLabel, DependenciesRunLabel, DependenciesPostLabel, DependenciesCompileAgainstLabel,
            DependenciesFetchLabel, DependenciesInstallLabel, DependenciesSuggestionLabel,
            DependenciesRecommendationLabel, DependenciesTestLabel>::Type>
    {
        public:
            ///\name Basic operations
            ///\{

            DependenciesLabel() = default;
            virtual ~DependenciesLabel() = 0;

            DependenciesLabel(const DependenciesLabel &) = delete;
            DependenciesLabel & operator= (const DependenciesLabel &) = delete;

            ///\}

            /// Our text.
            virtual const std::string text() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Are we enabled?
             *
             * \since 0.58 takes env, package_id
             */
            virtual bool enabled(
                    const Environment * const,
                    const std::shared_ptr<const PackageID> &) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A concrete dependencies label class.
     *
     * \since 0.42
     * \ingroup g_dep_spec
     */
    template <typename T_>
    class PALUDIS_VISIBLE SpecificDependenciesLabel :
        public DependenciesLabel,
        public ImplementAcceptMethods<DependenciesLabel, SpecificDependenciesLabel<T_> >
    {
        public:
            /// Convenience typedef alias to obtain our tag.
            typedef T_ Tag;
    };

    extern template class PALUDIS_VISIBLE SpecificURILabel<URIMirrorsThenListedLabelTag>;
    extern template class PALUDIS_VISIBLE SpecificURILabel<URIMirrorsOnlyLabelTag>;
    extern template class PALUDIS_VISIBLE SpecificURILabel<URIListedOnlyLabelTag>;
    extern template class PALUDIS_VISIBLE SpecificURILabel<URIListedThenMirrorsLabelTag>;
    extern template class PALUDIS_VISIBLE SpecificURILabel<URILocalMirrorsOnlyLabelTag>;
    extern template class PALUDIS_VISIBLE SpecificURILabel<URIManualOnlyLabelTag>;

    extern template class PALUDIS_VISIBLE SpecificDependenciesLabel<DependenciesBuildLabelTag>;
    extern template class PALUDIS_VISIBLE SpecificDependenciesLabel<DependenciesRunLabelTag>;
    extern template class PALUDIS_VISIBLE SpecificDependenciesLabel<DependenciesPostLabelTag>;
    extern template class PALUDIS_VISIBLE SpecificDependenciesLabel<DependenciesCompileAgainstLabelTag>;
    extern template class PALUDIS_VISIBLE SpecificDependenciesLabel<DependenciesFetchLabelTag>;
    extern template class PALUDIS_VISIBLE SpecificDependenciesLabel<DependenciesInstallLabelTag>;
    extern template class PALUDIS_VISIBLE SpecificDependenciesLabel<DependenciesSuggestionLabelTag>;
    extern template class PALUDIS_VISIBLE SpecificDependenciesLabel<DependenciesRecommendationLabelTag>;
    extern template class PALUDIS_VISIBLE SpecificDependenciesLabel<DependenciesTestLabelTag>;

    extern template class PALUDIS_VISIBLE Sequence<std::shared_ptr<const DependenciesLabel> >;
    extern template class PALUDIS_VISIBLE WrappedForwardIterator<Sequence<std::shared_ptr<const DependenciesLabel> >::ConstIteratorTag,
             const std::shared_ptr<const DependenciesLabel> >;
}

#endif
