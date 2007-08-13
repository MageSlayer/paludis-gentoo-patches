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

#include <boost/python.hpp>
#include <boost/version.hpp>

void expose_action();
void expose_contents();
void expose_dep_list();
void expose_dep_spec();
void expose_dep_tag();
void expose_environment();
void expose_exception();
void expose_fs_entry();
void expose_log();
void expose_mask();
void expose_metadata_key();
void expose_name();
void expose_package_database();
void expose_package_id();
void expose_qa();
void expose_query();
void expose_repository();
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
    expose_contents();
    expose_dep_spec();
    expose_dep_tag();
    expose_dep_list();
    expose_name();
    expose_log();
    expose_query();
    expose_environment();
    expose_metadata_key();
    expose_mask();
    expose_package_id();
    expose_action();
    expose_package_database();
    expose_repository();
    expose_qa();
}
