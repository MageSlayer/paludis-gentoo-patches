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

#ifndef PALUDIS_GUARD_PALUDIS_METADATA_KEY_RAW_PRINTER_HH
#define PALUDIS_GUARD_PALUDIS_METADATA_KEY_RAW_PRINTER_HH 1

#include <paludis/metadata_key_raw_printer-fwd.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace paludis
{
    class MetadataKeyRawPrinter :
        public ConstVisitor<MetadataKeyVisitorTypes>,
        private PrivateImplementationPattern<MetadataKeyRawPrinter>
    {
        friend std::ostream & operator<< (std::ostream &, const MetadataKeyRawPrinter &);

        public:
            MetadataKeyRawPrinter();
            ~MetadataKeyRawPrinter();

            void visit(const MetadataCollectionKey<IUseFlagCollection> &);
            void visit(const MetadataCollectionKey<InheritedCollection> &);
            void visit(const MetadataCollectionKey<UseFlagNameCollection> &);
            void visit(const MetadataCollectionKey<KeywordNameCollection> &);
            void visit(const MetadataSpecTreeKey<DependencySpecTree> &);
            void visit(const MetadataSpecTreeKey<URISpecTree> &);
            void visit(const MetadataSpecTreeKey<LicenseSpecTree> &);
            void visit(const MetadataSpecTreeKey<ProvideSpecTree> &);
            void visit(const MetadataSpecTreeKey<RestrictSpecTree> &);
            void visit(const MetadataPackageIDKey &);
            void visit(const MetadataStringKey &);
    };
}

#endif
