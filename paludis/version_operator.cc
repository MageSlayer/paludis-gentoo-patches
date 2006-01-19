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

#include "version_operator.hh"
#include "internal_error.hh"
#include "stringify.hh"

using namespace paludis;

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
                    return vo_tidle;
            }

        case 2:
            if ('=' == v[1])
                switch (v[0])
                {
                    case '>':
                        return vo_greater_equal;

                    case '<':
                        return vo_less_equal;
                }
    }

    throw InternalError(PALUDIS_HERE, "todo"); /// \bug
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

            case vo_tidle:
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

            case last_vo:
                break;
        }

        throw InternalError(PALUDIS_HERE, "v._v is " + stringify(v._v));

    } while (false);

    return s;
}

// The function below makes Doxygen crap itself... It's a bug in doxygen, not
// a genuinely undocumented function.
bool (VersionSpec::* VersionOperator::as_version_spec_operator() const)(const VersionSpec &) const
{
    switch (_v)
    {
        case vo_less:
            return &VersionSpec::operator< ;
        case vo_less_equal:
            return &VersionSpec::operator<= ;
        case vo_equal:
            return &VersionSpec::operator== ;
        case vo_tidle:
            return &VersionSpec::tilde_compare ;
        case vo_greater:
            return &VersionSpec::operator> ;
        case vo_greater_equal:
            return &VersionSpec::operator>= ;
        case vo_equal_star:
            return &VersionSpec::equal_star_compare ;
        case last_vo:
            break;
    }

    throw InternalError(PALUDIS_HERE, "_v is " + stringify(_v));
}

