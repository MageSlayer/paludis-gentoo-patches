/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
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

#include <paludis/host_tuple_name.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/operators.hh>
#include <paludis/util/tokeniser.hh>
#include <vector>
#include <ostream>

/** \file
 * Implementation of host_tuple_name.hh things.
 *
 * \ingroup grpnames
 */

using namespace paludis;

#include <paludis/host_tuple_name-sr.cc>

HostTupleNameError::HostTupleNameError(const std::string & name) throw () :
    NameError(name, "host tuple name")
{
}

HostTupleNameError::HostTupleNameError(const std::string & name,
        const std::string & type) throw () :
    NameError(name, type)
{
}

HostTupleName::HostTupleName(const std::string & s) :
    architecture("architecture"),
    manufacturer("manufacturer"),
    kernel("kernel"),
    userland("userland")
{
    Context c("When creating a HostTupleName from '" + s + "':");

    Tokeniser<delim_kind::AnyOfTag, delim_mode::DelimiterTag> tokeniser("-");
    std::vector<std::string> tokens;

    tokeniser.tokenise(s, std::back_inserter(tokens));
    switch (tokens.size())
    {
        case 2: // Type 'arch'-'userland', i.e. 'spu-elf'.
            architecture = ArchitectureNamePart(tokens[0]);
            manufacturer = ManufacturerNamePart("");
            kernel = KernelNamePart("");
            userland = UserlandNamePart(tokens[1]);

            break;

        case 3: /*
            * Type 'arch'-'manufacturer'-'os', i.e. 'i386-unknown-freebsd6.0'.
            * Let's duplicate the 'os' field into kernel and userland.
            */
            architecture = ArchitectureNamePart(tokens[0]);
            manufacturer = ManufacturerNamePart(tokens[1]);
            kernel = KernelNamePart(tokens[2]);
            userland = UserlandNamePart(tokens[2]);

            break;

        case 4: // Type full
            architecture = ArchitectureNamePart(tokens[0]);
            manufacturer = ManufacturerNamePart(tokens[1]);
            kernel = KernelNamePart(tokens[2]);
            userland = UserlandNamePart(tokens[3]);

            break;

        default:
            throw HostTupleNameError(s);
    }
}

std::ostream &
paludis::operator<< (std::ostream & s, const HostTupleName & c)
{
    s << c.architecture;
    if (! c.manufacturer.data().empty())
        s << "-" << c.manufacturer;
    if ((! c.kernel.data().empty()) && (c.kernel.data() != c.userland.data()))
        s << "-" << c.kernel;
    s << "-" << c.userland;
    return s;
}

void
ArchitectureNamePartValidator::validate(const std::string & s)
{
    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789_.");
    // '.' is included, as 'HPPA1.x' is a valid ArchitectureNamePart.

    do
    {
        if (s.empty())
            break;

        if ('-' == s.at(0))
            break;

        if (std::string::npos != s.find_first_not_of(allowed_chars))
            break;

        return;

    } while (false);

    Context c("When validating architecture name '" + s + "':");

    throw ArchitectureNamePartError(s);
}

ArchitectureNamePartError::ArchitectureNamePartError(const std::string & name) throw () :
    HostTupleNameError(name, "architecture name part")
{
}

void
ManufacturerNamePartValidator::validate(const std::string & s)
{
    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789_");

    do
    {
        if (s.empty())
            return;

        if ('-' == s.at(0))
            break;

        if (std::string::npos != s.find_first_not_of(allowed_chars))
            break;

        return;

    } while (false);

    Context c("When validating manufacturer name '" + s + "':");

    throw ManufacturerNamePartError(s);
}

ManufacturerNamePartError::ManufacturerNamePartError(const std::string & name) throw () :
    HostTupleNameError(name, "manufacturer name part")
{
}

void
KernelNamePartValidator::validate(const std::string & s)
{
    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789_.");

    do
    {
        if (s.empty())
            return;

        if ('-' == s.at(0))
            break;

        if (std::string::npos != s.find_first_not_of(allowed_chars))
            break;

        return;

    } while (false);

    Context c("When validating kernel name '" + s + "':");

    throw KernelNamePartError(s);
}

KernelNamePartError::KernelNamePartError(const std::string & name) throw () :
    HostTupleNameError(name, "kernel name part")
{
}

void
UserlandNamePartValidator::validate(const std::string & s)
{
    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789_.");

    do
    {
        if (s.empty())
            break;

        if ('-' == s.at(0))
            break;

        if (std::string::npos != s.find_first_not_of(allowed_chars))
            break;

        return;

    } while (false);

    Context c("When validating manufacturer name '" + s + "':");

    throw UserlandNamePartError(s);
}

UserlandNamePartError::UserlandNamePartError(const std::string & name) throw () :
    HostTupleNameError(name, "userland name part")
{
}
