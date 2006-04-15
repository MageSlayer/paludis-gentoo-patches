/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include <paludis/match_package.hh>

using namespace paludis;

bool
match_package_internals::do_match(
        const PackageDatabase * const db,
        const PackageDepAtom * const atom,
        const PackageDatabaseEntry * const entry)
{
    if (atom->package() != entry->get<pde_name>())
        return false;

    if (atom->version_spec_ptr() && ! (((entry->get<pde_version>()).*
                    (atom->version_operator().as_version_spec_operator()))
                (*atom->version_spec_ptr())))
        return false;

    if (atom->repository_ptr())
        if (*atom->repository_ptr() != entry->get<pde_repository>())
            return false;

    if (atom->slot_ptr())
    {
        VersionMetadata::ConstPointer metadata(db->fetch_metadata(*entry));
        if (*atom->slot_ptr() != SlotName(metadata->get(vmk_slot)))
            return false;
    }

    return true;
}

bool
match_package_internals::do_match(
        const PackageDatabase * const,
        const PackageDepAtom * const atom,
        const DepListEntry * const entry)
{
    if (atom->package() != entry->get<dle_name>())
        return false;

    if (atom->version_spec_ptr() && ! (((entry->get<dle_version>()).*
                    (atom->version_operator().as_version_spec_operator()))
                (*atom->version_spec_ptr())))
        return false;

    if (atom->repository_ptr() && (*atom->repository_ptr() != entry->get<dle_repository>()))
        return false;

    if (atom->slot_ptr() && (atom->slot_ptr()->data() != entry->get<dle_metadata>()->get(vmk_slot)))
        return false;

    return true;
}

