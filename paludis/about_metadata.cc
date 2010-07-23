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

#include <paludis/about_metadata.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/about.hh>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<AboutMetadata>
    {
        std::shared_ptr<LiteralMetadataValueKey<std::string> > package_key;
        std::shared_ptr<LiteralMetadataValueKey<std::string> > version_key;

        std::shared_ptr<LiteralMetadataValueKey<std::string> > build_cxxflags_key;
        std::shared_ptr<LiteralMetadataValueKey<std::string> > build_ldflags_key;
        std::shared_ptr<LiteralMetadataValueKey<std::string> > build_cxx_key;
        std::shared_ptr<LiteralMetadataValueKey<std::string> > build_date_key;

        Imp() :
            package_key(new LiteralMetadataValueKey<std::string>("PALUDIS_PACKAGE", "Package Name", mkt_significant, PALUDIS_PACKAGE)),
            version_key(new LiteralMetadataValueKey<std::string>("PALUDIS_VERSION", "Package Version", mkt_significant,
                        stringify(PALUDIS_VERSION_MAJOR) + "." + stringify(PALUDIS_VERSION_MINOR) + "." +
                        stringify(PALUDIS_VERSION_MICRO) + PALUDIS_VERSION_SUFFIX + (std::string(PALUDIS_GIT_HEAD).empty() ? "" :
                            " " + stringify(PALUDIS_GIT_HEAD)))),
            build_cxxflags_key(new LiteralMetadataValueKey<std::string>("PALUDIS_BUILD_CXXFLAGS", "Built with CXXFLAGS", mkt_normal,
                        PALUDIS_BUILD_CXXFLAGS)),
            build_ldflags_key(new LiteralMetadataValueKey<std::string>("PALUDIS_BUILD_LDFLAGS", "Built with LDFLAGS", mkt_normal,
                        PALUDIS_BUILD_LDFLAGS)),
            build_cxx_key(new LiteralMetadataValueKey<std::string>("PALUDIS_BUILD_CXX", "Built with CXX", mkt_normal,
                        stringify(PALUDIS_BUILD_CXX)
#ifdef __VERSION__
                        + " " + stringify(__VERSION__)
#endif
                        )),
            build_date_key(new LiteralMetadataValueKey<std::string>("PALUDIS_BUILD_DATE", "Build Date", mkt_normal,
                        PALUDIS_BUILD_DATE))
        {
        }
    };
}

AboutMetadata::AboutMetadata() :
    Pimp<AboutMetadata>(),
    _imp(Pimp<AboutMetadata>::_imp)
{
    add_metadata_key(_imp->package_key);
    add_metadata_key(_imp->version_key);
    add_metadata_key(_imp->build_cxxflags_key);
    add_metadata_key(_imp->build_ldflags_key);
    add_metadata_key(_imp->build_cxx_key);
    add_metadata_key(_imp->build_date_key);
}

AboutMetadata::~AboutMetadata()
{
}

void
AboutMetadata::need_keys_added() const
{
}

template class Pimp<AboutMetadata>;
template class Singleton<AboutMetadata>;

