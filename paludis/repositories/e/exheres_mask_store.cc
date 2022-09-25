/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/exheres_mask_store.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_parser.hh>

#include <paludis/util/pimp-impl.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/fs_stat.hh>

#include <paludis/dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/match_package.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/dep_spec_annotations.hh>

#include <algorithm>
#include <unordered_map>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

typedef std::unordered_map<QualifiedPackageName,
        std::list<std::pair<PackageDepSpec, std::shared_ptr<const MaskInfo> > >,
        Hash<QualifiedPackageName> > RepositoryMaskMap;

namespace paludis
{
    template <>
    struct Imp<ExheresMaskStore>
    {
        const Environment * const env;
        RepositoryName repo_name;
        const std::shared_ptr<const FSPathSequence> files;
        EAPIForFileFunction eapi_for_file;

        RepositoryMaskMap repo_mask;

        Imp(const Environment * const e, const RepositoryName & r, const std::shared_ptr<const FSPathSequence> & f, const EAPIForFileFunction & n) :
            env(e),
            repo_name(r),
            files(f),
            eapi_for_file(n)
        {
        }
    };
}

ExheresMaskStore::ExheresMaskStore(
        const Environment * const e,
        const RepositoryName & r,
        const std::shared_ptr<const FSPathSequence> & f,
        const EAPIForFileFunction & n) :
    _imp(e, r, f, n)
{
    _populate();
}

ExheresMaskStore::~ExheresMaskStore() = default;

namespace
{
    std::string extract_annotation(const PackageDepSpec & s, const DepSpecAnnotationRole role)
    {
        if (! s.maybe_annotations())
            return "";

        auto a(s.maybe_annotations()->find(role));
        if (a == s.maybe_annotations()->end())
            return "";
        else
            return a->value();
    }

    std::string extract_comment(const PackageDepSpec & s)
    {
        const auto author(extract_annotation(s, dsar_general_author));
        const auto description(extract_annotation(s, dsar_general_description));
        const auto date(extract_annotation(s, dsar_general_date));

        std::string result;

        if (! description.empty())
            result = description;

        if (! author.empty())
        {
            if (! author.empty())
                result.append(" ");
            result.append("(" + author + ")");
        }

        if (! date.empty())
        {
            if (! result.empty())
                result.append(", ");
            result.append(date);
        }

        return result;
    }

    std::shared_ptr<MaskInfo> make_mask_info(const PackageDepSpec & s, const FSPath & f)
    {
        auto result(std::make_shared<MaskInfo>(make_named_values<MaskInfo>(
                        n::comment() = extract_comment(s),
                        n::mask_file() = f,
                        n::token() = extract_annotation(s, dsar_general_token)
                        )));

        return result;
    }
}

void
ExheresMaskStore::_populate()
{
    Context context("When loading repository masks for '" + stringify(_imp->repo_name) + "':");

    using namespace std::placeholders;

    for (const auto & f : *_imp->files)
    {
        if (! f.stat().exists())
            continue;

        SafeIFStream file(f);
        std::string file_text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        try
        {
            auto specs(parse_commented_set(file_text, _imp->env, *EAPIData::get_instance()->eapi_from_string(_imp->eapi_for_file(f))));
            DepSpecFlattener<SetSpecTree, PackageDepSpec> flat_specs(_imp->env, nullptr);
            specs->top()->accept(flat_specs);

            for (const auto & flat_spec : flat_specs)
            {
                if (flat_spec->package_ptr())
                    _imp->repo_mask[*flat_spec->package_ptr()].push_back(std::make_pair(*flat_spec, make_mask_info(*flat_spec, f)));
                else
                    Log::get_instance()->message("e.package_mask.bad_spec", ll_warning, lc_context)
                        << "Loading package mask spec '" << *flat_spec << "' failed because specification does not restrict to a "
                        "unique package";
            }
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.exheres_mask.bad", ll_warning, lc_context) << "Loading package mask file '"
                << f << "' failed due to exception '" << e.message() << "' ("
                << e.what() << ")";
        }
    }
}

const std::shared_ptr<const MasksInfo>
ExheresMaskStore::query(const std::shared_ptr<const PackageID> & id) const
{
    auto result(std::make_shared<MasksInfo>());
    auto r(_imp->repo_mask.find(id->name()));
    if (_imp->repo_mask.end() != r)
        for (const auto & k : r->second)
            if (match_package(*_imp->env, k.first, id, nullptr, { }))
                result->push_back(*k.second);

    return result;
}

