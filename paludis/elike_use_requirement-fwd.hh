/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_ELIKE_USE_REQUIREMENT_FWD_HH
#define PALUDIS_GUARD_PALUDIS_ELIKE_USE_REQUIREMENT_FWD_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/options-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/additional_package_dep_spec_requirement-fwd.hh>
#include <iosfwd>
#include <string>
#include <memory>

namespace paludis
{
    class ELikeUseRequirementError;

#include <paludis/elike_use_requirement-se.hh>

    typedef Options<ELikeUseRequirementOption> ELikeUseRequirementOptions;

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> parse_elike_use_requirement(
            const std::string &, const ELikeUseRequirementOptions &)
        PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> make_elike_presumed_choices_requirement()
        PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;
}

#endif
