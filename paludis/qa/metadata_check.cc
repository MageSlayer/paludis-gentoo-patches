/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <ctime>
#include <fstream>
#include <paludis/qa/metadata_check.hh>
#include <paludis/util/pstream.hh>
#include <paludis/util/system.hh>
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>

#include <libxml/tree.h>
#include <libxml/parser.h>

#if !defined(LIBXML_XPATH_ENABLED) || !defined(LIBXML_SAX1_ENABLED)
#  error "Need libxml2 built with SAX and XPath."
#endif

using namespace paludis;
using namespace paludis::qa;

MetadataCheck::MetadataCheck()
{
}

CheckResult
MetadataCheck::operator() (const FSEntry & f) const
{
    CheckResult result(f, identifier());

    if (f.basename() != "metadata.xml")
        result << Message(qal_skip, "Not a metadata.xml file");
    else if (! f.is_regular_file())
        result << Message(qal_major, "Not a regular file");
    else
    {
        FSEntry dtd(FSEntry(getenv_or_error("HOME")) / ".qualudis");
        if (! dtd.exists())
        {
            Log::get_instance()->message(ll_warning, lc_no_context, "Creating ~/.qualudis "
                    "with mode 0755");
            if (! dtd.mkdir(0755))
                throw ConfigurationError("~/.qualudis/ does not exist, please create it");
        }

        dtd /= "cache";
        if (! dtd.exists())
            if (! dtd.mkdir(0755))
                throw ConfigurationError("~/.qualudis/cache/ does not exist and cannot be created");

        dtd /= "metadata.dtd";
        if (! dtd.exists() || dtd.mtime() < (std::time(0) - (24 * 60 * 60)))
        {
            PStream wget("wget -O- http://www.gentoo.org/dtd/metadata.dtd");

            // see \ref EffSTL item 29
            std::string dtd_data((std::istreambuf_iterator<char>(wget)),
                    std::istreambuf_iterator<char>());
            if (0 != wget.exit_status())
                throw ConfigurationError("Couldn't get a new metadata.dtd");

            std::ofstream dtd_file(stringify(dtd).c_str());
            dtd_file << dtd_data;
            if (! dtd_file)
                throw ConfigurationError("Error writing to '" + stringify(dtd) +
                        "' -- you should remove this file manually before continuing");
        }

        tr1::shared_ptr<xmlParserCtxt> xml_parser_context(xmlNewParserCtxt(), &xmlFreeParserCtxt);
        tr1::shared_ptr<xmlDtd> xml_dtd(
                xmlParseDTD(0, reinterpret_cast<const xmlChar *>(stringify(dtd).c_str())), &xmlFreeDtd);

        if (! xml_dtd)
            result << Message(qal_major, "Unable to parse DTD '" + stringify(dtd) + "'");
        else
        {
            tr1::shared_ptr<xmlDoc> xml_doc(xmlCtxtReadFile(
                        xml_parser_context.get(), stringify(f).c_str(), 0, XML_PARSE_NONET), &xmlFreeDoc);
            if (! xml_doc)
                result << Message(qal_major, "Unable to parse '" + stringify(f) + "'");
            else
            {
                tr1::shared_ptr<xmlValidCtxt> xml_valid_context(xmlNewValidCtxt(), &xmlFreeValidCtxt);
                if (! xmlValidateDtd(xml_valid_context.get(), xml_doc.get(), xml_dtd.get()))
                    result << Message(qal_major, "Validation of '" + stringify(f) + "' against '"
                            + stringify(dtd) + "' failed");
            }
        }
    }

    return result;
}

const std::string &
MetadataCheck::identifier()
{
    static const std::string id("metadata");
    return id;
}

