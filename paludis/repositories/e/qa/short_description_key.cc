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

#include "short_description_key.hh"
#include <paludis/qa.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/stringify.hh>
#include <paludis/name.hh>

bool
paludis::erepository::short_description_key_check(
        QAReporter & reporter,
        const tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & name)
{
    Context context("When performing check '" + name + "' using short_description_key_check on ID '" + stringify(*id) + "':");

    if (! id->short_description_key())
        reporter.message(qaml_normal, name, "No description available for '" + stringify(*id) + "'");
    else if (id->short_description_key()->value() == stringify(id->name()))
        reporter.message(qaml_normal, name, "Description for '" + stringify(*id) + "' is equal to PN");
    else if (std::string::npos != id->short_description_key()->value().find("Based on the")
            && std::string::npos != id->short_description_key()->value().find("eclass"))
        reporter.message(qaml_normal, name, "Description for '" + stringify(*id) + "' is about as useful as a chocolate teapot");
    else if (id->short_description_key()->value().length() < 10)
        reporter.message(qaml_normal, name, "Description for '" + stringify(*id) + "' is suspiciously short");
    else if (id->short_description_key()->value().length() > 300)
        reporter.message(qaml_normal, name, "Description for '" + stringify(*id) + "' written by Duncan?");
    else if (id->short_description_key()->value().length() > 120)
        reporter.message(qaml_normal, name, "Description for '" + stringify(*id) + "' is too long");

    return true;
}

