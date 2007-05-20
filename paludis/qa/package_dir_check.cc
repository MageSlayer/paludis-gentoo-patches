/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/qa/package_dir_check.hh>
#include <paludis/qa/digest_collisions_check.hh>
#include <paludis/qa/ebuild_count_check.hh>
#include <paludis/qa/files_dir_size_check.hh>
#include <paludis/qa/gpg_check.hh>
#include <paludis/qa/has_ebuilds_check.hh>
#include <paludis/qa/has_misc_files_check.hh>
#include <paludis/qa/package_name_check.hh>

#include <paludis/util/virtual_constructor-impl.hh>

using namespace paludis;
using namespace paludis::qa;

template class VirtualConstructor<std::string, tr1::shared_ptr<PackageDirCheck> (*) (),
         virtual_constructor_not_found::ThrowException<NoSuchPackageDirCheckTypeError> >;

PackageDirCheck::PackageDirCheck()
{
}

NoSuchPackageDirCheckTypeError::NoSuchPackageDirCheckTypeError(const std::string & s) throw () :
    Exception("No such file check type: '" + s + "'")
{
}

PackageDirCheckMaker::PackageDirCheckMaker()
{
    register_maker(DigestCollisionsCheck::identifier(),
            &MakePackageDirCheck<DigestCollisionsCheck>::make_package_dir_check);
    register_maker(EbuildCountCheck::identifier(),
            &MakePackageDirCheck<EbuildCountCheck>::make_package_dir_check);
    register_maker(FilesDirSizeCheck::identifier(),
            &MakePackageDirCheck<FilesDirSizeCheck>::make_package_dir_check);
    register_maker(GPGCheck::identifier(),
            &MakePackageDirCheck<GPGCheck>::make_package_dir_check);
    register_maker(HasEbuildsCheck::identifier(),
            &MakePackageDirCheck<HasEbuildsCheck>::make_package_dir_check);
    register_maker(HasMiscFilesCheck::identifier(),
            &MakePackageDirCheck<HasMiscFilesCheck>::make_package_dir_check);
    register_maker(PackageNameCheck::identifier(),
            &MakePackageDirCheck<PackageNameCheck>::make_package_dir_check);
}

