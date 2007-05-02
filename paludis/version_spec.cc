/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/util/compare.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/log.hh>
#include <paludis/version_spec.hh>
#include <vector>
#include <limits>

/** \file
 * Implementation of VersionSpec.
 *
 * \ingroup grpversions
 */

using namespace paludis;

BadVersionSpecError::BadVersionSpecError(const std::string & name) throw () :
    NameError(name, "version spec")
{
}

BadVersionSpecError::BadVersionSpecError(const std::string & name, const std::string & msg) throw () :
    NameError(name, "version spec", msg)
{
}

namespace
{
    /**
     * Type of a Part in a VersionSpec data vector.
     *
     * \ingroup grpversions
     */
    enum PartKind
    {
        alpha,
        beta,
        pre,
        rc,
        empty,
        revision,
        patch,
        trypart,
        letter,
        number,
        scm
    };

#include <paludis/version_spec-sr.hh>
#include <paludis/version_spec-sr.cc>

}

namespace paludis
{
    /**
     * Implementation data for a VersionSpec.
     */
    template<>
    struct Implementation<VersionSpec>
    {
        /// Our raw string representation.
        std::string text;

        /// Our parts.
        std::vector<Part> parts;

        /// Our hash
        mutable bool has_hash;
        mutable std::size_t hash;

        /// Our is_scm
        mutable bool has_is_scm;
        mutable bool is_scm;

        Implementation() :
            has_hash(false),
            has_is_scm(false)
        {
        }
    };
}

VersionSpec::VersionSpec(const std::string & text) :
    PrivateImplementationPattern<VersionSpec>(new Implementation<VersionSpec>),
    ComparisonPolicyType(&VersionSpec::compare)
{
    Context c("When parsing version spec '" + text + "':");

    if (text.empty())
        throw BadVersionSpecError(text, "cannot be empty");

    /* set us up with some sane defaults */
    _imp->text = text;

    /* parse */
    std::string::size_type p(0);

    if (0 == text.compare(p, 3, "scm"))
    {
        _imp->parts.push_back(Part(scm, ""));
        p += 3;
    }
    else
    {
        /* numbers... */
        while (p < text.length())
        {
            std::string::size_type q(text.find_first_not_of("0123456789", p));
            std::string number_part(std::string::npos == q ? text.substr(p) : text.substr(p, q - p));
            p = std::string::npos == q ? text.length() : q;

            if (number_part.empty())
                throw BadVersionSpecError(text, "Expected number part not found at offset " + stringify(p));

            if (number_part.size() > 8)
                Log::get_instance()->message(ll_qa, lc_context) <<
                    "Number part '" << number_part << "' exceeds 8 digit limit";

            _imp->parts.push_back(Part(number, number_part));

            if (p < text.length() && ('.' != text.at(p) || p + 1 == text.length()))
                break;

            ++p;
        }

        /* letter */
        if (p < text.length())
            if (text.at(p) >= 'a' && text.at(p) <= 'z')
                _imp->parts.push_back(Part(letter, text.substr(p++, 1)));

        bool suffix(true);
        while (suffix)
        {
            suffix = false;

            /* suffix */
            if (p < text.length())
                do
                {
                    PartKind k(empty);
                    if (0 == text.compare(p, 6, "_alpha"))
                    {
                        k = alpha;
                        p += 6;
                    }
                    else if (0 == text.compare(p, 5, "_beta"))
                    {
                        k = beta;
                        p += 5;
                    }
                    else if (0 == text.compare(p, 4, "_pre"))
                    {
                        k = pre;
                        p += 4;
                    }
                    else if (0 == text.compare(p, 3, "_rc"))
                    {
                        k = rc;
                        p += 3;
                    }
                    else
                        break;

                    std::string::size_type q(text.find_first_not_of("0123456789", p));
                    std::string number_part(std::string::npos == q ? text.substr(p) : text.substr(p, q - p));
                    p = std::string::npos == q ? text.length() : q;

                    if (number_part.size() > 8)
                        Log::get_instance()->message(ll_qa, lc_context) <<
                            "Number part '" << number_part << "' exceeds 8 digit limit";

                    if (number_part.size() > 0)
                    {
                        number_part = strip_leading(number_part, "0");
                        if (number_part.empty())
                            number_part = "0";
                    }
                    _imp->parts.push_back(Part(k, number_part));
                    suffix = true;
                } while (false);
        }

        /* patch level */
        if (p < text.length() && 0 == text.compare(p, 2, "_p"))
        {
            p += 2;

            std::string::size_type q(text.find_first_not_of("0123456789", p));
            std::string number_part(std::string::npos == q ? text.substr(p) : text.substr(p, q - p));
            p = std::string::npos == q ? text.length() : q;

            if (number_part.size() > 8)
                Log::get_instance()->message(ll_qa, lc_context) <<
                    "Number part '" << number_part << "' exceeds 8 digit limit";

            if (number_part.size() > 0)
            {
                number_part = strip_leading(number_part, "0");
                if (number_part.empty())
                    number_part = "0";
            }
            _imp->parts.push_back(Part(patch, number_part));
        }

        /* try */
        if (p < text.length() && 0 == text.compare(p, 4, "-try"))
        {
            p += 4;

            std::string::size_type q(text.find_first_not_of("0123456789", p));
            std::string number_part(std::string::npos == q ? text.substr(p) : text.substr(p, q - p));
            p = std::string::npos == q ? text.length() : q;

            if (number_part.size() > 8)
                Log::get_instance()->message(ll_qa, lc_context) <<
                    "Number part '" << number_part << "' exceeds 8 digit limit";

            if (number_part.size() > 0)
            {
                number_part = strip_leading(number_part, "0");
                if (number_part.empty())
                    number_part = "0";
            }
            _imp->parts.push_back(Part(trypart, number_part));
        }

        /* scm */
        if ((p < text.length()) && (0 == text.compare(p, 4, "-scm")))
        {
            p += 4;
            /* _suffix-scm? */
            if (_imp->parts.back().value.empty())
                _imp->parts.back().value = "MAX";

            _imp->parts.push_back(Part(scm, ""));
        }
        /* Now we can change empty values to "0" */
        for (std::vector<Part>::iterator i(_imp->parts.begin()),
                i_end(_imp->parts.end()) ; i != i_end ; ++i)
            if (i->value.empty())
                i->value = "0";
    }

    /* revision */
    if (p < text.length())
        if (0 == text.compare(p, 2, "-r"))
        {
            p += 2;
            do
            {
                std::string::size_type q(text.find_first_not_of("0123456789", p));
                std::string number_part(std::string::npos == q ? text.substr(p) : text.substr(p, q - p));
                p = std::string::npos == q ? text.length() : q;

                if (number_part.size() > 8)
                    Log::get_instance()->message(ll_qa, lc_context) <<
                        "Number part '" << number_part << "' exceeds 8 digit limit";

                /* Are we -r */
                bool empty(number_part.empty());

                number_part = strip_leading(number_part, "0");
                if (number_part.empty())
                    number_part = "0";
                _imp->parts.push_back(Part(revision, number_part));

                if (p >= text.length())
                    break;
                if (text.at(p) != '.')
                    break;
                else if (empty)
                {
                    /* we don't like -r. */
                    break;
                }
                else if (! (p + 1 < text.length() && text.at(p + 1) >= '0' && text.at(p + 1) <= '9'))
                {
                    /* we don't like -rN.x where x is not a digit */
                    break;
                }
                else
                    ++p;
            }
            while (true);
        }

    /* trailing stuff? */
    if (p < text.length())
        throw BadVersionSpecError(text, "unexpected trailing text '" + text.substr(p) + "'");
}

VersionSpec::VersionSpec(const VersionSpec & other) :
    PrivateImplementationPattern<VersionSpec>(new Implementation<VersionSpec>),
    ComparisonPolicyType(other)
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
        _imp->has_hash = other._imp->has_hash;
        _imp->hash = other._imp->hash;
        _imp->has_is_scm = other._imp->has_is_scm;
        _imp->is_scm = other._imp->is_scm;
    }
    return *this;
}

VersionSpec::~VersionSpec()
{
}

int
VersionSpec::compare(const VersionSpec & other) const
{
    std::vector<Part>::const_iterator
        v1(_imp->parts.begin()), v1_end(_imp->parts.end()),
        v2(other._imp->parts.begin()), v2_end(other._imp->parts.end());

    Part end_part(empty, "");
    bool first(true);
    while (true)
    {
        const Part * const p1(v1 == v1_end ? &end_part : &*v1++);
        const Part * const p2(v2 == v2_end ? &end_part : &*v2++);

        if (&end_part == p1 && &end_part == p2)
            break;

        if (p1 == &end_part && p2->kind == revision && p2->value == "0")
            continue;

        if (p2 == &end_part && p1->kind == revision && p1->value == "0")
            continue;

        if (p1->kind < p2->kind)
            return -1;
        if (p1->kind > p2->kind)
            return 1;

        std::string p1s, p2s;
        bool length_cmp(true);

        /* number parts */
        if (p1->kind == number)
        {
            if (first)
            {
                /* first component - always as integer (leading zeroes removed) */
                first = false;
                p1s = strip_leading(p1->value, "0");
                p2s = strip_leading(p2->value, "0");
            }
            else if ((! p1->value.empty() && p1->value.at(0) == '0') ||
                    (! p2->value.empty() && p2->value.at(0) == '0'))
            {
                /* leading zeroes - stringwise compare with trailing zeroes removed */
                length_cmp = false;
                p1s = strip_trailing(p1->value, "0");
                p2s = strip_trailing(p2->value, "0");
            }
            else
            {
                p1s = p1->value;
                p2s = p2->value;
            }
        }
        /* anything else than number parts */
        else
        {
            p1s = p1->value;
            p2s = p2->value;

            /* _suffix-scm? */
            if (p1s == "MAX" && p2s == "MAX")
                continue;
            else if (p1s == "MAX")
                return 1;
            else if (p2s == "MAX")
                return -1;
        }
        /* common part */
        if (length_cmp)
        {
            /* length compare (integers) */
            int c = p1s.size() - p2s.size();
            if (c < 0)
                return -1;
            else if (c > 0)
                return 1;
        }
        /* stringwise compare (also for integers with the same size) */
        int c(p1s.compare(p2s));
        if (c != 0)
            return c;
    }

    return 0;
}

bool
VersionSpec::tilde_compare(const VersionSpec & other) const
{
    std::vector<Part>::const_iterator
        v1(_imp->parts.begin()), v1_end(_imp->parts.end()),
        v2(other._imp->parts.begin()), v2_end(other._imp->parts.end());

    Part end_part(empty, "");
    bool first(true);
    while (true)
    {
        const Part * const p1(v1 == v1_end ? &end_part : &*v1++);
        const Part * const p2(v2 == v2_end ? &end_part : &*v2++);
        if (&end_part == p1 && &end_part == p2)
            break;

        if (p1->kind != p2->kind)
        {
            if (p2 != &end_part || p1->kind != revision)
                return false;
        }
        else
        {
            std::string p1s, p2s;
            /* number part */
            if (p1->kind == number)
            {
                if (first)
                {
                    /* first component - remove leading zeroes and check whether equal */
                    first = false;
                    if (strip_leading(p1->value, "0") != strip_leading(p2->value, "0"))
                        return false;
                }
                else if ((! p1->value.empty() && p1->value.at(0) == '0') ||
                        (! p2->value.empty() && p2->value.at(0) == '0'))
                {
                    /* leading zeroes - remove trailing zeroes and check whether equal */
                    if (strip_trailing(p1->value, "0") != strip_trailing(p2->value, "0"))
                        return false;
                }
                else
                {
                    /* normal(!) case */
                    if (p1->value != p2->value)
                        return false;
                }
            }
            /* revision - compare as integers */
            else if (p1->kind == revision)
            {
                int c = p1->value.size() - p2->value.size();
                if (c < 0)
                    return false;
                else if (c == 0 && p1->value.compare(p2->value) == -1)
                    return false;
            }
            /* not a number part nor revision - must be just equal */
            else if (p1->value != p2->value)
                return false;
        }
    }

    return true;
}

bool
VersionSpec::equal_star_compare(const VersionSpec & other) const
{
    return 0 == _imp->text.compare(0, other._imp->text.length(), other._imp->text);
}

std::size_t
VersionSpec::hash_value() const
{
    if (_imp->has_hash)
        return _imp->hash;

    size_t result(0);

    const std::size_t h_shift = std::numeric_limits<std::size_t>::digits - 5;
    const std::size_t h_mask = static_cast<std::size_t>(0x1f) << h_shift;

    do
    {
        bool first(true);
        for (std::vector<Part>::const_iterator r(_imp->parts.begin()), r_end(_imp->parts.end()) ;
                r != r_end ; ++r)
        {
            if (r->value == "0" && r->kind == revision)
                continue;

            std::size_t hh(result & h_mask);
            result <<= 5;
            result ^= (hh >> h_shift);

            std::string r_v;
            if (! r->value.empty() && r->value.at(0) == '0')
                r_v = strip_trailing(r->value, "0");
            else
                r_v = r->value;

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

            result ^= (static_cast<std::size_t>(r->kind) + (x << 3) + (zeroes << 12));
        }
    } while (false);

    _imp->has_hash = true;
    _imp->hash = result;
    return result;
}

namespace
{
    /**
     * Identify Part instances that are of a certain kind.
     *
     * \ingroup grpversions
     */
    template <PartKind p_>
    struct IsPart :
        std::unary_function<Part, bool>
    {
        bool operator() (const Part & p) const
        {
            return p.kind == p_;
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
                IsPart<revision>()), result._imp->parts.end());

    std::string::size_type p;
    if (std::string::npos != ((p = result._imp->text.rfind("-r"))))
        if (std::string::npos == result._imp->text.find_first_not_of("0123456789.", p + 2))
            result._imp->text.erase(p);

    return result;
}

std::string
VersionSpec::revision_only() const
{
    std::vector<Part>::const_iterator r(std::find_if(_imp->parts.begin(), _imp->parts.end(), IsPart<revision>()));
    if (r != _imp->parts.end())
    {
        std::string result;
        do
        {
            if (! result.empty())
                result.append(".");
            else
                result = "r";

            result.append(r->value);
            r = std::find_if(next(r), _imp->parts.end(), IsPart<revision>());
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
    if (_imp->has_is_scm)
        return _imp->is_scm;

    bool result(false);
    do
    {
        std::vector<Part>::const_iterator r;

        if (_imp->parts.empty())
            break;

        /* are we an obvious scm version? */
        r = std::find_if(_imp->parts.begin(), _imp->parts.end(), IsPart<scm>());
        if (r != _imp->parts.end())
        {
            result = true;
            break;
        }

        /* are we a -r9999? */
        r = std::find_if(_imp->parts.begin(), _imp->parts.end(), IsPart<revision>());
        if (r != _imp->parts.end())
            if (r->value == "9999")
            {
                result = true;
                break;
            }

        /* is our version without revisions exactly 9999? */
        if (remove_revision() == VersionSpec("9999"))
        {
            result = true;
            break;
        }
    } while (false);

    _imp->is_scm = result;
    _imp->has_is_scm = true;

    return result;
}

bool
VersionSpec::has_try_part() const
{
    return _imp->parts.end() != std::find_if(_imp->parts.begin(), _imp->parts.end(), IsPart<trypart>());
}

bool
VersionSpec::has_scm_part() const
{
    return _imp->parts.end() != std::find_if(_imp->parts.begin(), _imp->parts.end(), IsPart<scm>());
}

VersionSpec
VersionSpec::bump() const
{
    std::vector<Part> number_parts;
    std::copy(_imp->parts.begin(),
            std::find_if(_imp->parts.begin(), _imp->parts.end(), std::not1(IsPart<number>())),
            std::back_inserter(number_parts));

    if (number_parts.empty())
        return *this;
    if (number_parts.size() > 1)
        number_parts.pop_back();

    /* ++string */
    std::string::reverse_iterator i(number_parts.back().value.rbegin()),
        i_end(number_parts.back().value.rend());
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
        number_parts.back().value.insert(0, "1");

    bool need_dot(false);
    std::string str;
    for (std::vector<Part>::const_iterator r(number_parts.begin()), r_end(number_parts.end()) ;
            r != r_end ; ++r)
    {
        if (need_dot)
            str.append(".");
        str.append(r->value);
        need_dot = true;
    }
    return VersionSpec(str);
}

bool
VersionSpec::tilde_greater_compare(const VersionSpec & v) const
{
    return operator>= (v) && operator< (v.bump());
}

