/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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
#include <paludis/util/exception.hh>
#include <paludis/util/smart_record.hh>
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
        letter,
        number,
        scm
    };

    /**
     * Keys for a Part.
     *
     * \see Part
     * \ingroup grpversions
     */
    enum PartKeys
    {
        part_kind,
        part_value,
        last_part
    };

    /**
     * Tag for a Part.
     *
     * \see Part
     * \ingroup grpversions
     */
    struct PartTag :
        SmartRecordTag<comparison_mode::FullComparisonTag, comparison_method::SmartRecordCompareByAllTag>,
        SmartRecordKeys<PartKeys, last_part>,
        SmartRecordKey<part_kind, PartKind>,
        SmartRecordKey<part_value, unsigned long>
    {
    };

    /**
     * An entry in a VersionSpec data vector.
     */
    typedef MakeSmartRecord<PartTag>::Type Part;
}

namespace paludis
{
    /**
     * Implementation data for a VersionSpec.
     */
    template<>
    struct Implementation<VersionSpec> :
        InternalCounted<Implementation<VersionSpec> >
    {
        /// Our raw string representation.
        std::string text;

        /// Our parts.
        std::vector<Part> parts;
    };
}

VersionSpec::VersionSpec(const std::string & text) :
    PrivateImplementationPattern<VersionSpec>(new Implementation<VersionSpec>),
    ComparisonPolicyType(&VersionSpec::compare)
{
    Context c("When parsing version spec '" + text + "':");

    /* set us up with some sane defaults */
    _imp->text = text;

    /* parse */
    std::string::size_type p(0);

    if (text == "scm")
    {
        _imp->parts.push_back(Part(scm, 0));
        return;
    }

    /* numbers... */
    unsigned long x(0);
    while (p < text.length())
    {
        if (text.at(p) < '0' || text.at(p) > '9')
            throw BadVersionSpecError(text);

        x *= 10;
        x += text.at(p) - '0';

        if (++p >= text.length())
            break;

        if ('.' == text.at(p))
        {
            ++p;
            _imp->parts.push_back(Part(number, x));
            x = 0;
        }

        if (text.at(p) < '0' || text.at(p) > '9')
            break;
    }
    _imp->parts.push_back(Part(number, x));

    while (_imp->parts.size() > 1)
    {
        if (0 == _imp->parts[_imp->parts.size() - 1].get<part_value>())
            _imp->parts.pop_back();
        else
            break;
    }

    /* letter */
    if (p < text.length())
        if (text.at(p) >= 'a' && text.at(p) <= 'z')
            _imp->parts.push_back(Part(letter, text.at(p++)));

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

            x = std::numeric_limits<unsigned long>::max();
            for ( ; p < text.length() ; ++p)
            {
                if (text.at(p) < '0' || text.at(p) > '9')
                    break;
                if (x == std::numeric_limits<unsigned long>::max())
                    x = 0;
                x *= 10;
                x += text.at(p) - '0';
            }

            _imp->parts.push_back(Part(k, x));
        } while (false);

    /* patch level */
    if (p < text.length() && 0 == text.compare(p, 2, "_p"))
    {
        x = std::numeric_limits<unsigned long>::max();
        for (p += 2 ; p < text.length() ; ++p)
        {
            if (text.at(p) < '0' || text.at(p) > '9')
                break;
            if (x == std::numeric_limits<unsigned long>::max())
                x = 0;
            x *= 10;
            x += text.at(p) - '0';
        }
        _imp->parts.push_back(Part(patch, x));
    }

    /* scm */
    if ((p < text.length()) && (0 == text.compare(p, 4, "-scm")))
    {
        p += 4;
        _imp->parts.push_back(Part(scm, 0));
    }
    else
    {
        for (std::vector<Part>::iterator i(_imp->parts.begin()),
                i_end(_imp->parts.end()) ; i != i_end ; ++i)
            if (std::numeric_limits<unsigned long>::max() == i->get<part_value>())
                i->set<part_value>(0);
    }

    /* revision */
    if (p < text.length())
        if (0 == text.compare(p, 2, "-r"))
        {
            p += 2;
            x = 0;
            while (p < text.length())
            {
                if (text.at(p) < '0' || text.at(p) > '9')
                    break;
                x *= 10;
                x += text.at(p) - '0';
                ++p;
            }
            _imp->parts.push_back(Part(revision, x));
        }


    /* trailing stuff? */
    if (p < text.length())
        throw BadVersionSpecError(text);
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

    Part end_part(empty, 0);
    while (true)
    {
        const Part * const p1(v1 == v1_end ? &end_part : &*v1++);
        const Part * const p2(v2 == v2_end ? &end_part : &*v2++);
        if (&end_part == p1 && &end_part == p2)
            break;

        if (*p1 < *p2)
            return -1;
        if (*p1 > *p2)
            return 1;
    }

    return 0;
}

bool
VersionSpec::tilde_compare(const VersionSpec & other) const
{
    std::vector<Part>::const_iterator
        v1(_imp->parts.begin()), v1_end(_imp->parts.end()),
        v2(other._imp->parts.begin()), v2_end(other._imp->parts.end());

    Part end_part(empty, 0);
    while (true)
    {
        const Part * p1(v1 == v1_end ? &end_part : &*v1++);
        while (p1 != &end_part && p1->get<part_kind>() == revision)
            p1 = (v1 == v1_end ? &end_part : &*v1++);

        const Part * p2(v2 == v2_end ? &end_part : &*v2++);
        while (p2 != &end_part && p2->get<part_kind>() == revision)
            p2 = (v2 == v2_end ? &end_part : &*v2++);

        if (&end_part == p1 && &end_part == p2)
            break;

        if (*p1 != *p2)
            return false;
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
    /// \todo Improve this;
    if (_imp->parts.empty())
        return 0;
    return _imp->parts[0].get<part_value>();
}

namespace
{
    /**
     * Identify Part instances that are revisions.
     *
     * \ingroup grpversions
     */
    struct IsRevisionPart
    {
        bool operator() (const Part & p) const
        {
            return p.get<part_kind>() == revision;
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
                IsRevisionPart()), result._imp->parts.end());

    std::string::size_type p;
    if (std::string::npos != ((p = result._imp->text.rfind("-r"))))
        if (std::string::npos == result._imp->text.find_first_not_of("0123456789", p + 2))
            result._imp->text.erase(p);

    return result;
}

std::string
VersionSpec::revision_only() const
{
    std::vector<Part>::const_iterator r(std::find_if(_imp->parts.begin(),
                _imp->parts.end(), IsRevisionPart()));
    if (r != _imp->parts.end())
        return "r" + stringify(r->get<part_value>());
    else
        return "r0";
}

std::ostream &
paludis::operator<< (std::ostream & s, const VersionSpec & v)
{
    s << v._imp->text;
    return s;
}

