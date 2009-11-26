/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_SOURCE_URI_FINDER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_SOURCE_URI_FINDER_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <paludis/dep_label.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <tr1/functional>

namespace paludis
{
    namespace erepository
    {
        typedef std::tr1::function<std::tr1::shared_ptr<const MirrorsSequence> (const std::string &)> GetMirrorsFunction;

        class PALUDIS_VISIBLE SourceURIFinder :
            private PrivateImplementationPattern<SourceURIFinder>
        {
            private:
                void add_local_mirrors();
                void add_mirrors();
                void add_listed();

            public:
                SourceURIFinder(const Environment * const env,
                        const Repository * const repo,
                        const std::string & url,
                        const std::string & filename,
                        const std::string & mirrors_name,
                        const GetMirrorsFunction & fn);

                ~SourceURIFinder();

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag, const std::pair<std::string, std::string> > ConstIterator;
                ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

                void visit(const URIMirrorsOnlyLabel &);
                void visit(const URIMirrorsThenListedLabel &);
                void visit(const URIListedOnlyLabel &);
                void visit(const URIListedThenMirrorsLabel &);
                void visit(const URILocalMirrorsOnlyLabel &);
                void visit(const URIManualOnlyLabel &);
        };
    }
}

#endif
