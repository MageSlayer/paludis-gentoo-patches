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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_DECISION_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_DECISION_HH 1

#include <paludis/resolver/decision-fwd.hh>
#include <paludis/resolver/destination-fwd.hh>
#include <paludis/resolver/unsuitable_candidates-fwd.hh>
#include <paludis/resolver/change_type-fwd.hh>
#include <paludis/resolver/resolvent-fwd.hh>
#include <paludis/resolver/required_confirmations-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/simple_visitor.hh>
#include <paludis/util/type_list.hh>
#include <paludis/serialise-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/changed_choices-fwd.hh>
#include <paludis/name-fwd.hh>

namespace paludis
{
    namespace resolver
    {
        class PALUDIS_VISIBLE Decision :
            public virtual DeclareAbstractAcceptMethods<Decision, MakeTypeList<
                NothingNoChangeDecision, ExistingNoChangeDecision, ChangesToMakeDecision,
                RemoveDecision, UnableToMakeDecision, BreakDecision>::Type>
        {
            public:
                virtual ~Decision() = 0;

                virtual const Resolvent resolvent() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual bool taken() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual void serialise(Serialiser &) const = 0;

                static const std::shared_ptr<Decision> deserialise(
                        Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE NothingNoChangeDecision :
            public Decision,
            public ImplementAcceptMethods<Decision, NothingNoChangeDecision>,
            private Pimp<NothingNoChangeDecision>
        {
            public:
                NothingNoChangeDecision(const Resolvent &, const bool);
                ~NothingNoChangeDecision();

                virtual const Resolvent resolvent() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual bool taken() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
        };

        class PALUDIS_VISIBLE ExistingNoChangeDecision :
            public Decision,
            public ImplementAcceptMethods<Decision, ExistingNoChangeDecision>,
            private Pimp<ExistingNoChangeDecision>
        {
            public:
                ExistingNoChangeDecision(
                        const Resolvent &,
                        const std::shared_ptr<const PackageID> &,
                        const bool is_same,
                        const bool is_same_version,
                        const bool is_transient,
                        const bool taken
                        );
                ~ExistingNoChangeDecision();

                const std::shared_ptr<const PackageID> existing_id() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                bool is_same() const PALUDIS_ATTRIBUTE((warn_unused_result));
                bool is_same_version() const PALUDIS_ATTRIBUTE((warn_unused_result));
                bool is_transient() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const Resolvent resolvent() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual bool taken() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
        };

        class PALUDIS_VISIBLE ConfirmableDecision :
            public Decision,
            public virtual DeclareAbstractAcceptMethods<ConfirmableDecision, MakeTypeList<
                ChangesToMakeDecision, RemoveDecision, BreakDecision>::Type>
        {
            public:
                typedef DeclareAbstractAcceptMethods<ConfirmableDecision,
                        MakeTypeList<ChangesToMakeDecision, RemoveDecision, BreakDecision>::Type> MoreSpecificVisitor;

                typedef MoreSpecificVisitor::VisitableTypeList VisitableTypeList;
                typedef MoreSpecificVisitor::VisitableBaseClass VisitableBaseClass;

                using MoreSpecificVisitor::accept_returning;
                using MoreSpecificVisitor::accept;

                static const std::shared_ptr<ConfirmableDecision> deserialise(
                        Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::shared_ptr<const RequiredConfirmations>
                    required_confirmations_if_any() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
        };

        class PALUDIS_VISIBLE ChangeOrRemoveDecision :
            public ConfirmableDecision,
            public virtual DeclareAbstractAcceptMethods<ChangeOrRemoveDecision, MakeTypeList<
                ChangesToMakeDecision, RemoveDecision>::Type>
        {
            public:
                typedef DeclareAbstractAcceptMethods<ChangeOrRemoveDecision,
                        MakeTypeList<ChangesToMakeDecision, RemoveDecision>::Type> MoreSpecificVisitor;

                typedef MoreSpecificVisitor::VisitableTypeList VisitableTypeList;
                typedef MoreSpecificVisitor::VisitableBaseClass VisitableBaseClass;

                using MoreSpecificVisitor::accept_returning;
                using MoreSpecificVisitor::accept;

                static const std::shared_ptr<ChangeOrRemoveDecision> deserialise(
                        Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE ChangesToMakeDecision :
            public ChangeOrRemoveDecision,
            public ImplementAcceptMethods<Decision, ChangesToMakeDecision>,
            public ImplementAcceptMethods<ConfirmableDecision, ChangesToMakeDecision>,
            public ImplementAcceptMethods<ChangeOrRemoveDecision, ChangesToMakeDecision>,
            private Pimp<ChangesToMakeDecision>
        {
            public:
                ChangesToMakeDecision(
                        const Resolvent &,
                        const std::shared_ptr<const PackageID> &,
                        const std::shared_ptr<const ChangedChoices> &,
                        const bool best,
                        const ChangeType,
                        const bool taken,
                        const std::shared_ptr<const Destination> &,
                        const std::function<void (ChangesToMakeDecision &)> &
                        );

                ~ChangesToMakeDecision();

                const std::shared_ptr<const Destination> destination() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                void set_destination(const std::shared_ptr<const Destination> &);

                const std::shared_ptr<const PackageID> origin_id() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::shared_ptr<const ChangedChoices> if_changed_choices() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::shared_ptr<const RepositoryName> if_via_new_binary_in() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                void set_via_new_binary_in(const RepositoryName &);

                virtual bool best() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual ChangeType change_type() const PALUDIS_ATTRIBUTE((warn_unused_result));

                void set_change_type(ChangeType);

                virtual const Resolvent resolvent() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual bool taken() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::shared_ptr<const RequiredConfirmations> required_confirmations_if_any() const PALUDIS_ATTRIBUTE((warn_unused_result));
                void add_required_confirmation(const std::shared_ptr<const RequiredConfirmation> &);

                virtual void serialise(Serialiser &) const;

                static const std::shared_ptr<ChangesToMakeDecision> deserialise(
                        Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE RemoveDecision :
            public ChangeOrRemoveDecision,
            public ImplementAcceptMethods<Decision, RemoveDecision>,
            public ImplementAcceptMethods<ConfirmableDecision, RemoveDecision>,
            public ImplementAcceptMethods<ChangeOrRemoveDecision, RemoveDecision>,
            private Pimp<RemoveDecision>
        {
            public:
                RemoveDecision(
                        const Resolvent &,
                        const std::shared_ptr<const PackageIDSequence> &,
                        const bool taken);
                ~RemoveDecision();

                virtual const Resolvent resolvent() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual bool taken() const PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::shared_ptr<const PackageIDSequence> ids() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::shared_ptr<const RequiredConfirmations> required_confirmations_if_any() const PALUDIS_ATTRIBUTE((warn_unused_result));
                void add_required_confirmation(const std::shared_ptr<const RequiredConfirmation> &);

                virtual void serialise(Serialiser &) const;

                static const std::shared_ptr<RemoveDecision> deserialise(
                        Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE UnableToMakeDecision :
            public Decision,
            public ImplementAcceptMethods<Decision, UnableToMakeDecision>,
            private Pimp<UnableToMakeDecision>
        {
            public:
                UnableToMakeDecision(
                        const Resolvent &,
                        const std::shared_ptr<const UnsuitableCandidates> &,
                        const bool taken);
                ~UnableToMakeDecision();

                virtual const Resolvent resolvent() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual bool taken() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;

                const std::shared_ptr<const UnsuitableCandidates> unsuitable_candidates() const PALUDIS_ATTRIBUTE((warn_unused_result));

                static const std::shared_ptr<UnableToMakeDecision> deserialise(
                        Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE BreakDecision :
            public ConfirmableDecision,
            public ImplementAcceptMethods<Decision, BreakDecision>,
            public ImplementAcceptMethods<ConfirmableDecision, BreakDecision>,
            private Pimp<BreakDecision>
        {
            public:
                BreakDecision(
                        const Resolvent &,
                        const std::shared_ptr<const PackageID> &,
                        const bool taken);
                ~BreakDecision();

                const std::shared_ptr<const PackageID> existing_id() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const Resolvent resolvent() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual bool taken() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::shared_ptr<const RequiredConfirmations> required_confirmations_if_any() const PALUDIS_ATTRIBUTE((warn_unused_result));
                void add_required_confirmation(const std::shared_ptr<const RequiredConfirmation> &);

                virtual void serialise(Serialiser &) const;

                static const std::shared_ptr<BreakDecision> deserialise(
                        Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

    extern template class Pimp<resolver::NothingNoChangeDecision>;
    extern template class Pimp<resolver::ExistingNoChangeDecision>;
    extern template class Pimp<resolver::ChangesToMakeDecision>;
    extern template class Pimp<resolver::UnableToMakeDecision>;
    extern template class Pimp<resolver::RemoveDecision>;
    extern template class Pimp<resolver::BreakDecision>;

}

#endif
