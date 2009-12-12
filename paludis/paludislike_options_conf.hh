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

#ifndef PALUDIS_GUARD_PALUDIS_PALUDISLIKE_OPTIONS_CONF_HH
#define PALUDIS_GUARD_PALUDIS_PALUDISLIKE_OPTIONS_CONF_HH 1

#include <paludis/paludislike_options_conf-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/tribool-fwd.hh>
#include <paludis/util/set-fwd.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/config_file-fwd.hh>
#include <paludis/choice-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <tr1/memory>
#include <tr1/functional>

namespace paludis
{
    namespace n
    {
        struct allow_locking;
        struct environment;
        struct make_config_file;
    }

    /**
     * Turn an FSEntry into a config file for PaludisLikeOptionsConf.
     *
     * This might need to deal with weird things like bash config files, so we
     * leave it up to the caller to specify how it works.
     *
     * \since 0.44
     */
    typedef std::tr1::function<
        const std::tr1::shared_ptr<const LineConfigFile> (
                const FSEntry &,
                const LineConfigFileOptions &)
        > PaludisLikeOptionsConfMakeConfigFileFunction;

    /**
     * Options for PaludisLikeOptionsConf.
     *
     * \since 0.44
     */
    struct PaludisLikeOptionsConfParams
    {
        NamedValue<n::allow_locking, bool> allow_locking;
        NamedValue<n::environment, const Environment *> environment;
        NamedValue<n::make_config_file, PaludisLikeOptionsConfMakeConfigFileFunction> make_config_file;
    };

    /**
     * Common helper class for a Paludis-format use.conf or options.conf.
     *
     * The format is shared closely between PaludisEnvironment and the Exheres
     * profile format for E repositories.
     *
     * \since 0.44
     */
    class PALUDIS_VISIBLE PaludisLikeOptionsConf :
        private PrivateImplementationPattern<PaludisLikeOptionsConf>
    {
        public:
            PaludisLikeOptionsConf(const PaludisLikeOptionsConfParams &);
            ~PaludisLikeOptionsConf();

            void add_file(const FSEntry &);

            const std::pair<Tribool, bool> want_choice_enabled_locked(
                    const std::tr1::shared_ptr<const PackageID> &,
                    const ChoicePrefixName &,
                    const UnprefixedChoiceName &
                    ) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::string value_for_choice_parameter(
                    const std::tr1::shared_ptr<const PackageID> &,
                    const ChoicePrefixName &,
                    const UnprefixedChoiceName &
                    ) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::tr1::shared_ptr<const Set<UnprefixedChoiceName> > known_choice_value_names(
                    const std::tr1::shared_ptr<const PackageID> &,
                    const ChoicePrefixName &
                    ) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<PaludisLikeOptionsConf>;
#endif

}

#endif
