/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include "resume_data.hh"
#include <paludis/args/do_help.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/resolver/job_lists.hh>
#include <paludis/serialise-impl.hh>
#include <paludis/about.hh>

using namespace paludis;
using namespace paludis::resolver;
using namespace cave;

const std::shared_ptr<ResumeData>
ResumeData::deserialise(Deserialisation & d)
{
    if (d.class_name() != "ResumeData@" + stringify(PALUDIS_VERSION))
        throw args::DoHelp("Resume file was created using a different version of Paludis, and cannot be used");

    Deserialisator v(d, "ResumeData@" + stringify(PALUDIS_VERSION));

    std::shared_ptr<Sequence<std::string> > targets(std::make_shared<Sequence<std::string>>());
    {
        Deserialisator vv(*v.find_remove_member("targets"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            targets->push_back(vv.member<std::string>(stringify(n)));
    }

    std::shared_ptr<Sequence<std::string> > world_specs(std::make_shared<Sequence<std::string>>());
    {
        Deserialisator vv(*v.find_remove_member("world_specs"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            world_specs->push_back(vv.member<std::string>(stringify(n)));
    }

    return make_shared_copy(make_named_values<ResumeData>(
                n::job_lists() = v.member<std::shared_ptr<JobLists> >("job_lists"),
                n::preserve_world() = v.member<bool>("preserve_world"),
                n::target_set() = v.member<bool>("target_set"),
                n::targets() = targets,
                n::world_specs() = world_specs
                ));
}

void
ResumeData::serialise(Serialiser & s) const
{
    s.object("ResumeData@" + stringify(PALUDIS_VERSION))
        .member(SerialiserFlags<serialise::might_be_null>(), "job_lists", job_lists())
        .member(SerialiserFlags<>(), "preserve_world", preserve_world())
        .member(SerialiserFlags<>(), "target_set", target_set())
        .member(SerialiserFlags<serialise::might_be_null, serialise::container>(), "targets", targets())
        .member(SerialiserFlags<serialise::might_be_null, serialise::container>(), "world_specs", world_specs())
        ;
}

