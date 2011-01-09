/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <algorithm>
#include <paludis/util/destringify.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/log.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/simple_parser.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/version_spec.hh>
#include <vector>
#include <limits>

using namespace paludis;

#include <paludis/version_spec-se.cc>

BadVersionSpecError::BadVersionSpecError(const std::string & name) throw () :
    NameError(name, "version spec")
{
}

BadVersionSpecError::BadVersionSpecError(const std::string & name, const std::string & msg) throw () :
    NameError(name, "version spec", msg)
{
}

typedef std::vector<VersionSpecComponent> Parts;

namespace paludis
{
    template<>
    struct Imp<VersionSpec>
    {
        std::string text;
        Parts parts;

        const VersionSpecOptions options;

        Imp(const VersionSpecOptions & o) :
            options(o)
        {
        }
    };

    template <>
    struct WrappedForwardIteratorTraits<VersionSpec::ConstIteratorTag>
    {
        typedef Parts::const_iterator UnderlyingIterator;
    };

    simple_parser::SimpleParserExpression make_dash_parse_expression(const std::string & strict_dash,
            const std::string & text,
            const bool ignore_case,
            const bool flexible_dashes)
    {
        simple_parser::SimpleParserExpression expr(ignore_case ? simple_parser::exact_ignoring_case(text) : simple_parser::exact(text));
        if (flexible_dashes)
            return -simple_parser::any_of("-_") & expr;
        else
            return simple_parser::exact(strict_dash) & expr;
    }
}

VersionSpec::VersionSpec(const std::string & text, const VersionSpecOptions & options) :
    _imp(options)
{
    Context c("When parsing version spec '" + text + "':");

    if (text.empty())
        throw BadVersionSpecError(text, "cannot be empty");

    /* set us up with some sane defaults */
    _imp->text = text;

    /* parse */
    SimpleParser parser(text);

    {
        std::string leading_v_str;
        if (options[vso_ignore_leading_v] && parser.consume(
                    (options[vso_ignore_case] ? simple_parser::exact_ignoring_case("v") : simple_parser::exact("v")) >> leading_v_str))
            _imp->parts.push_back(make_named_values<VersionSpecComponent>(
                        n::number_value() = "",
                        n::text() = leading_v_str,
                        n::type() = vsct_ignore
                        ));
    }

    std::string scm_str;
    if (parser.consume((options[vso_ignore_case] ? simple_parser::exact_ignoring_case("scm") : simple_parser::exact("scm")) >> scm_str))
    {
        _imp->parts.push_back(make_named_values<VersionSpecComponent>(
                    n::number_value() = "",
                    n::text() = scm_str,
                    n::type() = vsct_scm
                    ));
    }
    else
    {
        /* numbers... */
        bool first_number(true);
        std::string number_prefix;
        while (true)
        {
            if (first_number && options[vso_allow_leading_dot])
            {
                std::string dot_chars(".");
                if (options[vso_flexible_dots])
                {
                    dot_chars.append("-");
                    if (options[vso_flexible_dashes])
                        dot_chars.append("_");
                }

                std::string leading_ick;
                if (parser.consume(simple_parser::any_of(dot_chars) >> leading_ick))
                    number_prefix = leading_ick;
            }

            std::string number_part;
            if (! parser.consume(+simple_parser::any_of("0123456789") >> number_part))
                throw BadVersionSpecError(text, "Expected number part not found at offset " + stringify(parser.offset()));

            if (first_number || '0' != number_part[0])
                _imp->parts.push_back(make_named_values<VersionSpecComponent>(
                            n::number_value() = strip_leading(number_part, "0"),
                            n::text() = number_prefix + number_part,
                            n::type() = vsct_number
                            ));
            else
                _imp->parts.push_back(make_named_values<VersionSpecComponent>(
                            n::number_value() = number_part,
                            n::text() = number_prefix + number_part,
                            n::type() = vsct_floatlike
                            ));

            number_prefix.clear();
            if (parser.consume(simple_parser::exact(".")))
                number_prefix = ".";
            else if (options[vso_flexible_dots])
            {
                std::string allowed_dot_replacements("-");
                if (options[vso_flexible_dashes])
                    allowed_dot_replacements.append("_");
                if (parser.lookahead(simple_parser::any_of(allowed_dot_replacements) & simple_parser::any_of("0123456789")))
                    if (! parser.consume(simple_parser::any_of(allowed_dot_replacements) >> number_prefix))
                        throw InternalError(PALUDIS_HERE, "lookahead worked, parse failed. huh?");
            }

            if (number_prefix.empty())
                break;
            first_number = false;
        }

        /* letter (but not if it's two letters, since that's 1.23rc4) */
        if (! parser.lookahead(
                    simple_parser::any_of("abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ") &
                    simple_parser::any_of("abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ")))
        {
            std::string l;
            if (parser.consume(simple_parser::any_of("abcdefghijklmnopqrstuvwxyz") >> l))
                _imp->parts.push_back(make_named_values<VersionSpecComponent>(
                            n::number_value() = l,
                            n::text() = l,
                            n::type() = vsct_letter
                            ));
            else if (options[vso_ignore_case] && parser.consume(simple_parser::any_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ") >> l))
                _imp->parts.push_back(make_named_values<VersionSpecComponent>(
                            n::number_value() = std::string(1, std::tolower(l.at(0))),
                            n::text() = l,
                            n::type() = vsct_letter
                            ));
        }

        while (true)
        {
            std::string suffix_str, number_str;
            VersionSpecComponentType k(vsct_empty);
            if (parser.consume(make_dash_parse_expression("_", "alpha", options[vso_ignore_case], options[vso_flexible_dashes]) >> suffix_str))
                k = vsct_alpha;
            else if (parser.consume(make_dash_parse_expression("_", "beta", options[vso_ignore_case], options[vso_flexible_dashes]) >> suffix_str))
                k = vsct_beta;
            else if (parser.consume(make_dash_parse_expression("_", "pre", options[vso_ignore_case], options[vso_flexible_dashes]) >> suffix_str))
                k = vsct_pre;
            else if (parser.consume(make_dash_parse_expression("_", "rc", options[vso_ignore_case], options[vso_flexible_dashes]) >> suffix_str))
                k = vsct_rc;
            else if (parser.consume(make_dash_parse_expression("_", "p", options[vso_ignore_case], options[vso_flexible_dashes]) >> suffix_str))
                k = vsct_patch;
            else
                break;

            if (! parser.consume(*simple_parser::any_of("0123456789") >> number_str))
                throw BadVersionSpecError(text, "Expected optional number at offset " + stringify(parser.offset()));

            std::string raw_text(suffix_str + number_str);
            if (number_str.size() > 0)
            {
                number_str = strip_leading(number_str, "0");
                if (number_str.empty())
                    number_str = "0";
            }

            _imp->parts.push_back(make_named_values<VersionSpecComponent>(
                        n::number_value() = number_str,
                        n::text() = raw_text,
                        n::type() = k
                        ));
        }

        /* try */
        std::string try_str;
        if (parser.consume(make_dash_parse_expression("-", "try", options[vso_ignore_case], options[vso_flexible_dashes]) >> try_str))
        {
            std::string number_str;
            if (! parser.consume(*simple_parser::any_of("0123456789") >> number_str))
                throw BadVersionSpecError(text, "Expected optional number at offset " + stringify(parser.offset()));

            std::string raw_text(try_str + number_str);
            if (number_str.size() > 0)
            {
                number_str = strip_leading(number_str, "0");
                if (number_str.empty())
                    number_str = "0";
            }
            _imp->parts.push_back(make_named_values<VersionSpecComponent>(
                        n::number_value() = number_str,
                        n::text() = raw_text,
                        n::type() = vsct_trypart
                        ));
        }

        /* scm */
        if (parser.consume(make_dash_parse_expression("-", "scm", options[vso_ignore_case], options[vso_flexible_dashes]) >> scm_str))
        {
            /* _suffix-scm? */
            if (_imp->parts.back().number_value().empty())
                _imp->parts.back().number_value() = "MAX";

            _imp->parts.push_back(make_named_values<VersionSpecComponent>(
                        n::number_value() = "",
                        n::text() = scm_str,
                        n::type() = vsct_scm
                        ));
        }

        /* Now we can change empty values to "0" */
        for (Parts::iterator i(_imp->parts.begin()),
                i_end(_imp->parts.end()) ; i != i_end ; ++i)
            if ((*i).number_value().empty())
                (*i).number_value() = "0";
    }

    /* revision */
    std::string rev_str;
    if (parser.consume(make_dash_parse_expression("-", "r", options[vso_ignore_case], options[vso_flexible_dashes]) >> rev_str))
    {
        bool first_revision(true);
        do
        {
            std::string number_str;
            if (! parser.consume(*simple_parser::any_of("0123456789") >> number_str))
                throw BadVersionSpecError(text, "Expected optional number at offset " + stringify(parser.offset()));

            /* Are we -r */
            bool empty(number_str.empty());

            std::string raw_text(first_revision ? rev_str : ".");
            raw_text.append(number_str);

            number_str = strip_leading(number_str, "0");
            if (number_str.empty())
                number_str = "0";
            _imp->parts.push_back(make_named_values<VersionSpecComponent>(
                        n::number_value() = number_str,
                        n::text() = raw_text,
                        n::type() = vsct_revision
                        ));

            if (empty)
            {
                /* we don't like -r.x */
                break;
            }
            else if (! parser.lookahead(simple_parser::exact(".") & simple_parser::any_of("0123456789")))
            {
                /* we don't like -rN.x where x is not a digit */
                break;
            }
            else if (! parser.consume(simple_parser::exact(".")))
                throw BadVersionSpecError(text, "Expected . or end after revision number at offset " + stringify(parser.offset()));

            first_revision = false;
        }
        while (true);
    }

    /* trailing stuff? */
    if (! parser.eof())
        throw BadVersionSpecError(text, "unexpected trailing text '" + text.substr(parser.offset()) + "'");
}

VersionSpec::VersionSpec(const VersionSpec & other) :
    _imp(other._imp->options)
{
    _imp->text = other._imp->text;
    _imp->parts = other._imp->parts;
}

const VersionSpec &
VersionSpec::operator= (const VersionSpec & other)
{
    if (this != &other)
    {
        _imp->text = other._imp->text;
        _imp->parts = other._imp->parts;
    }
    return *this;
}

VersionSpec::~VersionSpec()
{
}

namespace
{
    template <typename R_>
    R_
    componentwise_compare(const Parts & a, const Parts & b,
            std::pair<R_, bool> (*comparator)(const VersionSpecComponent &, Parts::const_iterator, Parts::const_iterator,
                    const VersionSpecComponent &, Parts::const_iterator, Parts::const_iterator, int))
    {
        std::vector<VersionSpecComponent>::const_iterator
            v1(a.begin()), v1_end(a.end()), v2(b.begin()), v2_end(b.end());

        VersionSpecComponent end_part(make_named_values<VersionSpecComponent>(
                    n::number_value() = "",
                    n::text() = "",
                    n::type() = vsct_empty
                    ));
        while (true)
        {
            const VersionSpecComponent * const p1(v1 == v1_end ? &end_part : &*v1);
            const VersionSpecComponent * const p2(v2 == v2_end ? &end_part : &*v2);

            if (p1->type() == vsct_ignore)
            {
                ++v1;
                continue;
            }

            if (p2->type() == vsct_ignore)
            {
                ++v2;
                continue;
            }

            if (&end_part == p1 && &end_part == p2)
            {
                std::pair<R_, bool> result(comparator(*p1, v1, v1_end, *p2, v2, v2_end, 0));
                if (result.second)
                    return result.first;
                else
                    throw InternalError(PALUDIS_HERE, "comparator reached the end of the versions without deciding on a result");
            }

            int compared(-2);

            if (p1 == &end_part && (*p2).type() == vsct_revision && (*p2).number_value() == "0")
                compared = 0;

            else if (p2 == &end_part && (*p1).type() == vsct_revision && (*p1).number_value() == "0")
                compared = 0;

            else if ((*p1).type() < (*p2).type())
                compared = -1;
            else if ((*p1).type() > (*p2).type())
                compared = 1;

            else
            {
                std::string p1s((*p1).number_value()), p2s((*p2).number_value());
                if ((*p1).type() == vsct_floatlike)
                {
                    p1s = strip_trailing(p1s, "0");
                    p2s = strip_trailing(p2s, "0");
                }

                /* _suffix-scm? */
                if (p1s == "MAX" && p2s == "MAX")
                    compared = 0;
                else if (p1s == "MAX")
                    compared = 1;
                else if (p2s == "MAX")
                    compared = -1;

                if (compared == -2)
                {
                    /* common part */
                    if ((*p1).type() != vsct_floatlike)
                    {
                        /* length compare (integers) */
                        int c = p1s.size() - p2s.size();
                        if (c < 0)
                            compared = -1;
                        else if (c > 0)
                            compared = 1;
                    }
                }

                if (compared == -2)
                {
                    /* stringwise compare (also for integers with the same size) */
                    int c(p1s.compare(p2s));
                    compared = c < 0 ? -1 : c > 0 ? 1 : 0;
                }
            }

            std::pair<R_, bool> result(comparator(*p1, v1, v1_end, *p2, v2, v2_end, compared));
            if (result.second)
                return result.first;

            if (v1_end != v1)
                ++v1;
            if (v2_end != v2)
                ++v2;
        }
    }

    std::pair<int, bool>
    compare_comparator(const VersionSpecComponent & a, Parts::const_iterator, Parts::const_iterator,
            const VersionSpecComponent & b, Parts::const_iterator, Parts::const_iterator, int compared)
    {
        return std::make_pair(compared, compared != 0 || (a.type() == vsct_empty && b.type() == vsct_empty));
    }

    std::pair<bool, bool>
    tilde_compare_comparator(const VersionSpecComponent & a, Parts::const_iterator, Parts::const_iterator,
            const VersionSpecComponent & b, Parts::const_iterator, Parts::const_iterator, int compared)
    {
        if (compared != 0)
            return std::make_pair(a.type() == vsct_revision &&
                    (b.type() == vsct_empty || b.type() == vsct_revision) &&
                    compared == 1, true);
        else
            return std::make_pair(true, a.type() == vsct_empty && b.type() == vsct_empty);
    }

    std::pair<bool, bool>
    nice_equal_star_compare_comparator(const VersionSpecComponent & a, Parts::const_iterator, Parts::const_iterator,
            const VersionSpecComponent & b, Parts::const_iterator b_it, Parts::const_iterator b_it_end, int compared)
    {
        if (b.type() == vsct_empty)
            return std::make_pair(true, true);
        else if (a.type() == vsct_floatlike && b.type() == vsct_floatlike && next(b_it) == b_it_end &&
                 a.number_value().compare(0, b.number_value().length(), b.number_value()) == 0)
            return std::make_pair(true, true);
        else if (a.type() == b.type() && next(b_it) == b_it_end &&
                 (b.type() == vsct_alpha || b.type() == vsct_beta || b.type() == vsct_pre ||
                  b.type() == vsct_rc || b.type() == vsct_patch) &&
                 std::string::npos == b.text().find_first_of("0123456789"))
            return std::make_pair(true, true);
        else
            return std::make_pair(false, compared != 0);
    }
}

int
VersionSpec::compare(const VersionSpec & other) const
{
    return componentwise_compare(_imp->parts, other._imp->parts, compare_comparator);
}

bool
VersionSpec::tilde_compare(const VersionSpec & other) const
{
    return componentwise_compare(_imp->parts, other._imp->parts, tilde_compare_comparator);
}

bool
VersionSpec::nice_equal_star_compare(const VersionSpec & other) const
{
    return componentwise_compare(_imp->parts, other._imp->parts, nice_equal_star_compare_comparator);
}

bool
VersionSpec::stupid_equal_star_compare(const VersionSpec & other) const
{
    return 0 == _imp->text.compare(0, other._imp->text.length(), other._imp->text);
}

std::size_t
VersionSpec::hash() const
{
    size_t result(0);

    const std::size_t h_shift = std::numeric_limits<std::size_t>::digits - 5;
    const std::size_t h_mask = static_cast<std::size_t>(0x1f) << h_shift;

    do
    {
        bool first(true);
        for (std::vector<VersionSpecComponent>::const_iterator r(_imp->parts.begin()), r_end(_imp->parts.end()) ;
                r != r_end ; ++r)
        {
            if ((*r).number_value() == "0" && (*r).type() == vsct_revision)
                continue;
            if ((*r).type() == vsct_ignore)
                continue;

            std::size_t hh(result & h_mask);
            result <<= 5;
            result ^= (hh >> h_shift);

            std::string r_v;
            if ((*r).type() == vsct_floatlike)
                r_v = strip_trailing((*r).number_value(), "0");
            else
                r_v = (*r).number_value();

            size_t x(0);
            int zeroes(0);
            for (std::string::const_iterator i(r_v.begin()), i_end(r_v.end()) ;
                    i != i_end ; ++i)
            {
                /* count leading zeroes if we are not the first component */
                if (x == 0 && ! first)
                    ++zeroes;
                x *= 10;
                x += *i - '0';
            }
            first = false;

            result ^= (static_cast<std::size_t>((*r).type()) + (x << 3) + (zeroes << 12));
        }
    } while (false);

    return result;
}

namespace
{
    template <VersionSpecComponentType p_>
    struct IsVersionSpecComponentType :
        std::unary_function<VersionSpecComponent, bool>
    {
        bool operator() (const VersionSpecComponent & p) const
        {
            return p.type() == p_;
        }
    };

    template <VersionSpecComponentType p_, VersionSpecComponentType q_>
    struct IsEitherVersionSpecComponentType :
        std::unary_function<VersionSpecComponent, bool>
    {
        bool operator() (const VersionSpecComponent & p) const
        {
            return p.type() == p_ || p.type() == q_;
        }
    };
}

VersionSpec
VersionSpec::remove_revision() const
{
    VersionSpec result(*this);

    // see EffSTL item 9
    result._imp->parts.erase(std::remove_if(
                result._imp->parts.begin(),
                result._imp->parts.end(),
                IsVersionSpecComponentType<vsct_revision>()), result._imp->parts.end());

    std::string::size_type p;
    if (std::string::npos != ((p = result._imp->text.rfind("-r"))))
        if (std::string::npos == result._imp->text.find_first_not_of("0123456789.", p + 2))
            result._imp->text.erase(p);

    return result;
}

std::string
VersionSpec::revision_only() const
{
    std::vector<VersionSpecComponent>::const_iterator r(std::find_if(
                _imp->parts.begin(), _imp->parts.end(), IsVersionSpecComponentType<vsct_revision>()));
    if (r != _imp->parts.end())
    {
        std::string result;
        do
        {
            if (! result.empty())
                result.append(".");
            else
                result = "r";

            result.append((*r).number_value());
            r = std::find_if(next(r), _imp->parts.end(), IsVersionSpecComponentType<vsct_revision>());
        } while (r != _imp->parts.end());

        return result;
    }
    else
        return "r0";
}

std::ostream &
paludis::operator<< (std::ostream & s, const VersionSpec & v)
{
    s << v._imp->text;
    return s;
}

bool
VersionSpec::is_scm() const
{
    bool result(false);
    do
    {
        std::vector<VersionSpecComponent>::const_iterator r;

        if (_imp->parts.empty())
            break;

        /* are we an obvious scm version? */
        r = std::find_if(_imp->parts.begin(), _imp->parts.end(), IsVersionSpecComponentType<vsct_scm>());
        if (r != _imp->parts.end())
        {
            result = true;
            break;
        }

        /* are we a -r9999? */
        r = std::find_if(_imp->parts.begin(), _imp->parts.end(), IsVersionSpecComponentType<vsct_revision>());
        if (r != _imp->parts.end())
            if ((*r).number_value() == "9999")
            {
                result = true;
                break;
            }

        /* is our version without revisions a sequence of, at least,
         * four 9's? */
        std::string mystr = remove_revision()._imp->text;
        if ((mystr.length() >= 4) &&
            (std::string::npos == mystr.find_first_not_of("9")))
        {
            result = true;
            break;
        }
    } while (false);

    return result;
}

bool
VersionSpec::has_try_part() const
{
    return _imp->parts.end() != std::find_if(_imp->parts.begin(), _imp->parts.end(),
            IsVersionSpecComponentType<vsct_trypart>());
}

bool
VersionSpec::has_scm_part() const
{
    return _imp->parts.end() != std::find_if(_imp->parts.begin(), _imp->parts.end(),
            IsVersionSpecComponentType<vsct_scm>());
}

bool
VersionSpec::has_local_revision() const
{
    return 1 < std::count_if(_imp->parts.begin(), _imp->parts.end(),
            IsVersionSpecComponentType<vsct_revision>());
}

VersionSpec
VersionSpec::bump() const
{
    std::vector<VersionSpecComponent> number_parts;
    std::copy(_imp->parts.begin(),
            std::find_if(_imp->parts.begin(), _imp->parts.end(), std::not1(IsEitherVersionSpecComponentType<vsct_number, vsct_floatlike>())),
            std::back_inserter(number_parts));

    if (number_parts.empty())
        return *this;
    if (number_parts.size() > 1)
        number_parts.pop_back();

    /* ++string */
    std::string::reverse_iterator i(number_parts.back().number_value().rbegin()),
        i_end(number_parts.back().number_value().rend());
    bool add1(true);
    while (i != i_end && add1)
    {
        if (*i != '9')
        {
            ++(*i);
            add1 = false;
        }
        else
            *i = '0';
        ++i;
    }
    if (add1)
        number_parts.back().number_value().insert(0, "1");

    bool need_dot(false);
    std::string str;
    for (std::vector<VersionSpecComponent>::const_iterator r(number_parts.begin()), r_end(number_parts.end()) ;
            r != r_end ; ++r)
    {
        if (need_dot)
            str.append(".");
        str.append((*r).number_value());
        need_dot = true;
    }
    return VersionSpec(str, _imp->options);
}

bool
VersionSpec::tilde_greater_compare(const VersionSpec & v) const
{
    return (*this >= v) && (*this < v.bump());
}

bool
VersionSpec::operator< (const VersionSpec & v) const
{
    return -1 == compare(v);
}

bool
VersionSpec::operator== (const VersionSpec & v) const
{
    return 0 == compare(v);
}

VersionSpec::ConstIterator
VersionSpec::begin() const
{
    return ConstIterator(_imp->parts.begin());
}

VersionSpec::ConstIterator
VersionSpec::end() const
{
    return ConstIterator(_imp->parts.end());
}

template class WrappedForwardIterator<VersionSpec::ConstIteratorTag, const VersionSpecComponent>;

