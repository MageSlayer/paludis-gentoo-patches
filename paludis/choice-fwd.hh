/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_CHOICE_FWD_HH
#define PALUDIS_GUARD_PALUDIS_CHOICE_FWD_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/wrapped_value-fwd.hh>

/** \file
 * Forward declarations for paludis/choice.hh .
 *
 * \ingroup g_choices
 * \since 0.32
 */

namespace paludis
{
    class Choices;

    class Choice;
    class ChoiceValue;

    class ChoiceParams;

    struct ChoicePrefixNameTag;
    struct ChoiceNameWithPrefixTag;
    struct UnprefixedChoiceNameTag;

    template <> struct WrappedValueTraits<ChoicePrefixNameTag>;
    template <> struct WrappedValueTraits<ChoiceNameWithPrefixTag>;
    template <> struct WrappedValueTraits<UnprefixedChoiceNameTag>;

    /**
     * A prefix for a choice name ('' for USE, 'linguas' for LINGUAS etc).
     *
     * \ingroup g_choices
     * \since 0.32
     */
    typedef WrappedValue<ChoicePrefixNameTag> ChoicePrefixName;

    /**
     * A choice name, including prefix and delim ('nls' for USE, 'linguas_en' for LINGUAS etc).
     *
     * \ingroup g_choices
     * \since 0.32
     */
    typedef WrappedValue<ChoiceNameWithPrefixTag> ChoiceNameWithPrefix;

    /**
     * A choice name, without prefix or delim ('nls' for USE, 'en' for LINGUAS etc).
     *
     * \ingroup g_choices
     * \since 0.32
     */
    typedef WrappedValue<UnprefixedChoiceNameTag> UnprefixedChoiceName;
}

#endif
