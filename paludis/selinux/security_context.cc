/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Stephen Bennett
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License, version 2, as published by the Free Software Foundation.
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

#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/selinux/security_context.hh>

#include "config.h"
#include <stdint.h>
#include <dlfcn.h>

// I think the name explains it. C++ is picky about casting to function pointers.
#define STUPID_CAST(type, val) reinterpret_cast<type>(reinterpret_cast<uintptr_t>(val))

using namespace paludis;

template class Singleton<MatchPathCon>;

namespace
{
    // Declared here to remove dep on <selinux/selinux.h>
    typedef char *security_context_t;

    class LibSELinux
    {
        private:
            void *_handle;

            void (*_freecon)(security_context_t);
            int (*_getcon)(security_context_t*);
            int (*_getfscreatecon)(security_context_t*);
            int (*_setfscreatecon)(security_context_t);
            int (*_matchpathcon)(const char *, mode_t, security_context_t*);
            int (*_matchpathcon_init)(const char *);
            int (*_setfilecon)(const char *, security_context_t);
            int (*_is_selinux_enabled)(void);

        public:
            LibSELinux() :
                _handle(0), _freecon(0), _getcon(0),
                _getfscreatecon(0), _setfscreatecon(0),
                _matchpathcon(0), _matchpathcon_init(0),
                _is_selinux_enabled(0)
            {
                _handle = dlopen("libselinux.so", RTLD_LAZY | RTLD_LOCAL);
                if (0 != _handle)
                {
                    _freecon = STUPID_CAST(void (*)(security_context_t), dlsym(_handle, "freecon"));
                    _getcon = STUPID_CAST(int (*)(security_context_t*), dlsym(_handle, "getcon"));
                    _getfscreatecon = STUPID_CAST(int (*) (security_context_t*),
                            dlsym(_handle, "getfscreatecon"));
                    _setfscreatecon = STUPID_CAST(int (*) (security_context_t),
                            dlsym(_handle, "setfscreatecon"));
                    _matchpathcon = STUPID_CAST(int (*) (const char *, mode_t, security_context_t *),
                            dlsym(_handle, "matchpathcon"));
                    _matchpathcon_init = STUPID_CAST(int (*) (const char *),
                            dlsym(_handle, "matchpathcon_init"));
                    _setfilecon = STUPID_CAST(int (*) (const char *, security_context_t),
                            dlsym(_handle, "lsetfilecon"));
                    _is_selinux_enabled = STUPID_CAST(int (*)(void),
                            dlsym(_handle, "is_selinux_enabled"));
                }
            }

            ~LibSELinux()
            {
                if (0 != _handle)
                    dlclose(_handle);
            }

            void freecon(security_context_t c)
            {
                if (0 != _freecon && is_selinux_enabled())
                    _freecon(c);
            }

            int getcon(security_context_t *c)
            {
                if (0 != _getcon && is_selinux_enabled())
                    return _getcon(c);
                return 0;
            }

            int getfscreatecon(security_context_t *c)
            {
                if (0 != _getfscreatecon && is_selinux_enabled())
                    return _getfscreatecon(c);
                return 0;
            }

            int setfscreatecon(security_context_t c)
            {
                if (0 != _setfscreatecon && is_selinux_enabled())
                    return _setfscreatecon(c);
                return 0;
            }

            int matchpathcon(const char *path, mode_t mode, security_context_t *con)
            {
                if (0 != _matchpathcon && is_selinux_enabled())
                    return _matchpathcon(path, mode, con);
                return 0;
            }

            int setfilecon(const char *path, security_context_t con)
            {
                if (0 != _setfilecon && is_selinux_enabled())
                    return _setfilecon(path, con);
                return 0;
            }

            int matchpathcon_init(const char *path)
            {
                if (0 != _matchpathcon_init && is_selinux_enabled())
                    return _matchpathcon_init(path);
                return 0;
            }

            int is_selinux_enabled()
            {
                // Assume that if this returns an error we can't effectively use selinux.
                if (0 != _is_selinux_enabled)
                    return _is_selinux_enabled() > 0 ? 1 : 0;
                return 0;
            }
    } libselinux;
}

namespace paludis
{
    template<>
    struct Imp<SecurityContext>
    {
        security_context_t _context;

        Imp(security_context_t con)
            : _context(con)
        { }

        ~Imp()
        {
            if (0 != _context)
                libselinux.freecon(_context);
        }

        void set(security_context_t newcon)
        {
            if (0 != _context)
                libselinux.freecon(_context);

            _context = newcon;
        }
    };
}

SecurityContext::SecurityContext() :
    Pimp<SecurityContext>(security_context_t(0))
{
}

SecurityContext::~SecurityContext()
{
}

std::shared_ptr<const SecurityContext> SecurityContext::current_context()
{
    std::shared_ptr<SecurityContext> p(std::make_shared<SecurityContext>());
    security_context_t con;
    if (0 != libselinux.getcon(&con))
        throw SELinuxException("Couldn't get current security context.");
    p->_imp->set(con);
    return p;
}

std::shared_ptr<const SecurityContext> SecurityContext::fs_create_context()
{
    std::shared_ptr<SecurityContext> p(std::make_shared<SecurityContext>());
    security_context_t con;
    if (0 != libselinux.getfscreatecon(&con))
        throw SELinuxException("Couldn't get current filesystem creation context.");
    p->_imp->set(con);
    return p;
}

std::ostream & paludis::operator<<(std::ostream & os, const SecurityContext & context)
{
    os << static_cast<const char *>(context._imp->_context);
    return os;
}

FSCreateCon::FSCreateCon(const std::shared_ptr<const SecurityContext> & newfscreatecon)
    : _context(newfscreatecon), _prev_context(SecurityContext::fs_create_context())
{
    if (0 != libselinux.setfscreatecon(_context->_imp->_context))
        throw SELinuxException("Couldn't set filesystem creation context to '" + stringify(*_context) + "'.");
}

FSCreateCon::~FSCreateCon()
{
    if (0 != libselinux.setfscreatecon(_prev_context->_imp->_context))
        throw SELinuxException("Couldn't reset filesystem creation context to '" + stringify(*_prev_context) + "'.");
}

MatchPathCon::MatchPathCon()
{
    if (0 != libselinux.matchpathcon_init(0))
    {
        _good=false;
//        throw SELinuxException("Failed running matchpathcon_init.");
    }
    else
        _good=true;
}

MatchPathCon::~MatchPathCon()
{
}

bool MatchPathCon::good() const
{
    return _good;
}

std::shared_ptr<const SecurityContext> MatchPathCon::match(const std::string & path, mode_t mode) const
{
    std::shared_ptr<SecurityContext> p(std::make_shared<SecurityContext>());
    security_context_t context;
    if (0 != libselinux.matchpathcon(path.c_str(), mode, &context))
    {
        Log::get_instance()->message("selinux.get_context", ll_warning, lc_no_context) <<
                "Couldn't get default security context for '" << path << "'.";
//        throw SELinuxException("Couldn't get default security context for '" + path + "'.");
    }
    else
    {
        p->_imp->set(context);
    }
    return p;
}

int paludis::setfilecon(const paludis::FSEntry & path, const std::shared_ptr<const SecurityContext> & con)
{
    return libselinux.setfilecon(stringify(path).c_str(), con->_imp->_context);
}

bool paludis::is_selinux_enabled()
{
    return libselinux.is_selinux_enabled() == 1;
}
