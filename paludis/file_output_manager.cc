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
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/fs_entry.hh>

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
        const bool keep_on_success;

        Implementation(
                const FSEntry & o,
                const FSEntry & e,
                const bool k
                ) :
            stdout_file(o),
            stderr_file(e),
            stdout_stream(new SafeOFStream(o)),
            keep_on_success(k)
        {
            if (o == e)
                stderr_stream = stdout_stream;
            else
                stderr_stream.reset(new SafeOFStream(e));
        }
    };
}

FileOutputManager::FileOutputManager(const FSEntry & o, const FSEntry & e, const bool k) :
    PrivateImplementationPattern<FileOutputManager>(new Implementation<FileOutputManager>(o, e, k))
{
}

FileOutputManager::~FileOutputManager()
{
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
    if (! _imp->keep_on_success)
    {
        _imp->stdout_file.unlink();
        _imp->stderr_file.unlink();
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
        const OutputManagerFactory::CreateChildFunction &,
        const OutputManagerFactory::ReplaceVarsFunc & replace_vars_func)
{
    std::string o_s(key_func("stdout")), e_s(key_func("stderr")), k_s(key_func("keep_on_success"));

    if (o_s.empty())
        throw ConfigurationError("Key 'stdout' not specified when creating a file output manager");
    o_s = replace_vars_func(o_s);

    if (e_s.empty())
        throw ConfigurationError("Key 'stderr' not specified when creating a file output manager");
    e_s = replace_vars_func(e_s);

    if (k_s.empty())
        throw ConfigurationError("Key 'keep_on_success' not specified when creating a file output manager");

    return make_shared_ptr(new FileOutputManager(FSEntry(o_s), FSEntry(e_s), destringify<bool>(k_s)));
}

template class PrivateImplementationPattern<FileOutputManager>;

