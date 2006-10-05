/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "metadata_file.hh"
#include <paludis/qa/libxml_utils.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/save.hh>
#include <paludis/util/join.hh>
#include <paludis/util/tokeniser.hh>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <set>
#include <list>

using namespace paludis;
using namespace paludis::qa;

namespace paludis
{
    template<>
    class Implementation<MetadataFile> :
        public InternalCounted<Implementation<MetadataFile> >
    {
        private:
            std::string _email, _name;

            std::string normalise(const std::string & s)
            {
                std::list<std::string> words;
                WhitespaceTokeniser::get_instance()->tokenise(s, std::back_inserter(words));
                return join(words.begin(), words.end(), " ");
            }

        public:
            std::set<std::string> herds;
            std::set<std::pair<std::string, std::string> > maintainers;

            void handle_node(xmlDocPtr doc, xmlNode * const node)
            {
                for (xmlNode * n(node) ; n ; n = n->next)
                {
                    if (n->type == XML_ELEMENT_NODE)
                    {
                        std::string name(reinterpret_cast<const char *>(n->name));
                        if (name == "maintainer")
                        {
                            _email.clear();
                            _name.clear();
                            handle_node(doc, n->children);
                            maintainers.insert(std::make_pair(_email, _name));
                        }
                        else if (name == "herd")
                            herds.insert(normalise(reinterpret_cast<const char *>(xmlNodeListGetString(doc,
                                                n->xmlChildrenNode, 1))));
                        else if (name == "email")
                            _email = normalise(reinterpret_cast<const char *>(xmlNodeListGetString(
                                            doc, n->xmlChildrenNode, 1)));
                        else if (name == "name")
                            _name = normalise(reinterpret_cast<const char *>(xmlNodeListGetString(
                                            doc, n->xmlChildrenNode, 1)));
                        else
                            handle_node(doc, n->children);
                    }
                    else
                        handle_node(doc, n->children);
                }
            }
    };
}

MetadataFile::MetadataFile(const FSEntry & f) :
    PrivateImplementationPattern<MetadataFile>(new Implementation<MetadataFile>)
{
    LibXmlPtrHolder<xmlDocPtr> xml_doc(xmlReadFile(stringify(f).c_str(), 0, 0), &xmlFreeDoc);
    if (! xml_doc)
    {
        Log::get_instance()->message(ll_warning, lc_no_context, "Couldn't parse xml file '" +
                stringify(f) + "'");
        return;
    }

    _imp->handle_node(xml_doc, xmlDocGetRootElement(xml_doc));
}

MetadataFile::MetadataFile(const std::string & text) :
    PrivateImplementationPattern<MetadataFile>(new Implementation<MetadataFile>)
{
    LibXmlPtrHolder<xmlDocPtr> xml_doc(xmlReadDoc(reinterpret_cast<const xmlChar *>(text.c_str()),
                "file:///var/empty/", 0, 0), &xmlFreeDoc);
    if (! xml_doc)
    {
        Log::get_instance()->message(ll_warning, lc_no_context, "Couldn't parse xml text");
        return;
    }

    _imp->handle_node(xml_doc, xmlDocGetRootElement(xml_doc));
}

MetadataFile::~MetadataFile()
{
}

MetadataFile::HerdsIterator
MetadataFile::begin_herds() const
{
    return HerdsIterator(_imp->herds.begin());
}

MetadataFile::HerdsIterator
MetadataFile::end_herds() const
{
    return HerdsIterator(_imp->herds.end());
}

MetadataFile::MaintainersIterator
MetadataFile::begin_maintainers() const
{
    return MaintainersIterator(_imp->maintainers.begin());
}

MetadataFile::MaintainersIterator
MetadataFile::end_maintainers() const
{
    return MaintainersIterator(_imp->maintainers.end());
}

