/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/version_requirements.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>

using namespace paludis;

std::ostream &
paludis::operator<< (std::ostream & o, const VersionRequirementsMode & s)
{
    do
    {
        switch (s)
        {
            case vr_and:
                o << "and";
                continue;

            case vr_or:
                o << "or";
                continue;

            case last_vr:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Bad VersionRequirementsMode");
    } while (false);

    return o;
}

namespace paludis
{
    template class Sequence<VersionRequirement>;
    template class WrappedForwardIterator<Sequence<VersionRequirement>::ConstIteratorTag, const VersionRequirement>;
    template class WrappedOutputIterator<Sequence<VersionRequirement>::InserterTag, VersionRequirement>;
}
