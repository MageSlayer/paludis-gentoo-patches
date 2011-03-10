/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_SPEC_ANNOTATIONS_HH
#define PALUDIS_GUARD_PALUDIS_DEP_SPEC_ANNOTATIONS_HH 1

#include <paludis/dep_spec_annotations-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_key> key;
        typedef Name<struct name_kind> kind;
        typedef Name<struct name_role> role;
        typedef Name<struct name_value> value;
    }

    struct DepSpecAnnotation
    {
        NamedValue<n::key, std::string> key;
        NamedValue<n::kind, DepSpecAnnotationKind> kind;
        NamedValue<n::role, DepSpecAnnotationRole> role;
        NamedValue<n::value, std::string> value;
    };

    class PALUDIS_VISIBLE DepSpecAnnotations
    {
        private:
            Pimp<DepSpecAnnotations> _imp;

        public:
            DepSpecAnnotations();
            ~DepSpecAnnotations();

            void add(const DepSpecAnnotation &);

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag, const DepSpecAnnotation> ConstIterator;

            ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
            ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));
            ConstIterator find(const std::string &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            ConstIterator find(const DepSpecAnnotationRole) const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    extern template class Pimp<DepSpecAnnotations>;
    extern template class WrappedForwardIterator<DepSpecAnnotations::ConstIteratorTag, const DepSpecAnnotation>;
}

#endif
