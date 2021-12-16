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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_CAVE_COLOUR_PRETTY_PRINTER_HH
#define PALUDIS_GUARD_SRC_CLIENTS_CAVE_COLOUR_PRETTY_PRINTER_HH 1

#include <paludis/formatted_pretty_printer.hh>
#include <paludis/choice-fwd.hh>

namespace paludis
{
    namespace cave
    {
        class ColourPrettyPrinter :
            public FormattedPrettyPrinter
        {
            private:
                int _initial_indent;

            protected:
                const std::string format_enabled(const std::string &) const override;
                const std::string format_disabled(const std::string &) const override;
                const std::string format_installed(const std::string &) const override;
                const std::string format_installable(const std::string &) const override;
                const std::string format_masked(const std::string &) const override;
                const std::string format_plain(const std::string &) const override;

            public:
                ColourPrettyPrinter(
                        const Environment * const env,
                        const std::shared_ptr<const PackageID> & id,
                        const int initial_indent);

                const std::string indentify(const int) const override;
                const std::string newline() const override;

                const std::string prettify_choice_value_forced(const std::shared_ptr<const ChoiceValue> &) const;
                const std::string prettify_choice_value_enabled(const std::shared_ptr<const ChoiceValue> &) const;
                const std::string prettify_choice_value_masked(const std::shared_ptr<const ChoiceValue> &) const;
                const std::string prettify_choice_value_disabled(const std::shared_ptr<const ChoiceValue> &) const;
        };
    }
}


#endif
