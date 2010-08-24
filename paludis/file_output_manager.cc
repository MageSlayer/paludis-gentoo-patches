/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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

#include <paludis/file_output_manager.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/set.hh>
#include <paludis/util/map.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/stringify.hh>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<FileOutputManager>
    {
        FSPath filename;
        std::shared_ptr<SafeOFStream> stream;
        const bool keep_on_success, keep_on_empty;
        const std::shared_ptr<OutputManager> summary_output_manager;
        const std::string summary_output_message;

        bool succeeded, unlinked, nothing_more_to_come;

        Imp(
                const FSPath & o,
                const bool k,
                const bool l,
                const std::shared_ptr<OutputManager> & m,
                const std::string & s
                ) :
            filename(o),
            stream(std::make_shared<SafeOFStream>(filename)),
            keep_on_success(k),
            keep_on_empty(l),
            summary_output_manager(m),
            summary_output_message(s),
            succeeded(false),
            unlinked(false),
            nothing_more_to_come(false)
        {
        }
    };
}

FileOutputManager::FileOutputManager(const FSPath & o, const bool k, const bool l,
        const std::shared_ptr<OutputManager> & m, const std::string & s) :
    Pimp<FileOutputManager>(o, k, l, m, s)
{
}

FileOutputManager::~FileOutputManager()
{
    nothing_more_to_come();

    if (_imp->summary_output_manager)
    {
        if ((! _imp->unlinked) && (! _imp->summary_output_message.empty()))
            _imp->summary_output_manager->message(_imp->succeeded ? mt_info : mt_error,
                    _imp->summary_output_message);
    }
}

std::ostream &
FileOutputManager::stdout_stream()
{
    return *_imp->stream;
}

std::ostream &
FileOutputManager::stderr_stream()
{
    return *_imp->stream;
}

void
FileOutputManager::message(const MessageType, const std::string &)
{
}

void
FileOutputManager::succeeded()
{
    _imp->succeeded = true;

    if (! _imp->keep_on_success)
    {
        _imp->filename.unlink();
        _imp->unlinked = true;
    }
}

void
FileOutputManager::flush()
{
}

bool
FileOutputManager::want_to_flush() const
{
    return _imp->nothing_more_to_come &&
        (! _imp->unlinked) &&
        (! _imp->summary_output_message.empty());
}

void
FileOutputManager::nothing_more_to_come()
{
    _imp->nothing_more_to_come = true;

    if (! _imp->stream)
        return;

    *_imp->stream << std::flush;
    _imp->stream.reset();

    if (! _imp->keep_on_empty)
    {
        FSPath filename_now(stringify(_imp->filename));
        FSStat filename_now_stat(filename_now);
        if (filename_now_stat.exists() && 0 == filename_now_stat.file_size())
        {
            filename_now.unlink();
            _imp->unlinked = true;
        }
    }
}

const std::shared_ptr<const Set<std::string> >
FileOutputManager::factory_managers()
{
    std::shared_ptr<Set<std::string> > result(std::make_shared<Set<std::string>>());
    result->insert("file");
    return result;
}

const std::shared_ptr<OutputManager>
FileOutputManager::factory_create(
        const OutputManagerFactory::KeyFunction & key_func,
        const OutputManagerFactory::CreateChildFunction & create_child_function,
        const OutputManagerFactory::ReplaceVarsFunc & replace_vars_func)
{
    std::string filename_s(key_func("filename")),
        keep_on_success_s(key_func("keep_on_success")),
        keep_on_empty_s(key_func("keep_on_empty")),
        summary_output_manager_s(key_func("summary_output_manager")),
        summary_output_message_s(key_func("summary_output_message"));

    if (filename_s.empty())
        throw ConfigurationError("Key 'filename' not specified when creating a file output manager");
    filename_s = replace_vars_func(filename_s, std::make_shared<Map<std::string, std::string>>());

    if (keep_on_success_s.empty())
        keep_on_success_s = "true";

    if (keep_on_empty_s.empty())
        keep_on_empty_s = "true";

    std::shared_ptr<OutputManager> summary_output_manager;
    if (! summary_output_manager_s.empty())
        summary_output_manager = create_child_function(summary_output_manager_s);

    summary_output_message_s = replace_vars_func(summary_output_message_s, std::make_shared<Map<std::string, std::string>>());

    return std::make_shared<FileOutputManager>(FSPath(filename_s),
                destringify<bool>(keep_on_success_s), destringify<bool>(keep_on_empty_s),
                summary_output_manager, summary_output_message_s);
}

template class Pimp<FileOutputManager>;

