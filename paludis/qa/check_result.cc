/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "check_result.hh"

using namespace paludis;
using namespace paludis::qa;

CheckResult::CheckResult(const FSEntry & f, const std::string & r) :
    _item(stringify(f)),
    _rule(stringify(r))
{
}

QALevel
CheckResult::most_severe_level() const
{
    QALevel result(static_cast<QALevel>(0));
    for (Iterator i(begin()), i_end(end()) ; i != i_end ; ++i)
        result = std::max(result, i->get<mk_level>());
    return result;
}
