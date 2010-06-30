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

#ifndef PALUDIS_GUARD_PALUDIS_SERIALISE_HH
#define PALUDIS_GUARD_PALUDIS_SERIALISE_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <paludis/serialise-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <tr1/memory>
#include <string>
#include <ostream>

namespace paludis
{
    namespace serialise
    {
        struct container;
        struct might_be_null;
    }

    template <
        typename Flag1_ = void,
        typename Flag2_ = void,
        typename Flag3_ = void
        >
    struct SerialiserFlags
    {
        typedef Flag1_ Flag1;
        typedef Flag2_ Flag2;
        typedef Flag3_ Flag3;
    };

    class PALUDIS_VISIBLE SerialiserObjectWriter
    {
        private:
            Serialiser & _serialiser;

        public:
            SerialiserObjectWriter(Serialiser &);
            ~SerialiserObjectWriter();

            template <
                typename Flags_,
                typename T_>
            SerialiserObjectWriter & member(
                    const Flags_ &,
                    const std::string & item_name,
                    const T_ &);
    };

    class PALUDIS_VISIBLE Serialiser
    {
        private:
            std::ostream & _stream;

        public:
            Serialiser(std::ostream &);
            ~Serialiser();

            SerialiserObjectWriter object(const std::string & class_name)
                PALUDIS_ATTRIBUTE((warn_unused_result));

            std::ostream & raw_stream() PALUDIS_ATTRIBUTE((warn_unused_result));

            void escape_write(const std::string &);
    };

    class PALUDIS_VISIBLE Deserialiser :
        private PrivateImplementationPattern<Deserialiser>
    {
        public:
            Deserialiser(const Environment * const, std::istream &);
            ~Deserialiser();

            const Environment * environment() const PALUDIS_ATTRIBUTE((warn_unused_result));

            std::istream & stream() PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE Deserialisation :
        private PrivateImplementationPattern<Deserialisation>
    {
        public:
            Deserialisation(
                    const std::string & class_name,
                    Deserialiser &);
            ~Deserialisation();

            const Deserialiser & deserialiser() const PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::string item_name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::string class_name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::string string_value() const PALUDIS_ATTRIBUTE((warn_unused_result));

            bool null() const PALUDIS_ATTRIBUTE((warn_unused_result));

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag,
                    const std::tr1::shared_ptr<Deserialisation> > ConstIterator;
            ConstIterator begin_children() const PALUDIS_ATTRIBUTE((warn_unused_result));
            ConstIterator end_children() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE Deserialisator :
        private PrivateImplementationPattern<Deserialisator>
    {
        public:
            Deserialisator(
                    Deserialisation &,
                    const std::string & class_name);

            ~Deserialisator();

            const Deserialisation & deserialisation() const PALUDIS_ATTRIBUTE((warn_unused_result));

            template <typename T_> T_ member(const std::string & key_name);

            const std::tr1::shared_ptr<Deserialisation> find_remove_member(
                    const std::string &) PALUDIS_ATTRIBUTE((warn_unused_result));

    };

    template <typename T_>
    std::tr1::shared_ptr<T_> deserialise(
            const Environment * const,
            const std::string &,
            const std::string &) PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));

    extern template class PrivateImplementationPattern<Deserialiser>;
    extern template class PrivateImplementationPattern<Deserialisation>;
    extern template class PrivateImplementationPattern<Deserialisator>;
}

#endif
