#if 0
ifdef(`__gnu__',`',`errprint(`This is not GNU m4...
')m4exit(1)') include(`misc/generated-file.txt')
dnl vim: set ft=cpp et sw=4 sts=4 :
#endif

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_PALUDIS_HH
#define PALUDIS_GUARD_PALUDIS_PALUDIS_HH 1

/** \file
 * Master include file.
 */

define(`addhh', `dnl
#include <paludis/$1.hh>
')dnl
define(`addimpl', `dnl
#include <paludis/$1-impl.hh>
')dnl
define(`addthis', `ifelse(`$2', `hh', `addhh(`$1')', ifelse(`$2', `impl', `addimpl(`$1')', `' ) )')
define(`add', `addthis(`$1',`$2')addthis(`$1',`$3')addthis(`$1',`$4')
addthis(`$1',`$5')addthis(`$1',`$6')')dnl

include(`paludis/files.m4')

#include <paludis/util/util.hh>

#endif

