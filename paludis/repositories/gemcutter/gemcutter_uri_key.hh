/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMCUTTER_GEMCUTTER_URI_KEY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMCUTTER_GEMCUTTER_URI_KEY_HH 1

#include <paludis/metadata_key.hh>
#include <paludis/util/pimp.hh>

namespace paludis
{
    namespace gemcutter_repository
    {
        class GemcutterURIKey :
            public MetadataSpecTreeKey<SimpleURISpecTree>,
            private Pimp<GemcutterURIKey>
        {
            private:
                Pimp<GemcutterURIKey>::ImpPtr & _imp;

            public:
                GemcutterURIKey(
                        const std::string * const,
                        const std::string * const,
                        const MetadataKeyType,
                        const std::string &);

                ~GemcutterURIKey();

                ///\}

                virtual const std::shared_ptr<const SimpleURISpecTree> value() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string pretty_print_value(
                        const PrettyPrinter &,
                        const PrettyPrintOptions &) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
