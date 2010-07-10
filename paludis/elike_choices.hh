/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/choice.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>

namespace paludis
{
    class PALUDIS_VISIBLE ELikeStripChoiceValue :
        public ChoiceValue
    {
        private:
            const bool _enabled;

        public:
            ELikeStripChoiceValue(const std::tr1::shared_ptr<const PackageID> &,
                    const Environment * const env, const std::tr1::shared_ptr<const Choice> &);

            virtual const UnprefixedChoiceName unprefixed_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const ChoiceNameWithPrefix name_with_prefix() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool enabled() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool enabled_by_default() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool locked() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string description() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool explicitly_listed() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string parameter() const PALUDIS_ATTRIBUTE((warn_unused_result));

            static const UnprefixedChoiceName canonical_unprefixed_name() PALUDIS_ATTRIBUTE((warn_unused_result));
            static const ChoiceNameWithPrefix canonical_name_with_prefix() PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE ELikeSplitChoiceValue :
        public ChoiceValue
    {
        private:
            const bool _enabled;

        public:
            ELikeSplitChoiceValue(const std::tr1::shared_ptr<const PackageID> &,
                    const Environment * const env, const std::tr1::shared_ptr<const Choice> &);

            virtual const UnprefixedChoiceName unprefixed_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const ChoiceNameWithPrefix name_with_prefix() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool enabled() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool enabled_by_default() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool locked() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string description() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool explicitly_listed() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string parameter() const PALUDIS_ATTRIBUTE((warn_unused_result));

            static const UnprefixedChoiceName canonical_unprefixed_name() PALUDIS_ATTRIBUTE((warn_unused_result));
            static const ChoiceNameWithPrefix canonical_name_with_prefix() PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE ELikeOptionalTestsChoiceValue :
        public ChoiceValue
    {
        private:
            const bool _enabled;

        public:
            ELikeOptionalTestsChoiceValue(const std::tr1::shared_ptr<const PackageID> &,
                    const Environment * const env, const std::tr1::shared_ptr<const Choice> &);

            virtual const UnprefixedChoiceName unprefixed_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const ChoiceNameWithPrefix name_with_prefix() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool enabled() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool enabled_by_default() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool locked() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string description() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool explicitly_listed() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string parameter() const PALUDIS_ATTRIBUTE((warn_unused_result));

            static const UnprefixedChoiceName canonical_unprefixed_name() PALUDIS_ATTRIBUTE((warn_unused_result));
            static const ChoiceNameWithPrefix canonical_name_with_prefix() PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE ELikeRecommendedTestsChoiceValue :
        public ChoiceValue
    {
        private:
            const bool _enabled;

        public:
            ELikeRecommendedTestsChoiceValue(const std::tr1::shared_ptr<const PackageID> &,
                    const Environment * const env, const std::tr1::shared_ptr<const Choice> &);

            virtual const UnprefixedChoiceName unprefixed_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const ChoiceNameWithPrefix name_with_prefix() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool enabled() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool enabled_by_default() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool locked() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string description() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool explicitly_listed() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string parameter() const PALUDIS_ATTRIBUTE((warn_unused_result));

            static const UnprefixedChoiceName canonical_unprefixed_name() PALUDIS_ATTRIBUTE((warn_unused_result));
            static const ChoiceNameWithPrefix canonical_name_with_prefix() PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE ELikeExpensiveTestsChoiceValue :
        public ChoiceValue
    {
        private:
            const bool _enabled;

        public:
            ELikeExpensiveTestsChoiceValue(const std::tr1::shared_ptr<const PackageID> &,
                    const Environment * const env, const std::tr1::shared_ptr<const Choice> &);

            virtual const UnprefixedChoiceName unprefixed_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const ChoiceNameWithPrefix name_with_prefix() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool enabled() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool enabled_by_default() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool locked() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string description() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool explicitly_listed() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string parameter() const PALUDIS_ATTRIBUTE((warn_unused_result));

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
            ELikeJobsChoiceValue(const std::tr1::shared_ptr<const PackageID> &,
                    const Environment * const env, const std::tr1::shared_ptr<const Choice> &);

            virtual const UnprefixedChoiceName unprefixed_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const ChoiceNameWithPrefix name_with_prefix() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool enabled() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool enabled_by_default() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool locked() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string description() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool explicitly_listed() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string parameter() const PALUDIS_ATTRIBUTE((warn_unused_result));

            static const UnprefixedChoiceName canonical_unprefixed_name() PALUDIS_ATTRIBUTE((warn_unused_result));
            static const ChoiceNameWithPrefix canonical_name_with_prefix() PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE ELikeTraceChoiceValue :
        public ChoiceValue
    {
        private:
            const bool _enabled;

        public:
            ELikeTraceChoiceValue(const std::tr1::shared_ptr<const PackageID> &,
                    const Environment * const env, const std::tr1::shared_ptr<const Choice> &);

            virtual const UnprefixedChoiceName unprefixed_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const ChoiceNameWithPrefix name_with_prefix() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool enabled() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool enabled_by_default() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool locked() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string description() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool explicitly_listed() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string parameter() const PALUDIS_ATTRIBUTE((warn_unused_result));

            static const UnprefixedChoiceName canonical_unprefixed_name() PALUDIS_ATTRIBUTE((warn_unused_result));
            static const ChoiceNameWithPrefix canonical_name_with_prefix() PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE ELikePreserveWorkChoiceValue :
        public ChoiceValue
    {
        private:
            const bool _enabled;

        public:
            /**
             * \since 0.48.1
             */
            ELikePreserveWorkChoiceValue(
                    const std::tr1::shared_ptr<const PackageID> &,
                    const Environment * const env,
                    const std::tr1::shared_ptr<const Choice> &,
                    const bool enabled_by_default);

            virtual const UnprefixedChoiceName unprefixed_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const ChoiceNameWithPrefix name_with_prefix() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool enabled() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool enabled_by_default() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool locked() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string description() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual bool explicitly_listed() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const std::string parameter() const PALUDIS_ATTRIBUTE((warn_unused_result));

            static const UnprefixedChoiceName canonical_unprefixed_name() PALUDIS_ATTRIBUTE((warn_unused_result));
            static const ChoiceNameWithPrefix canonical_name_with_prefix() PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
