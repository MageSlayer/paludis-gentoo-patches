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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_INQUISITIO_EXTRACTOR_HH
#define PALUDIS_GUARD_SRC_CLIENTS_INQUISITIO_EXTRACTOR_HH 1

#include <string>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/virtual_constructor.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/environment.hh>

namespace inquisitio
{
    class Extractor :
        private paludis::InstantiationPolicy<Extractor, paludis::instantiation_method::NonCopyableTag>
    {
        protected:
            Extractor();

        public:
            virtual ~Extractor();

            virtual std::string operator() (const paludis::PackageDatabaseEntry &) const = 0;
    };

    class NoSuchExtractorError :
        public paludis::Exception
    {
        public:
            NoSuchExtractorError(const std::string &) throw ();
    };

    class ExtractorMaker :
        public paludis::InstantiationPolicy<ExtractorMaker, paludis::instantiation_method::SingletonTag>,
        public paludis::VirtualConstructor<
            std::string,
            paludis::tr1::shared_ptr<Extractor> (*) (const paludis::Environment &),
            paludis::virtual_constructor_not_found::ThrowException<NoSuchExtractorError> >
    {
        friend class paludis::InstantiationPolicy<ExtractorMaker, paludis::instantiation_method::SingletonTag>;

        private:
            ExtractorMaker();
    };
}

#endif
