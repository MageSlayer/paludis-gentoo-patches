/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "version_spec.hh"
#include "exception.hh"
#include <algorithm>

using namespace paludis;

BadVersionSpecError::BadVersionSpecError(const std::string & name) throw () :
    NameError(name, "version spec")
{
}

enum SuffixPart
{
    alpha,
    beta,
    pre,
    rc,
    none
};

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

        /// The x.y.z parts of our version (zero filled).
        long version_parts[VersionSpec::max_version_parts_count];

        /// The letter (1.2.3a -> a) part of our version.
        char letter_part;

        /// The suffix level (alpha, beta, pre, rc, none) part of our version.
        SuffixPart suffix_part;

        /// The suffix level of our version.
        unsigned long suffix_level_part;

        /// The patch level of our revision.
        long patch_level_part;

        /// The revision (-r1 -> 1) part of our version.
        unsigned long revision_part;

        /// If we are used in an =* comparison, the number of digits to consider
        unsigned long max_star_count;
    };
}

VersionSpec::VersionSpec(const std::string & text) :
    PrivateImplementationPattern<VersionSpec>(new Implementation<VersionSpec>),
    ComparisonPolicyType(&VersionSpec::compare)
{
    Context c("When parsing version spec '" + text + "':");

    /* set us up with some sane defaults */
    _implementation->text = text;
    std::fill_n(&_implementation->version_parts[0],
            VersionSpec::max_version_parts_count, 0);
    _implementation->letter_part = '\0';
    _implementation->revision_part = 0;
    _implementation->suffix_part = none;
    _implementation->suffix_level_part = 0;
    _implementation->patch_level_part = -1;

    /* parse */
    unsigned version_part_idx(0);
    std::string::size_type p(0);

    /* numbers... */
    while (p < text.length())
    {
        if (text.at(p) < '0' || text.at(p) > '9')
            throw BadVersionSpecError(text);

        _implementation->version_parts[version_part_idx] *= 10;
        _implementation->version_parts[version_part_idx] += text.at(p) - '0';

        if (++p >= text.length())
            break;

        if ('.' == text.at(p))
        {
            ++p;
            if (++version_part_idx >= VersionSpec::max_version_parts_count)
                throw BadVersionSpecError(text);
        }

        if (text.at(p) < '0' || text.at(p) > '9')
            break;
    }
    _implementation->max_star_count = version_part_idx + 1;

    /* letter */
    if (p < text.length())
        if (text.at(p) >= 'a' && text.at(p) <= 'z')
            _implementation->letter_part = text.at(p++);

    /* suffix */
    if (p < text.length())
        do
        {
            if (0 == text.compare(p, 6, "_alpha"))
            {
                _implementation->suffix_part = alpha;
                p += 6;
            }
            else if (0 == text.compare(p, 5, "_beta"))
            {
                _implementation->suffix_part = beta;
                p += 5;
            }
            else if (0 == text.compare(p, 4, "_pre"))
            {
                _implementation->suffix_part = pre;
                p += 4;
            }
            else if (0 == text.compare(p, 3, "_rc"))
            {
                _implementation->suffix_part = rc;
                p += 3;
            }
            else
                break;

            for ( ; p < text.length() ; ++p)
            {
                if (text.at(p) < '0' || text.at(p) > '9')
                    break;
                _implementation->suffix_level_part *= 10;
                _implementation->suffix_level_part += text.at(p) - '0';
            }
        } while (false);

    /* patch level */
    if (p < text.length() && 0 == text.compare(p, 2, "_p"))
    {
        _implementation->patch_level_part = 0;

        for (p += 2 ; p < text.length() ; ++p)
        {
            if (text.at(p) < '0' || text.at(p) > '9')
                break;
            _implementation->patch_level_part *= 10;
            _implementation->patch_level_part += text.at(p) - '0';
        }
    }

    /* revision */
    if (p < text.length())
        if (0 == text.compare(p, 2, "-r"))
        {
            p += 2;
            while (p < text.length())
            {
                if (text.at(p) < '0' || text.at(p) > '9')
                    break;
                _implementation->revision_part *= 10;
                _implementation->revision_part += text.at(p) - '0';
                ++p;
            }
        }


    /* trailing stuff? */
    if (p < text.length())
        throw BadVersionSpecError(text);
}

VersionSpec::VersionSpec(const VersionSpec & other) :
    PrivateImplementationPattern<VersionSpec>(new Implementation<VersionSpec>),
    ComparisonPolicyType(other)
{
    _implementation->text = other._implementation->text;
    /* don't change this to std::copy_n, it's non-portable. */
    std::copy(&other._implementation->version_parts[0],
            &other._implementation->version_parts[max_version_parts_count] + 1,
            &_implementation->version_parts[0]);
    _implementation->letter_part = other._implementation->letter_part;
    _implementation->revision_part = other._implementation->revision_part;
    _implementation->suffix_part = other._implementation->suffix_part;
    _implementation->suffix_level_part = other._implementation->suffix_level_part;
    _implementation->patch_level_part = other._implementation->patch_level_part;
    _implementation->max_star_count = other._implementation->max_star_count;
}

const VersionSpec &
VersionSpec::operator= (const VersionSpec & other)
{
    if (this != &other)
    {
        _implementation->text = other._implementation->text;
        /* don't change this to std::copy_n, it's non-portable. */
        std::copy(&other._implementation->version_parts[0],
                &other._implementation->version_parts[max_version_parts_count] + 1,
                &_implementation->version_parts[0]);
        _implementation->letter_part = other._implementation->letter_part;
        _implementation->revision_part = other._implementation->revision_part;
        _implementation->suffix_part = other._implementation->suffix_part;
        _implementation->suffix_level_part = other._implementation->suffix_level_part;
        _implementation->patch_level_part = other._implementation->patch_level_part;
        _implementation->max_star_count = other._implementation->max_star_count;
    }
    return *this;
}

VersionSpec::~VersionSpec()
{
}

int
VersionSpec::compare(const VersionSpec & other) const
{
    for (unsigned i(0) ; i < VersionSpec::max_version_parts_count ; ++i)
    {
        if (_implementation->version_parts[i] < other._implementation->version_parts[i])
            return -1;
        else if (_implementation->version_parts[i] > other._implementation->version_parts[i])
            return 1;
    }

    if (_implementation->letter_part < other._implementation->letter_part)
        return -1;
    if (_implementation->letter_part > other._implementation->letter_part)
        return 1;

    if (_implementation->suffix_part < other._implementation->suffix_part)
        return -1;
    if (_implementation->suffix_part > other._implementation->suffix_part)
        return 1;

    if (_implementation->suffix_level_part < other._implementation->suffix_level_part)
        return -1;
    if (_implementation->suffix_level_part > other._implementation->suffix_level_part)
        return 1;

    if (_implementation->patch_level_part < other._implementation->patch_level_part)
        return -1;
    if (_implementation->patch_level_part > other._implementation->patch_level_part)
        return 1;

    if (_implementation->revision_part < other._implementation->revision_part)
        return -1;
    if (_implementation->revision_part > other._implementation->revision_part)
        return 1;

    return 0;
}

bool
VersionSpec::tilde_compare(const VersionSpec & other) const
{
    for (unsigned i(0) ; i < VersionSpec::max_version_parts_count ; ++i)
        if (_implementation->version_parts[i] != other._implementation->version_parts[i])
            return false;

    if (_implementation->letter_part != other._implementation->letter_part)
        return false;

    if (_implementation->suffix_part != other._implementation->suffix_part)
        return false;

    if (_implementation->suffix_level_part != other._implementation->suffix_level_part)
        return false;

    if (_implementation->patch_level_part != other._implementation->patch_level_part)
        return false;

    return true;
}

bool
VersionSpec::equal_star_compare(const VersionSpec & other) const
{
    return 0 == _implementation->text.compare(0, other._implementation->text.length(), other._implementation->text);
}

std::size_t
VersionSpec::hash_value() const
{
    /// \todo Improve this;
    return _implementation->version_parts[0];
}

VersionSpec
VersionSpec::remove_revision() const
{
    VersionSpec result(*this);
    result._implementation->revision_part = 0;

    std::string::size_type p;
    if (std::string::npos != ((p = result._implementation->text.rfind("-r"))))
        if (std::string::npos == result._implementation->text.find_first_not_of("0123456789", p + 2))
            result._implementation->text.erase(p);

    return result;
}

std::string
VersionSpec::revision_only() const
{
    return "r" + stringify(_implementation->revision_part);
}

std::ostream &
paludis::operator<< (std::ostream & s, const VersionSpec & v)
{
    s << v._implementation->text;
    return s;
}
