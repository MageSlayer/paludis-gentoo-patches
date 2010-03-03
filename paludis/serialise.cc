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

#include <paludis/serialise-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/simple_parser.hh>
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
    struct Implementation<Deserialiser>
    {
        const Environment * const env;
        SimpleParser parser;

        Implementation(const Environment * const e, const std::string & s) :
            env(e),
            parser(s)
        {
        }
    };

    template <>
    struct Implementation<Deserialisation>
    {
        Deserialiser & deserialiser;
        const std::string item_name;

        std::string class_name;
        std::string string_value;
        std::string raw_string;
        bool null;
        std::list<std::tr1::shared_ptr<Deserialisation> > children;

        Implementation(Deserialiser & d, const std::string & i) :
            deserialiser(d),
            item_name(i),
            null(false)
        {
        }
    };

    template <>
    struct Implementation<Deserialisator>
    {
        const std::string class_name;
        std::map<std::string, std::tr1::shared_ptr<Deserialisation> > keys;

        Implementation(const std::string & c) :
            class_name(c)
        {
        }
    };

    template <>
    struct WrappedForwardIteratorTraits<Deserialisation::ConstIteratorTag>
    {
        typedef std::list<std::tr1::shared_ptr<Deserialisation> >::const_iterator UnderlyingIterator;
    };
}

Deserialiser::Deserialiser(const Environment * const e, const std::string & s) :
    PrivateImplementationPattern<Deserialiser>(new Implementation<Deserialiser>(e, s))
{
}

Deserialiser::~Deserialiser()
{
}

SimpleParser &
Deserialiser::parser()
{
    return _imp->parser;
}

const Environment *
Deserialiser::environment() const
{
    return _imp->env;
}

Deserialisation::Deserialisation(const std::string & i, Deserialiser & d) :
    PrivateImplementationPattern<Deserialisation>(new Implementation<Deserialisation>(d, i))
{
    std::string::size_type before_p(d.parser().offset());

    if (d.parser().consume(simple_parser::exact("null;")))
        _imp->null = true;
    else if (d.parser().consume(simple_parser::exact("\"")))
    {
        while (true)
        {
            std::string v;
            if (d.parser().consume(simple_parser::exact("\\")))
            {
                if (! d.parser().consume(simple_parser::any_except("") >> v))
                    throw InternalError(PALUDIS_HERE, "can't parse string escape");
                _imp->string_value.append(v);
            }
            else if (d.parser().consume(simple_parser::exact("\";")))
                break;
            else if (d.parser().consume((+simple_parser::any_except("\\\"")) >> v))
                _imp->string_value.append(v);
            else
                throw InternalError(PALUDIS_HERE, "can't parse string");
        }
    }
    else if (d.parser().consume(((+simple_parser::any_of(
                        "abcdefghijklmnopqrstuvwxyz"
                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                        "_"
                        )) >> _imp->class_name)
                & simple_parser::exact("(")))
    {
        while (true)
        {
            std::string k;
            if (d.parser().consume(simple_parser::exact(");")))
                break;
            else if (d.parser().consume(((+simple_parser::any_of(
                                    "abcdefghijklmnopqrstuvwxyz"
                                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                    "0123456789"
                                    "-_"
                                    )) >> k)
                        & simple_parser::exact("=")))
            {
                std::tr1::shared_ptr<Deserialisation> c(new Deserialisation(k, d));
                _imp->children.push_back(c);
            }
            else
                throw InternalError(PALUDIS_HERE, "can't parse keys, remaining text is '"
                        + d.parser().text().substr(d.parser().offset()));
        }
    }
    else
        throw InternalError(PALUDIS_HERE, "can't parse keys, remaining text is '"
                + d.parser().text().substr(d.parser().offset()));

    std::string::size_type after_p(d.parser().offset());
    _imp->raw_string = d.parser().text().substr(before_p, after_p - before_p);
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

const std::string
Deserialisation::raw_string() const
{
    return _imp->raw_string;
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
    PrivateImplementationPattern<Deserialisator>(new Implementation<Deserialisator>(c))
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

const std::tr1::shared_ptr<Deserialisation>
Deserialisator::find_remove_member(const std::string & s)
{
    std::map<std::string, std::tr1::shared_ptr<Deserialisation> >::iterator i(_imp->keys.find(s));
    if (i == _imp->keys.end())
        throw InternalError(PALUDIS_HERE, "no key '" + s + "'");
    std::tr1::shared_ptr<Deserialisation> result(i->second);
    _imp->keys.erase(i);
    return result;
}

std::tr1::shared_ptr<const PackageID>
DeserialisatorHandler<std::tr1::shared_ptr<const PackageID> >::handle(Deserialisation & v)
{
    Context context("When deserialising '" + v.raw_string() + "':");

    if (v.null())
        return make_null_shared_ptr();

    return *(*v.deserialiser().environment())[
        selection::RequireExactlyOne(generator::Matches(
                    parse_elike_package_dep_spec(v.string_value(),
                        ELikePackageDepSpecOptions() + epdso_allow_tilde_greater_deps + epdso_nice_equal_star +
                        epdso_allow_ranged_deps + epdso_allow_use_deps + epdso_allow_use_deps_portage +
                        epdso_allow_use_dep_defaults + epdso_allow_repository_deps + epdso_allow_slot_star_deps +
                        epdso_allow_slot_equal_deps + epdso_allow_slot_deps + epdso_allow_key_requirements +
                        epdso_allow_use_dep_question_defaults,
                        VersionSpecOptions() + vso_flexible_dashes + vso_flexible_dots + vso_ignore_case +
                        vso_letters_anywhere + vso_dotted_suffixes,
                        make_null_shared_ptr()), MatchPackageOptions()))]->begin();
}

template class PrivateImplementationPattern<Deserialiser>;
template class PrivateImplementationPattern<Deserialisation>;
template class PrivateImplementationPattern<Deserialisator>;
template class WrappedForwardIterator<Deserialisation::ConstIteratorTag, const std::tr1::shared_ptr<Deserialisation> >;

