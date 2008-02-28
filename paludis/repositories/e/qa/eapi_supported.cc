/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#include "eapi_supported.hh"
#include <paludis/package_id.hh>
#include <paludis/qa.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/kc.hh>

bool
paludis::erepository::eapi_supported_check(
        const FSEntry & entry,
        QAReporter & reporter,
        const tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & name)
{
    Context context("When performing check '" + name + "' using eapi_supported_check on ID '" + stringify(*id) + "':");

    if (! (*id->eapi())[k::supported()])
    {
        reporter.message(QAMessage(entry, qaml_severe, name,
                    "EAPI '" + stringify((*id->eapi())[k::name()]) + "' not supported")
                .with_associated_id(id));
        return false;
    }

    return true;
}


