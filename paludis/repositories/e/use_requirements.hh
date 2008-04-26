/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_USE_REQUIREMENTS_HH
#define PALUDIS_GUARD_PALUDIS_USE_REQUIREMENTS_HH 1

#include <paludis/dep_spec.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/name.hh>

namespace paludis
{
    namespace erepository
    {
        class PALUDIS_VISIBLE UseRequirement :
            public AdditionalPackageDepSpecRequirement
        {
            private:
                const std::string _raw;
                const UseFlagName _name;

            public:
                ///\name Basic operations
                ///\{

                UseRequirement(const std::string &, const UseFlagName &);

                ///\}

                /// Our use flag.
                const UseFlagName flag() const PALUDIS_ATTRIBUTE((warn_unused_result))
                {
                    return _name;
                }

                virtual const std::string as_raw_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        /**
         * An enabled requirement for a use flag.
         *
         * \since 0.26
         */
        class PALUDIS_VISIBLE EnabledUseRequirement :
            public UseRequirement
        {
            public:
                ///\name Basic operations
                ///\{

                EnabledUseRequirement(const std::string &, const UseFlagName &);
                ~EnabledUseRequirement();

                ///\}

                virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE DisabledUseRequirement :
            public UseRequirement
        {
            public:
                ///\name Basic operations
                ///\{

                DisabledUseRequirement(const std::string &, const UseFlagName &);
                ~DisabledUseRequirement();

                ///\}

                virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        /**
         * A use requirement that depends on the use flags of the package
         * it appears in.
         *
         * \since 0.26
         * \ingroup g_dep_spec
         */
        class PALUDIS_VISIBLE ConditionalUseRequirement :
            public UseRequirement
        {
            private:
                const std::tr1::shared_ptr<const PackageID> _id;

            public:
                ///\name Basic operations
                ///\{

                ConditionalUseRequirement(const std::string &, const UseFlagName &, const std::tr1::shared_ptr<const PackageID> &);
                ~ConditionalUseRequirement();

                ///\}

                /// Our package.
                const std::tr1::shared_ptr<const PackageID> package_id() const PALUDIS_ATTRIBUTE((warn_unused_result))
                {
                    return _id;
                }
        };

        /**
         * An if-then requirement for a use flag.
         *
         * \since 0.26
         * \ingroup g_dep_spec
         */
        class PALUDIS_VISIBLE IfMineThenUseRequirement :
            public ConditionalUseRequirement
        {
            public:
                ///\name Basic operations
                ///\{

                IfMineThenUseRequirement(const std::string &, const UseFlagName &, const std::tr1::shared_ptr<const PackageID> &);
                ~IfMineThenUseRequirement();

                ///\}

                virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        /**
         * An if-not-then requirement for a use flag.
         *
         * \since 0.26
         * \ingroup g_dep_spec
         */
        class PALUDIS_VISIBLE IfNotMineThenUseRequirement :
            public ConditionalUseRequirement
        {
            public:
                ///\name Basic operations
                ///\{

                IfNotMineThenUseRequirement(const std::string &, const UseFlagName &, const std::tr1::shared_ptr<const PackageID> &);
                ~IfNotMineThenUseRequirement();

                ///\}

                virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        /**
         * An if-then-not requirement for a use flag.
         *
         * \since 0.26
         * \ingroup g_dep_spec
         */
        class PALUDIS_VISIBLE IfMineThenNotUseRequirement :
            public ConditionalUseRequirement
        {
            public:
                ///\name Basic operations
                ///\{

                IfMineThenNotUseRequirement(const std::string &, const UseFlagName &, const std::tr1::shared_ptr<const PackageID> &);
                ~IfMineThenNotUseRequirement();

                ///\}

                virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        /**
         * An if-not-then-not requirement for a use flag.
         *
         * \since 0.26
         * \ingroup g_dep_spec
         */
        class PALUDIS_VISIBLE IfNotMineThenNotUseRequirement :
            public ConditionalUseRequirement
        {
            public:
                ///\name Basic operations
                ///\{

                IfNotMineThenNotUseRequirement(const std::string &, const UseFlagName &, const std::tr1::shared_ptr<const PackageID> &);
                ~IfNotMineThenNotUseRequirement();

                ///\}

                virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        /**
         * An equal requirement for a use flag.
         *
         * \since 0.26
         * \ingroup g_dep_spec
         */
        class PALUDIS_VISIBLE EqualUseRequirement :
            public ConditionalUseRequirement
        {
            public:
                ///\name Basic operations
                ///\{

                EqualUseRequirement(const std::string &, const UseFlagName &, const std::tr1::shared_ptr<const PackageID> &);
                ~EqualUseRequirement();

                ///\}

                virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        /**
         * A not equal requirement for a use flag.
         *
         * \since 0.26
         * \ingroup g_dep_spec
         */
        class PALUDIS_VISIBLE NotEqualUseRequirement :
            public ConditionalUseRequirement
        {
            public:
                ///\name Basic operations
                ///\{

                NotEqualUseRequirement(const std::string &, const UseFlagName &, const std::tr1::shared_ptr<const PackageID> &);
                ~NotEqualUseRequirement();

                ///\}

                virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
