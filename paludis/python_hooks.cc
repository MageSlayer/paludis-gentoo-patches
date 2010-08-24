/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include <boost/python.hpp>

#include <paludis/hooker.hh>
#include <paludis/hook.hh>
#include <paludis/environment.hh>
#include <paludis/util/graph-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/system.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_named_values.hh>

#include <set>

using namespace paludis;
namespace bp = boost::python;

extern "C"
{
    std::shared_ptr<HookFile> create_py_hook_file(const FSPath &, const bool, const Environment * const)
        PALUDIS_VISIBLE;
}

namespace
{
    class PyHookFile :
        public HookFile
    {
        private:
            static Mutex _mutex;

            static bp::dict _local_namespace_base;
            static bp::dict _output_wrapper_namespace;
            static bp::object _format_exception;

            const FSPath _file_name;
            const Environment * const _env;
            const bool _run_prefixed;
            bool _loaded;
            bp::dict _local_namespace;

            friend class Prefix;

            class Prefix
            {
                private:
                    const PyHookFile * const phf;

                public:
                    Prefix(const PyHookFile * const, const std::string &);
                    ~Prefix();
            };

            std::string _get_traceback() const;

            void _add_dependency_class(const Hook &, DirectedGraph<std::string, int> &, bool);

        public:
            PyHookFile(const FSPath &, const bool, const Environment * const);

            virtual HookResult run(const Hook &, const std::shared_ptr<OutputManager> &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const FSPath file_name() const
            {
                return _file_name;
            }

            virtual void add_dependencies(const Hook &, DirectedGraph<std::string, int> &);

            virtual const std::shared_ptr<const Sequence<std::string> > auto_hook_names() const
            {
                return std::make_shared<Sequence<std::string>>();
            }
    };

    Mutex PyHookFile::_mutex;
    bp::dict PyHookFile::_output_wrapper_namespace;
    bp::dict PyHookFile::_local_namespace_base;
    bp::object PyHookFile::_format_exception;
}

PyHookFile::PyHookFile(const FSPath & f, const bool r, const Environment * const e) :
    _file_name(f),
    _env(e),
    _run_prefixed(r),
    _loaded(false)
{
    Lock l(_mutex);

    static bool initialized(false);

    if (! initialized)
    {
        initialized = true;
        try
        {
            Py_Initialize();

            bp::object main = bp::import("__main__");
            bp::object global_namespace = main.attr("__dict__");

            _local_namespace_base["__builtins__"] = global_namespace["__builtins__"];
            _output_wrapper_namespace = _local_namespace_base.copy();

            bp::object traceback = bp::import("traceback");
            bp::object traceback_namespace = traceback.attr("__dict__");
            _format_exception = traceback_namespace["format_exception"];

            bp::exec_file(
                    (getenv_with_default("PALUDIS_PYTHON_DIR", PYTHONINSTALLDIR)
                     + "/paludis_output_wrapper.py").c_str(),
                    _output_wrapper_namespace, _output_wrapper_namespace);
            bp::import("paludis");
        }
        catch (const bp::error_already_set &)
        {
            Log::get_instance()->message("hook.python.interpreter_failure", ll_warning, lc_no_context)
                << "Initializing Python interpreter failed:";
            PyErr_Print();
            return;
        }
        catch (const std::exception & ex)
        {
            Log::get_instance()->message("hook.python.interpreter_failure", ll_warning, lc_no_context)
                << "Initializing Python interpreter failed: '" << ex.what() << "'";
            return;
        }
    }
    _local_namespace = _local_namespace_base.copy();

    try
    {
        bp::exec_file(stringify(f).c_str(), _local_namespace, _local_namespace);
        _loaded = true;
    }
    catch (const bp::error_already_set &)
    {
        Log::get_instance()->message("hook.python.failure", ll_warning, lc_no_context)
            << "Loading hook '" << f << "' failed: '" << _get_traceback() << "'";
    }
    catch (const std::exception & ex)
    {
        Log::get_instance()->message("hook.python.failure", ll_warning, lc_no_context)
            << "Loading hook '" << f << "' failed: '" << ex.what() << "'";
    }
}

HookResult
PyHookFile::run(const Hook & hook, const std::shared_ptr<OutputManager> &) const
{
    Context c("When running hook '" + stringify(file_name()) + "' for hook '" + hook.name() + "':");

    if (! _loaded)
        return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");

    Lock l(_mutex);

    bp::object _run;

    try
    {
        _run = _local_namespace["hook_run_" + hook.name()];
    }
    catch (const bp::error_already_set &)
    {
        if (PyErr_ExceptionMatches(PyExc_KeyError))
        {
            Log::get_instance()->message("hook.python.undefined", ll_warning, lc_no_context)
                << "Hook '" << file_name() << "' does not define the hook_run_"
                << hook.name() << " function";
            PyErr_Clear();
        }
        else
        {
            Log::get_instance()->message("hook.python.failure", ll_warning, lc_no_context) <<
                "Hook '" << file_name() << "' failed unexpectedly: '"
                << _get_traceback() << "'";
        }

        return make_named_values<HookResult>(n::max_exit_status() = 1, n::output() = "");
    }

    Prefix p(this, _run_prefixed ? strip_trailing_string(file_name().basename(), ".py") + "> " : "");

    Log::get_instance()->message("hook.python.starting", ll_debug, lc_no_context)
        << "Starting hook '" << file_name() << "' for '" << hook.name() << "'";

    bp::dict hook_env;
    hook_env["HOOK"] = hook.name();
    hook_env["HOOK_FILE"] = stringify(file_name());

    for (Hook::ConstIterator x(hook.begin()), x_end(hook.end()) ; x != x_end ; ++x)
        hook_env[x->first] = x->second;

    bp::object result;
    try
    {
        result = _run(bp::ptr(_env), hook_env);
    }
    catch (const bp::error_already_set &)
    {
        Log::get_instance()->message("hook.python.failure", ll_warning, lc_no_context) <<
            "Hook '" << file_name() << "': running hook_run_" << hook.name()
            << " function failed: '" << _get_traceback() << "'";
        return make_named_values<HookResult>(n::max_exit_status() = 1, n::output() = "");
    }

    if (hook.output_dest == hod_grab)
    {
        if (bp::extract<std::string>(result).check())
        {
            std::string result_s = bp::extract<std::string>(result);

            Log::get_instance()->message("hook.python.output", ll_debug, lc_no_context)
                << "Hook '" << file_name() << "':  hook_run_" << hook.name()
                << " function returned '" << result_s << "'";

            return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = result_s);
        }
        else
        {
            Log::get_instance()->message("hook.python.bad_output", ll_warning, lc_no_context)
                << "Hook '" << file_name() << "':  hook_run_" << hook.name()
                << " function returned not a string.";
            return make_named_values<HookResult>(n::max_exit_status() = 1, n::output() = "");
        }
    }
    else
        return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
}

void
PyHookFile::add_dependencies(const Hook & hook, DirectedGraph<std::string, int> & g)
{
    Context c("When finding dependencies of hook script '" + stringify(file_name())
            + "' for hook '" + hook.name() + "':");

    if (! _loaded)
        return;

    Lock l(_mutex);

    Prefix p(this, _run_prefixed ? strip_trailing_string(file_name().basename(), ".py") + "> " : "");

    _add_dependency_class(hook, g, false);
    _add_dependency_class(hook, g, true);
}

void
PyHookFile::_add_dependency_class(const Hook & hook, DirectedGraph<std::string, int> & g, bool depend)
{
    Context context("When adding dependency class '" + stringify(depend ? "depend" : "after") + "' for hook '"
            + stringify(hook.name()) + "' file '" + stringify(file_name()) + "':");

    bp::object _run;
    try
    {
        _run = _local_namespace["hook_" + stringify(depend ? "depend" : "after") + "_" + hook.name()];
    }
    catch (const bp::error_already_set &)
    {
        if (PyErr_ExceptionMatches(PyExc_KeyError))
        {
            Log::get_instance()->message("hook.python.undefined", ll_debug, lc_no_context)
                << "Hook '" << file_name() << "' does not define the hook_"
                << (depend ? "depend" : "after") << "_" << hook.name() << " function";
            PyErr_Clear();
        }
        else
        {
            Log::get_instance()->message("hook.python.failure", ll_warning, lc_no_context)
                << "Hook '" << file_name() << "' failed unexpectedly: '" << _get_traceback() << "'";
        }

        return;
    }

    bp::dict hook_env;
    hook_env["HOOK"] = hook.name();
    hook_env["HOOK_FILE"] = stringify(file_name());

    for (Hook::ConstIterator x(hook.begin()), x_end(hook.end()) ; x != x_end ; ++x)
        hook_env[x->first] = x->second;

    bp::object result;
    try
    {
        result = _run(hook_env);
    }
    catch (const bp::error_already_set &)
    {
        Log::get_instance()->message("hook.python.failure", ll_warning, lc_no_context) <<
                "Hook '" << file_name() << "': running hook_"
                << (depend ? "depend" : "after") << "_" << hook.name() << " function failed: '"
                << _get_traceback() << "'";
        return;
    }

    bp::str py_deps(result);
    std::string deps = bp::extract<std::string>(py_deps);

    std::set<std::string> deps_s;

    if (bp::extract<bp::list>(result).check())
    {
        bp::list l = bp::extract<bp::list>(result);

        while (PyList_Size(l.ptr()))
        {
            bp::object o(l.pop());
            if (bp::extract<std::string>(o).check())
            {
                deps_s.insert(bp::extract<std::string>(o));
            }
            else
            {
                Log::get_instance()->message("hook.python.bad_output", ll_warning, lc_no_context)
                    << "Hook '" << file_name() << "':  hook_"
                    << stringify(depend ? "depend" : "after") << "_" << hook.name()
                    << " function returned not a list of strings: '" << deps << "'";
                return;
            }
        }
    }
    else
    {
        Log::get_instance()->message("hook.python.bad_output", ll_warning, lc_no_context)
            << "Hook '" << file_name() << "':  hook_"
            << (depend ? "depend" : "after") << "_" << hook.name()
            << " function returned not a list: '" << deps << "'";
        return;
    }

    Log::get_instance()->message("hook.python.output", ll_debug, lc_no_context)
        << "Hook '" << file_name() <<  "':  hook_"
        << stringify(depend ? "depend" : "after") << "_" << hook.name() << " function returned '" << deps << "'";

    for (std::set<std::string>::const_iterator d(deps_s.begin()), d_end(deps_s.end()) ;
            d != d_end ; ++d)
    {
        if (g.has_node(*d))
            g.add_edge(strip_trailing_string(file_name().basename(), ".py"), *d, 0);
        else if (depend)
            Log::get_instance()->message("hook.python.dependency_not_found", ll_warning, lc_context)
                << "Hook dependency '" << *d << "' for '" << file_name() << "' not found";
        else
            Log::get_instance()->message("hook.python.after_not_found", ll_debug, lc_context)
                << "Hook after '" << *d << "' for '" << file_name() << "' not found";
    }
}

PyHookFile::Prefix::Prefix(const PyHookFile * const f, const std::string & prefix):
    phf(f)
{
    try
    {
        phf->_output_wrapper_namespace["set_prefix"](prefix);
    }
    catch (const bp::error_already_set &)
    {
        Log::get_instance()->message("hook.python.failure", ll_warning, lc_no_context)
            << "Hook '" << phf->file_name() << "' failed unexpectedly: '"
            << phf->_get_traceback() << "'";
    }
}

PyHookFile::Prefix::~Prefix()
{
    try
    {
        phf->_output_wrapper_namespace["restore_prefix"]();
    }
    catch (const bp::error_already_set &)
    {
        Log::get_instance()->message("hook.python.failure", ll_warning, lc_no_context)
            << "Hook '" << phf->file_name() << "' failed unexpectedly: '"
            << phf->_get_traceback() << "'";
    }
}

std::string
PyHookFile::_get_traceback() const
{
    if (! PyErr_Occurred())
        return "";

    Context c("When getting traceback");

    PyObject * ptype, * pvalue, * ptraceback;
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);

    if (ptype == NULL)
        ptype = Py_None;
    if (pvalue == NULL)
        pvalue = Py_None;
    if (ptraceback == NULL)
        ptraceback = Py_None;

    PyObject * result(PyObject_CallFunctionObjArgs(_format_exception.ptr(), ptype, pvalue, ptraceback, NULL));

    if (result == NULL)
    {
        Log::get_instance()->message("hook.python.traceback_failed", ll_warning, lc_context) <<
            "Hook '" << file_name() << "': _get_traceback(): traceback.format_exception failed";
        return "Getting traceback failed";
    }

    bp::list l;
    if (bp::extract<bp::list>(result).check())
    {
        l = bp::extract<bp::list>(result);
    }
    else
    {
        Log::get_instance()->message("hook.python.traceback_failed", ll_warning, lc_context) <<
            "Hook '" << file_name() << "': _get_traceback(): cannot extract list of lines";
        return "Getting traceback failed";
    }

    bp::str result_str;
    try
    {
        result_str = result_str.join(l);
    }
    catch (const bp::error_already_set &)
    {
        Log::get_instance()->message("hook.python.traceback_failed", ll_warning, lc_context) <<
            "Hook '" << file_name() << "': _get_traceback(): joining list of lines failed";
        return "Getting traceback failed";
    }

    return strip_trailing(bp::extract<std::string>(result_str)(), "\n");
}

std::shared_ptr<HookFile>
create_py_hook_file(const FSPath & f, const bool b, const Environment * const e)
{
    return std::make_shared<PyHookFile>(f, b, e);
}
