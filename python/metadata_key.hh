/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
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

#ifndef PYTHON_GUARD_PYTHON_METADATA_KEY_HH
#define PYTHON_GUARD_PYTHON_METADATA_KEY_HH 1

#include <python/paludis_python.hh>

#include <paludis/metadata_key.hh>

namespace paludis
{
    namespace python
    {
        struct PALUDIS_VISIBLE MetadataKeyToPython :
            ConstVisitor<MetadataKeyVisitorTypes>
        {
            boost::python::object value;

            void visit(const MetadataPackageIDKey & k);
            void visit(const MetadataStringKey & k);
            void visit(const MetadataTimeKey & k);
            void visit(const MetadataContentsKey & k);
            void visit(const MetadataRepositoryMaskInfoKey & k);
            void visit(const MetadataSetKey<KeywordNameSet> & k);
            void visit(const MetadataSetKey<UseFlagNameSet> & k);
            void visit(const MetadataSetKey<IUseFlagSet> & k);
            void visit(const MetadataSetKey<InheritedSet> & k);
            void visit(const MetadataSpecTreeKey<LicenseSpecTree> & k);
            void visit(const MetadataSpecTreeKey<ProvideSpecTree> & k);
            void visit(const MetadataSpecTreeKey<DependencySpecTree> & k);
            void visit(const MetadataSpecTreeKey<RestrictSpecTree> & k);
            void visit(const MetadataSpecTreeKey<URISpecTree> & k);
            void visit(const MetadataSetKey<PackageIDSequence> & k);
        };
    }
}

#endif
