/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/serialise-impl.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/package_id.hh>
#include <paludis/dep_spec.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/environment.hh>
#include <paludis/elike_package_dep_spec.hh>
#include <list>
#include <map>

using namespace paludis;

SerialiserObjectWriter::SerialiserObjectWriter(Serialiser & s) :
    _serialiser(s)
{
}

SerialiserObjectWriter::~SerialiserObjectWriter()
{
    _serialiser.raw_stream() << ");";
}

Serialiser::Serialiser(std::ostream & s) :
    _stream(s)
{
}

Serialiser::~Serialiser()
{
}

std::ostream &
Serialiser::raw_stream()
{
    return _stream;
}

SerialiserObjectWriter
Serialiser::object(const std::string & c)
{
    raw_stream() << c << "(";
    return SerialiserObjectWriter(*this);
}

void
SerialiserObjectWriterHandler<false, false, bool>::write(Serialiser & s, const bool t)
{
    if (t)
        s.raw_stream() << "\"true\";";
    else
        s.raw_stream() << "\"false\";";
}

void
SerialiserObjectWriterHandler<false, false, int>::write(Serialiser & s, const int i)
{
    s.raw_stream() << "\"" << i << "\";";
}

void
SerialiserObjectWriterHandler<false, false, std::string>::write(Serialiser & s, const std::string & t)
{
    s.raw_stream() << "\"";
    s.escape_write(t);
    s.raw_stream() << "\";";
}

void
SerialiserObjectWriterHandler<false, false, const PackageID>::write(Serialiser & s, const PackageID & t)
{
    s.raw_stream() << "\"";
    s.escape_write(stringify(t.uniquely_identifying_spec()));
    s.raw_stream() << "\";";
}

void
Serialiser::escape_write(const std::string & t)
{
    for (std::string::const_iterator c(t.begin()), c_end(t.end()) ;
            c != c_end ; ++c)
        switch (*c)
        {
            case '\\':
            case '"':
            case ';':
            case '(':
            case ')':
                raw_stream() << '\\';
                /* fall through */
            default:
                raw_stream() << *c;
        }
}

namespace paludis
{
    template <>
    struct Imp<Deserialiser>
    {
        const Environment * const env;
        std::istream & stream;

        Imp(const Environment * const e, std::istream & s) :
            env(e),
            stream(s)
        {
        }
    };

    template <>
    struct Imp<Deserialisation>
    {
        Deserialiser & deserialiser;
        const std::string item_name;

        std::string class_name;
        std::string string_value;
        bool null;
        std::list<std::shared_ptr<Deserialisation> > children;

        Imp(Deserialiser & d, const std::string & i) :
            deserialiser(d),
            item_name(i),
            null(false)
        {
        }
    };

    template <>
    struct Imp<Deserialisator>
    {
        const std::string class_name;
        std::map<std::string, std::shared_ptr<Deserialisation> > keys;

        Imp(const std::string & c) :
            class_name(c)
        {
        }
    };

    template <>
    struct WrappedForwardIteratorTraits<Deserialisation::ConstIteratorTag>
    {
        typedef std::list<std::shared_ptr<Deserialisation> >::const_iterator UnderlyingIterator;
    };
}

Deserialiser::Deserialiser(const Environment * const e, std::istream & s) :
    _imp(e, s)
{
}

Deserialiser::~Deserialiser()
{
}

std::istream &
Deserialiser::stream()
{
    return _imp->stream;
}

const Environment *
Deserialiser::environment() const
{
    return _imp->env;
}

Deserialisation::Deserialisation(const std::string & i, Deserialiser & d) :
    _imp(d, i)
{
    char c;
    if (! d.stream().get(c))
        throw InternalError(PALUDIS_HERE, "can't parse string");

    if (c == '"')
    {
        while (true)
        {
            if (! d.stream().get(c))
                throw InternalError(PALUDIS_HERE, "can't parse string");

            if (c == '\\')
            {
                if (! d.stream().get(c))
                    throw InternalError(PALUDIS_HERE, "can't parse string");
                _imp->string_value.append(1, c);
            }
            else if (c == '"')
                break;
            else
                _imp->string_value.append(1, c);
        }

        if (! d.stream().get(c))
            throw InternalError(PALUDIS_HERE, "can't parse string");
        if (c != ';')
            throw InternalError(PALUDIS_HERE, "can't parse string");
    }
    else
    {
        _imp->class_name.append(1, c);
        while (true)
        {
            if (! d.stream().get(c))
                throw InternalError(PALUDIS_HERE, "can't parse string");

            if (c == ';')
            {
                if (_imp->class_name != "null")
                    throw InternalError(PALUDIS_HERE, "can't parse string");
                _imp->null = true;
                _imp->class_name.clear();
                break;
            }
            else if (c == '(')
                break;
            else
                _imp->class_name.append(1, c);
        }

        if (! _imp->null)
        {
            while (true)
            {
                if (! d.stream().get(c))
                    throw InternalError(PALUDIS_HERE, "can't parse string");

                if (c == ')')
                {
                    if (! d.stream().get(c))
                        throw InternalError(PALUDIS_HERE, "can't parse string");
                    if (c != ';')
                        throw InternalError(PALUDIS_HERE, "can't parse string");
                    break;
                }
                else
                {
                    std::string k;
                    k.append(1, c);
                    while (true)
                    {
                        if (! d.stream().get(c))
                            throw InternalError(PALUDIS_HERE, "can't parse string");
                        if (c == '=')
                            break;
                        k.append(1, c);
                    }

                    std::shared_ptr<Deserialisation> de(std::make_shared<Deserialisation>(k, d));
                    _imp->children.push_back(de);
                }
            }
        }
    }
}

Deserialisation::~Deserialisation()
{
}

const std::string
Deserialisation::item_name() const
{
    return _imp->item_name;
}

const std::string
Deserialisation::class_name() const
{
    return _imp->class_name;
}

bool
Deserialisation::null() const
{
    return _imp->null;
}

const std::string
Deserialisation::string_value() const
{
    return _imp->string_value;
}

Deserialisation::ConstIterator
Deserialisation::begin_children() const
{
    return ConstIterator(_imp->children.begin());
}

Deserialisation::ConstIterator
Deserialisation::end_children() const
{
    return ConstIterator(_imp->children.end());
}

const Deserialiser &
Deserialisation::deserialiser() const
{
    return _imp->deserialiser;
}

Deserialisator::Deserialisator(Deserialisation & d, const std::string & c) :
    _imp(c)
{
    if (c != d.class_name())
        throw InternalError(PALUDIS_HERE, "expected class name '" + stringify(c) + "' but got '"
                + d.class_name() + "'");

    for (Deserialisation::ConstIterator i(d.begin_children()), i_end(d.end_children()) ;
            i != i_end ; ++i)
        _imp->keys.insert(std::make_pair((*i)->item_name(), *i));
}

Deserialisator::~Deserialisator()
{
    if (! std::uncaught_exception())
    {
        if (! _imp->keys.empty())
            throw InternalError(PALUDIS_HERE, "keys not empty when deserialising '" + _imp->class_name + "', keys remaining are { "
                    + join(first_iterator(_imp->keys.begin()), first_iterator(_imp->keys.end()), ", ") + " }");
    }
}

const std::shared_ptr<Deserialisation>
Deserialisator::find_remove_member(const std::string & s)
{
    std::map<std::string, std::shared_ptr<Deserialisation> >::iterator i(_imp->keys.find(s));
    if (i == _imp->keys.end())
        throw InternalError(PALUDIS_HERE, "no key '" + s + "'");
    std::shared_ptr<Deserialisation> result(i->second);
    _imp->keys.erase(i);
    return result;
}

std::shared_ptr<const PackageID>
DeserialisatorHandler<std::shared_ptr<const PackageID> >::handle(Deserialisation & v)
{
    Context context("When deserialising:");

    if (v.null())
        return make_null_shared_ptr();

    return *(*v.deserialiser().environment())[
        selection::RequireExactlyOne(generator::Matches(
                    parse_elike_package_dep_spec(v.string_value(),
                        { epdso_allow_tilde_greater_deps, epdso_nice_equal_star,
                        epdso_allow_ranged_deps, epdso_allow_use_deps, epdso_allow_use_deps_portage,
                        epdso_allow_use_dep_defaults, epdso_allow_repository_deps, epdso_allow_slot_star_deps,
                        epdso_allow_slot_equal_deps, epdso_allow_slot_deps, epdso_allow_key_requirements,
                        epdso_allow_use_dep_question_defaults },
                        { vso_flexible_dashes, vso_flexible_dots, vso_ignore_case,
                        vso_letters_anywhere, vso_dotted_suffixes }), make_null_shared_ptr(), { }))]->begin();
}

namespace paludis
{
    template class Pimp<Deserialiser>;
    template class Pimp<Deserialisation>;
    template class Pimp<Deserialisator>;
    template class WrappedForwardIterator<Deserialisation::ConstIteratorTag, const std::shared_ptr<Deserialisation> >;
}
