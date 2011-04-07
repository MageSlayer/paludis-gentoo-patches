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

#ifndef PALUDIS_GUARD_PALUDIS_PACKAGE_DEP_SPEC_REQUIREMENT_HH
#define PALUDIS_GUARD_PALUDIS_PACKAGE_DEP_SPEC_REQUIREMENT_HH 1

#include <paludis/package_dep_spec_requirement-fwd.hh>
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
    class PALUDIS_VISIBLE PackageDepSpecRequirement :
        public virtual DeclareAbstractAcceptMethods<PackageDepSpecRequirement, MakeTypeList<
            NameRequirement,
            PackageNamePartRequirement,
            CategoryNamePartRequirement,
            VersionRequirement,
            InRepositoryRequirement,
            FromRepositoryRequirement,
            InstalledAtPathRequirement,
            InstallableToPathRequirement,
            InstallableToRepositoryRequirement,
            AnySlotRequirement,
            ExactSlotRequirement,
            KeyRequirement,
            ChoiceRequirement
        >::Type>
    {
        public:
            virtual ~PackageDepSpecRequirement() = 0;
    };

    class PALUDIS_VISIBLE NameRequirement :
        public PackageDepSpecRequirement,
        public ImplementAcceptMethods<PackageDepSpecRequirement, NameRequirement>
    {
        friend class Pool<NameRequirement>;

        private:
            QualifiedPackageName _name;

            NameRequirement(const QualifiedPackageName &);

            NameRequirement(const NameRequirement &) = delete;

        public:
            ~NameRequirement();

            const QualifiedPackageName name() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE CategoryNamePartRequirement :
        public PackageDepSpecRequirement,
        public ImplementAcceptMethods<PackageDepSpecRequirement, CategoryNamePartRequirement>
    {
        friend class Pool<CategoryNamePartRequirement>;

        private:
            CategoryNamePart _name_part;

            CategoryNamePartRequirement(const CategoryNamePart &);

            CategoryNamePartRequirement(const CategoryNamePartRequirement &) = delete;

        public:
            ~CategoryNamePartRequirement();

            const CategoryNamePart name_part() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE PackageNamePartRequirement :
        public PackageDepSpecRequirement,
        public ImplementAcceptMethods<PackageDepSpecRequirement, PackageNamePartRequirement>
    {
        friend class Pool<PackageNamePartRequirement>;

        private:
            PackageNamePart _name_part;

            PackageNamePartRequirement(const PackageNamePart &);

            PackageNamePartRequirement(const PackageNamePartRequirement &) = delete;

        public:
            ~PackageNamePartRequirement();

            const PackageNamePart name_part() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE VersionRequirement :
        public PackageDepSpecRequirement,
        public ImplementAcceptMethods<PackageDepSpecRequirement, VersionRequirement>
    {
        private:
            Pimp<VersionRequirement> _imp;

            VersionRequirement(const VersionRequirement &) = delete;

        public:
            /* not pooled for now, since VersionSpec gives equality for 1 and 1-r0 */
            VersionRequirement(const VersionSpec &, const VersionOperator &, const VersionRequirementCombiner);

            ~VersionRequirement();

            const VersionSpec version_spec() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const VersionOperator version_operator() const PALUDIS_ATTRIBUTE((warn_unused_result));
            VersionRequirementCombiner combiner() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE InRepositoryRequirement :
        public PackageDepSpecRequirement,
        public ImplementAcceptMethods<PackageDepSpecRequirement, InRepositoryRequirement>
    {
        friend class Pool<InRepositoryRequirement>;

        private:
            RepositoryName _name;

            InRepositoryRequirement(const RepositoryName &);

            InRepositoryRequirement(const InRepositoryRequirement &) = delete;

        public:
            ~InRepositoryRequirement();

            const RepositoryName name() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE FromRepositoryRequirement :
        public PackageDepSpecRequirement,
        public ImplementAcceptMethods<PackageDepSpecRequirement, FromRepositoryRequirement>
    {
        friend class Pool<FromRepositoryRequirement>;

        private:
            RepositoryName _name;

            FromRepositoryRequirement(const RepositoryName &);

            FromRepositoryRequirement(const FromRepositoryRequirement &) = delete;

        public:
            ~FromRepositoryRequirement();

            const RepositoryName name() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE InstalledAtPathRequirement :
        public PackageDepSpecRequirement,
        public ImplementAcceptMethods<PackageDepSpecRequirement, InstalledAtPathRequirement>
    {
        friend class Pool<InstalledAtPathRequirement>;

        private:
            FSPath _path;

            InstalledAtPathRequirement(const FSPath &);

            InstalledAtPathRequirement(const InstalledAtPathRequirement &) = delete;

        public:
            ~InstalledAtPathRequirement();

            const FSPath path() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE InstallableToPathRequirement :
        public PackageDepSpecRequirement,
        public ImplementAcceptMethods<PackageDepSpecRequirement, InstallableToPathRequirement>
    {
        friend class Pool<InstallableToPathRequirement>;

        private:
            FSPath _path;
            bool _include_masked;

            InstallableToPathRequirement(const FSPath &, const bool);

            InstallableToPathRequirement(const InstallableToPathRequirement &) = delete;

        public:
            ~InstallableToPathRequirement();

            const FSPath path() const PALUDIS_ATTRIBUTE((warn_unused_result));
            bool include_masked() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE InstallableToRepositoryRequirement :
        public PackageDepSpecRequirement,
        public ImplementAcceptMethods<PackageDepSpecRequirement, InstallableToRepositoryRequirement>
    {
        friend class Pool<InstallableToRepositoryRequirement>;

        private:
            RepositoryName _name;
            bool _include_masked;

            InstallableToRepositoryRequirement(const RepositoryName &, const bool);

            InstallableToRepositoryRequirement(const InstallableToRepositoryRequirement &) = delete;

        public:
            ~InstallableToRepositoryRequirement();

            const RepositoryName name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            bool include_masked() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE ExactSlotRequirement :
        public PackageDepSpecRequirement,
        public ImplementAcceptMethods<PackageDepSpecRequirement, ExactSlotRequirement>
    {
        friend class Pool<ExactSlotRequirement>;

        private:
            SlotName _name;
            bool _locked;

            ExactSlotRequirement(const SlotName &, const bool);

            ExactSlotRequirement(const ExactSlotRequirement &) = delete;

        public:
            ~ExactSlotRequirement();

            const SlotName name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            bool locked() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE AnySlotRequirement :
        public PackageDepSpecRequirement,
        public ImplementAcceptMethods<PackageDepSpecRequirement, AnySlotRequirement>
    {
        friend class Pool<AnySlotRequirement>;

        private:
            bool _locking;

            AnySlotRequirement(const bool);

            AnySlotRequirement(const AnySlotRequirement &) = delete;

        public:
            ~AnySlotRequirement();

            bool locking() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE KeyRequirement :
        public PackageDepSpecRequirement,
        public ImplementAcceptMethods<PackageDepSpecRequirement, KeyRequirement>
    {
        friend class Pool<KeyRequirement>;

        private:
            KeyRequirementKeyType _key_type;
            std::string _key;
            KeyRequirementOperation _operation;
            std::string _pattern;

            KeyRequirement(const KeyRequirementKeyType, const std::string &, const KeyRequirementOperation, const std::string &);

            KeyRequirement(const KeyRequirement &) = delete;

        public:
            ~KeyRequirement();

            KeyRequirementKeyType key_type() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string key() const PALUDIS_ATTRIBUTE((warn_unused_result));
            KeyRequirementOperation operation() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string pattern() const PALUDIS_ATTRIBUTE((warn_unused_result));

           bool matches(
                    const Environment * const env,
                    const std::shared_ptr<const PackageID> & id) const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return a raw string representation of ourself.
             */
            const std::string as_raw_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE ChoiceRequirement :
        public PackageDepSpecRequirement,
        public ImplementAcceptMethods<PackageDepSpecRequirement, ChoiceRequirement>
    {
        private:
            ChoiceRequirement(const ChoiceRequirement &) = delete;

        protected:
            ChoiceRequirement();

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
             * \since 0.61 is in ChoiceRequirement
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
             * \since 0.61 is in ChoiceRequirement
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
             * \since 0.61 is in ChoiceRequirement
             */
            virtual const std::string as_human_string(
                    const std::shared_ptr<const PackageID> & spec_id) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a raw string representation of ourself.
             */
            virtual const std::string as_raw_string() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    extern template class Pool<NameRequirement>;
    extern template class Pool<PackageNamePartRequirement>;
    extern template class Pool<CategoryNamePartRequirement>;
    extern template class Pool<VersionRequirement>;
    extern template class Pool<InRepositoryRequirement>;
    extern template class Pool<FromRepositoryRequirement>;
    extern template class Pool<InstalledAtPathRequirement>;
    extern template class Pool<InstallableToPathRequirement>;
    extern template class Pool<InstallableToRepositoryRequirement>;
    extern template class Pool<ExactSlotRequirement>;
    extern template class Pool<AnySlotRequirement>;
    extern template class Pool<KeyRequirement>;

    extern template class Pimp<VersionRequirement>;
}

#endif
