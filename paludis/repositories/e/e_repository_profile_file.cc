/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/mask.hh>
#include <list>
#include <set>
#include <algorithm>
#include <tr1/type_traits>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    template <typename T_>
    struct FileEntryTraits;

    template<>
    struct FileEntryTraits<std::string>
    {
        static const std::string extract_key(const std::string & k)
        {
            return k;
        }
    };

    template <typename F_, typename S_>
    struct FileEntryTraits<std::pair<F_, S_> >
    {
        static const std::string extract_key(const std::pair<F_, S_> & p)
        {
            return p.first;
        }
    };

    template <typename T_>
    struct MatchesKey
    {
        const T_ & _x;

        explicit MatchesKey(const T_ & x) :
            _x(x)
        {
        }

        template <typename U_>
        bool operator() (const U_ & y)
        {
            return FileEntryTraits<U_>::extract_key(y) == _x;
        }
    };
}

namespace paludis
{
    template <typename F_>
    struct Implementation<ProfileFile<F_> >
    {
        typedef std::list<typename std::tr1::remove_const<typename std::tr1::remove_reference<
            typename F_::ConstIterator::value_type>::type>::type> Lines;
        Lines lines;

        std::set<std::string> removed;
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
    for (typename F_::ConstIterator line(file.begin()), line_end(file.end()) ; line != line_end ; ++line)
    {
        const std::string & key(FileEntryTraits<typename std::tr1::remove_const<typename std::tr1::remove_reference<
                typename F_::ConstIterator::value_type>::type>::type>::extract_key(*line));
        if (0 == key.compare(0, 1, "-", 0, 1))
        {
            typename Implementation<ProfileFile>::Lines::iterator i(
                std::find_if(this->_imp->lines.begin(), this->_imp->lines.end(),
                             MatchesKey<std::string>(key.substr(1))));
            if (this->_imp->lines.end() == i)
            {
                /* annoying: Gentoo profiles like to remove the same mask entry
                 * more than once, especially when a particular subprofile
                 * section is inherited more than once. Don't warn when this
                 * happens. */
                if (this->_imp->removed.end() == this->_imp->removed.find(key.substr(1)))
                    Log::get_instance()->message("e.profile.no_match", ll_qa, lc_context)
                        << "No match for '" << key << "'. This usually indicates a bug in your profile.";
            }
            else
            {
                this->_imp->removed.insert(key.substr(1));
                while (this->_imp->lines.end() != i)
                {
                    this->_imp->lines.erase(i++);
                    i = std::find_if(i, this->_imp->lines.end(),
                                     MatchesKey<std::string>(key.substr(1)));
                }
            }
        }
        else
            this->_imp->lines.push_back(*line);
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
typename ProfileFile<F_>::ConstIterator
ProfileFile<F_>::begin() const
{
    return ConstIterator(this->_imp->lines.begin());
}

template <typename F_>
typename ProfileFile<F_>::ConstIterator
ProfileFile<F_>::end() const
{
    return ConstIterator(this->_imp->lines.end());
}

template class ProfileFile<LineConfigFile>;
template class WrappedForwardIterator<ProfileFile<LineConfigFile>::ConstIteratorTag, LineConfigFile::ConstIterator::value_type>;

template class ProfileFile<MaskFile>;
template class WrappedForwardIterator<ProfileFile<MaskFile>::ConstIteratorTag, MaskFile::ConstIterator::value_type>;

