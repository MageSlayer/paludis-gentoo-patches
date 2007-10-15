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

#include "vdb_merger.hh"
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/system.hh>
#include <paludis/util/join.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/strip.hh>
#include <paludis/hook.hh>
#include <paludis/package_id.hh>
#include <paludis/md5.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <list>

using namespace paludis;

#include <paludis/repositories/e/vdb_merger-sr.cc>

namespace paludis
{
    template<>
    struct Implementation<VDBMerger>
    {
        VDBMergerOptions options;
        FSEntry realroot;
        tr1::shared_ptr<std::ofstream> contents_file;

        std::list<std::string> config_protect;
        std::list<std::string> config_protect_mask;

        Implementation(const VDBMergerOptions & o) :
            options(o),
            realroot(options.root.realpath())
        {
            WhitespaceTokeniser::tokenise(o.config_protect,
                    std::back_inserter(config_protect));
            WhitespaceTokeniser::tokenise(o.config_protect_mask,
                    std::back_inserter(config_protect_mask));
        }
    };
}

VDBMerger::VDBMerger(const VDBMergerOptions & o) :
    Merger(MergerOptions::create()
            .environment(o.environment)
            .image(o.image)
            .root(o.root)
            .no_chown(! getenv_with_default("PALUDIS_NO_CHOWN", "").empty())),
    PrivateImplementationPattern<VDBMerger>(new Implementation<VDBMerger>(o))
{
}

VDBMerger::~VDBMerger()
{
}

Hook
VDBMerger::extend_hook(const Hook & h)
{
    tr1::shared_ptr<const FSEntrySequence> bashrc_files(_imp->options.environment->bashrc_files());

    if (_imp->options.package_id)
    {
        std::string cat(stringify(_imp->options.package_id->name().category));
        std::string pn(stringify(_imp->options.package_id->name().package));
        std::string pvr(stringify(_imp->options.package_id->version()));
        std::string pv(stringify(_imp->options.package_id->version().remove_revision()));
        std::string slot(stringify(_imp->options.package_id->slot()));

        return Merger::extend_hook(h)
            ("P", pn + "-" + pv)
            ("PN", pn)
            ("CATEGORY", cat)
            ("PR", _imp->options.package_id->version().revision_only())
            ("PV", pv)
            ("PVR", pvr)
            ("PF", pn + "-" + pvr)
            ("SLOT", slot)
            ("CONFIG_PROTECT", _imp->options.config_protect)
            ("CONFIG_PROTECT_MASK", _imp->options.config_protect_mask)
            ("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "));
    }
    else
        return Merger::extend_hook(h)
            ("CONFIG_PROTECT", _imp->options.config_protect)
            ("CONFIG_PROTECT_MASK", _imp->options.config_protect_mask)
            ("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "));
}

void
VDBMerger::record_install_file(const FSEntry & src, const FSEntry & dst_dir, const std::string & dst_name)
{
    std::string tidy(stringify((dst_dir / dst_name).strip_leading(_imp->realroot))),
            tidy_real(stringify((dst_dir / src.basename()).strip_leading(_imp->realroot)));
    time_t timestamp((dst_dir / dst_name).mtime());

    std::ifstream infile(stringify(FSEntry(dst_dir / dst_name)).c_str());
    if (! infile)
        throw MergerError("Cannot read '" + stringify(FSEntry(dst_dir / dst_name)) + "'");

    MD5 md5(infile);

    std::string line(">>> [obj] " + tidy_real);
    if (tidy_real != tidy)
        line.append(" (" + FSEntry(tidy).basename() + ")");
    display_override(line);

    *_imp->contents_file << "obj " << tidy_real << " " << md5.hexsum() << " " << timestamp << std::endl;
}

void
VDBMerger::record_install_dir(const FSEntry & src, const FSEntry & dst_dir)
{
    std::string tidy(stringify((dst_dir / src.basename()).strip_leading(_imp->realroot)));
    display_override(">>> [dir] " + tidy);

    *_imp->contents_file << "dir " << tidy << std::endl;
}

void
VDBMerger::record_install_sym(const FSEntry & src, const FSEntry & dst_dir)
{
    std::string tidy(stringify((dst_dir / src.basename()).strip_leading(_imp->realroot)));
    std::string target((dst_dir / src.basename()).readlink());
    time_t timestamp((dst_dir / src.basename()).mtime());

    display_override(">>> [sym] " + tidy);

    *_imp->contents_file << "sym " << tidy << " -> " << target << " " << timestamp << std::endl;
}

void
VDBMerger::on_error(bool is_check, const std::string & s)
{
    make_check_fail();

    if (is_check)
        std::cout << "." << std::endl << "!!! " << s << std::endl;
    else
        throw MergerError(s);
}

void
VDBMerger::on_warn(bool is_check, const std::string & s)
{
    if (is_check)
        Log::get_instance()->message(ll_warning, lc_context, s);
}

bool
VDBMerger::config_protected(const FSEntry & src, const FSEntry & dst_dir)
{
    std::string tidy(stringify((dst_dir / src.basename()).strip_leading(_imp->realroot)));

    bool result(false);
    for (std::list<std::string>::const_iterator c(_imp->config_protect.begin()),
            c_end(_imp->config_protect.end()) ; c != c_end && ! result ; ++c)
    {
        std::string cc(strip_trailing(*c, "/") + "/");
        if (tidy == *c || 0 == tidy.compare(0, cc.length(), cc))
            result = true;
    }
    if (result)
        for (std::list<std::string>::const_iterator c(_imp->config_protect_mask.begin()),
                c_end(_imp->config_protect_mask.end()) ; c != c_end && result ; ++c)
        {
            std::string cc(strip_trailing(*c, "/") + "/");
            if (tidy == *c || 0 == tidy.compare(0, cc.length(), cc))
                result = false;
        }

    return result;
}

std::string
VDBMerger::make_config_protect_name(const FSEntry & src, const FSEntry & dst)
{
    std::string result_name(src.basename());
    int n(0);

    std::ifstream our_md5_file(stringify(src).c_str());
    if (! our_md5_file)
        throw MergerError("Could not get md5 for '" + stringify((dst / src.basename()).strip_leading(_imp->realroot)) + "'");
    MD5 our_md5(our_md5_file);

    while (true)
    {
        if (! (dst / result_name).exists())
            break;

        if ((dst / result_name).is_regular_file_or_symlink_to_regular_file())
        {
            std::ifstream other_md5_file(stringify(dst / result_name).c_str());
            if (other_md5_file)
            {
                MD5 other_md5(other_md5_file);
                if (our_md5.hexsum() == other_md5.hexsum())
                    break;
            }
        }

        std::stringstream s;
        s << std::setw(4) << std::setfill('0') << std::right << n++;
        result_name = "._cfg" + s.str() + "_" + src.basename();
    }

    return result_name;
}

void
VDBMerger::merge()
{
    display_override(">>> Merging to " + stringify(_imp->options.root));
    _imp->contents_file.reset(new std::ofstream(stringify(_imp->options.contents_file).c_str()));
    Merger::merge();
}

bool
VDBMerger::check()
{
    std::cout << ">>> Checking whether we can merge to " << _imp->options.root << " ";
    bool result(Merger::check());
    std::cout << std::endl;
    return result;
}

void
VDBMerger::on_enter_dir(bool is_check, const FSEntry)
{
    if (! is_check)
        return;

    std::cout << "." << std::flush;
}

void
VDBMerger::display_override(const std::string & message) const
{
    std::cout << message << std::endl;
}

