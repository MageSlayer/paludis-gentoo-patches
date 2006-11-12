/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/repositories/portage/portage_repository_profile.hh>
#include <paludis/repositories/portage/portage_repository_profile_file.hh>
#include <paludis/repositories/portage/portage_repository_exceptions.hh>

#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/save.hh>

#include <paludis/config_file.hh>
#include <paludis/environment.hh>
#include <paludis/match_package.hh>
#include <paludis/hashed_containers.hh>

#include <list>
#include <algorithm>
#include <set>

#include <strings.h>
#include <ctype.h>

using namespace paludis;

namespace
{
    typedef MakeHashedSet<UseFlagName>::Type UseFlagSet;
    typedef MakeHashedMap<std::string, std::string>::Type EnvironmentVariablesMap;
    typedef MakeHashedMap<QualifiedPackageName, PackageDepAtom::ConstPointer>::Type VirtualsMap;
    typedef MakeHashedMap<QualifiedPackageName, std::list<PackageDepAtom::ConstPointer> >::Type PackageMaskMap;

    typedef MakeHashedMap<UseFlagName, bool>::Type FlagStatusMap;
    typedef std::list<std::pair<PackageDepAtom::ConstPointer, FlagStatusMap> > PackageFlagStatusMapList;

    struct StackedValues
    {
        std::string origin;

        FlagStatusMap use;
        FlagStatusMap use_mask;
        FlagStatusMap use_force;
        PackageFlagStatusMapList package_use;
        PackageFlagStatusMapList package_use_mask;
        PackageFlagStatusMapList package_use_force;

        StackedValues(const std::string & o) :
            origin(o)
        {
        }
    };

    typedef std::list<StackedValues> StackedValuesList;
}

namespace paludis
{
    /**
     * Implementation for PortageRepositoryProfile.
     *
     * \ingroup grpportagerepository
     * \see PortageRepositoryProfile
     */
    template<>
    class Implementation<PortageRepositoryProfile> :
        public InternalCounted<PortageRepositoryProfile>
    {
        private:
            void load_profile_directory_recursively(const FSEntry & dir);
            void load_profile_parent(const FSEntry & dir);
            void load_profile_make_defaults(const FSEntry & dir);

            void load_basic_use_file(const FSEntry & file, FlagStatusMap & m);
            void load_atom_use_file(const FSEntry & file, PackageFlagStatusMapList & m);

            void add_use_expand_to_use();
            void make_vars_from_file_vars();
            void handle_profile_arch_var();

            ProfileFile packages_file;
            ProfileFile virtuals_file;
            ProfileFile package_mask_file;

        public:
            ///\name General variables
            ///\{

            const Environment * const env;

            ///\}

            ///\name Environment variables
            ///\{

            EnvironmentVariablesMap environment_variables;

            ///\}

            ///\name System package set
            ///\{

            AllDepAtom::Pointer system_packages;
            GeneralSetDepTag::Pointer system_tag;

            ///\}

            ///\name Virtuals
            ///\{

            VirtualsMap virtuals;

            ///\}

            ///\name USE related values
            ///\{

            UseFlagSet use_expand;
            UseFlagSet use_expand_hidden;
            StackedValuesList stacked_values_list;

            ///\}

            ///\name Masks
            ///\{

            PackageMaskMap package_mask;

            ///\}

            ///\name Basic operations
            ///\{

            Implementation(const Environment * const e, const RepositoryName & name,
                    const FSEntryCollection & dirs) :
                env(e),
                system_packages(new AllDepAtom),
                system_tag(new GeneralSetDepTag(SetName("system"), stringify(name)))
            {
                for (FSEntryCollection::Iterator d(dirs.begin()), d_end(dirs.end()) ;
                        d != d_end ; ++d)
                    load_profile_directory_recursively(*d);

                add_use_expand_to_use();
                make_vars_from_file_vars();
                handle_profile_arch_var();
            }

            ~Implementation()
            {
            }

            ///\}
    };
}

void
Implementation<PortageRepositoryProfile>::load_profile_directory_recursively(const FSEntry & dir)
{
    Context context("When adding profile directory '" + stringify(dir) + ":");
    Log::get_instance()->message(ll_debug, lc_context, "Loading profile directory '" + stringify(dir) + "'");

    if (! dir.is_directory())
    {
        Log::get_instance()->message(ll_warning, lc_context,
                "Profile component '" + stringify(dir) + "' is not a directory");
        return;
    }

    stacked_values_list.push_back(StackedValues(stringify(dir)));

    load_profile_parent(dir);
    load_profile_make_defaults(dir);

    load_basic_use_file(dir / "use.mask", stacked_values_list.back().use_mask);
    load_basic_use_file(dir / "use.force", stacked_values_list.back().use_force);
    load_atom_use_file(dir / "package.use", stacked_values_list.back().package_use);
    load_atom_use_file(dir / "package.use.mask", stacked_values_list.back().package_use_mask);
    load_atom_use_file(dir / "package.use.force", stacked_values_list.back().package_use_force);

    packages_file.add_file(dir / "packages");
    virtuals_file.add_file(dir / "virtuals");
    package_mask_file.add_file(dir / "package.mask");
}

void
Implementation<PortageRepositoryProfile>::load_profile_parent(const FSEntry & dir)
{
    Context context("When handling parent file for profile directory '" + stringify(dir) + ":");

    if (! (dir / "parent").exists())
        return;

    LineConfigFile file(dir / "parent");

    LineConfigFile::Iterator i(file.begin()), i_end(file.end());
    if (i == i_end)
        Log::get_instance()->message(ll_warning, lc_context, "parent file is empty");
    else
        for ( ; i != i_end ; ++i)
        {
            FSEntry parent_dir(dir);
            do
            {
                try
                {
                    parent_dir = (parent_dir / *i).realpath();
                }
                catch (const FSError & e)
                {
                    Log::get_instance()->message(ll_warning, lc_context, "Skipping parent '"
                            + *i + "' due to exception: " + e.message() + " (" + e.what() + ")");
                    continue;
                }

                load_profile_directory_recursively(parent_dir);

            } while (false);
        }
}

void
Implementation<PortageRepositoryProfile>::load_profile_make_defaults(const FSEntry & dir)
{
    Context context("When handling make.defaults file for profile directory '" + stringify(dir) + ":");

    if (! (dir / "make.defaults").exists())
        return;

    KeyValueConfigFile file(dir / "make.defaults");

    try
    {
        std::list<std::string> uses;
        WhitespaceTokeniser::get_instance()->tokenise(file.get("USE"), std::back_inserter(uses));

        for (std::list<std::string>::const_iterator u(uses.begin()), u_end(uses.end()) ;
                u != u_end ; ++u)
            if ('-' == u->at(0))
                stacked_values_list.back().use[UseFlagName(u->substr(1))] = false;
            else
                stacked_values_list.back().use[UseFlagName(*u)] = true;
    }
    catch (const NameError & e)
    {
        Log::get_instance()->message(ll_warning, lc_context, "Loading USE failed due to exception: "
                + e.message() + " (" + e.what() + ")");
    }

    try
    {
        WhitespaceTokeniser::get_instance()->tokenise(
                file.get("USE_EXPAND"), create_inserter<UseFlagName>(
                    std::inserter(use_expand, use_expand.begin())));
    }
    catch (const NameError & e)
    {
        Log::get_instance()->message(ll_warning, lc_context, "Loading USE_EXPAND failed due to exception: "
                + e.message() + " (" + e.what() + ")");
    }

    try
    {
        WhitespaceTokeniser::get_instance()->tokenise(
                file.get("USE_EXPAND_HIDDEN"), create_inserter<UseFlagName>(
                    std::inserter(use_expand_hidden, use_expand_hidden.begin())));
    }
    catch (const NameError & e)
    {
        Log::get_instance()->message(ll_warning, lc_context, "Loading USE_EXPAND_HIDDEN failed due to exception: "
                + e.message() + " (" + e.what() + ")");
    }

    for (KeyValueConfigFile::Iterator k(file.begin()), k_end(file.end()) ;
            k != k_end ; ++k)
    {
        Log::get_instance()->message(ll_debug, lc_context, "Profile environment variable '" +
                stringify(k->first) + "' is '" + stringify(k->second) + "'");
        environment_variables[k->first] = k->second;
    }
}

void
Implementation<PortageRepositoryProfile>::make_vars_from_file_vars()
{
    try
    {
        for (ProfileFile::Iterator i(packages_file.begin()), i_end(packages_file.end()) ; i != i_end ; ++i)
        {
            if (0 != i->compare(0, 1, "*", 0, 1))
                continue;

            Context context_atom("When parsing '" + *i + "':");
            PackageDepAtom::Pointer atom(new PackageDepAtom(i->substr(1)));
            atom->set_tag(system_tag);
            system_packages->add_child(atom);
        }
    }
    catch (const NameError & e)
    {
        Log::get_instance()->message(ll_warning, lc_context, "Loading packages "
                " failed due to exception: " + e.message() + " (" + e.what() + ")");
    }

    try
    {
        for (ProfileFile::Iterator line(virtuals_file.begin()), line_end(virtuals_file.end()) ;
                line != line_end ; ++line)
        {
            std::vector<std::string> tokens;
            WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(tokens));
            if (tokens.size() < 2)
                continue;

            QualifiedPackageName v(tokens[0]);
            virtuals.erase(v);
            virtuals.insert(std::make_pair(v, PackageDepAtom::Pointer(new PackageDepAtom(tokens[1]))));
        }
    }
    catch (const NameError & e)
    {
        Log::get_instance()->message(ll_warning, lc_context, "Loading virtuals "
                " failed due to exception: " + e.message() + " (" + e.what() + ")");
    }

    for (ProfileFile::Iterator line(package_mask_file.begin()), line_end(package_mask_file.end()) ;
            line != line_end ; ++line)
    {
        if (line->empty())
            continue;

        try
        {
            PackageDepAtom::ConstPointer a(new PackageDepAtom(*line));
            package_mask[a->package()].push_back(a);
        }
        catch (const NameError & e)
        {
            Log::get_instance()->message(ll_warning, lc_context, "Loading package.mask atom '"
                    + stringify(*line) + "' failed due to exception '" + e.message() + "' ("
                    + e.what() + ")");
        }
    }
}

void
Implementation<PortageRepositoryProfile>::load_basic_use_file(const FSEntry & file, FlagStatusMap & m)
{
    if (! file.exists())
        return;

    Context context("When loading basic use file '" + stringify(file) + ":");
    LineConfigFile f(file);
    for (LineConfigFile::Iterator line(f.begin()), line_end(f.end()) ;
            line != line_end ; ++line)
    {
        std::list<std::string> tokens;
        WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(tokens));

        for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
                t != t_end ; ++t)
        {
            try
            {
                if (t->empty())
                    continue;
                if ('-' == t->at(0))
                    m[UseFlagName(t->substr(1))] = false;
                else
                    m[UseFlagName(*t)] = true;
            }
            catch (const NameError & e)
            {
                Log::get_instance()->message(ll_warning, lc_context, "Ignoring token '"
                        + *t + "' due to exception '" + e.message() + "' (" + e.what() + ")");
            }
        }
    }
}

void
Implementation<PortageRepositoryProfile>::load_atom_use_file(const FSEntry & file, PackageFlagStatusMapList & m)
{
    if (! file.exists())
        return;

    Context context("When loading atomised use file '" + stringify(file) + ":");
    LineConfigFile f(file);
    for (LineConfigFile::Iterator line(f.begin()), line_end(f.end()) ;
            line != line_end ; ++line)
    {
        std::list<std::string> tokens;
        WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(tokens));

        if (tokens.empty())
            continue;

        try
        {
            PackageDepAtom::ConstPointer atom(new PackageDepAtom(*tokens.begin()));
            PackageFlagStatusMapList::iterator n(m.insert(m.end(), std::make_pair(atom, FlagStatusMap())));

            for (std::list<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                    t != t_end ; ++t)
            {
                try
                {
                    if (t->empty())
                        continue;
                    if ('-' == t->at(0))
                        n->second[UseFlagName(t->substr(1))] = false;
                    else
                        n->second[UseFlagName(*t)] = true;
                }
                catch (const NameError & e)
                {
                    Log::get_instance()->message(ll_warning, lc_context, "Ignoring token '"
                            + *t + "' due to exception '" + e.message() + "' (" + e.what() + ")");
                }
            }
        }
        catch (const PackageDepAtomError & e)
        {
            Log::get_instance()->message(ll_warning, lc_context, "Ignoring line '"
                    + *line + "' due to exception '" + e.message() + "' (" + e.what() + ")");
        }
    }
}

void
Implementation<PortageRepositoryProfile>::add_use_expand_to_use()
{
    Context context("When adding USE_EXPAND to USE:");

    stacked_values_list.push_back(StackedValues("use_expand special values"));

    for (UseFlagSet::const_iterator x(use_expand.begin()), x_end(use_expand.end()) ;
            x != x_end ; ++x)
    {
        std::string lower_x;
        std::transform(x->data().begin(), x->data().end(), std::back_inserter(lower_x),
                &::tolower);

        std::list<std::string> uses;
        WhitespaceTokeniser::get_instance()->tokenise(environment_variables[stringify(*x)],
                std::back_inserter(uses));
        for (std::list<std::string>::const_iterator u(uses.begin()), u_end(uses.end()) ;
                u != u_end ; ++u)
            stacked_values_list.back().use[UseFlagName(lower_x + "_" + *u)] = true;
    }
}

void
Implementation<PortageRepositoryProfile>::handle_profile_arch_var()
{
    Context context("When handling profile ARCH variable:");

    std::string arch_s(environment_variables["ARCH"]);
    if (arch_s.empty())
        throw PortageRepositoryConfigurationError("ARCH variable is unset or empty");

    stacked_values_list.push_back(StackedValues("arch special values"));
    try
    {
        UseFlagName arch(arch_s);

        stacked_values_list.back().use[arch] = true;
        stacked_values_list.back().use_force[arch] = true;
#if 0
        if (use_mask.end() != use_mask.find(arch))
            throw PortageRepositoryConfigurationError("ARCH USE '" + arch_s + "' is use masked");
#endif
    }
    catch (const NameError & e)
    {
        throw PortageRepositoryConfigurationError("ARCH variable has invalid value '" + arch_s + "'");
    }
}

PortageRepositoryProfile::PortageRepositoryProfile(
        const Environment * const env, const RepositoryName & name, const FSEntryCollection & location) :
    PrivateImplementationPattern<PortageRepositoryProfile>(
            new Implementation<PortageRepositoryProfile>(env, name, location))
{
}

PortageRepositoryProfile::~PortageRepositoryProfile()
{
}

bool
PortageRepositoryProfile::use_masked(const UseFlagName & u,
        const PackageDatabaseEntry * const e) const
{
    bool result(false);
    for (StackedValuesList::const_iterator i(_imp->stacked_values_list.begin()),
            i_end(_imp->stacked_values_list.end()) ; i != i_end ; ++i)
    {
        FlagStatusMap::const_iterator f(i->use_mask.find(u));
        if (i->use_mask.end() != f)
            result = f->second;

        if (e)
            for (PackageFlagStatusMapList::const_iterator g(i->package_use_mask.begin()),
                    g_end(i->package_use_mask.end()) ; g != g_end ; ++g)
            {
                if (! match_package(_imp->env, g->first, e))
                    continue;

                FlagStatusMap::const_iterator h(g->second.find(u));
                if (g->second.end() != h)
                    result = h->second;
            }
    }

    return result;
}

bool
PortageRepositoryProfile::use_forced(const UseFlagName & u,
        const PackageDatabaseEntry * const e) const
{
    bool result(false);
    for (StackedValuesList::const_iterator i(_imp->stacked_values_list.begin()),
            i_end(_imp->stacked_values_list.end()) ; i != i_end ; ++i)
    {
        FlagStatusMap::const_iterator f(i->use_force.find(u));
        if (i->use_force.end() != f)
            result = f->second;

        if (e)
            for (PackageFlagStatusMapList::const_iterator g(i->package_use_force.begin()),
                    g_end(i->package_use_force.end()) ; g != g_end ; ++g)
            {
                if (! match_package(_imp->env, g->first, e))
                    continue;

                FlagStatusMap::const_iterator h(g->second.find(u));
                if (g->second.end() != h)
                    result = h->second;
            }
    }

    return result;
}

UseFlagState
PortageRepositoryProfile::use_state_ignoring_masks(const UseFlagName & u,
        const PackageDatabaseEntry * const e) const
{
    UseFlagState result(use_unspecified);

    for (StackedValuesList::const_iterator i(_imp->stacked_values_list.begin()),
            i_end(_imp->stacked_values_list.end()) ; i != i_end ; ++i)
    {
        FlagStatusMap::const_iterator f(i->use.find(u));
        if (i->use.end() != f)
            result = f->second ? use_enabled : use_disabled;

        if (e)
            for (PackageFlagStatusMapList::const_iterator g(i->package_use.begin()),
                    g_end(i->package_use.end()) ; g != g_end ; ++g)
            {
                if (! match_package(_imp->env, g->first, e))
                    continue;

                FlagStatusMap::const_iterator h(g->second.find(u));
                if (g->second.end() != h)
                    result = h->second ? use_enabled : use_disabled;
            }
    }

    return result;
}

std::string
PortageRepositoryProfile::environment_variable(const std::string & s) const
{
    EnvironmentVariablesMap::const_iterator i(_imp->environment_variables.find(s));
    if (_imp->environment_variables.end() == i)
    {
        Log::get_instance()->message(ll_debug, lc_no_context, "Environment variable '" + s + "' is unset");
        return "";
    }
    else
    {
        Log::get_instance()->message(ll_debug, lc_no_context, "Environment variable '" + s +
                "' is '" + i->second + "'");
        return i->second;
    }
}

AllDepAtom::Pointer
PortageRepositoryProfile::system_packages() const
{
    return _imp->system_packages;
}

PortageRepositoryProfile::UseExpandIterator
PortageRepositoryProfile::begin_use_expand() const
{
    return UseExpandIterator(_imp->use_expand.begin());
}

PortageRepositoryProfile::UseExpandIterator
PortageRepositoryProfile::end_use_expand() const
{
    return UseExpandIterator(_imp->use_expand.end());
}

PortageRepositoryProfile::UseExpandIterator
PortageRepositoryProfile::begin_use_expand_hidden() const
{
    return UseExpandIterator(_imp->use_expand_hidden.begin());
}

PortageRepositoryProfile::UseExpandIterator
PortageRepositoryProfile::end_use_expand_hidden() const
{
    return UseExpandIterator(_imp->use_expand_hidden.end());
}

PortageRepositoryProfile::VirtualsIterator
PortageRepositoryProfile::begin_virtuals() const
{
    return VirtualsIterator(_imp->virtuals.begin());
}

PortageRepositoryProfile::VirtualsIterator
PortageRepositoryProfile::end_virtuals() const
{
    return VirtualsIterator(_imp->virtuals.end());
}

bool
PortageRepositoryProfile::profile_masked(const QualifiedPackageName & n,
        const VersionSpec & v, const RepositoryName & r) const
{
    PackageMaskMap::const_iterator rr(_imp->package_mask.find(n));
    if (_imp->package_mask.end() == rr)
        return false;
    else
    {
        PackageDatabaseEntry dbe(n, v, r);
        for (std::list<PackageDepAtom::ConstPointer>::const_iterator k(rr->second.begin()),
                k_end(rr->second.end()) ; k != k_end ; ++k)
            if (match_package(_imp->env, **k, dbe))
                return true;
    }

    return false;
}

