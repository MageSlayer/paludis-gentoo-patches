/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011, 2012 David Leverton
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_DIGEST_REGISTRY_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_DIGEST_REGISTRY_HH 1

#include <iosfwd>
#include <string>
#include <paludis/util/attributes.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/singleton.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <functional>
#include <utility>

namespace paludis
{
    class DigestRegistry;

    extern template class Pimp<DigestRegistry>;
    extern template class PALUDIS_VISIBLE Singleton<DigestRegistry>;

    class PALUDIS_VISIBLE DigestRegistry :
        public Singleton<DigestRegistry>
    {
        friend class Singleton<DigestRegistry>;

        public:
            typedef std::function<std::string (std::istream &)> Function;

            Function get(const std::string & algo) const;

            struct AlgorithmsConstIteratorTag;
            typedef WrappedForwardIterator<AlgorithmsConstIteratorTag, const std::pair<const std::string, Function> > AlgorithmsConstIterator;

            AlgorithmsConstIterator begin_algorithms() const PALUDIS_ATTRIBUTE((warn_unused_result));
            AlgorithmsConstIterator end_algorithms() const PALUDIS_ATTRIBUTE((warn_unused_result));

            template <typename T_>
            class Registration
            {
                public:
                    Registration(const std::string & algo)
                    {
                        get_instance()->register_function(algo, do_digest<T_>);
                    }
            };

        private:
            DigestRegistry();
            ~DigestRegistry();

            Pimp<DigestRegistry> _imp;

            void register_function(const std::string & algo, const Function & func);

            template <typename T_>
            static std::string
            do_digest(std::istream & stream)
            {
                T_ digest(stream);
                return digest.hexsum();
            }
    };
}

#endif
