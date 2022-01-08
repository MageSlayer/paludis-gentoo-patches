/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh
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

#include <paludis/util/stringify.hh>
#include <paludis/util/operators.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_spec.hh>

using namespace paludis;

#include <paludis/version_operator-se.cc>

VersionOperatorValue
VersionOperator::_decode(const std::string & v)
{
    switch (v.length())
    {
        case 1:
            switch (v[0])
            {
                case '>':
                    return vo_greater;

                case '=':
                    return vo_equal;

                case '<':
                    return vo_less;

                case '~':
                    return vo_tilde;
            }
            break;

        case 2:
            if ('=' == v[1])
                switch (v[0])
                {
                    case '>':
                        return vo_greater_equal;

                    case '<':
                        return vo_less_equal;
                }
            else if ('>' == v[1])
                switch (v[0])
                {
                    case '~':
                        return vo_tilde_greater;
                }
    }

    throw BadVersionOperatorError(v);
}

std::ostream &
paludis::operator<< (std::ostream & s, const VersionOperator & v)
{
    do
    {
        switch (v._v)
        {
            case vo_greater:
                s << ">";
                continue;

            case vo_equal:
                s << "=";
                continue;

            case vo_less:
                s << "<";
                continue;

            case vo_tilde:
                s << "~";
                continue;

            case vo_greater_equal:
                s << ">=";
                continue;

            case vo_less_equal:
                s << "<=";
                continue;

            case vo_equal_star:
                s << "=*";
                continue;

            case vo_tilde_greater:
                s << "~>";
                continue;

            case last_vo:
                break;
        }

        throw InternalError(PALUDIS_HERE, "v._v is " + stringify(v._v));

    } while (false);

    return s;
}

namespace
{
    template <bool (VersionSpec::* m_) (const VersionSpec &) const>
    bool
    member_to_comparator(const VersionSpec & a, const VersionSpec & b)
    {
        return (a.*m_)(b);
    }
}

VersionOperator::VersionSpecComparator
VersionOperator::as_version_spec_comparator() const
{
    switch (_v)
    {
        case vo_less:
            return &member_to_comparator<&VersionSpec::operator<>;
        case vo_less_equal:
            return &relational_operators::operator<= <const VersionSpec>;
        case vo_equal:
            return &member_to_comparator<&VersionSpec::operator==>;
        case vo_tilde:
            return &member_to_comparator<&VersionSpec::tilde_compare>;
        case vo_greater:
            return &relational_operators::operator> <const VersionSpec>;
        case vo_greater_equal:
            return &relational_operators::operator>= <const VersionSpec>;
        case vo_equal_star:
            return &member_to_comparator<&VersionSpec::equal_star_compare>;
        case vo_tilde_greater:
            return &member_to_comparator<&VersionSpec::tilde_greater_compare>;
        case last_vo:
            break;
    }

    throw InternalError(PALUDIS_HERE, "_v is " + stringify(_v));

}

BadVersionOperatorError::BadVersionOperatorError(const std::string & msg) noexcept :
    Exception("Bad version operator '" + msg + "'")
{
}
