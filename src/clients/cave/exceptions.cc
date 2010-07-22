/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#include "exceptions.hh"
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/package_id.hh>
#include <paludis/dep_spec.hh>

using namespace paludis;
using namespace cave;

NothingMatching::NothingMatching(const PackageDepSpec & spec) throw () :
    Exception("Found nothing suitable matching '" + stringify(spec) + "'")
{
}

NothingMatching::NothingMatching(const std::string & name) throw () :
    Exception("Found nothing suitable matching '" + name + "'")
{
}

BeMoreSpecific::BeMoreSpecific(const PackageDepSpec & spec, const std::shared_ptr<const PackageIDSequence> & s) throw () :
    Exception("Found multiple suitable IDs matching '" + stringify(spec) + "': { '" + join(indirect_iterator(s->begin()),
                indirect_iterator(s->end()), "', '") + "' }")
{
}

BadIDForCommand::BadIDForCommand(const PackageDepSpec & spec, const std::shared_ptr<const PackageID> & s,
        const std::string & r) throw () :
    Exception("Spec '" + stringify(spec) + "' resolves to ID '" + stringify(*s) + "', which " + r)
{
}

BadRepositoryForCommand::BadRepositoryForCommand(const RepositoryName & name, const std::string & r) throw () :
    Exception("Repository '" + stringify(name) + "' unsuitable: " + r)
{
}

