/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_CAVE_FORMAT_USER_CONFIG_HH
#define PALUDIS_GUARD_SRC_CLIENTS_CAVE_FORMAT_USER_CONFIG_HH 1

#include <paludis/util/map.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/singleton.hh>
#include <string>
#include <memory>
#include <utility>
#include "format_string.hh"

namespace paludis
{
    namespace cave
    {
        class FormatUserConfigFile :
            private Pimp<FormatUserConfigFile>,
            public Singleton<FormatUserConfigFile>
        {
            friend class Singleton<FormatUserConfigFile>;

            private:
                FormatUserConfigFile();
                ~FormatUserConfigFile();

            public:
                std::string fetch(const std::string & v, int vi, const std::string & d) const;
        };

        template <char c_>
        struct FormatValue
        {
            std::string value;
        };

        template <char c_>
        FormatValue<c_> fv(const std::string & s)
        {
            return FormatValue<c_>{s};
        }

        template <char... cs_>
        struct FormatString
        {
            std::string format_string;
        };

        template <char c_>
        struct FormatValuesBase
        {
            std::string value;

            FormatValuesBase(const std::string & v) :
                value(v)
            {
            }
        };

        template <char... cs_>
        struct FormatValues :
            FormatValuesBase<cs_>...
        {
            template <char... c_>
            FormatValues(const FormatValue<c_> & ... c) :
                FormatValuesBase<c_>(c.value)...
            {
            }
        };

        struct OrderedFormatValuesBaseMap
        {
            std::shared_ptr<Map<char, std::string> > map;

            OrderedFormatValuesBaseMap() :
                map(std::make_shared<Map<char, std::string> >())
            {
            }
        };

        template <char c_>
        struct OrderedFormatValuesBase
        {
            OrderedFormatValuesBase(
                    const std::shared_ptr<Map<char, std::string> > & m,
                    const FormatValuesBase<c_> & v)
            {
                m->insert(c_, v.value);
            }
        };

        template <char... cs_>
        struct OrderedFormatValues :
            OrderedFormatValuesBaseMap,
            OrderedFormatValuesBase<cs_>...
        {
            template <char... c_>
            OrderedFormatValues(const FormatValues<c_...> & c) :
                OrderedFormatValuesBase<cs_>(this->map, c)...
            {
            }
        };

        template <char... cs_>
        struct MakeOrderedFormatValues;

        template <char a_>
        struct MakeOrderedFormatValues<a_>
        {
            typedef OrderedFormatValues<a_> Type;
        };

        template <char a_, char b_>
        struct MakeOrderedFormatValues<a_, b_>
        {
            typedef OrderedFormatValues<(a_ < b_ ? a_ : b_), (a_ < b_ ? b_ : a_)> Type;
        };

        template <char... cs_>
        typename MakeOrderedFormatValues<cs_...>::Type
        order(const FormatValues<cs_...> & f)
        {
            return typename MakeOrderedFormatValues<cs_...>::Type(f);
        }

        template <typename T_>
        struct ExtractFormatValue;

        template <char c_>
        struct ExtractFormatValue<FormatValue<c_> >
        {
            static const char value = c_;
        };

        template <char... cs_>
        std::string fuc_parameters(const FormatString<cs_...> & s, OrderedFormatValues<cs_...> && p)
        {
            return format_string(s.format_string, p.map);
        }

        template <char... cs_, typename... T_>
        std::string fuc(const FormatString<cs_...> & f, const T_ & ... t)
        {
            return fuc_parameters(f, order(FormatValues<ExtractFormatValue<T_>::value...>{t...}));
        }

        template <char... cs_>
        struct MakeFormatStringFetcher
        {
            std::string user_key;
            int user_key_version;

            std::string text;

            FormatString<cs_...> operator() () const
            {
                return FormatString<cs_...>{FormatUserConfigFile::get_instance()->fetch(user_key, user_key_version, text)};
            }
        };

        inline MakeFormatStringFetcher<>
        make_format_string_fetcher(const std::string & u, const int v)
        {
            return MakeFormatStringFetcher<>{u, v, ""};
        }

        template <char... cs_>
        struct MakeMakeFormatStringFetcher;

        template <char a_>
        struct MakeMakeFormatStringFetcher<a_>
        {
            typedef MakeFormatStringFetcher<a_> Type;
        };

        template <char a_, char b_>
        struct MakeMakeFormatStringFetcher<a_, b_>
        {
            typedef MakeFormatStringFetcher<(a_ < b_ ? a_ : b_), (a_ < b_ ? b_ : a_)> Type;
        };

        template <char c_>
        struct FormatParam
        {
        };

        template <char c_>
        FormatParam<c_> param()
        {
            return FormatParam<c_>();
        }

        template <char... cs_>
        MakeFormatStringFetcher<cs_...> operator<< (MakeFormatStringFetcher<cs_...> && f, const std::string & s)
        {
            MakeFormatStringFetcher<cs_...> result{std::move(f.user_key), f.user_key_version, std::move(f.text)};
            result.text.append(s);
            return result;
        }

        template <char c_, char... cs_>
        typename MakeMakeFormatStringFetcher<c_, cs_...>::Type
        operator<< (MakeFormatStringFetcher<cs_...> && f, const FormatParam<c_> &)
        {
            typename MakeMakeFormatStringFetcher<c_, cs_...>::Type result{std::move(f.user_key), f.user_key_version, std::move(f.text)};
            result.text.append("%");
            result.text.append(1, c_);
            return result;
        }
    }

    extern template class Singleton<cave::FormatUserConfigFile>;
}

#endif
