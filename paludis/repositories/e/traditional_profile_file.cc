/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/traditional_profile_file.hh>
#include <paludis/repositories/e/e_repository_exceptions.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/traditional_mask_file.hh>
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/mask.hh>
#include <list>
#include <set>
#include <algorithm>
#include <type_traits>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    template <typename T_>
    struct FileEntryTraits;

    template<>
    struct FileEntryTraits<const std::string>
    {
        static const std::string extract_key(const std::string & k)
        {
            return k;
        }
    };

    template <typename F_, typename S_>
    struct FileEntryTraits<const std::pair<F_, S_> >
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
            return FileEntryTraits<typename U_::second_type>::extract_key(y.second) == _x;
        }
    };
}

namespace paludis
{
    template <typename F_>
    struct Imp<TraditionalProfileFile<F_> >
    {
        const EAPIForFileFunction eapi_for_file;

        typedef std::list<std::pair<std::shared_ptr<const EAPI>,
                const typename std::remove_reference<typename F_::ConstIterator::value_type>::type> > Lines;
        Lines lines;

        std::set<std::string> removed;

        Imp(const EAPIForFileFunction & e) :
            eapi_for_file(e)
        {
        }
    };

    template <>
    struct WrappedForwardIteratorTraits<TraditionalProfileFile<LineConfigFile>::ConstIteratorTag>
    {
        typedef std::list<std::pair<std::shared_ptr<const EAPI>,
                const std::remove_reference<LineConfigFile::ConstIterator::value_type>::type> >::const_iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<TraditionalProfileFile<TraditionalMaskFile>::ConstIteratorTag>
    {
        typedef std::list<std::pair<std::shared_ptr<const EAPI>,
                const std::remove_reference<TraditionalMaskFile::ConstIterator::value_type>::type> >::const_iterator UnderlyingIterator;
    };

    template <typename T_>
    struct MakeFile
    {
        static std::shared_ptr<T_> make(const FSPath & p, const LineConfigFileOptions & o, const EAPI & e)
        {
            return std::make_shared<T_>(p, o, e);
        }
    };

    template <>
    struct MakeFile<LineConfigFile>
    {
        static std::shared_ptr<LineConfigFile> make(const FSPath & p, const LineConfigFileOptions & o, const EAPI &)
        {
            return std::make_shared<LineConfigFile>(p, o);
        }
    };
}

template <typename F_>
void
TraditionalProfileFile<F_>::add_file(const FSPath & f)
{
    Context context("When adding profile configuration file '" + stringify(f) + "':");

    if (! f.stat().exists())
        return;

    const std::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string(_imp->eapi_for_file(f)));
    if (! eapi->supported())
        throw ERepositoryConfigurationError("Can't use profile file '" + stringify(f) +
                "' because it uses an unsupported EAPI");

    std::shared_ptr<F_> file(MakeFile<F_>::make(f, { lcfo_disallow_continuations }, *eapi));
    for (typename F_::ConstIterator line(file->begin()), line_end(file->end()) ; line != line_end ; ++line)
    {
        const std::string key(FileEntryTraits<const typename std::remove_reference<typename F_::ConstIterator::value_type>::type>::extract_key(*line));
        if (0 == key.compare(0, 1, "-", 0, 1))
        {
            typename Imp<TraditionalProfileFile>::Lines::iterator i(
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
            this->_imp->lines.push_back(std::make_pair(eapi, *line));
    }
}

template <typename F_>
TraditionalProfileFile<F_>::TraditionalProfileFile(const EAPIForFileFunction & f) :
    _imp(f)
{
}

template <typename F_>
TraditionalProfileFile<F_>::~TraditionalProfileFile()
{
}

template <typename F_>
typename TraditionalProfileFile<F_>::ConstIterator
TraditionalProfileFile<F_>::begin() const
{
    return ConstIterator(this->_imp->lines.begin());
}

template <typename F_>
typename TraditionalProfileFile<F_>::ConstIterator
TraditionalProfileFile<F_>::end() const
{
    return ConstIterator(this->_imp->lines.end());
}

template class TraditionalProfileFile<LineConfigFile>;
template class WrappedForwardIterator<TraditionalProfileFile<LineConfigFile>::ConstIteratorTag, const std::pair<
    std::shared_ptr<const EAPI>,
    const std::remove_reference<LineConfigFile::ConstIterator::value_type>::type> >;

template class TraditionalProfileFile<TraditionalMaskFile>;
template class WrappedForwardIterator<TraditionalProfileFile<TraditionalMaskFile>::ConstIteratorTag, const std::pair<
    std::shared_ptr<const EAPI>,
    const std::remove_reference<TraditionalMaskFile::ConstIterator::value_type>::type> >;

