/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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


#include <paludis/mask_reasons.hh>
#include <paludis/util/exception.hh>
#include <ostream>

/** \file
 * Implementation of MaskReason classes.
 *
 * \ingroup grpmaskreasons
 */

std::ostream &
paludis::operator<< (std::ostream & s, const MaskReason & r)
{
    do
    {
        switch (r)
        {
            case mr_keyword:
                s << "keyword";
                continue;

            case mr_user_mask:
                s << "user mask";
                continue;

            case mr_profile_mask:
                s << "profile mask";
                continue;

            case mr_repository_mask:
                s << "repository mask";
                continue;

            case mr_eapi:
                s << "EAPI";
                continue;

            case mr_license:
                s << "license";
                continue;

            case mr_by_association:
                s << "by association";
                continue;

            case last_mr:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Bad MaskReason value");
    }
    while (false);

    return s;
}

