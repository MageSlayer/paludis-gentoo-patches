/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_NAME_HH
#define PALUDIS_GUARD_PALUDIS_NAME_HH 1

#include <paludis/name-fwd.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/validated.hh>
#include <paludis/util/named_value.hh>

#include <string>
#include <iosfwd>

/** \file
 * Declarations for Name classes.
 *
 * \ingroup g_names
 *
 * \section Examples
 *
 * - \ref example_name.cc "example_name.cc"
 */

namespace paludis
{
    namespace n
    {
        typedef Name<struct category_name> category;
        typedef Name<struct package_name> package;
    }

    /**
     * A PackageNamePartError is thrown if an invalid value is assigned to
     * a PackageNamePart.
     *
     * \ingroup g_names
     * \ingroup g_exceptions
     */
    class PALUDIS_VISIBLE PackageNamePartError :
        public NameError
    {
        public:
            /**
             * Constructor.
             */
            PackageNamePartError(const std::string & name) throw ();
    };

    /**
     * A PackageNamePartValidator handles validation rules for the value
     * of a PackageNamePart.
     *
     * \ingroup g_names
     */
    struct PALUDIS_VISIBLE PackageNamePartValidator :
        private InstantiationPolicy<PackageNamePartValidator, instantiation_method::NonInstantiableTag>
    {
        /**
         * If the parameter is not a valid value for a PackageNamePart,
         * throw a PackageNamePartError.
         */
        static void validate(const std::string &);
    };

    /**
     * A CategoryNamePartError is thrown if an invalid value is assigned to
     * a CategoryNamePart.
     *
     * \ingroup g_exceptions
     * \ingroup g_names
     */
    class PALUDIS_VISIBLE CategoryNamePartError :
        public NameError
    {
        public:
            /**
             * Constructor.
             */
            CategoryNamePartError(const std::string & name) throw ();
    };

    /**
     * A CategoryNamePartValidator handles validation rules for the value
     * of a CategoryNamePart.
     *
     * \ingroup g_names
     */
    struct PALUDIS_VISIBLE CategoryNamePartValidator :
        private InstantiationPolicy<CategoryNamePartValidator, instantiation_method::NonInstantiableTag>
    {
        /**
         * If the parameter is not a valid value for a CategoryNamePart,
         * throw a CategoryNamePartError.
         */
        static void validate(const std::string &);
    };

    /**
     * Represents a category plus package name.
     *
     * \ingroup g_names
     * \nosubgrouping
     */
    struct PALUDIS_VISIBLE QualifiedPackageName :
        relational_operators::HasRelationalOperators
    {
        NamedValue<n::category, CategoryNamePart> category;
        NamedValue<n::package, PackageNamePart> package;

        QualifiedPackageName(const CategoryNamePart &, const PackageNamePart &);
        explicit QualifiedPackageName(const std::string &);

        std::size_t hash() const PALUDIS_ATTRIBUTE((warn_unused_result));

        bool operator< (const QualifiedPackageName &) const PALUDIS_ATTRIBUTE((warn_unused_result));
        bool operator== (const QualifiedPackageName &) const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A QualifiedPackageNameError may be thrown if an invalid name is
     * assigned to a QualifiedPackageName (alternatively, the exception
     * raised may be a PackageNamePartError or a CategoryNamePartError).
     *
     * \ingroup g_names
     * \ingroup g_exceptions
     */
    class PALUDIS_VISIBLE QualifiedPackageNameError : public NameError
    {
        public:
            /**
             * Constructor.
             */
            QualifiedPackageNameError(const std::string &) throw ();
    };

    inline const QualifiedPackageName
    operator+ (const CategoryNamePart & c, const PackageNamePart & p)
    {
        return QualifiedPackageName(c, p);
    }

    /**
     * A SlotNameError is thrown if an invalid value is assigned to
     * a SlotName.
     *
     * \ingroup g_names
     * \ingroup g_exceptions
     */
    class PALUDIS_VISIBLE SlotNameError : public NameError
    {
        public:
            /**
             * Constructor.
             */
            SlotNameError(const std::string & name) throw ();
    };

    /**
     * A SlotNameValidator handles validation rules for the value of a
     * SlotName.
     *
     * \ingroup g_names
     */
    struct PALUDIS_VISIBLE SlotNameValidator :
        private InstantiationPolicy<SlotNameValidator, instantiation_method::NonInstantiableTag>
    {
        /**
         * If the parameter is not a valid value for a SlotName,
         * throw a SlotNameError.
         */
        static void validate(const std::string &);
    };

    /**
     * A RepositoryNameError is thrown if an invalid value is assigned to
     * a RepositoryName.
     *
     * \ingroup g_exceptions
     * \ingroup g_names
     */
    class PALUDIS_VISIBLE RepositoryNameError : public NameError
    {
        public:
            /**
             * Constructor.
             */
            RepositoryNameError(const std::string & name) throw ();
    };

    /**
     * A RepositoryNameValidator handles validation rules for the value
     * of a RepositoryName.
     *
     * \ingroup g_names
     */
    struct PALUDIS_VISIBLE RepositoryNameValidator :
        private InstantiationPolicy<RepositoryNameValidator, instantiation_method::NonInstantiableTag>
    {
        /**
         * If the parameter is not a valid value for a RepositoryName,
         * throw a RepositoryNameError.
         */
        static void validate(const std::string &);
    };

    /**
     * Arbitrary useless comparator for RepositoryName.
     *
     * \ingroup g_names
     */
    struct PALUDIS_VISIBLE RepositoryNameComparator
    {
        /**
         * Perform the comparison.
         */
        bool operator() (const RepositoryName & lhs, const RepositoryName & rhs) const
        {
            return lhs.data() < rhs.data();
        }
    };

    /**
     * A KeywordNameValidator handles validation rules for the value of a
     * KeywordName.
     *
     * \ingroup g_names
     */
    struct PALUDIS_VISIBLE KeywordNameValidator :
        private InstantiationPolicy<KeywordNameValidator, instantiation_method::NonInstantiableTag>
    {
        /**
         * If the parameter is not a valid value for a KeywordName,
         * throw a KeywordNameError.
         */
        static void validate(const std::string &);
    };

    /**
     * A KeywordNameError is thrown if an invalid value is assigned to
     * a KeywordName.
     *
     * \ingroup g_names
     * \ingroup g_exceptions
     */
    class PALUDIS_VISIBLE KeywordNameError : public NameError
    {
        public:
            /**
             * Constructor.
             */
            KeywordNameError(const std::string & name) throw ();
    };

    /**
     * Comparator for a KeywordName.
     *
     * \ingroup g_names
     * \since 0.26
     */
    class PALUDIS_VISIBLE KeywordNameComparator
    {
        public:
            /**
             * Perform a less-than comparison.
             */
            bool operator() (const std::string &, const std::string &) const;
    };

    /**
     * A SetNameValidator handles validation rules for the value of a
     * SetName.
     *
     * \ingroup g_exceptions
     */
    struct PALUDIS_VISIBLE SetNameValidator :
        private InstantiationPolicy<SetNameValidator, instantiation_method::NonInstantiableTag>
    {
        /**
         * If the parameter is not a valid value for a SetName,
         * throw a SetNameError.
         */
        static void validate(const std::string &);
    };

    /**
     * A SetNameError is thrown if an invalid value is assigned to
     * a SetName.
     *
     * \ingroup g_exceptions
     * \ingroup g_names
     */
    class PALUDIS_VISIBLE SetNameError : public NameError
    {
        public:
            /**
             * Constructor.
             */
            SetNameError(const std::string & name) throw ();
    };
}

#endif
