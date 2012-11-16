/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2011 Ciaran McCreesh
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

#include <paludis/elike_slot_requirement.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/visitor_cast.hh>

using namespace paludis;

ELikeSlotExactFullRequirement::ELikeSlotExactFullRequirement(const std::pair<SlotName, SlotName> & s, const std::shared_ptr<const SlotRequirement> & o) :
    _s(s),
    _maybe_original_requirement_if_rewritten(o)
{
}

const std::string
ELikeSlotExactFullRequirement::as_string() const
{
    if (_maybe_original_requirement_if_rewritten)
    {
        if (visitor_cast<const SlotUnknownRewrittenRequirement>(*_maybe_original_requirement_if_rewritten))
            return ":" + stringify(_s.first) + "/" + stringify(_s.second) + "=";
        else
            return _maybe_original_requirement_if_rewritten->as_string() + stringify(_s.first) + "/" + stringify(_s.second);
    }
    else
        return ":" + stringify(_s.first) + "/" + stringify(_s.second);
}

const std::shared_ptr<const SlotRequirement>
ELikeSlotExactFullRequirement::maybe_original_requirement_if_rewritten() const
{
    return _maybe_original_requirement_if_rewritten;
}

ELikeSlotExactPartialRequirement::ELikeSlotExactPartialRequirement(const SlotName & s, const std::shared_ptr<const SlotRequirement> & o) :
    _s(s),
    _maybe_original_requirement_if_rewritten(o)
{
}

const std::pair<SlotName, SlotName>
ELikeSlotExactFullRequirement::slots() const
{
    return _s;
}

const std::string
ELikeSlotExactPartialRequirement::as_string() const
{
    if (_maybe_original_requirement_if_rewritten)
        return _maybe_original_requirement_if_rewritten->as_string() + stringify(_s);
    else
        return ":" + stringify(_s);
}

const std::shared_ptr<const SlotRequirement>
ELikeSlotExactPartialRequirement::maybe_original_requirement_if_rewritten() const
{
    return _maybe_original_requirement_if_rewritten;
}

const SlotName
ELikeSlotExactPartialRequirement::slot() const
{
    return _s;
}

const std::string
ELikeSlotAnyUnlockedRequirement::as_string() const
{
    return ":*";
}

const std::shared_ptr<const SlotRequirement>
ELikeSlotAnyUnlockedRequirement::maybe_original_requirement_if_rewritten() const
{
    return make_null_shared_ptr();
}

const std::string
ELikeSlotAnyAtAllLockedRequirement::as_string() const
{
    return ":=";
}

const std::shared_ptr<const SlotRequirement>
ELikeSlotAnyAtAllLockedRequirement::maybe_original_requirement_if_rewritten() const
{
    return make_null_shared_ptr();
}

ELikeSlotAnyPartialLockedRequirement::ELikeSlotAnyPartialLockedRequirement(const SlotName & s) :
    _s(s)
{
}

const std::string
ELikeSlotAnyPartialLockedRequirement::as_string() const
{
    return ":" + stringify(_s) + "=";
}

const std::shared_ptr<const SlotRequirement>
ELikeSlotAnyPartialLockedRequirement::maybe_original_requirement_if_rewritten() const
{
    return make_null_shared_ptr();
}

const SlotName
ELikeSlotAnyPartialLockedRequirement::slot() const
{
    return _s;
}

ELikeSlotUnknownRewrittenRequirement::ELikeSlotUnknownRewrittenRequirement(const SlotName & s) :
    _s(s)
{
}

const std::string
ELikeSlotUnknownRewrittenRequirement::as_string() const
{
    throw InternalError(PALUDIS_HERE, "Should not be stringifying ELikeUnknownRewrittenSlotRequirement");
}

const std::shared_ptr<const SlotRequirement>
ELikeSlotUnknownRewrittenRequirement::maybe_original_requirement_if_rewritten() const
{
    return make_null_shared_ptr();
}

const SlotName
ELikeSlotUnknownRewrittenRequirement::slot() const
{
    return _s;
}

