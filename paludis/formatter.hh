/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/dep_spec-fwd.hh>
#include <paludis/util/attributes.hh>
#include <string>

namespace paludis
{
    namespace format
    {
        struct Plain
        {
        };

        struct Enabled
        {
        };

        struct Disabled
        {
        };

        struct Masked
        {
        };

        struct Forced
        {
        };

        struct Changed
        {
        };

        struct Added
        {
        };

        struct Accepted
        {
        };

        struct Unaccepted
        {
        };

        struct Installed
        {
        };

        struct Installable
        {
        };

        struct PlainRoles;
        struct IUseRoles;
        struct UseRoles;
        struct AcceptableRoles;
        struct PackageRoles;
        struct NoRoles;

        template <typename T_>
        struct CategorySelector
        {
            typedef PlainRoles Category;
        };

        template <>
        struct CategorySelector<IUseFlag>
        {
            typedef IUseRoles Category;
        };

        template <>
        struct CategorySelector<UseFlagName>
        {
            typedef UseRoles Category;
        };

        template <>
        struct CategorySelector<UseDepSpec>
        {
            typedef UseRoles Category;
        };

        template <>
        struct CategorySelector<LicenseDepSpec>
        {
            typedef AcceptableRoles Category;
        };

        template <>
        struct CategorySelector<KeywordName>
        {
            typedef AcceptableRoles Category;
        };

        template <>
        struct CategorySelector<PackageDepSpec>
        {
            typedef PackageRoles Category;
        };

        template <>
        struct CategorySelector<PackageID>
        {
            typedef PackageRoles Category;
        };

        template <typename T_>
        struct CategorySelector<tr1::shared_ptr<T_> >
        {
            typedef typename CategorySelector<T_>::Category Category;
        };

        template <typename T_>
        struct CategorySelector<const T_>
        {
            typedef typename CategorySelector<T_>::Category Category;
        };

        template <unsigned u_>
        struct CategorySelector<NoType<u_> >
        {
            typedef NoRoles Category;
        };
    }

    template <typename T_, typename C_>
    struct CanFormatBase;

    template <typename T_>
    class PALUDIS_VISIBLE CanFormatBase<T_, format::PlainRoles>
    {
        public:
            CanFormatBase()
            {
            }

            virtual ~CanFormatBase()
            {
            }

            virtual std::string format(const T_ &, const format::Plain &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    template <typename T_>
    class PALUDIS_VISIBLE CanFormatBase<T_, format::AcceptableRoles>
    {
        public:
            CanFormatBase()
            {
            }

            virtual ~CanFormatBase()
            {
            }

            virtual std::string format(const T_ &, const format::Plain &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::string format(const T_ &, const format::Accepted &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::string format(const T_ &, const format::Unaccepted &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    template <typename T_>
    class PALUDIS_VISIBLE CanFormatBase<T_, format::UseRoles>
    {
        public:
            CanFormatBase()
            {
            }

            virtual ~CanFormatBase()
            {
            }

            virtual std::string format(const T_ &, const format::Plain &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::string format(const T_ &, const format::Enabled &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::string format(const T_ &, const format::Disabled &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::string format(const T_ &, const format::Forced &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::string format(const T_ &, const format::Masked &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    template <typename T_>
    class PALUDIS_VISIBLE CanFormatBase<T_, format::IUseRoles>
    {
        public:
            CanFormatBase()
            {
            }

            virtual ~CanFormatBase()
            {
            }

            virtual std::string format(const T_ &, const format::Plain &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::string format(const T_ &, const format::Enabled &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::string format(const T_ &, const format::Disabled &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::string format(const T_ &, const format::Forced &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::string format(const T_ &, const format::Masked &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::string decorate(const T_ &, const std::string &, const format::Added &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::string decorate(const T_ &, const std::string &, const format::Changed &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    template <typename T_>
    class PALUDIS_VISIBLE CanFormatBase<T_, format::PackageRoles>
    {
        public:
            CanFormatBase()
            {
            }

            virtual ~CanFormatBase()
            {
            }

            virtual std::string format(const T_ &, const format::Plain &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::string format(const T_ &, const format::Installed &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::string format(const T_ &, const format::Installable &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    template <unsigned u_>
    class PALUDIS_VISIBLE CanFormatBase<NoType<u_>, format::NoRoles>
    {
        public:
            CanFormatBase()
            {
            }
    };

    template <typename T_>
    class PALUDIS_VISIBLE CanFormat :
        public CanFormatBase<T_, typename format::CategorySelector<T_>::Category>
    {
        public:
            CanFormat()
            {
            }
    };

    class PALUDIS_VISIBLE CanSpace
    {
        public:
            CanSpace()
            {
            }

            virtual ~CanSpace()
            {
            }

            virtual std::string newline() const = 0;
            virtual std::string indent(const int) const = 0;
    };

    template <typename T_, typename C_, unsigned u_>
    class FormatFunctionsByProxy;

    template <typename T_, unsigned u_>
    class PALUDIS_VISIBLE FormatFunctionsByProxy<T_, format::PlainRoles, u_> :
        public CanFormat<T_>
    {
        private:
            const CanFormat<T_> * const _proxy;

        public:
            FormatFunctionsByProxy(const CanFormat<T_> * const p) :
                _proxy(p)
            {
            }

            virtual std::string format(const T_ & s, const format::Plain & p) const
            {
                return _proxy->format(s, p);
            }
    };

    template <typename T_, unsigned u_>
    class PALUDIS_VISIBLE FormatFunctionsByProxy<T_, format::AcceptableRoles, u_> :
        public CanFormat<T_>
    {
        private:
            const CanFormat<T_> * const _proxy;

        public:
            FormatFunctionsByProxy(const CanFormat<T_> * const p) :
                _proxy(p)
            {
            }

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

    template <typename T_, unsigned u_>
    class PALUDIS_VISIBLE FormatFunctionsByProxy<T_, format::UseRoles, u_> :
        public CanFormat<T_>
    {
        private:
            const CanFormat<T_> * const _proxy;

        public:
            FormatFunctionsByProxy(const CanFormat<T_> * const p) :
                _proxy(p)
            {
            }

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
    };

    template <typename T_, unsigned u_>
    class PALUDIS_VISIBLE FormatFunctionsByProxy<T_, format::IUseRoles, u_> :
        public CanFormat<T_>
    {
        private:
            const CanFormat<T_> * const _proxy;

        public:
            FormatFunctionsByProxy(const CanFormat<T_> * const p) :
                _proxy(p)
            {
            }

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

    template <typename T_, unsigned u_>
    class PALUDIS_VISIBLE FormatFunctionsByProxy<T_, format::PackageRoles, u_> :
        public CanFormat<T_>
    {
        private:
            const CanFormat<T_> * const _proxy;

        public:
            FormatFunctionsByProxy(const CanFormat<T_> * const p) :
                _proxy(p)
            {
            }

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

    template <unsigned u_>
    class PALUDIS_VISIBLE FormatFunctionsByProxy<NoType<u_>, format::NoRoles, u_>
    {
        public:
            FormatFunctionsByProxy(const void * const)
            {
            }

            void format(const NoType<u_> &) const;
    };

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
