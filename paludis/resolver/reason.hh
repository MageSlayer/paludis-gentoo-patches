/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_REASON_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_REASON_HH 1

#include <paludis/resolver/reason-fwd.hh>
#include <paludis/resolver/resolvent-fwd.hh>
#include <paludis/resolver/sanitised_dependencies-fwd.hh>
#include <paludis/resolver/change_by_resolvent-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/simple_visitor.hh>
#include <paludis/util/type_list.hh>
#include <paludis/name-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/serialise-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace resolver
    {
        class Reason :
            public virtual DeclareAbstractAcceptMethods<Reason, MakeTypeList<
                TargetReason, DependencyReason, DependentReason, WasUsedByReason, PresetReason,
                SetReason, LikeOtherDestinationTypeReason, ViaBinaryReason>::Type>
        {
            public:
                virtual ~Reason() = 0;

                virtual void serialise(Serialiser &) const = 0;

                static const std::tr1::shared_ptr<Reason> deserialise(
                        Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class TargetReason :
            public Reason,
            public ImplementAcceptMethods<Reason, TargetReason>
        {
            public:
                virtual void serialise(Serialiser &) const;
        };

        class DependencyReason :
            private PrivateImplementationPattern<DependencyReason>,
            public Reason,
            public ImplementAcceptMethods<Reason, DependencyReason>
        {
            public:
                DependencyReason(
                        const std::tr1::shared_ptr<const PackageID> & id,
                        const Resolvent &,
                        const SanitisedDependency & s,
                        const bool already_met);

                ~DependencyReason();

                const std::tr1::shared_ptr<const PackageID> from_id() const;
                const Resolvent from_resolvent() const;
                const SanitisedDependency & sanitised_dependency() const;
                bool already_met() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
        };

        class DependentReason :
            private PrivateImplementationPattern<DependentReason>,
            public Reason,
            public ImplementAcceptMethods<Reason, DependentReason>
        {
            public:
                DependentReason(const ChangeByResolvent &);
                ~DependentReason();

                const ChangeByResolvent id_and_resolvent_being_removed() const;

                virtual void serialise(Serialiser &) const;
        };

        class WasUsedByReason :
            private PrivateImplementationPattern<WasUsedByReason>,
            public Reason,
            public ImplementAcceptMethods<Reason, WasUsedByReason>
        {
            public:
                WasUsedByReason(const std::tr1::shared_ptr<const ChangeByResolventSequence> & ids);
                ~WasUsedByReason();

                const std::tr1::shared_ptr<const ChangeByResolventSequence> ids_and_resolvents_being_removed() const;

                virtual void serialise(Serialiser &) const;
        };

        class PresetReason :
            private PrivateImplementationPattern<PresetReason>,
            public Reason,
            public ImplementAcceptMethods<Reason, PresetReason>
        {
            public:
                PresetReason(
                        const std::string &,
                        const std::tr1::shared_ptr<const Reason> &);
                ~PresetReason();

                const std::tr1::shared_ptr<const Reason> maybe_reason_for_preset() const PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::string maybe_explanation() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
        };

        class SetReason :
            public Reason,
            private PrivateImplementationPattern<SetReason>,
            public ImplementAcceptMethods<Reason, SetReason>
        {
            public:
                SetReason(const SetName &, const std::tr1::shared_ptr<const Reason> &);
                ~SetReason();

                const SetName set_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                const std::tr1::shared_ptr<const Reason> reason_for_set() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
        };

        class LikeOtherDestinationTypeReason :
            public Reason,
            private PrivateImplementationPattern<LikeOtherDestinationTypeReason>,
            public ImplementAcceptMethods<Reason, LikeOtherDestinationTypeReason>
        {
            public:
                LikeOtherDestinationTypeReason(const Resolvent &, const std::tr1::shared_ptr<const Reason> &);
                ~LikeOtherDestinationTypeReason();

                const Resolvent other_resolvent() const PALUDIS_ATTRIBUTE((warn_unused_result));
                const std::tr1::shared_ptr<const Reason> reason_for_other() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
        };

        class ViaBinaryReason :
            public Reason,
            private PrivateImplementationPattern<ViaBinaryReason>,
            public ImplementAcceptMethods<Reason, ViaBinaryReason>
        {
            public:
                ViaBinaryReason(const Resolvent &);
                ~ViaBinaryReason();

                const Resolvent other_resolvent() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
        };
    }

    extern template class PrivateImplementationPattern<resolver::DependencyReason>;
    extern template class PrivateImplementationPattern<resolver::DependentReason>;
    extern template class PrivateImplementationPattern<resolver::WasUsedByReason>;
    extern template class PrivateImplementationPattern<resolver::SetReason>;
    extern template class PrivateImplementationPattern<resolver::ViaBinaryReason>;

}

#endif
