/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_NAME_FWD_HH
#define PALUDIS_GUARD_PALUDIS_NAME_FWD_HH 1

#include <paludis/util/wrapped_value-fwd.hh>
#include <paludis/util/set-fwd.hh>
#include <paludis/util/sequence-fwd.hh>
#include <paludis/util/attributes.hh>
#include <iosfwd>

/** \file
 * Forward declarations for paludis/name.hh .
 *
 * \ingroup g_names
 */

namespace paludis
{
    class PackageNamePartTag;
    class PackageNamePartError;
    template <> struct WrappedValueTraits<PackageNamePartTag>;

    /**
     * A PackageNamePart holds a std::string that is a valid name for the
     * package part of a QualifiedPackageName.
     *
     * \ingroup g_names
     */
    typedef WrappedValue<PackageNamePartTag> PackageNamePart;

    typedef Set<PackageNamePart> PackageNamePartSet;

    class CategoryNamePartTag;
    class CategoryNamePartError;
    template <> struct WrappedValueTraits<CategoryNamePartTag>;

    /**
     * A CategoryNamePart holds a std::string that is a valid name for the
     * category part of a QualifiedPackageName.
     *
     * \ingroup g_names
     */
    typedef WrappedValue<CategoryNamePartTag> CategoryNamePart;

    typedef Set<CategoryNamePart> CategoryNamePartSet;

    class QualifiedPackageName;

    /**
     * Output a QualifiedPackageName to a stream.
     *
     * \ingroup g_names
     */
    std::ostream & operator<< (std::ostream &, const QualifiedPackageName &) PALUDIS_VISIBLE;

    /**
     * A CategoryNamePart plus a PackageNamePart is a QualifiedPackageName.
     *
     * \ingroup g_names
     */
    inline const QualifiedPackageName
    operator+ (const CategoryNamePart & c, const PackageNamePart & p) PALUDIS_ATTRIBUTE((warn_unused_result));

    class SlotNameTag;
    class SlotNameError;
    template <> struct WrappedValueTraits<SlotNameTag>;

    typedef Set<QualifiedPackageName> QualifiedPackageNameSet;

    /**
     * A SlotName holds a std::string that is a valid name for a SLOT.
     *
     * \ingroup g_names
     */
    typedef WrappedValue<SlotNameTag> SlotName;

    class RepositoryNameTag;
    class RepositoryNameError;
    template <> struct WrappedValueTraits<RepositoryNameTag>;

    /**
     * A RepositoryName holds a std::string that is a valid name for a
     * Repository.
     *
     * \ingroup g_names
     */
    typedef WrappedValue<RepositoryNameTag> RepositoryName;

    typedef Set<RepositoryName> RepositoryNameSet;

    class KeywordNameTag;
    class KeywordNameError;
    template <> struct WrappedValueTraits<KeywordNameTag>;

    /**
     * A KeywordName holds a std::string that is a valid name for a KEYWORD.
     *
     * \ingroup g_names
     */
    typedef WrappedValue<KeywordNameTag> KeywordName;

    typedef Set<KeywordName> KeywordNameSet;

    class SetNameTag;
    class SetNameError;
    template <> struct WrappedValueTraits<SetNameTag>;

    /**
     * A SetName holds a std::string that is a valid name for a set.
     *
     * \ingroup g_names
     */
    typedef WrappedValue<SetNameTag> SetName;

    typedef Set<SetName> SetNameSet;
}

#endif
