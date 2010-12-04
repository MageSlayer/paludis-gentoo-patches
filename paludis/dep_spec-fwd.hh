/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_SPEC_FWD_HH
#define PALUDIS_GUARD_PALUDIS_DEP_SPEC_FWD_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/dep_label-fwd.hh>
#include <iosfwd>

/** \file
 * Forward declarations for paludis/dep_spec.hh .
 *
 * \ingroup g_dep_spec
 */

namespace paludis
{

#include <paludis/dep_spec-se.hh>

    class DepSpec;
    class PackageDepSpec;
    class PlainTextDepSpec;
    class LicenseDepSpec;
    class FetchableURIDepSpec;
    class SimpleURIDepSpec;
    class AllDepSpec;
    class ExactlyOneDepSpec;
    class AnyDepSpec;
    class ConditionalDepSpec;
    class BlockDepSpec;
    class StringDepSpec;
    class NamedSetDepSpec;
    class PlainTextLabelDepSpec;
    template <typename T_> class LabelsDepSpec;

    /**
     * A URILabelsDepSpec represents labels in a FetchableURISpecTree.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef LabelsDepSpec<URILabel> URILabelsDepSpec;

    /**
     * A DependenciesLabelsDepSpec represents labels in a DependenciesSpecTree.
     *
     * \ingroup g_dep_spec
     * \since 0.42
     */
    typedef LabelsDepSpec<DependenciesLabel> DependenciesLabelsDepSpec;

    struct InstallableToRepository;
    struct InstallableToPath;

    /**
     * A PlainTextDepSpec can be written to an ostream.
     *
     * \ingroup g_dep_spec
     */
    std::ostream & operator<< (std::ostream &, const PlainTextDepSpec &) PALUDIS_VISIBLE;

    class PackageDepSpecError;

    /**
     * A PackageDepSpec can be written to an ostream.
     *
     * \ingroup g_dep_spec
     */
    std::ostream & operator<< (std::ostream &, const PackageDepSpec &) PALUDIS_VISIBLE;

    /**
     * A URILabelsDepSpec can be written to an ostream.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    std::ostream & operator<< (std::ostream &, const URILabelsDepSpec &) PALUDIS_VISIBLE;

    /**
     * A DependenciesLabelsDepSpec can be written to an ostream.
     *
     * \ingroup g_dep_spec
     * \since 0.42
     */
    std::ostream & operator<< (std::ostream &, const DependenciesLabelsDepSpec &) PALUDIS_VISIBLE;

    /**
     * A BlockDepSpec can be written to an ostream.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    std::ostream & operator<< (std::ostream &, const BlockDepSpec &) PALUDIS_VISIBLE;

    /**
     * A SimpleURIDepSpec can be written to an ostream.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    std::ostream & operator<< (std::ostream &, const SimpleURIDepSpec &) PALUDIS_VISIBLE;

    /**
     * A FetchableURIDepSpec can be written to an ostream.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    std::ostream & operator<< (std::ostream &, const FetchableURIDepSpec &) PALUDIS_VISIBLE;

    /**
     * A LicenseDepSpec can be written to an ostream.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    std::ostream & operator<< (std::ostream &, const LicenseDepSpec &) PALUDIS_VISIBLE;

    /**
     * A ConditionalDepSpec can be written to an ostream.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    std::ostream & operator<< (std::ostream &, const ConditionalDepSpec &) PALUDIS_VISIBLE;

    /**
     * A NamedSetDepSpec can be written to an ostream.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    std::ostream & operator<< (std::ostream &, const NamedSetDepSpec &) PALUDIS_VISIBLE;

    /**
     * A PlainTextLabelDepSpec can be written to an ostream.
     *
     * \ingroup g_dep_spec
     * \since 0.32
     */
    std::ostream & operator<< (std::ostream &, const PlainTextLabelDepSpec &) PALUDIS_VISIBLE;
}

#endif
