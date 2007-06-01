/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include "pcre_matcher.hh"
#include "text_matcher.hh"
#include <paludis/util/virtual_constructor-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>

using namespace inquisitio;
using namespace paludis;

template class paludis::VirtualConstructor<std::string,
         tr1::shared_ptr<Matcher> (*) (const std::string &),
         paludis::virtual_constructor_not_found::ThrowException<NoSuchMatcherError> >;

template class paludis::InstantiationPolicy<MatcherMaker, paludis::instantiation_method::SingletonTag>;

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
    static tr1::shared_ptr<Matcher>
    make(const std::string & s)
    {
        return tr1::shared_ptr<Matcher>(new M_(s));
    }
}

MatcherMaker::MatcherMaker()
{
    register_maker("pcre", &make<PCREMatcher>);
    register_maker("text", &make<TextMatcher>);
}

