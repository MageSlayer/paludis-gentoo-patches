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
#include <paludis/util/wrapped_value.hh>
#include <paludis/util/operators.hh>

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

    template <>
    struct PALUDIS_VISIBLE WrappedValueTraits<PackageNamePartTag>
    {
        typedef std::string UnderlyingType;
        typedef void ValidationParamsType;
        typedef PackageNamePartError ExceptionType;

        static bool validate(const std::string &) PALUDIS_ATTRIBUTE((warn_unused_result));
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

    template <>
    struct PALUDIS_VISIBLE WrappedValueTraits<CategoryNamePartTag>
    {
        typedef std::string UnderlyingType;
        typedef void ValidationParamsType;
        typedef CategoryNamePartError ExceptionType;

        static bool validate(const std::string &) PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * Represents a category plus package name.
     *
     * \ingroup g_names
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE QualifiedPackageName :
        public relational_operators::HasRelationalOperators
    {
        private:
            CategoryNamePart _cat;
            PackageNamePart _pkg;

        public:
            QualifiedPackageName(const CategoryNamePart &, const PackageNamePart &);
            explicit QualifiedPackageName(const std::string &);

            std::size_t hash() const PALUDIS_ATTRIBUTE((warn_unused_result));

            bool operator< (const QualifiedPackageName &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            bool operator== (const QualifiedPackageName &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            const CategoryNamePart category() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return _cat;
            }

            const PackageNamePart package() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return _pkg;
            }
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

    template <>
    struct PALUDIS_VISIBLE WrappedValueTraits<SlotNameTag>
    {
        typedef std::string UnderlyingType;
        typedef void ValidationParamsType;
        typedef SlotNameError ExceptionType;

        static bool validate(const std::string &) PALUDIS_ATTRIBUTE((warn_unused_result));
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

    template <>
    struct PALUDIS_VISIBLE WrappedValueTraits<RepositoryNameTag>
    {
        typedef std::string UnderlyingType;
        typedef void ValidationParamsType;
        typedef RepositoryNameError ExceptionType;

        static bool validate(const std::string &) PALUDIS_ATTRIBUTE((warn_unused_result));
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

    template <>
    struct PALUDIS_VISIBLE WrappedValueTraits<KeywordNameTag>
    {
        typedef std::string UnderlyingType;
        typedef void ValidationParamsType;
        typedef KeywordNameError ExceptionType;

        static bool validate(const std::string &) PALUDIS_ATTRIBUTE((warn_unused_result));
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

    template <>
    struct PALUDIS_VISIBLE WrappedValueTraits<SetNameTag>
    {
        typedef std::string UnderlyingType;
        typedef void ValidationParamsType;
        typedef SetNameError ExceptionType;

        static bool validate(const std::string &) PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
