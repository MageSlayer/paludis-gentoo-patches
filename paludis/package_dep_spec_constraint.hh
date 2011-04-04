/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_PACKAGE_DEP_SPEC_CONSTRAINT_HH
#define PALUDIS_GUARD_PALUDIS_PACKAGE_DEP_SPEC_CONSTRAINT_HH 1

#include <paludis/package_dep_spec_constraint-fwd.hh>
#include <paludis/name.hh>
#include <paludis/version_operator-fwd.hh>
#include <paludis/version_spec-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/changed_choices-fwd.hh>

#include <paludis/util/attributes.hh>
#include <paludis/util/pool.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/type_list.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/tribool-fwd.hh>

namespace paludis
{
    class PALUDIS_VISIBLE PackageDepSpecConstraint :
        public virtual DeclareAbstractAcceptMethods<PackageDepSpecConstraint, MakeTypeList<
            NameConstraint,
            PackageNamePartConstraint,
            CategoryNamePartConstraint,
            VersionConstraint,
            InRepositoryConstraint,
            FromRepositoryConstraint,
            InstalledAtPathConstraint,
            InstallableToPathConstraint,
            InstallableToRepositoryConstraint,
            AnySlotConstraint,
            ExactSlotConstraint,
            KeyConstraint,
            ChoiceConstraint
        >::Type>
    {
        public:
            virtual ~PackageDepSpecConstraint() = 0;
    };

    class PALUDIS_VISIBLE NameConstraint :
        public PackageDepSpecConstraint,
        public ImplementAcceptMethods<PackageDepSpecConstraint, NameConstraint>
    {
        friend class Pool<NameConstraint>;

        private:
            QualifiedPackageName _name;

            NameConstraint(const QualifiedPackageName &);

            NameConstraint(const NameConstraint &) = delete;

        public:
            ~NameConstraint();

            const QualifiedPackageName name() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE CategoryNamePartConstraint :
        public PackageDepSpecConstraint,
        public ImplementAcceptMethods<PackageDepSpecConstraint, CategoryNamePartConstraint>
    {
        friend class Pool<CategoryNamePartConstraint>;

        private:
            CategoryNamePart _name_part;

            CategoryNamePartConstraint(const CategoryNamePart &);

            CategoryNamePartConstraint(const CategoryNamePartConstraint &) = delete;

        public:
            ~CategoryNamePartConstraint();

            const CategoryNamePart name_part() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE PackageNamePartConstraint :
        public PackageDepSpecConstraint,
        public ImplementAcceptMethods<PackageDepSpecConstraint, PackageNamePartConstraint>
    {
        friend class Pool<PackageNamePartConstraint>;

        private:
            PackageNamePart _name_part;

            PackageNamePartConstraint(const PackageNamePart &);

            PackageNamePartConstraint(const PackageNamePartConstraint &) = delete;

        public:
            ~PackageNamePartConstraint();

            const PackageNamePart name_part() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE VersionConstraint :
        public PackageDepSpecConstraint,
        public ImplementAcceptMethods<PackageDepSpecConstraint, VersionConstraint>
    {
        private:
            Pimp<VersionConstraint> _imp;

            VersionConstraint(const VersionConstraint &) = delete;

        public:
            /* not pooled for now, since VersionSpec gives equality for 1 and 1-r0 */
            VersionConstraint(const VersionSpec &, const VersionOperator &, const VersionConstraintCombiner);

            ~VersionConstraint();

            const VersionSpec version_spec() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const VersionOperator version_operator() const PALUDIS_ATTRIBUTE((warn_unused_result));
            VersionConstraintCombiner combiner() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE InRepositoryConstraint :
        public PackageDepSpecConstraint,
        public ImplementAcceptMethods<PackageDepSpecConstraint, InRepositoryConstraint>
    {
        friend class Pool<InRepositoryConstraint>;

        private:
            RepositoryName _name;

            InRepositoryConstraint(const RepositoryName &);

            InRepositoryConstraint(const InRepositoryConstraint &) = delete;

        public:
            ~InRepositoryConstraint();

            const RepositoryName name() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE FromRepositoryConstraint :
        public PackageDepSpecConstraint,
        public ImplementAcceptMethods<PackageDepSpecConstraint, FromRepositoryConstraint>
    {
        friend class Pool<FromRepositoryConstraint>;

        private:
            RepositoryName _name;

            FromRepositoryConstraint(const RepositoryName &);

            FromRepositoryConstraint(const FromRepositoryConstraint &) = delete;

        public:
            ~FromRepositoryConstraint();

            const RepositoryName name() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE InstalledAtPathConstraint :
        public PackageDepSpecConstraint,
        public ImplementAcceptMethods<PackageDepSpecConstraint, InstalledAtPathConstraint>
    {
        friend class Pool<InstalledAtPathConstraint>;

        private:
            FSPath _path;

            InstalledAtPathConstraint(const FSPath &);

            InstalledAtPathConstraint(const InstalledAtPathConstraint &) = delete;

        public:
            ~InstalledAtPathConstraint();

            const FSPath path() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE InstallableToPathConstraint :
        public PackageDepSpecConstraint,
        public ImplementAcceptMethods<PackageDepSpecConstraint, InstallableToPathConstraint>
    {
        friend class Pool<InstallableToPathConstraint>;

        private:
            FSPath _path;
            bool _include_masked;

            InstallableToPathConstraint(const FSPath &, const bool);

            InstallableToPathConstraint(const InstallableToPathConstraint &) = delete;

        public:
            ~InstallableToPathConstraint();

            const FSPath path() const PALUDIS_ATTRIBUTE((warn_unused_result));
            bool include_masked() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE InstallableToRepositoryConstraint :
        public PackageDepSpecConstraint,
        public ImplementAcceptMethods<PackageDepSpecConstraint, InstallableToRepositoryConstraint>
    {
        friend class Pool<InstallableToRepositoryConstraint>;

        private:
            RepositoryName _name;
            bool _include_masked;

            InstallableToRepositoryConstraint(const RepositoryName &, const bool);

            InstallableToRepositoryConstraint(const InstallableToRepositoryConstraint &) = delete;

        public:
            ~InstallableToRepositoryConstraint();

            const RepositoryName name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            bool include_masked() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE ExactSlotConstraint :
        public PackageDepSpecConstraint,
        public ImplementAcceptMethods<PackageDepSpecConstraint, ExactSlotConstraint>
    {
        friend class Pool<ExactSlotConstraint>;

        private:
            SlotName _name;
            bool _locked;

            ExactSlotConstraint(const SlotName &, const bool);

            ExactSlotConstraint(const ExactSlotConstraint &) = delete;

        public:
            ~ExactSlotConstraint();

            const SlotName name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            bool locked() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE AnySlotConstraint :
        public PackageDepSpecConstraint,
        public ImplementAcceptMethods<PackageDepSpecConstraint, AnySlotConstraint>
    {
        friend class Pool<AnySlotConstraint>;

        private:
            bool _locking;

            AnySlotConstraint(const bool);

            AnySlotConstraint(const AnySlotConstraint &) = delete;

        public:
            ~AnySlotConstraint();

            bool locking() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE KeyConstraint :
        public PackageDepSpecConstraint,
        public ImplementAcceptMethods<PackageDepSpecConstraint, KeyConstraint>
    {
        friend class Pool<KeyConstraint>;

        private:
            std::string _key;
            KeyConstraintOperation _operation;
            std::string _pattern;

            KeyConstraint(const std::string &, const KeyConstraintOperation, const std::string &);

            KeyConstraint(const KeyConstraint &) = delete;

        public:
            ~KeyConstraint();

            const std::string key() const PALUDIS_ATTRIBUTE((warn_unused_result));
            KeyConstraintOperation operation() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string pattern() const PALUDIS_ATTRIBUTE((warn_unused_result));

           bool matches(
                    const Environment * const env,
                    const std::shared_ptr<const PackageID> & id) const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE ChoiceConstraint :
        public PackageDepSpecConstraint,
        public ImplementAcceptMethods<PackageDepSpecConstraint, ChoiceConstraint>
    {
        private:
            ChoiceConstraint(const ChoiceConstraint &) = delete;

        protected:
            ChoiceConstraint();

        public:
            /**
             * Is our requirement met for a given PackageID?
             *
             * The string in the return type might be a description of why the
             * requirement was not met. Sometimes better messages can be given
             * than simply the return value of as_human_string() when the ID to
             * be matched is known. If the bool is false, the string is
             * meaningless.
             *
             * \param spec_id The PackageID the spec comes from. May be null. Used for
             * [use=] style dependencies.
             *
             * \since 0.61 is in ChoiceConstraint
             */
            virtual const std::pair<bool, std::string> requirement_met(
                    const Environment * const,
                    const ChangedChoices * const maybe_changes_to_owner,
                    const std::shared_ptr<const PackageID> & target_id,
                    const std::shared_ptr<const PackageID> & spec_id,
                    const ChangedChoices * const maybe_changes_to_target) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * If possible, indicate which choices to change to make our
             * requirement met for a particular ID.
             *
             * Verifies that the ID has the appropriate choice, and that that
             * choice isn't locked.
             *
             * Returns true for changes made, false for not possible,
             * indeterminate for nothing needs changing.
             *
             * \param spec_id The PackageID the spec comes from. May be null. Used for
             * [use=] style dependencies.
             *
             * \since 0.61 is in ChoiceConstraint
             */
            virtual Tribool accumulate_changes_to_make_met(
                    const Environment * const,
                    const ChangedChoices * const maybe_changes_to_owner,
                    const std::shared_ptr<const PackageID> &,
                    const std::shared_ptr<const PackageID> & spec_id,
                    ChangedChoices &) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a human readable string representation of ourself.
             *
             * \param spec_id The PackageID the spec comes from. May be null. Used for
             * [use=] style dependencies.
             *
             * \since 0.61 is in ChoiceConstraint
             */
            virtual const std::string as_human_string(
                    const std::shared_ptr<const PackageID> & spec_id) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a raw string representation of ourself.
             */
            virtual const std::string as_raw_string() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    extern template class Pool<NameConstraint>;
    extern template class Pool<PackageNamePartConstraint>;
    extern template class Pool<CategoryNamePartConstraint>;
    extern template class Pool<VersionConstraint>;
    extern template class Pool<InRepositoryConstraint>;
    extern template class Pool<FromRepositoryConstraint>;
    extern template class Pool<InstalledAtPathConstraint>;
    extern template class Pool<InstallableToPathConstraint>;
    extern template class Pool<InstallableToRepositoryConstraint>;
    extern template class Pool<ExactSlotConstraint>;
    extern template class Pool<AnySlotConstraint>;
    extern template class Pool<KeyConstraint>;

    extern template class Pimp<VersionConstraint>;
}

#endif
