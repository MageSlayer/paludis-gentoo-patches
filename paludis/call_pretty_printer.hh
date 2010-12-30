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

#ifndef PALUDIS_GUARD_PALUDIS_CALL_PRETTY_PRINTER_HH
#define PALUDIS_GUARD_PALUDIS_CALL_PRETTY_PRINTER_HH 1

#include <paludis/call_pretty_printer-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/pretty_printer.hh>

namespace paludis
{
    class PALUDIS_VISIBLE CallPrettyPrinter
    {
        private:
            const PrettyPrinter & _pretty_printer;

        public:
            CallPrettyPrinter(const PrettyPrinter & p) :
                _pretty_printer(p)
            {
            }

            CallPrettyPrinter(const CallPrettyPrinter &) = default;

            template <typename T_>
            const std::string operator() (const T_ & v) const
            {
                return _pretty_printer.prettify(v);
            }
    };
}

#endif
