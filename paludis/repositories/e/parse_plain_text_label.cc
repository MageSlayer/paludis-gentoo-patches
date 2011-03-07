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

#include <paludis/repositories/e/parse_plain_text_label.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/util/log.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/dep_label.hh>
#include <paludis/dep_spec.hh>
#include <map>
#include <set>

using namespace paludis;
using namespace paludis::erepository;

std::shared_ptr<PlainTextLabelDepSpec>
paludis::erepository::parse_plain_text_label(const std::string & s)
{
    Context context("When parsing label string '" + s + "':");

    if (s.empty())
        throw EDepParseError(s, "Empty label");

    std::string c(s.substr(0, s.length() - 1));
    if (c.empty())
        throw EDepParseError(s, "Unknown label");

    return std::make_shared<PlainTextLabelDepSpec>(s);
}

