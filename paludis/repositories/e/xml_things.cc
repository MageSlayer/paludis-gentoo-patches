/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/xml_things.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map.hh>
#include <paludis/util/map-impl.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/choice.hh>
#include <paludis/maintainer.hh>
#include <cctype>
#include <set>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

using namespace paludis;

namespace
{
    class PALUDIS_VISIBLE XMLError :
        public Exception
    {
        public:
            XMLError(const std::string & w) noexcept :
                Exception("XML error: " + w)
            {
            }
    };

    const xmlChar * stupid_libxml_string(const char * const s)
    {
        return reinterpret_cast<const xmlChar *>(s);
    }

    std::string unstupid_libxml_string(const xmlChar * const s)
    {
        return s ? std::string(reinterpret_cast<const char *>(s)) : "";
    }

    std::string fix_whitespace(const std::string & s)
    {
        std::string t;
        t.reserve(s.length());

        std::string::size_type p(0);
        std::string::size_type p_end(s.length());

        while (p != p_end && std::isspace(s[p]))
            ++p;

        while (p_end > p && std::isspace(s[p_end - 1]))
            --p_end;

        while (p != p_end)
        {
            while (p != p_end && ! std::isspace(s[p]))
                t.append(1, s[p++]);

            if (p != p_end)
                t.append(1, ' ');

            while (p != p_end && std::isspace(s[p]))
                ++p;
        }

        return t;
    }

    std::string extract_child_text(const xmlNode * const node)
    {
        switch (node->type)
        {
            case XML_TEXT_NODE:
            case XML_CDATA_SECTION_NODE:
                return unstupid_libxml_string(node->content);

            default:
                throw XMLError("Node not an XML_TEXT_NODE or XML_CDATA_SECTION_NODE");
        };
    }

    std::string extract_children_text(const std::shared_ptr<const xmlXPathObject> & object)
    {
        std::string result;
        for (int j = 0 ; j != object->nodesetval->nodeNr ; ++j)
            result.append(extract_child_text(object->nodesetval->nodeTab[j]));
        return fix_whitespace(result);
    }

    template <typename T_>
    std::shared_ptr<T_> manage_libxml_ptr(T_ * const p, void (* d) (T_ * const))
    {
        if (! p)
            throw XMLError("libxml2 returned null for " + std::string(__PRETTY_FUNCTION__));
        return std::shared_ptr<T_>(p, d);
    }

#ifdef ENABLE_XML
    class GLSAHandler
    {
        private:
            std::shared_ptr<GLSA> _glsa;

        public:
            GLSAHandler() :
                _glsa(std::make_shared<GLSA>())
            {
            }

            void handle_glsa_attrs(xmlDocPtr doc, xmlAttr * const attr)
            {
                for (xmlAttr * a(attr) ; a ; a = a->next)
                {
                    if (a->type == XML_ATTRIBUTE_NODE)
                    {
                        std::string name(unstupid_libxml_string(a->name));
                        if (name == "id")
                            _glsa->set_id(fix_whitespace(unstupid_libxml_string(xmlNodeListGetString(doc, a->xmlChildrenNode, 1))));
                    }
                }
            }

            void handle_package_name(xmlDocPtr doc, xmlAttr * const attr, std::string & str)
            {
                for (xmlAttr * a(attr) ; a ; a = a->next)
                {
                    if (a->type == XML_ATTRIBUTE_NODE)
                    {
                        std::string name(unstupid_libxml_string(a->name));
                        if (name == "name")
                            str = fix_whitespace(unstupid_libxml_string(xmlNodeListGetString(doc,
                                            a->xmlChildrenNode, 1)));
                    }
                }
            }

            void handle_package_archs(xmlDocPtr doc, xmlAttr * const attr, std::shared_ptr<GLSAPackage> pkg)
            {
                for (xmlAttr * a(attr) ; a ; a = a->next)
                {
                    if (a->type == XML_ATTRIBUTE_NODE)
                    {
                        std::string name(unstupid_libxml_string(a->name));
                        if (name == "arch")
                        {
                            std::set<std::string> archs;
                            tokenise_whitespace(unstupid_libxml_string(xmlNodeListGetString(doc, a->xmlChildrenNode, 1)),
                                    std::inserter(archs, archs.end()));
                            archs.erase("*");
                            for (const auto & arch : archs)
                                pkg->add_arch(arch);
                        }
                    }
                }
            }

            void handle_range_range(xmlDocPtr doc, xmlAttr * const attr, std::string & op, std::string & slot)
            {
                for (xmlAttr * a(attr) ; a ; a = a->next)
                {
                    if (a->type == XML_ATTRIBUTE_NODE)
                    {
                        std::string name(unstupid_libxml_string(a->name));
                        if (name == "range")
                            op = fix_whitespace(unstupid_libxml_string(xmlNodeListGetString(doc, a->xmlChildrenNode, 1)));
                        else if (name == "slot")
                            slot = fix_whitespace(unstupid_libxml_string(xmlNodeListGetString(doc, a->xmlChildrenNode, 1)));
                    }
                }
            }

            void handle_package_children(xmlDocPtr doc, xmlNode * const node, std::shared_ptr<GLSAPackage> pkg)
            {
                for (xmlNode * n(node) ; n ; n = n->next)
                {
                    if (n->type == XML_ELEMENT_NODE)
                    {
                        std::string name(unstupid_libxml_string(n->name));
                        if (name == "unaffected" || name == "vulnerable")
                        {
                            std::string op;
                            std::string slot("*");
                            handle_range_range(doc, n->properties, op, slot);
                            std::string version(fix_whitespace(unstupid_libxml_string(xmlNodeListGetString(doc, n->xmlChildrenNode, 1))));
                            ((*pkg).*(name == "unaffected" ? &GLSAPackage::add_unaffected : &GLSAPackage::add_vulnerable))(
                            make_named_values<erepository::GLSARange>(
                                n::op() = op,
                                n::slot() = slot,
                                n::version() = version
                                ));
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
                        std::string name(unstupid_libxml_string(n->name));
                        if (name == "glsa")
                        {
                            handle_glsa_attrs(doc, n->properties);
                            handle_node(doc, n->children);
                        }
                        else if (name == "title")
                            _glsa->set_title(fix_whitespace(unstupid_libxml_string(xmlNodeListGetString(doc, n->xmlChildrenNode, 1))));
                        else if (name == "package")
                        {
                            std::string m;
                            handle_package_name(doc, n->properties, m);
                            std::shared_ptr<GLSAPackage> pkg(std::make_shared<GLSAPackage>(QualifiedPackageName(m)));
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

            std::shared_ptr<GLSA> glsa()
            {
                return _glsa;
            }
    };
#endif

}

#ifdef ENABLE_XML

std::shared_ptr<GLSA>
paludis_xml_things_create_glsa_from_xml_file(const std::string & filename)
{
    try
    {
        std::shared_ptr<xmlDoc> doc(manage_libxml_ptr(xmlParseFile(filename.c_str()), &xmlFreeDoc));

        GLSAHandler h;
        h.handle_node(doc.get(), xmlDocGetRootElement(doc.get()));
        return h.glsa();
    }
    catch (const XMLError & e)
    {
        throw GLSAError(e.message(), filename);
    }
}

std::shared_ptr<erepository::MetadataXML>
paludis_xml_things_create_metadata_xml_from_xml_file(const FSPath & filename)
{
    std::shared_ptr<erepository::MetadataXML> result(std::make_shared<erepository::MetadataXML>(
                make_named_values<erepository::MetadataXML>(
                    n::herds() = std::make_shared<Sequence<std::string>>(),
                    n::long_description() = "",
                    n::maintainers() = std::make_shared<Maintainers>(),
                    n::uses() = std::make_shared<Map<ChoiceNameWithPrefix, std::string>>()
                    )));

    std::shared_ptr<xmlDoc> doc(manage_libxml_ptr(xmlParseFile(stringify(filename).c_str()), &xmlFreeDoc));

    std::shared_ptr<xmlXPathContext>
        doc_context(manage_libxml_ptr(xmlXPathNewContext(doc.get()), &xmlXPathFreeContext));
    std::shared_ptr<xmlXPathContext>
        sub_context(manage_libxml_ptr(xmlXPathNewContext(doc.get()), &xmlXPathFreeContext));
    std::shared_ptr<xmlXPathContext>
        text_context(manage_libxml_ptr(xmlXPathNewContext(doc.get()), &xmlXPathFreeContext));

    std::shared_ptr<xmlXPathObject>
        herd_object(manage_libxml_ptr(xmlXPathEvalExpression(stupid_libxml_string(
                            "//pkgmetadata/herd"), doc_context.get()), xmlXPathFreeObject));
    std::shared_ptr<xmlXPathObject>
        maintainer_object(manage_libxml_ptr(xmlXPathEvalExpression(stupid_libxml_string(
                            "//pkgmetadata/maintainer"), doc_context.get()), xmlXPathFreeObject));
    std::shared_ptr<xmlXPathObject>
        use_object(manage_libxml_ptr(xmlXPathEvalExpression(stupid_libxml_string(
                            "//pkgmetadata/use"), doc_context.get()), xmlXPathFreeObject));

    for (int i = 0 ; i != herd_object->nodesetval->nodeNr ; ++i)
    {
        text_context->node = herd_object->nodesetval->nodeTab[i];
        std::shared_ptr<xmlXPathObject> text_object(manage_libxml_ptr(
                    xmlXPathEvalExpression(stupid_libxml_string("descendant::text()"),
                        text_context.get()), xmlXPathFreeObject));

        result->herds()->push_back(extract_children_text(text_object));
    }

    for (int i = 0 ; i != maintainer_object->nodesetval->nodeNr ; ++i)
    {
        Maintainer maintainer(make_named_values<Maintainer>(
                    n::author() = "",
                    n::description() = "",
                    n::email() = ""
                    ));

        sub_context->node = maintainer_object->nodesetval->nodeTab[i];
        std::shared_ptr<xmlXPathObject>
            name_object(manage_libxml_ptr(xmlXPathEvalExpression(stupid_libxml_string("./name[position()=1]"),
                        sub_context.get()), xmlXPathFreeObject));
        std::shared_ptr<xmlXPathObject>
            email_object(manage_libxml_ptr(xmlXPathEvalExpression(stupid_libxml_string("./email[position()=1]"),
                        sub_context.get()), xmlXPathFreeObject));
        std::shared_ptr<xmlXPathObject>
            description_object(manage_libxml_ptr(xmlXPathEvalExpression(stupid_libxml_string("./description[position()=1]"),
                        sub_context.get()), xmlXPathFreeObject));

        if (name_object->nodesetval->nodeNr)
        {
            text_context->node = name_object->nodesetval->nodeTab[0];
            std::shared_ptr<xmlXPathObject> text_object(manage_libxml_ptr(
                        xmlXPathEvalExpression(stupid_libxml_string("descendant::text()"),
                            text_context.get()), xmlXPathFreeObject));
            maintainer.author() = extract_children_text(text_object);
        }

        if (email_object->nodesetval->nodeNr)
        {
            text_context->node = email_object->nodesetval->nodeTab[0];
            std::shared_ptr<xmlXPathObject> text_object(manage_libxml_ptr(
                        xmlXPathEvalExpression(stupid_libxml_string("descendant::text()"),
                            text_context.get()), xmlXPathFreeObject));
            maintainer.email() = extract_children_text(text_object);
        }

        if (description_object->nodesetval->nodeNr)
        {
            text_context->node = description_object->nodesetval->nodeTab[0];
            std::shared_ptr<xmlXPathObject> text_object(manage_libxml_ptr(
                        xmlXPathEvalExpression(stupid_libxml_string("descendant::text()"),
                            text_context.get()), xmlXPathFreeObject));
            maintainer.description() = extract_children_text(text_object);
        }

        if ((! maintainer.email().empty()) || (! maintainer.author().empty()))
            result->maintainers()->push_back(maintainer);
    }

    for (int i = 0 ; i != use_object->nodesetval->nodeNr ; ++i)
    {
        sub_context->node = use_object->nodesetval->nodeTab[i];
        std::shared_ptr<xmlXPathObject> flag_object(
                manage_libxml_ptr(xmlXPathEvalExpression(stupid_libxml_string("./flag[@name]"),
                        sub_context.get()), xmlXPathFreeObject));

        for (int k = 0 ; k != flag_object->nodesetval->nodeNr ; ++k)
        {
            text_context->node = flag_object->nodesetval->nodeTab[k];
            std::shared_ptr<xmlXPathObject>
                text_object(manage_libxml_ptr(xmlXPathEvalExpression(stupid_libxml_string("descendant::text()"),
                            text_context.get()), xmlXPathFreeObject));
            std::shared_ptr<xmlXPathObject>
                name_object(manage_libxml_ptr(xmlXPathEvalExpression(stupid_libxml_string("@name"),
                                text_context.get()), xmlXPathFreeObject));

            std::string desc(extract_children_text(text_object));
            std::string name;

            if (! name_object->nodesetval->nodeNr)
                throw XMLError("no name attribute");

            if (name_object->nodesetval->nodeTab[0]->type != XML_ATTRIBUTE_NODE)
                throw XMLError("Node not an XML_ATTRIBUTE_NODE");

            name.append(unstupid_libxml_string(xmlNodeListGetString(doc.get(),
                            name_object->nodesetval->nodeTab[0]->xmlChildrenNode, 1)));

            result->uses()->insert(ChoiceNameWithPrefix(name), desc);
        }
    }

    std::shared_ptr<xmlXPathObject> longdesc_object;
    longdesc_object = manage_libxml_ptr(xmlXPathEvalExpression(stupid_libxml_string(
                    "//pkgmetadata/longdescription[@lang=\"en\"]"), doc_context.get()), xmlXPathFreeObject);
    if (0 == longdesc_object->nodesetval || 0 == longdesc_object->nodesetval->nodeNr)
        longdesc_object = manage_libxml_ptr(xmlXPathEvalExpression(stupid_libxml_string(
                        "//pkgmetadata/longdescription[not(@lang)]"), doc_context.get()), xmlXPathFreeObject);

    if (0 != longdesc_object->nodesetval)
    {
        for (int i = 0 ; i != longdesc_object->nodesetval->nodeNr ; ++i)
        {
            text_context->node = longdesc_object->nodesetval->nodeTab[i];
            std::shared_ptr<xmlXPathObject> text_object(manage_libxml_ptr(
                        xmlXPathEvalExpression(stupid_libxml_string("descendant::text()"),
                            text_context.get()), xmlXPathFreeObject));

            result->long_description() = extract_children_text(text_object);
        }
    }

    return result;
}

#endif

void
paludis_xml_things_init()
{
    xmlInitParser();
}

void
paludis_xml_things_cleanup()
{
    xmlCleanupParser();
}

namespace paludis
{
    class RepositoryFactory;
}

