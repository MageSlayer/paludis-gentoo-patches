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

#include "colour_formatter.hh"
#include "colour.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/set.hh>
#include <paludis/name.hh>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<ColourFormatter>
    {
        mutable std::string active_prefix;

        const tr1::shared_ptr<UseFlagNameSet> seen_new_use_flag_names;
        const tr1::shared_ptr<UseFlagNameSet> seen_changed_use_flag_names;
        const tr1::shared_ptr<UseFlagNameSet> seen_use_flag_names;
        const tr1::shared_ptr<UseFlagNameSet> seen_use_expand_prefixes;

        Implementation() :
            seen_new_use_flag_names(new UseFlagNameSet),
            seen_changed_use_flag_names(new UseFlagNameSet),
            seen_use_flag_names(new UseFlagNameSet),
            seen_use_expand_prefixes(new UseFlagNameSet)
        {
        }
    };
}

ColourFormatter::ColourFormatter() :
    PrivateImplementationPattern<ColourFormatter>(new Implementation<ColourFormatter>)
{
}

ColourFormatter::~ColourFormatter()
{
}

const tr1::shared_ptr<const UseFlagNameSet>
ColourFormatter::seen_new_use_flag_names() const
{
    return _imp->seen_new_use_flag_names;
}

const tr1::shared_ptr<const UseFlagNameSet>
ColourFormatter::seen_changed_use_flag_names() const
{
    return _imp->seen_changed_use_flag_names;
}

const tr1::shared_ptr<const UseFlagNameSet>
ColourFormatter::seen_use_flag_names() const
{
    return _imp->seen_use_flag_names;
}

const tr1::shared_ptr<const UseFlagNameSet>
ColourFormatter::seen_use_expand_prefixes() const
{
    return _imp->seen_use_expand_prefixes;
}

std::string
ColourFormatter::format(const IUseFlag & f, const format::Plain &) const
{
    _imp->seen_use_flag_names->insert(f.flag);
    std::string g(stringify(f.flag)), h;

    if (std::string::npos != f.prefix_delim_pos)
    {
        std::string p(g.substr(0, f.prefix_delim_pos));
        if (_imp->active_prefix == p)
            g.erase(0, f.prefix_delim_pos + 1);
        else
        {
            _imp->active_prefix = p;
            _imp->seen_use_expand_prefixes->insert(UseFlagName(_imp->active_prefix));
            h = _imp->active_prefix + ": ";
            g.erase(0, f.prefix_delim_pos + 1);
        }
    }

    return h + g;
}

std::string
ColourFormatter::format(const IUseFlag & f, const format::Enabled &) const
{
    _imp->seen_use_flag_names->insert(f.flag);
    std::string g(stringify(f.flag)), h;

    if (std::string::npos != f.prefix_delim_pos)
    {
        std::string p(g.substr(0, f.prefix_delim_pos));
        if (_imp->active_prefix == p)
            g.erase(0, f.prefix_delim_pos + 1);
        else
        {
            _imp->active_prefix = p;
            _imp->seen_use_expand_prefixes->insert(UseFlagName(_imp->active_prefix));
            h = _imp->active_prefix + ": ";
            g.erase(0, f.prefix_delim_pos + 1);
        }
    }

    return h + colour(cl_flag_on, g);
}

std::string
ColourFormatter::format(const IUseFlag & f, const format::Disabled &) const
{
    _imp->seen_use_flag_names->insert(f.flag);
    std::string g(stringify(f.flag)), h;

    if (std::string::npos != f.prefix_delim_pos)
    {
        std::string p(g.substr(0, f.prefix_delim_pos));
        if (_imp->active_prefix == p)
            g.erase(0, f.prefix_delim_pos + 1);
        else
        {
            _imp->active_prefix = p;
            _imp->seen_use_expand_prefixes->insert(UseFlagName(_imp->active_prefix));
            h = _imp->active_prefix + ": ";
            g.erase(0, f.prefix_delim_pos + 1);
        }
    }

    return h + colour(cl_flag_off, "-" + g);
}

std::string
ColourFormatter::format(const IUseFlag & f, const format::Forced &) const
{
    _imp->seen_use_flag_names->insert(f.flag);
    std::string g(stringify(f.flag)), h;

    if (std::string::npos != f.prefix_delim_pos)
    {
        std::string p(g.substr(0, f.prefix_delim_pos));
        if (_imp->active_prefix == p)
            g.erase(0, f.prefix_delim_pos + 1);
        else
        {
            _imp->active_prefix = p;
            _imp->seen_use_expand_prefixes->insert(UseFlagName(_imp->active_prefix));
            h = _imp->active_prefix + ": ";
            g.erase(0, f.prefix_delim_pos + 1);
        }
    }

    return h + colour(cl_flag_on, "(" + g + ")");
}

std::string
ColourFormatter::format(const IUseFlag & f, const format::Masked &) const
{
    _imp->seen_use_flag_names->insert(f.flag);
    std::string g(stringify(f.flag)), h;

    if (std::string::npos != f.prefix_delim_pos)
    {
        std::string p(g.substr(0, f.prefix_delim_pos));
        if (_imp->active_prefix == p)
            g.erase(0, f.prefix_delim_pos + 1);
        else
        {
            _imp->active_prefix = p;
            _imp->seen_use_expand_prefixes->insert(UseFlagName(_imp->active_prefix));
            h = _imp->active_prefix + ": ";
            g.erase(0, f.prefix_delim_pos + 1);
        }
    }

    return h + colour(cl_flag_off, "(-" + g + ")");
}

std::string
ColourFormatter::decorate(const IUseFlag & i, const std::string & f, const format::Added &) const
{
    _imp->seen_new_use_flag_names->insert(i.flag);
    return f + "+";
}

std::string
ColourFormatter::decorate(const IUseFlag & i, const std::string & f, const format::Changed &) const
{
    _imp->seen_changed_use_flag_names->insert(i.flag);
    return f + "*";
}

std::string
ColourFormatter::format(const UseFlagName & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const UseFlagName & f, const format::Enabled &) const
{
    return colour(cl_flag_on, f);
}

std::string
ColourFormatter::format(const UseFlagName & f, const format::Disabled &) const
{
    return colour(cl_flag_off, "-" + stringify(f));
}

std::string
ColourFormatter::format(const UseFlagName & f, const format::Forced &) const
{
    return colour(cl_flag_on, "(" + stringify(f) + ")");
}

std::string
ColourFormatter::format(const UseFlagName & f, const format::Masked &) const
{
    return colour(cl_flag_off, "(-" + stringify(f) + ")");
}

std::string
ColourFormatter::format(const UseDepSpec & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const UseDepSpec & f, const format::Enabled &) const
{
    return colour(cl_flag_on, f);
}

std::string
ColourFormatter::format(const UseDepSpec & f, const format::Disabled &) const
{
    return colour(cl_flag_off, f);
}

std::string
ColourFormatter::format(const UseDepSpec & f, const format::Forced &) const
{
    return colour(cl_flag_on, "(" + stringify(f) + ")");
}

std::string
ColourFormatter::format(const UseDepSpec & f, const format::Masked &) const
{
    return colour(cl_flag_off, "(" + stringify(f) + ")");
}

std::string
ColourFormatter::format(const PackageDepSpec & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const PackageDepSpec & f, const format::Installed &) const
{
    return colour(cl_package_name, f);
}

std::string
ColourFormatter::format(const PackageDepSpec & f, const format::Installable &) const
{
    return colour(cl_installable_package_name, f);
}

std::string
ColourFormatter::format(const PlainTextDepSpec & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const PlainTextDepSpec & f, const format::Accepted &) const
{
    return colour(cl_flag_on, f);
}

std::string
ColourFormatter::format(const PlainTextDepSpec & f, const format::Unaccepted &) const
{
    return colour(cl_flag_off, f);
}

std::string
ColourFormatter::format(const KeywordName & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const KeywordName & f, const format::Accepted &) const
{
    return colour(cl_flag_on, f);
}

std::string
ColourFormatter::format(const KeywordName & f, const format::Unaccepted &) const
{
    return colour(cl_flag_off, f);
}

std::string
ColourFormatter::format(const std::string & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const LabelsDepSpec<URILabelVisitorTypes> & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const LabelsDepSpec<DependencyLabelVisitorTypes> & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const URIDepSpec & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const BlockDepSpec & f, const format::Plain &) const
{
    return stringify(f);
}

std::string
ColourFormatter::format(const tr1::shared_ptr<const PackageID> & f, const format::Plain &) const
{
    return stringify(*f);
}

std::string
ColourFormatter::format(const tr1::shared_ptr<const PackageID> & f, const format::Installed &) const
{
    return colour(cl_package_name, *f);
}

std::string
ColourFormatter::format(const tr1::shared_ptr<const PackageID> & f, const format::Installable &) const
{
    return colour(cl_installable_package_name, *f);
}

std::string
ColourFormatter::newline() const
{
    return "\n";
}

std::string
ColourFormatter::indent(const int i) const
{
    return std::string(12 + (4 * i), ' ');
}

