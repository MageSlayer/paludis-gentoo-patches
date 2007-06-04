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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMS_GEM_SPECIFICATION_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMS_GEM_SPECIFICATION_HH 1

#include <paludis/repositories/gems/gem_specification-fwd.hh>
#include <paludis/repositories/gems/yaml-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/tr1_functional.hh>
#include <string>

namespace paludis
{
    namespace gems
    {
        /**
         * Thrown if a bad Gem specification is encountered.
         *
         * \ingroup grpexceptions
         * \ingroup grpgemsrepository
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE BadSpecificationError :
            public Exception
        {
            public:
                ///\name Basic operations
                ///\{

                BadSpecificationError(const std::string &) throw ();

                ///\}
        };

        /**
         * Represents a Gem specification.
         *
         * \ingroup grpgemsrepository
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE GemSpecification :
            private PrivateImplementationPattern<GemSpecification>
        {
            public:
                ///\name Basic operations
                ///\{

                GemSpecification(const yaml::Node &);
                ~GemSpecification();

                ///\}

                ///\name Specification data
                ///\{

                const tr1::function<std::string ()> name;
                const tr1::function<std::string ()> version;
                const tr1::function<std::string ()> homepage;
                const tr1::function<std::string ()> rubyforge_project;
                const tr1::function<std::string ()> authors;
                const tr1::function<std::string ()> date;
                const tr1::function<std::string ()> platform;
                const tr1::function<std::string ()> summary;
                const tr1::function<std::string ()> description;

                ///\}
        };
    }
}

#endif
