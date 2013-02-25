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

#include <paludis/resolver/required_confirmations.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/serialise-impl.hh>

using namespace paludis;
using namespace paludis::resolver;

const std::shared_ptr<RequiredConfirmation>
RequiredConfirmation::deserialise(Deserialisation & d)
{
    if (d.class_name() == "DowngradeConfirmation")
        return DowngradeConfirmation::deserialise(d);
    else if (d.class_name() == "NotBestConfirmation")
        return NotBestConfirmation::deserialise(d);
    else if (d.class_name() == "BreakConfirmation")
        return BreakConfirmation::deserialise(d);
    else if (d.class_name() == "RemoveSystemPackageConfirmation")
        return RemoveSystemPackageConfirmation::deserialise(d);
    else if (d.class_name() == "MaskedConfirmation")
        return MaskedConfirmation::deserialise(d);
    else if (d.class_name() == "ChangedChoicesConfirmation")
        return ChangedChoicesConfirmation::deserialise(d);

    throw InternalError(PALUDIS_HERE,
                        "unknown class '" + stringify(d.class_name()) + "'");
}

const std::shared_ptr<DowngradeConfirmation>
DowngradeConfirmation::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "DowngradeConfirmation");
    return std::make_shared<DowngradeConfirmation>();
}

void
DowngradeConfirmation::serialise(Serialiser & s) const
{
    s.object("DowngradeConfirmation")
        ;
}

const std::shared_ptr<NotBestConfirmation>
NotBestConfirmation::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "NotBestConfirmation");
    return std::make_shared<NotBestConfirmation>();
}

void
NotBestConfirmation::serialise(Serialiser & s) const
{
    s.object("NotBestConfirmation")
        ;
}

const std::shared_ptr<BreakConfirmation>
BreakConfirmation::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "BreakConfirmation");
    return std::make_shared<BreakConfirmation>();
}

void
BreakConfirmation::serialise(Serialiser & s) const
{
    s.object("BreakConfirmation")
        ;
}

const std::shared_ptr<RemoveSystemPackageConfirmation>
RemoveSystemPackageConfirmation::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "RemoveSystemPackageConfirmation");
    return std::make_shared<RemoveSystemPackageConfirmation>();
}

void
RemoveSystemPackageConfirmation::serialise(Serialiser & s) const
{
    s.object("RemoveSystemPackageConfirmation")
        ;
}

const std::shared_ptr<MaskedConfirmation>
MaskedConfirmation::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "MaskedConfirmation");
    return std::make_shared<MaskedConfirmation>();
}

void
MaskedConfirmation::serialise(Serialiser & s) const
{
    s.object("MaskedConfirmation")
        ;
}

const std::shared_ptr<ChangedChoicesConfirmation>
ChangedChoicesConfirmation::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "ChangedChoicesConfirmation");
    return std::make_shared<ChangedChoicesConfirmation>();
}

void
ChangedChoicesConfirmation::serialise(Serialiser & s) const
{
    s.object("ChangedChoicesConfirmation")
        ;
}

namespace paludis
{
    template class Sequence<std::shared_ptr<const RequiredConfirmation> >;
    template class WrappedForwardIterator<RequiredConfirmations::ConstIteratorTag, const std::shared_ptr<const RequiredConfirmation> >;
}
