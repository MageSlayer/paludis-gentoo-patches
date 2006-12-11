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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMS_YAML_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMS_YAML_HH 1

#include <paludis/util/visitor.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace paludis
{
    class FSEntry;

    class YamlNode;
    class YamlScalarNode;
    class YamlSequenceNode;
    class YamlMappingNode;

    typedef VisitorTypes<YamlScalarNode *, YamlSequenceNode *, YamlMappingNode *> YamlNodeVisitorTypes;

    class YamlError :
        public ConfigurationError
    {
        public:
            YamlError(const std::string & msg) throw ();
    };

    class YamlNode :
        public virtual VisitableInterface<YamlNodeVisitorTypes>,
        private InstantiationPolicy<YamlNode, instantiation_method::NonCopyableTag>,
        public InternalCounted<YamlNode>
    {
        protected:
            YamlNode();

        public:
            virtual ~YamlNode();
    };

    class YamlScalarNode :
        public YamlNode,
        public Visitable<YamlScalarNode, YamlNodeVisitorTypes>
    {
        private:
            std::string _value;
            std::string _tag;

        public:
            typedef CountedPtr<const YamlScalarNode, count_policy::InternalCountTag> ConstPointer;
            typedef CountedPtr<YamlScalarNode, count_policy::InternalCountTag> Pointer;

            YamlScalarNode(const std::string &, const std::string &);

            std::string value() const
            {
                return _value;
            }

            std::string tag() const
            {
                return _tag;
            }
    };

    class YamlMappingNode :
        public YamlNode,
        public Visitable<YamlMappingNode, YamlNodeVisitorTypes>,
        private PrivateImplementationPattern<YamlMappingNode>
    {
        public:
            typedef CountedPtr<const YamlMappingNode, count_policy::InternalCountTag> ConstPointer;
            typedef CountedPtr<YamlMappingNode, count_policy::InternalCountTag> Pointer;

            YamlMappingNode();

            void add(YamlScalarNode::Pointer, YamlNode::Pointer);

            typedef libwrapiter::ForwardIterator<YamlMappingNode,
                    const std::pair<YamlScalarNode::Pointer, YamlNode::Pointer> > Iterator;
            Iterator begin() const;
            Iterator end() const;

            std::pair<YamlScalarNode::Pointer, YamlNode::Pointer> & back();

            bool empty() const
            {
                return begin() == end();
            }
    };

    class YamlSequenceNode :
        public YamlNode,
        public Visitable<YamlSequenceNode, YamlNodeVisitorTypes>,
        private PrivateImplementationPattern<YamlSequenceNode>
    {
        public:
            typedef CountedPtr<YamlSequenceNode, count_policy::InternalCountTag> Pointer;
            typedef CountedPtr<const YamlSequenceNode, count_policy::InternalCountTag> ConstPointer;

            YamlSequenceNode();

            typedef libwrapiter::ForwardIterator<YamlSequenceNode, const YamlNode::Pointer> Iterator;
            Iterator begin() const;
            Iterator end() const;

            void add(YamlNode::Pointer);
    };

    class YamlDocument :
        private PrivateImplementationPattern<YamlDocument>,
        private InstantiationPolicy<YamlDocument, instantiation_method::NonCopyableTag>
    {
        public:
            YamlDocument(const std::string &);
            YamlDocument(const FSEntry &);
            ~YamlDocument();

            YamlNode::ConstPointer top() const;
    };
}

#endif
