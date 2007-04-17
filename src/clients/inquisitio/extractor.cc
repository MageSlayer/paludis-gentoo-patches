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

#include "extractor.hh"
#include "description_extractor.hh"
#include "name_extractor.hh"
#include <paludis/util/virtual_constructor-impl.hh>

using namespace inquisitio;
using namespace paludis;

template class paludis::VirtualConstructor<std::string,
         std::tr1::shared_ptr<Extractor> (*) (const paludis::Environment &),
         paludis::virtual_constructor_not_found::ThrowException<NoSuchExtractorError> >;

NoSuchExtractorError::NoSuchExtractorError(const std::string & m) throw () :
    Exception("No such extractor '" + m + "'")
{
}

namespace
{
    template <typename M_>
    std::tr1::shared_ptr<Extractor>
    make(const Environment & e)
    {
        return std::tr1::shared_ptr<Extractor>(new M_(e));
    }
}

ExtractorMaker::ExtractorMaker()
{
    register_maker("description", &make<DescriptionExtractor>);
    register_maker("name", &make<NameExtractor>);
}

Extractor::Extractor()
{
}

Extractor::~Extractor()
{
}

