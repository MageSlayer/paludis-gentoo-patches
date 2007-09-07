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

namespace paludis
{
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

    struct DependencyLabelVisitorTypes :
        VisitorTypes<
            DependencyLabelVisitorTypes,
            DependencyLabel,
            DependencyHostLabel,
            DependencyTargetLabel,
            DependencyBuildLabel,
            DependencyRunLabel,
            DependencyInstallLabel,
            DependencyCompileLabel,
            DependencySuggestedLabel,
            DependencyRecommendedLabel,
            DependencyRequiredLabel,
            DependencyAnyLabel,
            DependencyMineLabel,
            DependencyPrimaryLabel,
            DependencyABILabel
        >
    {
    };

    class PALUDIS_VISIBLE URILabel :
        private InstantiationPolicy<URILabel, instantiation_method::NonCopyableTag>,
        public virtual ConstAcceptInterface<URILabelVisitorTypes>
    {
        public:
            virtual ~URILabel() = 0;

            virtual const std::string text() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    template <typename T_>
    class PALUDIS_VISIBLE ConcreteURILabel :
        public URILabel,
        public ConstAcceptInterfaceVisitsThis<URILabelVisitorTypes, ConcreteURILabel<T_> >,
        private PrivateImplementationPattern<ConcreteURILabel<T_> >
    {
        private:
            using PrivateImplementationPattern<ConcreteURILabel<T_> >::_imp;

        public:
            ConcreteURILabel(const std::string &);
            ~ConcreteURILabel();

            virtual const std::string text() const PALUDIS_ATTRIBUTE((warn_unused_result));

            typedef T_ Tag;
    };

    class PALUDIS_VISIBLE DependencyLabel :
        private InstantiationPolicy<DependencyLabel, instantiation_method::NonCopyableTag>,
        public virtual ConstAcceptInterface<DependencyLabelVisitorTypes>
    {
        public:
            virtual ~DependencyLabel() = 0;

            virtual const std::string text() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    template <typename T_>
    class PALUDIS_VISIBLE ConcreteDependencyLabel :
        public DependencyLabel,
        public ConstAcceptInterfaceVisitsThis<DependencyLabelVisitorTypes, ConcreteDependencyLabel<T_> >,
        private PrivateImplementationPattern<ConcreteDependencyLabel<T_> >
    {
        private:
            using PrivateImplementationPattern<ConcreteDependencyLabel<T_> >::_imp;

        public:
            ConcreteDependencyLabel(const std::string &);
            ~ConcreteDependencyLabel();

            virtual const std::string text() const PALUDIS_ATTRIBUTE((warn_unused_result));

            typedef T_ Tag;
    };
}

#endif
