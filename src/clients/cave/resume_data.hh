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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_CAVE_RESUME_DATA_HH
#define PALUDIS_GUARD_SRC_CLIENTS_CAVE_RESUME_DATA_HH 1

#include <paludis/util/named_value.hh>
#include <paludis/util/sequence-fwd.hh>
#include <paludis/resolver/job_lists-fwd.hh>
#include <paludis/serialise-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct job_lists_name> job_lists;
        typedef Name<struct preserve_world_name> preserve_world;
        typedef Name<struct target_set_name> target_set;
        typedef Name<struct targets_name> targets;
    }

    namespace cave
    {
        struct ResumeData
        {
            NamedValue<n::job_lists, std::tr1::shared_ptr<resolver::JobLists> > job_lists;
            NamedValue<n::preserve_world, bool> preserve_world;
            NamedValue<n::target_set, bool> target_set;
            NamedValue<n::targets, std::tr1::shared_ptr<Sequence<std::string> > > targets;

            void serialise(Serialiser &) const;
            static const std::tr1::shared_ptr<ResumeData> deserialise(
                    Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
