/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/unchoices_key.hh>
#include <paludis/metadata_key.hh>
#include <paludis/choice.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_named_values.hh>

using namespace paludis;

namespace
{
    class PALUDIS_VISIBLE UnChoicesKey :
        public MetadataValueKey<std::shared_ptr<const Choices> >
    {
        private:
            std::shared_ptr<Choices> _value;

        public:
            UnChoicesKey() :
                _value(std::make_shared<Choices>())
            {
                _value->add(std::make_shared<Choice>(make_named_values<ChoiceParams>(
                                    n::consider_added_or_changed() = false,
                                    n::contains_every_value() = true,
                                    n::hidden() = true,
                                    n::hide_description() = false,
                                    n::human_name() = "Choices",
                                    n::prefix() = ChoicePrefixName(""),
                                    n::raw_name() = "Choices",
                                    n::show_with_no_prefix() = true
                                    )));
            }

            ~UnChoicesKey() override
            {
            }

            const std::shared_ptr<const Choices> parse_value() const override
            {
                return _value;
            }

            const std::string human_name() const override PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return "Choices";
            }

            const std::string raw_name() const override PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return "PALUDIS_CHOICES";
            }

            MetadataKeyType type() const override PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return mkt_internal;
            }
    };
}

std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > >
paludis::unchoices_key()
{
    static std::shared_ptr<const UnChoicesKey> result(std::make_shared<UnChoicesKey>());
    return result;
}

