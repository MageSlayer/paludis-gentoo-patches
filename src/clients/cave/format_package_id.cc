/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Alex Elsayed
 * Based in part on format_plain_metadata_key.hh, which is
 *     Copyright (c) 2008, 2010, 2011 Ciaran McCreesh
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

#include "format_package_id.hh"
#include "format_string.hh"

#include <paludis/metadata_key.hh>
#include <paludis/name.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/map.hh>
#include <paludis/util/stringify.hh>
#include <paludis/package_id.hh>
#include <paludis/version_spec.hh>
#include <paludis/dep_spec.hh>
#include <paludis/slot.hh>
#include <string>

using namespace paludis;
using namespace cave;

std::string
paludis::cave::format_package_id(
        const std::shared_ptr<const PackageID> & id,
        const std::string & format)
{
    std::shared_ptr<Map<char, std::string> > m(std::make_shared<Map<char, std::string>>());
    m->insert('c', stringify(id->name().category()));
    m->insert('p', stringify(id->name().package()));
    m->insert('v', stringify(id->version()));
    m->insert('s', id->slot_key() ? stringify(id->slot_key()->parse_value().parallel_value()) : "");
    m->insert('S', id->slot_key() ? stringify(id->slot_key()->parse_value().raw_value()) : "");
    m->insert(':', id->slot_key() ? ":" : "");
    m->insert('I', stringify(id->name().category()) + '/' + stringify(id->name().package()) +
         (id->slot_key() ?
             ":" + stringify(id->slot_key()->parse_value().parallel_value()) : ""));
    m->insert('r', stringify(id->repository_name()));
    m->insert('F', id->canonical_form(idcf_full));
    m->insert('V', id->canonical_form(idcf_version));
    m->insert('W', id->canonical_form(idcf_no_version));
    m->insert('N', id->canonical_form(idcf_no_name));
    m->insert('u', stringify(id->uniquely_identifying_spec()));
    return format_string(format, m);
}

const std::string paludis::cave::format_package_id_help(
    "Select the output format. Special tokens recognised are "
    "%c for category, %p for package, %v for version, %s for slot, %S for slot in its native format, "
    "%: for ':' if we have a slot and "
    "empty otherwise, %r for repository, %F for the canonical full form, %V for the canonical full "
    "version, %W for the canonical full unversioned form, %N for the canonical full unnamed form, "
    "%u for a uniquely identifying dependency spec, %I is equivalent to %c/%p%:%s,"
    "\\n for newline, \\t for tab. Default is '%F\\n'."
);
