/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_ELIKE_DEP_PARSER_FWD_HH
#define PALUDIS_GUARD_PALUDIS_ELIKE_DEP_PARSER_FWD_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/map-fwd.hh>
#include <paludis/util/options-fwd.hh>
#include <paludis/name-fwd.hh>
#include <functional>
#include <memory>
#include <string>

namespace paludis
{
    struct ELikeDepParserCallbacks;

#include <paludis/elike_dep_parser-se.hh>

    typedef Options<ELikeDepParserOption> ELikeDepParserOptions;

    typedef std::function<void (const std::string &)> ELikeDepParserStringFunction;
    typedef std::function<void (const std::string &, const std::string &)> ELikeDepParserArrowFunction;
    typedef std::function<void ()> ELikeDepParserAnyFunction;
    typedef std::function<void ()> ELikeDepParserAllFunction;
    typedef std::function<void ()> ELikeDepParserAtMostOneFunction;
    typedef std::function<void ()> ELikeDepParserExactlyOneFunction;
    typedef std::function<void (const std::string &)> ELikeDepParserUseFunction;
    typedef std::function<void (const std::string &)> ELikeDepParserLabelFunction;
    typedef std::function<void ()> ELikeDepParserPushFunction;
    typedef std::function<void ()> ELikeDepParserPopFunction;
    typedef std::function<void ()> ELikeDepParserShouldBeEmptyFunction;
    typedef std::function<void (const std::string &, const std::string::size_type &, const std::string &)> ELikeDepParserErrorFunction;
    typedef std::function<void ()> ELikeDepParserUseUnderAnyFunction;
    typedef std::function<void (const std::shared_ptr<const Map<std::string, std::string> > &)>
        ELikeDepParserAnnotationsFunction;
    typedef std::function<void ()> ELikeDepParserNoAnnotationsFunction;

    void parse_elike_dependencies(
            const std::string &,
            const ELikeDepParserCallbacks & callbacks,
            const ELikeDepParserOptions & options
            ) PALUDIS_VISIBLE;
}

#endif
