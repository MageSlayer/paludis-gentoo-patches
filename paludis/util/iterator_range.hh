/* vim: set sw=4 sts=4 et foldmethod=syntax : */
/*
 * Copyright (c) 2014 Saleem Abdulrasool
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_ITERATOR_RANGE_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_ITERATOR_RANGE_HH 1

#include <iterator>
#include <utility>

namespace paludis
{
    template <typename Iterator_>
    class IteratorRange
    {
        private:
            Iterator_ _begin, _end;

        public:
            using const_iterator = Iterator_;
            using value_type = typename Iterator_::value_type;

            // TODO(compnerd) use SFINAE to ensure that the container's
            // iterators match the range's iterator
            template <typename Container_>
            IteratorRange(Container_ && container)
                : _begin(std::begin(container)), _end(std::end(container))
            {
            }

            IteratorRange(Iterator_ begin, Iterator_ end)
                : _begin(std::move(begin)), _end(std::move(end))
            {
            }

            Iterator_ begin() const
            {
                return _begin;
            }

            Iterator_ end() const
            {
                return _end;
            }

            size_t size() const
            {
                return std::distance(_end, _begin);
            }
    };

    template <typename Iterator_>
    IteratorRange<Iterator_> make_range(Iterator_ begin, Iterator_ end)
    {
        return IteratorRange<Iterator_>(std::move(begin), std::move(end));
    }

    template <typename Iterator_>
    IteratorRange<Iterator_> make_range(std::pair<Iterator_, Iterator_> range)
    {
        return IteratorRange<Iterator_>(std::move(range.first),
                                        std::move(range.second));
    }
}

#endif

