/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_ELIKE_CHOICES_HH
#define PALUDIS_GUARD_PALUDIS_ELIKE_CHOICES_HH 1

#include <paludis/elike_choices-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/tribool-fwd.hh>
#include <paludis/choice.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>

namespace paludis
{
    class PALUDIS_VISIBLE ELikeOptionalTestsChoiceValue :
        public ChoiceValue
    {
        private:
            const bool _enabled;
            const bool _mask;

        public:
            ELikeOptionalTestsChoiceValue(const std::shared_ptr<const PackageID> &,
                    const Environment * const env, const std::shared_ptr<const Choice> &,
                    const bool mask);

            const UnprefixedChoiceName unprefixed_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const ChoiceNameWithPrefix name_with_prefix() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool enabled() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool enabled_by_default() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool locked() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string description() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            ChoiceOrigin origin() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string parameter() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::shared_ptr<const PermittedChoiceValueParameterValues> permitted_parameter_values() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool presumed() const override PALUDIS_ATTRIBUTE((warn_unused_result));

            static const UnprefixedChoiceName canonical_unprefixed_name() PALUDIS_ATTRIBUTE((warn_unused_result));
            static const ChoiceNameWithPrefix canonical_name_with_prefix() PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE ELikeRecommendedTestsChoiceValue :
        public ChoiceValue
    {
        private:
            const bool _enabled;
            const bool _mask;

        public:
            ELikeRecommendedTestsChoiceValue(const std::shared_ptr<const PackageID> &,
                    const Environment * const env, const std::shared_ptr<const Choice> &,
                    const bool mask);

            const UnprefixedChoiceName unprefixed_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const ChoiceNameWithPrefix name_with_prefix() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool enabled() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool enabled_by_default() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool locked() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string description() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            ChoiceOrigin origin() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string parameter() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::shared_ptr<const PermittedChoiceValueParameterValues> permitted_parameter_values() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool presumed() const override PALUDIS_ATTRIBUTE((warn_unused_result));

            static const UnprefixedChoiceName canonical_unprefixed_name() PALUDIS_ATTRIBUTE((warn_unused_result));
            static const ChoiceNameWithPrefix canonical_name_with_prefix() PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE ELikeExpensiveTestsChoiceValue :
        public ChoiceValue
    {
        private:
            const bool _enabled;
            const bool _mask;

        public:
            ELikeExpensiveTestsChoiceValue(const std::shared_ptr<const PackageID> &,
                    const Environment * const env, const std::shared_ptr<const Choice> &,
                    const bool mask);

            const UnprefixedChoiceName unprefixed_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const ChoiceNameWithPrefix name_with_prefix() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool enabled() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool enabled_by_default() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool locked() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string description() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            ChoiceOrigin origin() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string parameter() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::shared_ptr<const PermittedChoiceValueParameterValues> permitted_parameter_values() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool presumed() const override PALUDIS_ATTRIBUTE((warn_unused_result));

            static const UnprefixedChoiceName canonical_unprefixed_name() PALUDIS_ATTRIBUTE((warn_unused_result));
            static const ChoiceNameWithPrefix canonical_name_with_prefix() PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE ELikeJobsChoiceValue :
        public ChoiceValue
    {
        private:
            const bool _enabled;
            const std::string _parameter;

        public:
            ELikeJobsChoiceValue(const std::shared_ptr<const PackageID> &,
                    const Environment * const env, const std::shared_ptr<const Choice> &);

            const UnprefixedChoiceName unprefixed_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const ChoiceNameWithPrefix name_with_prefix() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool enabled() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool enabled_by_default() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool locked() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string description() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            ChoiceOrigin origin() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string parameter() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::shared_ptr<const PermittedChoiceValueParameterValues> permitted_parameter_values() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool presumed() const override PALUDIS_ATTRIBUTE((warn_unused_result));

            static const UnprefixedChoiceName canonical_unprefixed_name() PALUDIS_ATTRIBUTE((warn_unused_result));
            static const ChoiceNameWithPrefix canonical_name_with_prefix() PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE ELikeTraceChoiceValue :
        public ChoiceValue
    {
        private:
            const bool _enabled;

        public:
            ELikeTraceChoiceValue(const std::shared_ptr<const PackageID> &,
                    const Environment * const env, const std::shared_ptr<const Choice> &);

            const UnprefixedChoiceName unprefixed_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const ChoiceNameWithPrefix name_with_prefix() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool enabled() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool enabled_by_default() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool locked() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string description() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            ChoiceOrigin origin() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string parameter() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::shared_ptr<const PermittedChoiceValueParameterValues> permitted_parameter_values() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool presumed() const override PALUDIS_ATTRIBUTE((warn_unused_result));

            static const UnprefixedChoiceName canonical_unprefixed_name() PALUDIS_ATTRIBUTE((warn_unused_result));
            static const ChoiceNameWithPrefix canonical_name_with_prefix() PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE ELikeSymbolsChoiceValue :
        public ChoiceValue
    {
        private:
            const bool _enabled;
            const ELikeSymbolsChoiceValueParameter _param;

        public:
            ELikeSymbolsChoiceValue(const std::shared_ptr<const PackageID> &,
                    const Environment * const env, const std::shared_ptr<const Choice> &,
                    const ELikeSymbolsChoiceValueParameter _force);

            const UnprefixedChoiceName unprefixed_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const ChoiceNameWithPrefix name_with_prefix() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool enabled() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool enabled_by_default() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool locked() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string description() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            ChoiceOrigin origin() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string parameter() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::shared_ptr<const PermittedChoiceValueParameterValues> permitted_parameter_values() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool presumed() const override PALUDIS_ATTRIBUTE((warn_unused_result));

            static const UnprefixedChoiceName canonical_unprefixed_name() PALUDIS_ATTRIBUTE((warn_unused_result));
            static const ChoiceNameWithPrefix canonical_name_with_prefix() PALUDIS_ATTRIBUTE((warn_unused_result));

            static bool should_split(const std::string &) PALUDIS_ATTRIBUTE((warn_unused_result));
            static bool should_strip(const std::string &) PALUDIS_ATTRIBUTE((warn_unused_result));
            static bool should_compress(const std::string &) PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE ELikeWorkChoiceValue :
        public ChoiceValue
    {
        private:
            const bool _enabled;
            const ELikeWorkChoiceValueParameter _param;

        public:
            ELikeWorkChoiceValue(const std::shared_ptr<const PackageID> &,
                    const Environment * const env, const std::shared_ptr<const Choice> &,
                    const ELikeWorkChoiceValueParameter force);

            const UnprefixedChoiceName unprefixed_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const ChoiceNameWithPrefix name_with_prefix() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool enabled() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool enabled_by_default() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool locked() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string description() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            ChoiceOrigin origin() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string parameter() const override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::shared_ptr<const PermittedChoiceValueParameterValues> permitted_parameter_values() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool presumed() const override PALUDIS_ATTRIBUTE((warn_unused_result));

            static const UnprefixedChoiceName canonical_unprefixed_name() PALUDIS_ATTRIBUTE((warn_unused_result));
            static const ChoiceNameWithPrefix canonical_name_with_prefix() PALUDIS_ATTRIBUTE((warn_unused_result));

            static bool should_remove(const std::string &) PALUDIS_ATTRIBUTE((warn_unused_result));
            static bool should_remove_on_failure(const std::string &) PALUDIS_ATTRIBUTE((warn_unused_result));
            static bool should_merge_nondestructively(const std::string &) PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE ELikeDwarfCompressionChoiceValue :
        public ChoiceValue
    {
        private:
            bool _enabled;

        public:
            ELikeDwarfCompressionChoiceValue(const std::shared_ptr<const PackageID> &,
                                             const Environment * const,
                                             const std::shared_ptr<const Choice> &);

            ///\name Properties
            ///\{

            const UnprefixedChoiceName unprefixed_name() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));
            const ChoiceNameWithPrefix name_with_prefix() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool enabled() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool enabled_by_default() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool presumed() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));
            bool locked() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string description() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));
            ChoiceOrigin origin() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string parameter() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::shared_ptr<const PermittedChoiceValueParameterValues> permitted_parameter_values() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            /// \}

            static const UnprefixedChoiceName canonical_unprefixed_name()
                PALUDIS_ATTRIBUTE((warn_unused_result));
            static const ChoiceNameWithPrefix canonical_name_with_prefix()
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif

