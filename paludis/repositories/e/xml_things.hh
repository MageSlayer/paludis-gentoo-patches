/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_XML_THINGS_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_XML_THINGS_HH 1

#include <paludis/repositories/e/glsa.hh>
#include <paludis/repositories/e/metadata_xml.hh>
#include "config.h"

extern "C"
{
    void paludis_xml_things_init() PALUDIS_VISIBLE;
    void paludis_xml_things_cleanup() PALUDIS_VISIBLE;

#if ENABLE_XML
    std::shared_ptr<paludis::GLSA> PALUDIS_VISIBLE paludis_xml_things_create_glsa_from_xml_file(const std::string &);

    std::shared_ptr<paludis::erepository::MetadataXML> PALUDIS_VISIBLE paludis_xml_things_create_metadata_xml_from_xml_file(
            const paludis::FSPath &) PALUDIS_ATTRIBUTE((warn_unused_result));
#endif
}

#endif
