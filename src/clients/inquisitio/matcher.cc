/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2010 Ciaran McCreesh
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

#include "matcher.hh"
#include "exact_matcher.hh"
#include "pcre_matcher.hh"
#include "text_matcher.hh"
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/singleton-impl.hh>

using namespace inquisitio;
using namespace paludis;

template class paludis::Singleton<MatcherFactory>;

Matcher::Matcher()
{
}

Matcher::~Matcher()
{
}

NoSuchMatcherError::NoSuchMatcherError(const std::string & msg) throw () :
    Exception("No such matcher '" + msg + "'")
{
}

namespace
{
    template <typename M_>
    static std::shared_ptr<Matcher>
    make(const std::string & s)
    {
        return std::make_shared<M_>(s);
    }
}

MatcherFactory::MatcherFactory()
{
}

const std::shared_ptr<Matcher>
MatcherFactory::create(const std::string & s, const std::string & t) const
{
    if (s == "exact")
        return make<ExactMatcher>(t);
    if (s == "pcre")
        return make<PCREMatcher>(t);
    if (s == "text")
        return make<TextMatcher>(t);
    throw NoSuchMatcherError(t);
}

