/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_SERIALISE_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_SERIALISE_IMPL_HH 1

#include <paludis/serialise.hh>
#include <paludis/util/remove_shared_ptr.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/options.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <type_traits>
#include <unordered_set>
#include <vector>
#include <list>
#include <ostream>
#include <istream>

namespace paludis
{
    template <
        typename Flags_,
        typename Flag_>
    struct SerialiserFlagsInclude
    {
        static const bool value =
            std::is_same<typename Flags_::Flag1, Flag_>::value ||
            std::is_same<typename Flags_::Flag2, Flag_>::value ||
            std::is_same<typename Flags_::Flag3, Flag_>::value;
    };

    template <
        bool is_container_,
        bool might_be_null_,
        typename T_>
    struct SerialiserObjectWriterHandler;

    template <>
    struct PALUDIS_VISIBLE SerialiserObjectWriterHandler<false, false, bool>
    {
        static void write(Serialiser & s, const bool t);
    };

    template <>
    struct PALUDIS_VISIBLE SerialiserObjectWriterHandler<false, false, int>
    {
        static void write(Serialiser & s, const int t);
    };

    template <>
    struct PALUDIS_VISIBLE SerialiserObjectWriterHandler<false, false, std::string>
    {
        static void write(Serialiser & s, const std::string & t);
    };

    template <>
    struct PALUDIS_VISIBLE SerialiserObjectWriterHandler<false, false, const PackageID>
    {
        static void write(Serialiser & s, const PackageID & t);
    };

    template <
        typename T_>
    struct SerialiserObjectWriterHandler<false, false, Options<T_> >
    {
        static void write(Serialiser & s, const Options<T_> & t)
        {
            std::stringstream ss;
            for (T_ i(static_cast<T_>(0)), i_end(t.highest_bit()) ;
                    i != i_end ; i = static_cast<T_>(static_cast<int>(i) + 1))
            {
                if (! t[i])
                    continue;
                if (! ss.str().empty())
                    ss << ",";
                ss << i;
            }

            s.raw_stream() << "\"";
            s.escape_write(ss.str());
            s.raw_stream() << "\";";
        }
    };

    template <
        typename T_>
    struct SerialiserObjectWriterHandler<false, false, const T_>
    {
        static void write(Serialiser & s, const T_ & t)
        {
            SerialiserObjectWriterHandler<false, false, T_>::write(s, t);
        }
    };

    template <
        typename T_>
    struct SerialiserObjectWriterHandler<false, false, T_>
    {
        static void write(Serialiser & s, const T_ & t)
        {
            t.serialise(s);
        }
    };

    template <
        bool is_container_,
        typename T_>
    struct SerialiserObjectWriterHandler<is_container_, true, T_>
    {
        static void write(Serialiser & s, const T_ & t)
        {
            if (t)
                SerialiserObjectWriterHandler<is_container_, false, typename RemoveSharedPtr<T_>::Type>::write(
                        s, *t);
            else
                s.raw_stream() << "null;";
        }
    };

    template <typename T_>
    struct SerialiserConstIteratorType
    {
        typedef typename T_::ConstIterator Type;
    };

    template <typename T_>
    struct SerialiserConstIteratorType<std::list<T_> >
    {
        typedef typename std::list<T_>::const_iterator Type;
    };

    template <typename T_>
    struct SerialiserConstIteratorType<std::vector<T_> >
    {
        typedef typename std::vector<T_>::const_iterator Type;
    };

    template <typename T_, typename H_>
    struct SerialiserConstIteratorType<std::unordered_set<T_, H_> >
    {
        typedef typename std::unordered_set<T_, H_>::const_iterator Type;
    };

    template <
        typename T_>
    struct SerialiserObjectWriterHandler<true, false, T_>
    {
        static void write(Serialiser & s, const T_ & t)
        {
            s.raw_stream() << "c(";
            unsigned n(0);
            for (typename SerialiserConstIteratorType<T_>::Type i(t.begin()), i_end(t.end()) ;
                    i != i_end ; ++i)
            {
                typedef typename std::iterator_traits<
                    typename SerialiserConstIteratorType<T_>::Type>::value_type ItemValueType;
                typedef typename std::remove_reference<ItemValueType>::type ItemType;

                s.raw_stream() << ++n << "=";
                SerialiserObjectWriterHandler<
                    false,
                    ! std::is_same<ItemType, typename RemoveSharedPtr<ItemType>::Type>::value,
                    ItemType
                        >::write(s, *i);
            }

            s.raw_stream() << "count=";
            SerialiserObjectWriterHandler<false, false, int>::write(s, n);

            s.raw_stream() << ");";
        }
    };

    template <
        typename Flags_,
        typename T_>
    SerialiserObjectWriter &
    SerialiserObjectWriter::member(
            const Flags_ &,
            const std::string & item_name,
            const T_ & t)
    {
        _serialiser.raw_stream() << item_name << "=";

        SerialiserObjectWriterHandler<
            SerialiserFlagsInclude<Flags_, serialise::container>::value,
            SerialiserFlagsInclude<Flags_, serialise::might_be_null>::value,
            T_
            >::write(_serialiser, t);

        return *this;
    }

    template <typename T_>
    struct DeserialisatorHandler;

    template <>
    struct DeserialisatorHandler<bool>
    {
        static bool handle(Deserialisation & v)
        {
            return destringify<bool>(v.string_value());
        }
    };

    template <>
    struct DeserialisatorHandler<int>
    {
        static int handle(Deserialisation & v)
        {
            return destringify<int>(v.string_value());
        }
    };

    template <>
    struct DeserialisatorHandler<std::string>
    {
        static std::string handle(Deserialisation & v)
        {
            return v.string_value();
        }
    };

    template <>
    struct PALUDIS_VISIBLE DeserialisatorHandler<std::shared_ptr<const PackageID> >
    {
        static std::shared_ptr<const PackageID> handle(Deserialisation & v);
    };

    template <typename T_>
    struct PALUDIS_VISIBLE DeserialisatorHandler<std::shared_ptr<T_> >
    {
        static std::shared_ptr<T_> handle(Deserialisation & v)
        {
            if (v.null())
                return make_null_shared_ptr();
            else
                return T_::deserialise(v);
        }
    };

    template <typename T_>
    struct DeserialisatorHandler<Options<T_> >
    {
        static Options<T_> handle(Deserialisation & v)
        {
            Options<T_> result;
            std::list<std::string> tokens;
            tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(v.string_value(),
                    ",", "", std::back_inserter(tokens));

            for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
                    t != t_end ; ++t)
                result += destringify<T_>(*t);

            return result;
        }
    };

    template <typename T_>
    struct DeserialisatorHandler
    {
        static T_ handle(Deserialisation & v)
        {
            return T_::deserialise(v);
        }
    };

    template <typename T_>
    T_
    Deserialisator::member(const std::string & key_name)
    {
        return DeserialisatorHandler<T_>::handle(*find_remove_member(key_name));
    }

    template <typename T_>
    std::shared_ptr<T_> deserialise(
            const Environment * const env,
            const std::string & str,
            const std::string & class_name)
    {
        Deserialiser d(env, str);
        Deserialisation dd(class_name, d);
        return T_::deserialise(dd);
    }
}

#endif
