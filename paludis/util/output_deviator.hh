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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_OUTPUT_DEVIATOR_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_OUTPUT_DEVIATOR_HH 1

#include <paludis/util/output_deviator-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/sequence-fwd.hh>
#include <tr1/memory>

using namespace paludis;

namespace paludis
{
    class PALUDIS_VISIBLE OutputDeviator :
        private PrivateImplementationPattern<OutputDeviator>
    {
        public:
            OutputDeviator(const FSEntry & log_dir);
            ~OutputDeviator();

            const std::tr1::shared_ptr<OutputDeviant> make_output_deviant(
                    const std::string &, const unsigned int number_of_tail_lines) PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE OutputDeviant :
        private PrivateImplementationPattern<OutputDeviant>
    {
        friend class OutputDeviator;

        private:
            OutputDeviant(const FSEntry &, const unsigned int);

        public:
            ~OutputDeviant();

            std::ostream * stdout_stream() const PALUDIS_ATTRIBUTE((warn_unused_result));
            std::ostream * stderr_stream() const PALUDIS_ATTRIBUTE((warn_unused_result));

            void discard_log();
            const FSEntry log_file_name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::tr1::shared_ptr<const Sequence<std::string> > tail(const bool clear) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<OutputDeviator>;
    extern template class PrivateImplementationPattern<OutputDeviant>;
#endif
}

#endif
