/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/dep_list/override_functions.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/stringify.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/mask.hh>
#include <paludis/name.hh>

using namespace paludis;

namespace
{
    struct OverrideTildeKeywordsKeyVisitor :
        ConstVisitor<MetadataKeyVisitorTypes>
    {
        const Environment * const env;
        const PackageID & id;
        bool result;

        OverrideTildeKeywordsKeyVisitor(const Environment * const e, const PackageID & i) :
            env(e),
            id(i),
            result(false)
        {
        }

        void visit(const MetadataPackageIDKey &)
        {
        }

        void visit(const MetadataTimeKey &)
        {
        }

        void visit(const MetadataContentsKey &)
        {
        }

        void visit(const MetadataStringKey &)
        {
        }

        void visit(const MetadataSpecTreeKey<RestrictSpecTree> &)
        {
        }

        void visit(const MetadataSpecTreeKey<ProvideSpecTree> &)
        {
        }

        void visit(const MetadataSpecTreeKey<URISpecTree> &)
        {
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> &)
        {
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> &)
        {
        }

        void visit(const MetadataSetKey<InheritedSet> &)
        {
        }

        void visit(const MetadataSetKey<KeywordNameSet> & k)
        {
            tr1::shared_ptr<KeywordNameSet> kk(new KeywordNameSet);
            for (KeywordNameSet::Iterator i(k.value()->begin()), i_end(k.value()->end()) ;
                    i != i_end ; ++i)
            {
                kk->insert(*i);
                if ('~' == stringify(*i).at(0))
                    kk->insert(KeywordName(stringify(*i).substr(1)));
            }

            result |= env->accept_keywords(kk, id);
        }

        void visit(const MetadataSetKey<IUseFlagSet> &)
        {
        }

        void visit(const MetadataSetKey<UseFlagNameSet> &)
        {
        }
    };

    struct OverrideTildeKeywordsMaskVisitor :
        ConstVisitor<MaskVisitorTypes>
    {
        const Environment * const env;
        const PackageID & id;
        bool result;

        OverrideTildeKeywordsMaskVisitor(const Environment * const e, const PackageID & i) :
            env(e),
            id(i),
            result(false)
        {
        }

        void visit(const UserMask &)
        {
        }

        void visit(const UnacceptedMask & m)
        {
            OverrideTildeKeywordsKeyVisitor k(env, id);
            m.unaccepted_key()->accept(k);
            result |= k.result;
        }

        void visit(const RepositoryMask &)
        {
        }

        void visit(const AssociationMask &)
        {
        }

        void visit(const UnsupportedMask &)
        {
        }
    };

    struct OverrideUnkeywordedKeyVisitor :
        ConstVisitor<MetadataKeyVisitorTypes>
    {
        const Environment * const env;
        const PackageID & id;
        bool result;

        OverrideUnkeywordedKeyVisitor(const Environment * const e, const PackageID & i) :
            env(e),
            id(i),
            result(false)
        {
        }

        void visit(const MetadataPackageIDKey &)
        {
        }

        void visit(const MetadataTimeKey &)
        {
        }

        void visit(const MetadataContentsKey &)
        {
        }

        void visit(const MetadataStringKey &)
        {
        }

        void visit(const MetadataSpecTreeKey<RestrictSpecTree> &)
        {
        }

        void visit(const MetadataSpecTreeKey<ProvideSpecTree> &)
        {
        }

        void visit(const MetadataSpecTreeKey<URISpecTree> &)
        {
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> &)
        {
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> &)
        {
        }

        void visit(const MetadataSetKey<InheritedSet> &)
        {
        }

        void visit(const MetadataSetKey<KeywordNameSet> & k)
        {
            tr1::shared_ptr<KeywordNameSet> kk(new KeywordNameSet);
            for (KeywordNameSet::Iterator i(k.value()->begin()), i_end(k.value()->end()) ;
                    i != i_end ; ++i)
                if ('-' == stringify(*i).at(0))
                    kk->insert(KeywordName(stringify(*i).substr(1)));

            result |= ! env->accept_keywords(kk, id);
        }

        void visit(const MetadataSetKey<IUseFlagSet> &)
        {
        }

        void visit(const MetadataSetKey<UseFlagNameSet> &)
        {
        }
    };

    struct OverrideUnkeywordedMaskVisitor :
        ConstVisitor<MaskVisitorTypes>
    {
        const Environment * const env;
        const PackageID & id;
        bool result;

        OverrideUnkeywordedMaskVisitor(const Environment * const e, const PackageID & i) :
            env(e),
            id(i),
            result(false)
        {
        }

        void visit(const UserMask &)
        {
        }

        void visit(const UnacceptedMask & m)
        {
            OverrideUnkeywordedKeyVisitor k(env, id);
            m.unaccepted_key()->accept(k);
            result |= k.result;
        }

        void visit(const RepositoryMask &)
        {
        }

        void visit(const AssociationMask &)
        {
        }

        void visit(const UnsupportedMask &)
        {
        }
    };

    struct OverrideRepositoryMasksVisitor :
        ConstVisitor<MaskVisitorTypes>
    {
        bool result;

        OverrideRepositoryMasksVisitor() :
            result(false)
        {
        }

        void visit(const UserMask &)
        {
        }

        void visit(const UnacceptedMask &)
        {
        }

        void visit(const RepositoryMask &)
        {
            result = true;
        }

        void visit(const AssociationMask &)
        {
        }

        void visit(const UnsupportedMask &)
        {
        }
    };

    struct OverrideLicenseKeyVisitor :
        ConstVisitor<MetadataKeyVisitorTypes>
    {
        bool result;

        OverrideLicenseKeyVisitor() :
            result(false)
        {
        }

        void visit(const MetadataPackageIDKey &)
        {
        }

        void visit(const MetadataTimeKey &)
        {
        }

        void visit(const MetadataContentsKey &)
        {
        }

        void visit(const MetadataStringKey &)
        {
        }

        void visit(const MetadataSpecTreeKey<RestrictSpecTree> &)
        {
        }

        void visit(const MetadataSpecTreeKey<ProvideSpecTree> &)
        {
        }

        void visit(const MetadataSpecTreeKey<URISpecTree> &)
        {
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> &)
        {
            result = true;
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> &)
        {
        }

        void visit(const MetadataSetKey<InheritedSet> &)
        {
        }

        void visit(const MetadataSetKey<KeywordNameSet> &)
        {
        }

        void visit(const MetadataSetKey<IUseFlagSet> &)
        {
        }

        void visit(const MetadataSetKey<UseFlagNameSet> &)
        {
        }
    };

    struct OverrideLicenseVisitor :
        ConstVisitor<MaskVisitorTypes>
    {
        bool result;

        OverrideLicenseVisitor() :
            result(false)
        {
        }

        void visit(const UserMask &)
        {
        }

        void visit(const UnacceptedMask & m)
        {
            OverrideLicenseKeyVisitor k;
            m.unaccepted_key()->accept(k);
            result |= k.result;
        }

        void visit(const RepositoryMask &)
        {
        }

        void visit(const AssociationMask &)
        {
        }

        void visit(const UnsupportedMask &)
        {
        }
    };
}

bool
paludis::override_tilde_keywords(const Environment * const e, const PackageID & i, const Mask & m)
{
    Context c("When working out whether mask is a tilde keyword mask for override:");
    OverrideTildeKeywordsMaskVisitor k(e, i);
    m.accept(k);
    return k.result;
}

bool
paludis::override_unkeyworded(const Environment * const e, const PackageID & i, const Mask & m)
{
    Context c("When working out whether mask is an unkeyworded mask for override:");
    OverrideUnkeywordedMaskVisitor k(e, i);
    m.accept(k);
    return k.result;
}

bool
paludis::override_repository_masks(const Mask & m)
{
    Context c("When working out whether mask is a repository mask for override:");
    OverrideRepositoryMasksVisitor k;
    m.accept(k);
    return k.result;
}

bool
paludis::override_license(const Mask & m)
{
    Context c("When working out whether mask is a license mask for override:");
    OverrideLicenseVisitor k;
    m.accept(k);
    return k.result;
}


