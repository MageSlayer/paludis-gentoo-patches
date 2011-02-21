/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_PERMITTED_CHOICE_VALUE_PARAMETER_VALUES_HH
#define PALUDIS_GUARD_PALUDIS_PERMITTED_CHOICE_VALUE_PARAMETER_VALUES_HH 1

#include <paludis/permitted_choice_value_parameter_values-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/type_list.hh>
#include <paludis/util/map-fwd.hh>
#include <memory>

namespace paludis
{
    class PALUDIS_VISIBLE PermittedChoiceValueParameterValues :
        public virtual DeclareAbstractAcceptMethods<PermittedChoiceValueParameterValues, MakeTypeList<
            PermittedChoiceValueParameterIntegerValue,
            PermittedChoiceValueParameterEnumValue
            >::Type>
    {
        public:
            virtual ~PermittedChoiceValueParameterValues();
    };

    class PALUDIS_VISIBLE PermittedChoiceValueParameterIntegerValue :
        public PermittedChoiceValueParameterValues,
        public ImplementAcceptMethods<PermittedChoiceValueParameterValues, PermittedChoiceValueParameterIntegerValue>
    {
        private:
            int _min, _max;

        public:
            PermittedChoiceValueParameterIntegerValue(const int min, const int max);

            /**
             * Minimum permitted value, or std::numeric_limits<int>::min() for
             * no minumum.
             */
            int minimum_allowed_value() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Maximum permitted value, or std::numeric_limits<int>::max() for
             * no maximum.
             */
            int maximum_allowed_value() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE PermittedChoiceValueParameterEnumValue :
        public PermittedChoiceValueParameterValues,
        public ImplementAcceptMethods<PermittedChoiceValueParameterValues, PermittedChoiceValueParameterEnumValue>
    {
        private:
            const std::shared_ptr<const Map<std::string, std::string> > _allowed;

        public:
            PermittedChoiceValueParameterEnumValue(
                    const std::shared_ptr<const Map<std::string, std::string> > &);

            /**
             * Permitted values, with descriptions.
             */
            const std::shared_ptr<const Map<std::string, std::string> > allowed_values_and_descriptions() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
