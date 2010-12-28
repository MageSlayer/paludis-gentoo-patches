/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2010 Ciaran McCreesh
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

#include "name_description_extractor.hh"
#include "matcher.hh"
#include <paludis/package_id.hh>
#include <paludis/name.hh>
#include <paludis/util/stringify.hh>
#include <paludis/metadata_key.hh>

using namespace paludis;
using namespace inquisitio;

NameDescriptionExtractor::NameDescriptionExtractor()
{
}

NameDescriptionExtractor::~NameDescriptionExtractor()
{
}

bool
NameDescriptionExtractor::operator() (const Matcher & m, const std::shared_ptr<const PackageID> & id) const
{
    if (m(stringify(id->name())))
        return true;

    if (id->short_description_key())
        if (m(id->short_description_key()->value()))
            return true;

    if (id->long_description_key())
        if (m(id->long_description_key()->value()))
            return true;

    return false;
}

