/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_FORMATTER_HH
#define PALUDIS_GUARD_PALUDIS_FORMATTER_HH 1

#include <paludis/formatter-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/choice-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/util/attributes.hh>
#include <string>

/** \file
 * Declarations for the Formatter class.
 *
 * \ingroup g_formatters
 *
 * \section Examples
 *
 * - \ref example_formatter.cc "example_formatter.cc"
 * - \ref example_stringify_formatter.cc "example_stringify_formatter.cc"
 */

namespace paludis
{
    /** \namespace paludis::format
     *
     * The paludis::format:: namespace contains various Formatter related
     * utilities.
     *
     * \ingroup g_formatters
     * \since 0.26
     */
    namespace format
    {
        /**
         * Tag to indicate that an item should be formatted as 'plain'.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        struct Plain
        {
        };

        /**
         * Tag to indicate that an item should be formatted as 'enabled'.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        struct Enabled
        {
        };

        /**
         * Tag to indicate that an item should be formatted as 'disabled'.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        struct Disabled
        {
        };

        /**
         * Tag to indicate that an item should be formatted as 'masked'
         * (and disabled -- see format::Forced for masked and enabled).
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        struct Masked
        {
        };

        /**
         * Tag to indicate that an item should be formatted as 'forced'.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        struct Forced
        {
        };

        /**
         * Tag to indicate that an item should be decorated as 'changed'.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        struct Changed
        {
        };

        /**
         * Tag to indicate that an item should be decorated as 'added'.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        struct Added
        {
        };

        /**
         * Tag to indicate that an item should be formatted as 'accepted'.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        struct Accepted
        {
        };

        /**
         * Tag to indicate that an item should be formatted as 'unaccepted'.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        struct Unaccepted
        {
        };

        /**
         * Tag to indicate that an item should be formatted as 'installed'.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        struct Installed
        {
        };

        /**
         * Tag to indicate that an item should be formatted as 'installable'
         * (but not installed).
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        struct Installable
        {
        };

        /**
         * Used by CategorySelector<> to declare that format::Plain is the only
         * role supported by a particular class.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        struct PlainRoles;

        /**
         * Used by CategorySelector<> to declare that format::Plain,
         * format::Enabled, format::Disabled, format::Forced, format::Masked,
         * format::Added and format::Changed are the roles supported by a
         * particular class.
         *
         * \ingroup g_formatters
         * \since 0.32
         * \nosubgrouping
         */
        struct ChoiceRoles;

        /**
         * Used by CategorySelector<> to declare that format::Plain,
         * format::Accepted and format::Unaccepted are the roles supported by a
         * particular class.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        struct AcceptableRoles;

        /**
         * Used by CategorySelector<> to declare that format::Plain,
         * format::Installed and format::Installable are the roles supported by
         * a particular class.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        struct PackageRoles;

        /**
         * Used by CategorySelector<> to declare that no roles at all are
         * supported by a particular class.
         *
         * This category is not used by any 'real' class. It is used for
         * NoType<> to work around the lack of variadic templates in the current
         * C++ standard.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        struct NoRoles;

        /**
         * By default, a type supports format::PlainRoles.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        template <typename T_>
        struct CategorySelector
        {
            /// The roles this type supports.
            typedef PlainRoles Category;
        };

        /**
         * ChoiceValue supports ChoiceRoles.
         *
         * \ingroup g_formatters
         * \since 0.32
         * \nosubgrouping
         */
        template <>
        struct CategorySelector<ChoiceValue>
        {
            /// The roles this type supports.
            typedef ChoiceRoles Category;
        };

        /**
         * ConditionalDepSpec supports ChoiceRoles.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        template <>
        struct CategorySelector<ConditionalDepSpec>
        {
            /// The roles this type supports.
            typedef ChoiceRoles Category;
        };

        /**
         * LicenseDepSpec supports AcceptableRoles.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        template <>
        struct CategorySelector<LicenseDepSpec>
        {
            /// The roles this type supports.
            typedef AcceptableRoles Category;
        };

        /**
         * KeywordName supports AcceptableRoles.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        template <>
        struct CategorySelector<KeywordName>
        {
            /// The roles this type supports.
            typedef AcceptableRoles Category;
        };

        /**
         * PackageDepSpec supports PackageRoles.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        template <>
        struct CategorySelector<PackageDepSpec>
        {
            /// The roles this type supports.
            typedef PackageRoles Category;
        };

        /**
         * PackageID supports PackageRoles.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        template <>
        struct CategorySelector<PackageID>
        {
            /// The roles this type supports.
            typedef PackageRoles Category;
        };

        /**
         * A std::shared_ptr<T_> shouldn't be specified.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        template <typename T_>
        struct CategorySelector<std::shared_ptr<T_> >
        {
            /// This role is wrong.
            typedef typename CategorySelector<T_>::ThisRoleIsWrong ThisRoleIsWrong;
        };

        /**
         * A std::shared_ptr<const T_> shouldn't be specified.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        template <typename T_>
        struct CategorySelector<std::shared_ptr<const T_> >
        {
            /// This role is wrong.
            typedef typename CategorySelector<T_>::ThisRoleIsWrong ThisRoleIsWrong;
        };

        /**
         * A const T_ supports the same roles as T_.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        template <typename T_>
        struct CategorySelector<const T_>
        {
            /// The roles this type supports.
            typedef typename CategorySelector<T_>::Category Category;
        };

        /**
         * NoType<> doesn't support any format roles.
         *
         * Used to work around the lack of variadic templates in the current C++
         * standard.
         *
         * \ingroup g_formatters
         * \since 0.26
         * \nosubgrouping
         */
        template <unsigned u_>
        struct CategorySelector<NoType<u_> >
        {
            /// The roles this type supports.
            typedef NoRoles Category;
        };

        ///\}
    }

    template <typename T_, typename C_>
    struct CanFormatBase;

    /**
     * Base class for anything that implements the format functions for
     * format::PlainRoles on type T_.
     *
     * \ingroup g_formatters
     * \since 0.26
     * \nosubgrouping
     */
    template <typename T_>
    class PALUDIS_VISIBLE CanFormatBase<T_, format::PlainRoles>
    {
        public:
            ///\name Basic operations
            ///\{

            CanFormatBase()
            {
            }

            virtual ~CanFormatBase()
            {
            }

            ///\}

            /**
             * Format this item as 'Plain'.
             */
            virtual std::string format(const T_ &, const format::Plain &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * Base class for anything that implements the format functions for
     * format::AcceptableRoles on type T_.
     *
     * \ingroup g_formatters
     * \since 0.26
     * \nosubgrouping
     */
    template <typename T_>
    class PALUDIS_VISIBLE CanFormatBase<T_, format::AcceptableRoles>
    {
        public:
            ///\name Basic operations
            ///\{

            CanFormatBase()
            {
            }

            virtual ~CanFormatBase()
            {
            }

            ///\}

            /**
             * Format this item as 'Plain'.
             */
            virtual std::string format(const T_ &, const format::Plain &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Format this item as 'Accepted'.
             */
            virtual std::string format(const T_ &, const format::Accepted &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Format this item as 'Unaccepted'.
             */
            virtual std::string format(const T_ &, const format::Unaccepted &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * Base class for anything that implements the format functions for
     * format::ChoiceRoles on type T_.
     *
     * \ingroup g_formatters
     * \since 0.26
     * \nosubgrouping
     */
    template <typename T_>
    class PALUDIS_VISIBLE CanFormatBase<T_, format::ChoiceRoles>
    {
        public:
            ///\name Basic operations
            ///\{

            CanFormatBase()
            {
            }

            virtual ~CanFormatBase()
            {
            }

            ///\}

            /**
             * Format this item as 'Plain'.
             */
            virtual std::string format(const T_ &, const format::Plain &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Format this item as 'Enabled'.
             */
            virtual std::string format(const T_ &, const format::Enabled &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Format this item as 'Disabled'.
             */
            virtual std::string format(const T_ &, const format::Disabled &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Format this item as 'Forced'.
             */
            virtual std::string format(const T_ &, const format::Forced &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Format this item as 'Masked'.
             */
            virtual std::string format(const T_ &, const format::Masked &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Decorate this item as 'Added'.
             */
            virtual std::string decorate(const T_ &, const std::string &, const format::Added &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Decorate this item as 'Changed'.
             */
            virtual std::string decorate(const T_ &, const std::string &, const format::Changed &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * Base class for anything that implements the format functions for
     * format::PackageRoles on type T_.
     *
     * \ingroup g_formatters
     * \since 0.26
     * \nosubgrouping
     */
    template <typename T_>
    class PALUDIS_VISIBLE CanFormatBase<T_, format::PackageRoles>
    {
        public:
            ///\name Basic operations
            ///\{

            CanFormatBase()
            {
            }

            virtual ~CanFormatBase()
            {
            }

            ///\}

            /**
             * Format this item as 'Plain'.
             */
            virtual std::string format(const T_ &, const format::Plain &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Format this item as 'Installed'.
             */
            virtual std::string format(const T_ &, const format::Installed &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Format this item as 'Installable'.
             */
            virtual std::string format(const T_ &, const format::Installable &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * Base class for anything that implements the format functions for
     * format::NoRoles on type NoType<T_>.
     *
     * Used to work around the lack of variadic templates in the current C++
     * standard.
     *
     * \ingroup g_formatters
     * \since 0.26
     * \nosubgrouping
     */
    template <unsigned u_>
    class PALUDIS_VISIBLE CanFormatBase<NoType<u_>, format::NoRoles>
    {
        public:
            ///\name Basic operations
            ///\{

            CanFormatBase()
            {
            }

            ///\}
    };

    /**
     * Descendents of this class implement the necessary methods to format an
     * item of type T_.
     *
     * \ingroup g_formatters
     * \since 0.26
     * \nosubgrouping
     */
    template <typename T_>
    class PALUDIS_VISIBLE CanFormat :
        public CanFormatBase<T_, typename format::CategorySelector<T_>::Category>
    {
        public:
            ///\name Basic operations
            ///\{

            CanFormat()
            {
            }

            ///\}
    };

    /**
     * Descendents of this class implement the necessary methods to format
     * whitespace.
     *
     * \ingroup g_formatters
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE CanSpace
    {
        public:
            ///\name Basic operations
            ///\{

            CanSpace()
            {
            }

            virtual ~CanSpace()
            {
            }

            ///\}

            /**
             * Output a newline.
             */
            virtual std::string newline() const = 0;

            /**
             * Output an indent marker of the specified indent level.
             */
            virtual std::string indent(const int) const = 0;
    };

    template <typename T_, typename C_, unsigned u_>
    class FormatFunctionsByProxy;

    /**
     * Used by Formatter to implement the CanFormat<T_> interface.
     *
     * \ingroup g_formatters
     * \since 0.26
     * \nosubgrouping
     */
    template <typename T_, unsigned u_>
    class PALUDIS_VISIBLE FormatFunctionsByProxy<T_, format::PlainRoles, u_> :
        public CanFormat<T_>
    {
        private:
            const CanFormat<T_> * const _proxy;

        public:
            ///\name Basic operations
            ///\{

            FormatFunctionsByProxy(const CanFormat<T_> * const p) :
                _proxy(p)
            {
            }

            ///\}

            virtual std::string format(const T_ & s, const format::Plain & p) const
            {
                return _proxy->format(s, p);
            }
    };

    /**
     * Used by Formatter to implement the CanFormat<T_> interface.
     *
     * \ingroup g_formatters
     * \since 0.26
     * \nosubgrouping
     */
    template <typename T_, unsigned u_>
    class PALUDIS_VISIBLE FormatFunctionsByProxy<T_, format::AcceptableRoles, u_> :
        public CanFormat<T_>
    {
        private:
            const CanFormat<T_> * const _proxy;

        public:
            ///\name Basic operations
            ///\{

            FormatFunctionsByProxy(const CanFormat<T_> * const p) :
                _proxy(p)
            {
            }

            ///\}

            virtual std::string format(const T_ & s, const format::Plain & p) const
            {
                return _proxy->format(s, p);
            }

            virtual std::string format(const T_ & s, const format::Accepted & p) const
            {
                return _proxy->format(s, p);
            }

            virtual std::string format(const T_ & s, const format::Unaccepted & p) const
            {
                return _proxy->format(s, p);
            }
    };

    /**
     * Used by Formatter to implement the CanFormat<T_> interface.
     *
     * \ingroup g_formatters
     * \since 0.32
     * \nosubgrouping
     */
    template <typename T_, unsigned u_>
    class PALUDIS_VISIBLE FormatFunctionsByProxy<T_, format::ChoiceRoles, u_> :
        public CanFormat<T_>
    {
        private:
            const CanFormat<T_> * const _proxy;

        public:
            ///\name Basic operations
            ///\{

            FormatFunctionsByProxy(const CanFormat<T_> * const p) :
                _proxy(p)
            {
            }

            ///\}

            virtual std::string format(const T_ & s, const format::Plain & p) const
            {
                return _proxy->format(s, p);
            }

            virtual std::string format(const T_ & s, const format::Enabled & p) const
            {
                return _proxy->format(s, p);
            }

            virtual std::string format(const T_ & s, const format::Disabled & p) const
            {
                return _proxy->format(s, p);
            }

            virtual std::string format(const T_ & s, const format::Forced & p) const
            {
                return _proxy->format(s, p);
            }

            virtual std::string format(const T_ & s, const format::Masked & p) const
            {
                return _proxy->format(s, p);
            }

            virtual std::string decorate(const T_ & t, const std::string & s, const format::Changed & p) const
            {
                return _proxy->decorate(t, s, p);
            }

            virtual std::string decorate(const T_ & t, const std::string & s, const format::Added & p) const
            {
                return _proxy->decorate(t, s, p);
            }
    };

    /**
     * Used by Formatter to implement the CanFormat<T_> interface.
     *
     * \ingroup g_formatters
     * \since 0.26
     * \nosubgrouping
     */
    template <typename T_, unsigned u_>
    class PALUDIS_VISIBLE FormatFunctionsByProxy<T_, format::PackageRoles, u_> :
        public CanFormat<T_>
    {
        private:
            const CanFormat<T_> * const _proxy;

        public:
            ///\name Basic operations
            ///\{

            FormatFunctionsByProxy(const CanFormat<T_> * const p) :
                _proxy(p)
            {
            }

            ///\}

            virtual std::string format(const T_ & s, const format::Plain & p) const
            {
                return _proxy->format(s, p);
            }

            virtual std::string format(const T_ & s, const format::Installed & p) const
            {
                return _proxy->format(s, p);
            }

            virtual std::string format(const T_ & s, const format::Installable & p) const
            {
                return _proxy->format(s, p);
            }
    };

    /**
     * Used by Formatter to implement the CanFormat<T_> interface.
     *
     * \ingroup g_formatters
     * \since 0.26
     * \nosubgrouping
     */
    template <unsigned u_>
    class PALUDIS_VISIBLE FormatFunctionsByProxy<NoType<u_>, format::NoRoles, u_>
    {
        public:
            ///\name Basic operations
            ///\{

            FormatFunctionsByProxy(const void * const)
            {
            }

            ///\}

            void format(const NoType<u_> &) const;
    };

    /**
     * A Formatter is a class that implements all the format routines for each
     * of its template parameters.
     *
     * A Formatter is required by most MetadataKey pretty_print methods. Instead
     * of requiring that formatters support every format method with every
     * possible role for every class, scary template voodoo is used to ensure that
     * only the format methods appropriate for the classes passed as template
     * parameters with roles appropriate for those classes are required.
     *
     * A Formatter can be implicitly constructed from any type that implements
     * CanFormat<> for every requested type, as well as the CanSpace interface.
     *
     * For a basic formatter that uses paludis::stringify() to do all
     * formatting, see StringifyFormatter.
     *
     * \ingroup g_formatters
     * \since 0.26
     * \nosubgrouping
     */
    template <
        typename T1_,
        typename T2_,
        typename T3_,
        typename T4_,
        typename T5_,
        typename T6_,
        typename T7_,
        typename T8_,
        typename T9_,
        typename T10_,
        typename T11_,
        typename T12_,
        typename T13_,
        typename T14_,
        typename T15_
        >
    class PALUDIS_VISIBLE Formatter :
        public FormatFunctionsByProxy<T1_, typename format::CategorySelector<T1_>::Category, 1>,
        public FormatFunctionsByProxy<T2_, typename format::CategorySelector<T2_>::Category, 2>,
        public FormatFunctionsByProxy<T3_, typename format::CategorySelector<T3_>::Category, 3>,
        public FormatFunctionsByProxy<T4_, typename format::CategorySelector<T4_>::Category, 4>,
        public FormatFunctionsByProxy<T5_, typename format::CategorySelector<T5_>::Category, 5>,
        public FormatFunctionsByProxy<T6_, typename format::CategorySelector<T6_>::Category, 6>,
        public FormatFunctionsByProxy<T7_, typename format::CategorySelector<T7_>::Category, 7>,
        public FormatFunctionsByProxy<T8_, typename format::CategorySelector<T8_>::Category, 8>,
        public FormatFunctionsByProxy<T9_, typename format::CategorySelector<T9_>::Category, 9>,
        public FormatFunctionsByProxy<T10_, typename format::CategorySelector<T10_>::Category, 10>,
        public FormatFunctionsByProxy<T11_, typename format::CategorySelector<T11_>::Category, 11>,
        public FormatFunctionsByProxy<T12_, typename format::CategorySelector<T12_>::Category, 12>,
        public FormatFunctionsByProxy<T13_, typename format::CategorySelector<T13_>::Category, 13>,
        public FormatFunctionsByProxy<T14_, typename format::CategorySelector<T14_>::Category, 14>,
        public FormatFunctionsByProxy<T15_, typename format::CategorySelector<T15_>::Category, 15>,
        public CanSpace
    {
        private:
            const CanSpace * const _proxy;

        public:
            ///\name Basic operations
            ///\{

            /**
             * A Formatter is implicitly constructible from any type that
             * supports all the relevant CanFormat<> interfaces, as well as the
             * CanSpace interface.
             */
            template <typename T_>
            Formatter(const T_ & t) :
                FormatFunctionsByProxy<T1_, typename format::CategorySelector<T1_>::Category, 1>(&t),
                FormatFunctionsByProxy<T2_, typename format::CategorySelector<T2_>::Category, 2>(&t),
                FormatFunctionsByProxy<T3_, typename format::CategorySelector<T3_>::Category, 3>(&t),
                FormatFunctionsByProxy<T4_, typename format::CategorySelector<T4_>::Category, 4>(&t),
                FormatFunctionsByProxy<T5_, typename format::CategorySelector<T5_>::Category, 5>(&t),
                FormatFunctionsByProxy<T6_, typename format::CategorySelector<T6_>::Category, 6>(&t),
                FormatFunctionsByProxy<T7_, typename format::CategorySelector<T7_>::Category, 7>(&t),
                FormatFunctionsByProxy<T8_, typename format::CategorySelector<T8_>::Category, 8>(&t),
                FormatFunctionsByProxy<T9_, typename format::CategorySelector<T9_>::Category, 9>(&t),
                FormatFunctionsByProxy<T10_, typename format::CategorySelector<T10_>::Category, 10>(&t),
                FormatFunctionsByProxy<T11_, typename format::CategorySelector<T11_>::Category, 11>(&t),
                FormatFunctionsByProxy<T12_, typename format::CategorySelector<T12_>::Category, 12>(&t),
                FormatFunctionsByProxy<T13_, typename format::CategorySelector<T13_>::Category, 13>(&t),
                FormatFunctionsByProxy<T14_, typename format::CategorySelector<T14_>::Category, 14>(&t),
                FormatFunctionsByProxy<T15_, typename format::CategorySelector<T15_>::Category, 15>(&t),
                CanSpace(),
                _proxy(&t)
            {
            }

            Formatter(const Formatter & other) :
                FormatFunctionsByProxy<T1_, typename format::CategorySelector<T1_>::Category, 1>(other),
                FormatFunctionsByProxy<T2_, typename format::CategorySelector<T2_>::Category, 2>(other),
                FormatFunctionsByProxy<T3_, typename format::CategorySelector<T3_>::Category, 3>(other),
                FormatFunctionsByProxy<T4_, typename format::CategorySelector<T4_>::Category, 4>(other),
                FormatFunctionsByProxy<T5_, typename format::CategorySelector<T5_>::Category, 5>(other),
                FormatFunctionsByProxy<T6_, typename format::CategorySelector<T6_>::Category, 6>(other),
                FormatFunctionsByProxy<T7_, typename format::CategorySelector<T7_>::Category, 7>(other),
                FormatFunctionsByProxy<T8_, typename format::CategorySelector<T8_>::Category, 8>(other),
                FormatFunctionsByProxy<T9_, typename format::CategorySelector<T9_>::Category, 9>(other),
                FormatFunctionsByProxy<T10_, typename format::CategorySelector<T10_>::Category, 10>(other),
                FormatFunctionsByProxy<T11_, typename format::CategorySelector<T11_>::Category, 11>(other),
                FormatFunctionsByProxy<T12_, typename format::CategorySelector<T12_>::Category, 12>(other),
                FormatFunctionsByProxy<T13_, typename format::CategorySelector<T13_>::Category, 13>(other),
                FormatFunctionsByProxy<T14_, typename format::CategorySelector<T14_>::Category, 14>(other),
                FormatFunctionsByProxy<T15_, typename format::CategorySelector<T15_>::Category, 15>(other),
                CanSpace(other),
                _proxy(other._proxy)
            {
            }

            ///\}

            using FormatFunctionsByProxy<T1_, typename format::CategorySelector<T1_>::Category, 1>::format;
            using FormatFunctionsByProxy<T2_, typename format::CategorySelector<T2_>::Category, 2>::format;
            using FormatFunctionsByProxy<T3_, typename format::CategorySelector<T3_>::Category, 3>::format;
            using FormatFunctionsByProxy<T4_, typename format::CategorySelector<T4_>::Category, 4>::format;
            using FormatFunctionsByProxy<T5_, typename format::CategorySelector<T5_>::Category, 5>::format;
            using FormatFunctionsByProxy<T6_, typename format::CategorySelector<T6_>::Category, 6>::format;
            using FormatFunctionsByProxy<T7_, typename format::CategorySelector<T7_>::Category, 7>::format;
            using FormatFunctionsByProxy<T8_, typename format::CategorySelector<T8_>::Category, 8>::format;
            using FormatFunctionsByProxy<T9_, typename format::CategorySelector<T9_>::Category, 9>::format;
            using FormatFunctionsByProxy<T10_, typename format::CategorySelector<T10_>::Category, 10>::format;
            using FormatFunctionsByProxy<T11_, typename format::CategorySelector<T11_>::Category, 11>::format;
            using FormatFunctionsByProxy<T12_, typename format::CategorySelector<T12_>::Category, 12>::format;
            using FormatFunctionsByProxy<T13_, typename format::CategorySelector<T13_>::Category, 13>::format;
            using FormatFunctionsByProxy<T14_, typename format::CategorySelector<T14_>::Category, 14>::format;
            using FormatFunctionsByProxy<T15_, typename format::CategorySelector<T15_>::Category, 15>::format;

            virtual std::string newline() const
            {
                return _proxy->newline();
            }

            virtual std::string indent(const int i) const
            {
                return _proxy->indent(i);
            }
    };
}

#endif
