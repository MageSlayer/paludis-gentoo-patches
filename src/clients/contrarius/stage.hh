/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
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

#ifndef PALUDIS_GUARD_SRC_CONTRARIUS_STAGE_HH
#define PALUDIS_GUARD_SRC_CONTRARIUS_STAGE_HH 1

#include <paludis/name.hh>
#include <paludis/host_tuple_name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/tasks/stage_builder_task.hh>

#include <string>

namespace paludis
{
    class AdaptedEnvironment;

#include <src/clients/contrarius/contrarius_stage_options-sr.hh>

    class ContrariusStage :
        public StageBase
    {
        protected:
            std::tr1::shared_ptr<AdaptedEnvironment> _env;

        public:
            ContrariusStage(std::tr1::shared_ptr<AdaptedEnvironment> e) :
                _env(e)
            {
            }
    };

    class AuxiliaryStage :
        public ContrariusStage
    {
        public:
            AuxiliaryStage(std::tr1::shared_ptr<AdaptedEnvironment> e) :
                ContrariusStage(e)
            {
            }

            virtual int build(const StageOptions &) const;

            virtual std::string description() const
            {
                return "Building auxiliary dependencies of the cross toolchain";
            }

            virtual bool is_rebuild() const;

            virtual std::string short_name() const
            {
                return "cross-auxiliary stage";
            }
    };

    class BinutilsStage :
        public ContrariusStage
    {
        public:
            BinutilsStage(std::tr1::shared_ptr<AdaptedEnvironment> e) :
                ContrariusStage(e)
            {
            }

            virtual int build(const StageOptions &) const;

            virtual std::string description() const
            {
                return "Building the GNU binutils as part of the cross toolchain";
            }

            virtual bool is_rebuild() const;

            virtual std::string short_name() const
            {
                return "cross-binutils stage";
            }
    };

    class KernelHeadersStage :
        public ContrariusStage
    {
        public:
            KernelHeadersStage(std::tr1::shared_ptr<AdaptedEnvironment> e) :
                ContrariusStage(e)
            {
            }

            virtual int build(const StageOptions &) const;

            virtual std::string description() const
            {
                return "Building the kernel headers as part of the cross toolchain";
            }

            virtual bool is_rebuild() const;

            virtual std::string short_name() const
            {
                return "cross-kernel-headers stage";
            }
    };

    class MinimalStage :
        public ContrariusStage
    {
        public:
            MinimalStage(std::tr1::shared_ptr<AdaptedEnvironment> e) :
                ContrariusStage(e)
            {
            }

            virtual int build(const StageOptions &) const;

            virtual std::string description() const
            {
                return "Building a minimal GNU C compiler as part of the cross toolchain";
            }

            virtual bool is_rebuild() const;

            virtual std::string short_name() const
            {
                return "cross-minimal-gcc stage";
            }
    };

    class LibCHeadersStage :
        public ContrariusStage
    {
        public:
            LibCHeadersStage(std::tr1::shared_ptr<AdaptedEnvironment> e) :
                ContrariusStage(e)
            {
            }

            virtual int build(const StageOptions &) const;

            virtual std::string description() const
            {
                return "Building the C standard library headers as part of the cross toolchain";
            }

            virtual bool is_rebuild() const;

            virtual std::string short_name() const
            {
                return "cross-libc-headers stage";
            }
    };

    class LibCStage :
        public ContrariusStage
    {
        public:
            LibCStage(std::tr1::shared_ptr<AdaptedEnvironment> e) :
                ContrariusStage(e)
            {
            }

            virtual int build(const StageOptions &) const;

            virtual std::string description() const
            {
                return "Building the C standard library as part of the cross toolchain";
            }

            virtual bool is_rebuild() const;

            virtual std::string short_name() const
            {
                return "cross-libc stage";
            }
    };


    class FullStage :
        public ContrariusStage
    {
        public:
            FullStage(std::tr1::shared_ptr<AdaptedEnvironment> e) :
                ContrariusStage(e)
            {
            }

            virtual int build(const StageOptions &) const;

            virtual std::string description() const
            {
                return "Building the full GNU compiler collection as part of the cross toolchain";
            }

            virtual bool is_rebuild() const;

            virtual std::string short_name() const
            {
                return "cross-gcc stage";
            }
    };
}

#endif
