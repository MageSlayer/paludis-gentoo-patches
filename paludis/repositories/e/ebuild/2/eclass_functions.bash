#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
# Copyright (c) 2008 David Leverton
#
# This file is part of the Paludis package manager. Paludis is free software;
# you can redistribute it and/or modify it under the terms of the GNU General
# Public License, version 2, as published by the Free Software Foundation.
#
# Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA

ebuild_load_module 0/eclass_functions

default()
{
    [[ $(type -t "default_$(paludis_phase_to_function_name "${!PALUDIS_EBUILD_PHASE_VAR}")" ) == "function" ]] || \
        die "default_$(paludis_phase_to_function_name "${!PALUDIS_EBUILD_PHASE_VAR}") is not a function"
    default_$(paludis_phase_to_function_name "${!PALUDIS_EBUILD_PHASE_VAR}") "$@"
}
