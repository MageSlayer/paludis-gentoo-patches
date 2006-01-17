#ifndef PALUDIS_GUARD_ARGS_BAD_VALUE_HH
#define PALUDIS_GUARD_ARGS_BAD_VALUE_HH 1

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include <paludis/args/args_error.hh>

/** \file
 * Declaration for BadValue.
 *
 * \ingroup Args
 * \ingroup Exception
 */

namespace paludis 
{
    namespace args
    {
	/**
	 * Thrown if an invalid parameter is passed to a valid command line argument.
	 *
	 * \ingroup Args
	 * \ingroup Exception
	 */
	class BadValue : public ArgsError
	{
	    public:
		/**
		 * Constructor
		 */
		BadValue(const std::string& option, const std::string& value) throw();
	};
    }
}

#endif
