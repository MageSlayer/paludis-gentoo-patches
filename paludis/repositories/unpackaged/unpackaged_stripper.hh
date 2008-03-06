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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_UNPACKAGED_STRIPPER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_UNPACKAGED_STRIPPER_HH 1

#include <paludis/stripper.hh>

namespace paludis
{
    namespace unpackaged_repositories
    {
        typedef kc::KeyedClass<
            kc::Field<k::package_id, tr1::shared_ptr<const PackageID> >,
            kc::Field<k::image_dir, FSEntry>,
            kc::Field<k::debug_dir, FSEntry>,
            kc::Field<k::debug_build, InstallActionDebugOption>
                > UnpackagedStripperOptions;

        class UnpackagedStripper :
            public Stripper,
            private PrivateImplementationPattern<UnpackagedStripper>
        {
            private:
                PrivateImplementationPattern<UnpackagedStripper>::ImpPtr & _imp;

            protected:
                virtual void on_strip(const FSEntry &);
                virtual void on_split(const FSEntry &, const FSEntry &);
                virtual void on_enter_dir(const FSEntry &);
                virtual void on_leave_dir(const FSEntry &);
                virtual void on_unknown(const FSEntry &);

            public:
                UnpackagedStripper(const UnpackagedStripperOptions &);
                ~UnpackagedStripper();

                virtual void strip();
        };
    }
}

#endif
