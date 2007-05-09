/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMS_CACHE_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMS_CACHE_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/collection.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/version_requirements.hh>

#include <libwrapiter/libwrapiter.hh>

namespace paludis
{
    class FSEntry;

#include <paludis/repositories/gems/cache-sr.hh>

    class PALUDIS_VISIBLE GemsCache :
        private InstantiationPolicy<GemsCache, instantiation_method::NonCopyableTag>,
        private PrivateImplementationPattern<GemsCache>
    {
        public:
            GemsCache(const FSEntry &);
            ~GemsCache();

            typedef libwrapiter::ForwardIterator<GemsCache, const GemsCacheEntry> Iterator;
            Iterator begin() const;
            Iterator end() const;
    };
}

#endif
