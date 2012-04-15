/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2009, 2011 Ciaran McCreesh
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

#include <paludis/util/map.hh>
#include <paludis/util/map-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>
#include <string>

using namespace paludis;

namespace paludis
{
    template class Map<std::string, std::string>;
    template class PALUDIS_VISIBLE DefaultMapComparator<std::string>;
    template class WrappedForwardIterator<Map<std::string, std::string>::ConstIteratorTag, const std::pair<const std::string, std::string> >;
    template class PALUDIS_VISIBLE WrappedOutputIterator<Map<std::string, std::string>::InserterTag, std::pair<const std::string, std::string> >;
}
