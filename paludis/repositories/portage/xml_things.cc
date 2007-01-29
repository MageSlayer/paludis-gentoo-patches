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

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <paludis/repositories/portage/glsa.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/config_file.hh>
#include <paludis/libxml/libxml.hh>
#include <set>

using namespace paludis;

extern "C"
{
    std::tr1::shared_ptr<GLSA> create_glsa_from_xml_file(const std::string &);
}

namespace
{
    class Handler
    {
        private:
            std::tr1::shared_ptr<GLSA> _glsa;

        public:
            Handler() :
                _glsa(new GLSA)
            {
            }

            void handle_glsa_attrs(xmlDocPtr doc, xmlAttr * const attr)
            {
                for (xmlAttr * a(attr) ; a ; a = a->next)
                {
                    if (a->type == XML_ATTRIBUTE_NODE)
                    {
                        std::string name(retarded_libxml_string_to_string(a->name));
                        if (name == "id")
                            _glsa->set_id(normalise(retarded_libxml_string_to_string(xmlNodeListGetString(doc,
                                                a->xmlChildrenNode, 1))));
                    }
                }
            }

            void handle_package_name(xmlDocPtr doc, xmlAttr * const attr, std::string & str)
            {
                for (xmlAttr * a(attr) ; a ; a = a->next)
                {
                    if (a->type == XML_ATTRIBUTE_NODE)
                    {
                        std::string name(retarded_libxml_string_to_string(a->name));
                        if (name == "name")
                            str = normalise(retarded_libxml_string_to_string(xmlNodeListGetString(doc,
                                            a->xmlChildrenNode, 1)));
                    }
                }
            }

            void handle_package_archs(xmlDocPtr doc, xmlAttr * const attr, std::tr1::shared_ptr<GLSAPackage> pkg)
            {
                for (xmlAttr * a(attr) ; a ; a = a->next)
                {
                    if (a->type == XML_ATTRIBUTE_NODE)
                    {
                        std::string name(retarded_libxml_string_to_string(a->name));
                        if (name == "arch")
                        {
                            std::set<std::string> archs;
                            WhitespaceTokeniser::get_instance()->tokenise(retarded_libxml_string_to_string(
                                        xmlNodeListGetString(doc, a->xmlChildrenNode, 1)),
                                    std::inserter(archs, archs.end()));
                            archs.erase("*");
                            for (std::set<std::string>::const_iterator r(archs.begin()), r_end(archs.end()) ;
                                    r != r_end ; ++r)
                                pkg->add_arch(UseFlagName(*r));
                        }
                    }
                }
            }

            void handle_range_range(xmlDocPtr doc, xmlAttr * const attr, std::string & op)
            {
                for (xmlAttr * a(attr) ; a ; a = a->next)
                {
                    if (a->type == XML_ATTRIBUTE_NODE)
                    {
                        std::string name(retarded_libxml_string_to_string(a->name));
                        if (name == "range")
                            op = normalise(retarded_libxml_string_to_string(xmlNodeListGetString(doc,
                                            a->xmlChildrenNode, 1)));
                    }
                }
            }

            void handle_package_children(xmlDocPtr doc, xmlNode * const node, std::tr1::shared_ptr<GLSAPackage> pkg)
            {
                for (xmlNode * n(node) ; n ; n = n->next)
                {
                    if (n->type == XML_ELEMENT_NODE)
                    {
                        std::string name(retarded_libxml_string_to_string(n->name));
                        if (name == "unaffected" || name == "vulnerable")
                        {
                            std::string op;
                            handle_range_range(doc, n->properties, op);
                            std::string version(normalise(retarded_libxml_string_to_string(
                                            xmlNodeListGetString(doc, n->xmlChildrenNode, 1))));
                            ((*pkg).*(name == "unaffected" ? &GLSAPackage::add_unaffected : &GLSAPackage::add_vulnerable))
                                (GLSARange::create().op(op).version(version));
                        }
                        else
                            handle_node(doc, n->children);
                    }
                    else
                        handle_node(doc, n->children);
                }

            }

            void handle_node(xmlDocPtr doc, xmlNode * const node)
            {
                for (xmlNode * n(node) ; n ; n = n->next)
                {
                    if (n->type == XML_ELEMENT_NODE)
                    {
                        std::string name(retarded_libxml_string_to_string(n->name));
                        if (name == "glsa")
                        {
                            handle_glsa_attrs(doc, n->properties);
                            handle_node(doc, n->children);
                        }
                        else if (name == "title")
                            _glsa->set_title(normalise(retarded_libxml_string_to_string(xmlNodeListGetString(doc,
                                                n->xmlChildrenNode, 1))));
                        else if (name == "package")
                        {
                            std::string m;
                            handle_package_name(doc, n->properties, m);
                            std::tr1::shared_ptr<GLSAPackage> pkg(new GLSAPackage(QualifiedPackageName(m)));
                            handle_package_archs(doc, n->properties, pkg);
                            handle_package_children(doc, n->children, pkg);
                            _glsa->add_package(pkg);
                        }
                        else
                            handle_node(doc, n->children);
                    }
                    else
                        handle_node(doc, n->children);
                }

            }

            std::tr1::shared_ptr<GLSA> glsa()
            {
                return _glsa;
            }
    };
}

std::tr1::shared_ptr<GLSA>
create_glsa_from_xml_file(const std::string & filename)
{
    LibXmlPtrHolder<xmlDocPtr> xml_doc(xmlReadFile(filename.c_str(), 0, 0), &xmlFreeDoc);
    if (! xml_doc)
        throw GLSAError("Could not parse GLSA", filename);

    Handler h;
    h.handle_node(xml_doc, xmlDocGetRootElement(xml_doc));
    return h.glsa();
}

#ifndef MONOLITHIC

namespace paludis
{
    class RepositoryMaker;
}

extern "C"
{
    void register_repositories(RepositoryMaker * maker);
}

void register_repositories(RepositoryMaker *)
{
}

#endif

