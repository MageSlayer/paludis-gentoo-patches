/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010 Ciaran McCreesh
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

#include <paludis/repositories/gems/extra_distribution_data.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/distribution-impl.hh>

using namespace paludis;
using namespace paludis::gems;

namespace paludis
{
    template <>
    struct ExtraDistributionDataData<GemsDistribution>
    {
        static std::string config_file_name()
        {
            return "gems.conf";
        }

        static std::shared_ptr<GemsDistribution> make_data(const std::shared_ptr<const KeyValueConfigFile> & k)
        {
            return make_shared_ptr(new GemsDistribution(make_named_values<GemsDistribution>(
                            n::default_buildroot() = k->get("default_buildroot")
                            )));
        }
    };
}

template class ExtraDistributionData<GemsDistribution>;
template class Singleton<ExtraDistributionData<GemsDistribution>>;

