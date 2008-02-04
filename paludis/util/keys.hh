/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_KEYS_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_KEYS_HH 1

#include <paludis/util/kc-fwd.hh>

namespace paludis
{
    namespace k
    {
        typedef kc::Key<1> fetch_unneeded;
        typedef kc::Key<2> safe_resume;
        typedef kc::Key<3> no_config_protect;
        typedef kc::Key<4> debug_build;
        typedef kc::Key<5> checks;
        typedef kc::Key<6> destination;
        typedef kc::Key<7> visible;
        typedef kc::Key<8> target_file;
        typedef kc::Key<9> requires_manual_fetching;
        typedef kc::Key<10> failed_automatic_fetching;
        typedef kc::Key<11> failed_integrity_checks;
        typedef kc::Key<12> default_environment;
        typedef kc::Key<13> fallback_environment;
        typedef kc::Key<14> support_old_style_virtuals;
        typedef kc::Key<15> default_ebuild_distdir;
        typedef kc::Key<16> default_ebuild_write_cache;
        typedef kc::Key<17> default_ebuild_names_cache;
        typedef kc::Key<18> default_ebuild_builddir;
        typedef kc::Key<19> default_ebuild_layout;
        typedef kc::Key<20> default_ebuild_eapi_when_unknown;
        typedef kc::Key<21> default_ebuild_eapi_when_unspecified;
        typedef kc::Key<22> default_ebuild_profile_eapi;
        typedef kc::Key<23> default_vdb_provides_cache;
        typedef kc::Key<24> default_vdb_names_cache;
        typedef kc::Key<25> paludis_environment_use_conf_filename;
        typedef kc::Key<26> paludis_environment_keywords_conf_filename;
        typedef kc::Key<27> concept_use;
        typedef kc::Key<28> concept_keyword;
        typedef kc::Key<29> paludis_package;
        typedef kc::Key<30> kind;
        typedef kc::Key<31> value;
        typedef kc::Key<32> mask_file;
        typedef kc::Key<33> comment;
    }
}

#endif
