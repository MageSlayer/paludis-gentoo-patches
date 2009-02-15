/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/set.hh>
#include <paludis/util/map.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<FileOutputManager>
    {
        FSEntry stdout_file;
        FSEntry stderr_file;
        std::tr1::shared_ptr<SafeOFStream> stdout_stream;
        std::tr1::shared_ptr<SafeOFStream> stderr_stream;
        const bool keep_on_success, keep_on_empty;
        const std::tr1::shared_ptr<OutputManager> summary_output_manager;
        const std::string summary_output_stdout_message;
        const std::string summary_output_stderr_message;

        bool succeeded, unlinked_stdout, unlinked_stderr;

        Implementation(
                const FSEntry & o,
                const FSEntry & e,
                const bool k,
                const bool l,
                const std::tr1::shared_ptr<OutputManager> & m,
                const std::string & s,
                const std::string & t
                ) :
            stdout_file(o),
            stderr_file(e),
            stdout_stream(new SafeOFStream(o)),
            keep_on_success(k),
            keep_on_empty(l),
            summary_output_manager(m),
            summary_output_stdout_message(s),
            summary_output_stderr_message(t),
            succeeded(false),
            unlinked_stdout(false),
            unlinked_stderr(false)
        {
            if (o == e)
                stderr_stream = stdout_stream;
            else
                stderr_stream.reset(new SafeOFStream(e));
        }
    };
}

FileOutputManager::FileOutputManager(const FSEntry & o, const FSEntry & e, const bool k, const bool l,
        const std::tr1::shared_ptr<OutputManager> & m, const std::string & s, const std::string & t) :
    PrivateImplementationPattern<FileOutputManager>(new Implementation<FileOutputManager>(o, e, k, l, m, s, t))
{
}

FileOutputManager::~FileOutputManager()
{
    if (! _imp->keep_on_empty)
    {
        *_imp->stdout_stream << std::flush;
        *_imp->stderr_stream << std::flush;

        FSEntry stdout_file_now(stringify(_imp->stdout_file)), stderr_file_now(stringify(_imp->stderr_file));
        if (stdout_file_now.exists() && 0 == stdout_file_now.file_size())
        {
            _imp->stdout_file.unlink();
            _imp->unlinked_stdout = true;
        }

        if (stdout_file_now != stderr_file_now)
        {
            if (stderr_file_now.exists() && 0 == stderr_file_now.file_size())
            {
                _imp->stderr_file.unlink();
                _imp->unlinked_stderr = true;
            }
        }
    }

    if (_imp->summary_output_manager)
    {
        if ((! _imp->unlinked_stdout) && (! _imp->summary_output_stdout_message.empty()))
        {
            _imp->summary_output_manager->stdout_stream()
                << _imp->summary_output_stdout_message
                << std::endl;
        }

        if (_imp->stdout_file != _imp->stderr_file)
        {
            if ((! _imp->unlinked_stderr) && (! _imp->summary_output_stderr_message.empty()))
                _imp->summary_output_manager->stdout_stream()
                    << _imp->summary_output_stderr_message
                    << std::endl;
        }
    }
}

std::ostream &
FileOutputManager::stdout_stream()
{
    return *_imp->stdout_stream;
}

std::ostream &
FileOutputManager::stderr_stream()
{
    return *_imp->stderr_stream;
}

void
FileOutputManager::succeeded()
{
    _imp->succeeded = true;

    if (! _imp->keep_on_success)
    {
        _imp->stdout_file.unlink();
        _imp->stderr_file.unlink();
        _imp->unlinked_stdout = true;
        _imp->unlinked_stderr = true;
    }
}

void
FileOutputManager::message(const MessageType, const std::string &)
{
}

const std::tr1::shared_ptr<const Set<std::string> >
FileOutputManager::factory_managers()
{
    std::tr1::shared_ptr<Set<std::string> > result(new Set<std::string>);
    result->insert("file");
    return result;
}

const std::tr1::shared_ptr<OutputManager>
FileOutputManager::factory_create(
        const OutputManagerFactory::KeyFunction & key_func,
        const OutputManagerFactory::CreateChildFunction & create_child_function,
        const OutputManagerFactory::ReplaceVarsFunc & replace_vars_func)
{
    std::string stdout_s(key_func("stdout")), stderr_s(key_func("stderr")),
        keep_on_success_s(key_func("keep_on_success")), keep_on_empty_s(key_func("keep_on_empty")),
        summary_output_manager_s(key_func("summary_output_manager")),
        summary_output_stdout_message_s(key_func("summary_output_stdout_message")),
        summary_output_stderr_message_s(key_func("summary_output_stderr_message"));

    if (stdout_s.empty())
        throw ConfigurationError("Key 'stdout' not specified when creating a file output manager");
    stdout_s = replace_vars_func(stdout_s, make_shared_ptr(new Map<std::string, std::string>));

    if (stderr_s.empty())
        throw ConfigurationError("Key 'stderr' not specified when creating a file output manager");
    stderr_s = replace_vars_func(stderr_s, make_shared_ptr(new Map<std::string, std::string>));

    if (keep_on_success_s.empty())
        keep_on_success_s = "true";

    if (keep_on_empty_s.empty())
        keep_on_empty_s = "true";

    std::tr1::shared_ptr<OutputManager> summary_output_manager;
    if (! summary_output_manager_s.empty())
        summary_output_manager = create_child_function(summary_output_manager_s);

    summary_output_stdout_message_s = replace_vars_func(summary_output_stdout_message_s, make_shared_ptr(new Map<std::string, std::string>));
    summary_output_stderr_message_s = replace_vars_func(summary_output_stderr_message_s, make_shared_ptr(new Map<std::string, std::string>));

    return make_shared_ptr(new FileOutputManager(FSEntry(stdout_s), FSEntry(stderr_s),
                destringify<bool>(keep_on_success_s), destringify<bool>(keep_on_empty_s),
                summary_output_manager, summary_output_stdout_message_s,
                summary_output_stderr_message_s));
}

template class PrivateImplementationPattern<FileOutputManager>;

