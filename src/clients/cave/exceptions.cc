/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010 Ciaran McCreesh
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
#include <paludis/util/iterator_funcs.hh>
#include <paludis/package_id.hh>
#include <paludis/dep_spec.hh>
#include <paludis/fuzzy_finder.hh>

using namespace paludis;
using namespace cave;

NothingMatching::NothingMatching(const PackageDepSpec & spec) noexcept :
    Exception("Found nothing suitable matching '" + stringify(spec) + "'")
{
}

NothingMatching::NothingMatching(const std::string & name) noexcept :
    Exception("Found nothing suitable matching '" + name + "'")
{
}

NothingMatching::NothingMatching(const std::string & name, const std::string & extra_message) noexcept :
    Exception("Found nothing suitable matching '" + name + "'" + extra_message)
{
}

NothingMatchingWithSuggestions::NothingMatchingWithSuggestions(const std::string & name, const std::string & suggestions) noexcept :
    NothingMatching(name, "; did you mean " + suggestions + "?")
{
}

BeMoreSpecific::BeMoreSpecific(const PackageDepSpec & spec, const std::shared_ptr<const PackageIDSequence> & s) noexcept :
    Exception("Found multiple suitable IDs matching '" + stringify(spec) + "': { '" + join(indirect_iterator(s->begin()),
                indirect_iterator(s->end()), "', '") + "' }")
{
}

BeMoreSpecific::BeMoreSpecific(const PackageDepSpec & spec, const std::shared_ptr<const PackageIDSequence> & s,
        const std::string & extra_message) noexcept :
    Exception("Found multiple suitable IDs matching '" + stringify(spec) + "': { '" + join(indirect_iterator(s->begin()),
                indirect_iterator(s->end()), "', '") + "' }. " + extra_message)
{
}

BadIDForCommand::BadIDForCommand(const PackageDepSpec & spec, const std::shared_ptr<const PackageID> & s,
        const std::string & r) noexcept :
    Exception("Spec '" + stringify(spec) + "' resolves to ID '" + stringify(*s) + "', which " + r)
{
}

BadRepositoryForCommand::BadRepositoryForCommand(const RepositoryName & name, const std::string & r) noexcept :
    Exception("Repository '" + stringify(name) + "' unsuitable: " + r)
{
}

void
paludis::cave::nothing_matching_error(
        const Environment * const env,
        const std::string & s,
        const Filter & filter)
{
    FuzzyCandidatesFinder f(*env, s, filter);

    if (f.begin() == f.end())
        throw NothingMatching(s);
    else if (next(f.begin()) == f.end())
        throw NothingMatchingWithSuggestions(s, "'" + stringify(*f.begin()) + "'");
    else
        throw NothingMatchingWithSuggestions(s, "one of '" + join(f.begin(), f.end(), "', '") + "'");
}

