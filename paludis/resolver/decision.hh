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
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/simple_visitor.hh>
#include <paludis/util/type_list.hh>
#include <paludis/serialise-fwd.hh>
#include <paludis/package_id.hh>

namespace paludis
{
    namespace resolver
    {
        class PALUDIS_VISIBLE Decision :
            public virtual DeclareAbstractAcceptMethods<Decision, MakeTypeList<
                NothingNoChangeDecision, ExistingNoChangeDecision, ChangesToMakeDecision,
                RemoveDecision, UnableToMakeDecision>::Type>
        {
            public:
                virtual ~Decision() = 0;

                virtual bool taken() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual void serialise(Serialiser &) const = 0;

                static const std::tr1::shared_ptr<Decision> deserialise(
                        Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE NothingNoChangeDecision :
            public Decision,
            public ImplementAcceptMethods<Decision, NothingNoChangeDecision>,
            private PrivateImplementationPattern<NothingNoChangeDecision>
        {
            public:
                NothingNoChangeDecision(const bool);
                ~NothingNoChangeDecision();

                virtual bool taken() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
        };

        class PALUDIS_VISIBLE ExistingNoChangeDecision :
            public Decision,
            public ImplementAcceptMethods<Decision, ExistingNoChangeDecision>,
            private PrivateImplementationPattern<ExistingNoChangeDecision>
        {
            public:
                ExistingNoChangeDecision(
                        const std::tr1::shared_ptr<const PackageID> &,
                        const bool is_same,
                        const bool is_same_version,
                        const bool is_transient,
                        const bool taken
                        );
                ~ExistingNoChangeDecision();

                const std::tr1::shared_ptr<const PackageID> existing_id() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                bool is_same() const PALUDIS_ATTRIBUTE((warn_unused_result));
                bool is_same_version() const PALUDIS_ATTRIBUTE((warn_unused_result));
                bool is_transient() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual bool taken() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
        };

        class PALUDIS_VISIBLE ChangesToMakeDecision :
            public Decision,
            public ImplementAcceptMethods<Decision, ChangesToMakeDecision>,
            private PrivateImplementationPattern<ChangesToMakeDecision>,
            public std::tr1::enable_shared_from_this<ChangesToMakeDecision>
        {
            public:
                ChangesToMakeDecision(
                        const std::tr1::shared_ptr<const PackageID> &,
                        const bool best,
                        const ChangeType,
                        const bool taken,
                        const std::tr1::shared_ptr<const Destination> &
                        );

                ~ChangesToMakeDecision();

                const std::tr1::shared_ptr<const Destination> destination() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                void set_destination(const std::tr1::shared_ptr<const Destination> &);

                const std::tr1::shared_ptr<const PackageID> origin_id() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual bool best() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual ChangeType change_type() const PALUDIS_ATTRIBUTE((warn_unused_result));

                void set_change_type(ChangeType);

                virtual bool taken() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;

                static const std::tr1::shared_ptr<ChangesToMakeDecision> deserialise(
                        Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE RemoveDecision :
            public Decision,
            public ImplementAcceptMethods<Decision, RemoveDecision>,
            private PrivateImplementationPattern<RemoveDecision>,
            public std::tr1::enable_shared_from_this<RemoveDecision>
        {
            public:
                RemoveDecision(
                        const std::tr1::shared_ptr<const PackageIDSequence> &,
                        const bool taken);
                ~RemoveDecision();

                virtual bool taken() const PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::tr1::shared_ptr<const PackageIDSequence> ids() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;

                static const std::tr1::shared_ptr<RemoveDecision> deserialise(
                        Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE UnableToMakeDecision :
            public Decision,
            public ImplementAcceptMethods<Decision, UnableToMakeDecision>,
            private PrivateImplementationPattern<UnableToMakeDecision>,
            public std::tr1::enable_shared_from_this<UnableToMakeDecision>
        {
            public:
                UnableToMakeDecision(
                        const std::tr1::shared_ptr<const UnsuitableCandidates> &,
                        const bool taken);
                ~UnableToMakeDecision();

                virtual bool taken() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;

                const std::tr1::shared_ptr<const UnsuitableCandidates> unsuitable_candidates() const PALUDIS_ATTRIBUTE((warn_unused_result));

                static const std::tr1::shared_ptr<UnableToMakeDecision> deserialise(
                        Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<resolver::NothingNoChangeDecision>;
    extern template class PrivateImplementationPattern<resolver::ExistingNoChangeDecision>;
    extern template class PrivateImplementationPattern<resolver::ChangesToMakeDecision>;
    extern template class PrivateImplementationPattern<resolver::UnableToMakeDecision>;
    extern template class PrivateImplementationPattern<resolver::RemoveDecision>;
#endif

}

#endif
