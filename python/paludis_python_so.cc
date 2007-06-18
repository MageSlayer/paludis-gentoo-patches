/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
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

#include <paludis_python.hh>

#include <boost/version.hpp>

void expose_contents();
void expose_dep_spec();
void expose_eapi();
void expose_environment();
void expose_exception();
void expose_fs_entry();
void expose_log();
void expose_mask_reasons();
void expose_name();
void expose_package_database();
void expose_portage_dep_parser();
void expose_query();
void expose_repository();
void expose_version_metadata();
void expose_version_operator();
void expose_version_requirements();
void expose_version_spec();

BOOST_PYTHON_MODULE(paludis)
{
#if BOOST_VERSION >= 103400
    boost::python::docstring_options doc_options(true, false);
#endif

    expose_exception();
    expose_version_spec();
    expose_version_operator();
    expose_version_requirements();
    expose_fs_entry();
    expose_eapi();
    expose_contents();
    expose_mask_reasons();
    expose_version_metadata();
    expose_dep_spec();
    expose_portage_dep_parser();
    expose_name();
    expose_log();
    expose_query();
    expose_environment();
    expose_package_database();
    expose_repository();
}
