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

#include "e_repository_profile_file.hh"
#include "e_repository_mask_file.hh"
#include <paludis/util/tr1_type_traits.hh>
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <paludis/config_file.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/mask.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <map>
#include <set>
#include <algorithm>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    template <typename T_>
    struct FileEntryTraits
    {
        typedef std::multiset<typename tr1::remove_const<T_>::type> Storage;
        static T_ & extract_key(const T_ & k)
        {
            return k;
        }
    };

    template <typename F_, typename S_>
    struct FileEntryTraits<const std::pair<F_, S_> >
    {
        typedef std::multimap<F_, S_> Storage;
        static const F_ & extract_key(const std::pair<F_, S_> & p)
        {
            return p.first;
        }
    };
}

namespace paludis
{
    template <typename F_>
    struct Implementation<ProfileFile<F_> >
    {
        typename FileEntryTraits<typename F_::Iterator::value_type>::Storage lines;
    };
}

template <typename F_>
void
ProfileFile<F_>::add_file(const FSEntry & f)
{
    Context context("When adding profile configuration file '" + stringify(f) + "':");

    if (! f.exists())
        return;

    F_ file(f, LineConfigFileOptions());
    for (typename F_::Iterator line(file.begin()), line_end(file.end()) ; line != line_end ; ++line)
    {
        const std::string & key(FileEntryTraits<typename F_::Iterator::value_type>::extract_key(*line));
        if (0 == key.compare(0, 1, "-", 0, 1))
        {
            int erased(this->_imp->lines.erase(key.substr(1)));
            if (0 == erased)
                Log::get_instance()->message(ll_qa, lc_context, "No match for '" + key + "'");
        }
        else
            this->_imp->lines.insert(*line);
    }
}

template <typename F_>
ProfileFile<F_>::ProfileFile() :
    PrivateImplementationPattern<ProfileFile>(new Implementation<ProfileFile<F_> >)
{
}

template <typename F_>
ProfileFile<F_>::~ProfileFile()
{
}

template <typename F_>
typename ProfileFile<F_>::Iterator
ProfileFile<F_>::begin() const
{
    return Iterator(this->_imp->lines.begin());
}

template <typename F_>
typename ProfileFile<F_>::Iterator
ProfileFile<F_>::end() const
{
    return Iterator(this->_imp->lines.end());
}

template class ProfileFile<LineConfigFile>;
template class ProfileFile<MaskFile>;

