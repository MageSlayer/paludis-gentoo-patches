/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_KEYS_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_KEYS_HH 1

#include <paludis/util/kc-fwd.hh>

namespace paludis
{
    namespace k
    {
        typedef kc::Key<1> fetch_unneeded;
        typedef kc::Key<2> safe_resume;
        typedef kc::Key<3> no_config_protect;
        typedef kc::Key<4> debug_build;
        typedef kc::Key<5> checks;
        typedef kc::Key<6> destination;
        typedef kc::Key<7> visible;
        typedef kc::Key<8> target_file;
        typedef kc::Key<9> requires_manual_fetching;
        typedef kc::Key<10> failed_automatic_fetching;
        typedef kc::Key<11> failed_integrity_checks;
        typedef kc::Key<12> default_environment;
        typedef kc::Key<13> fallback_environment;
        typedef kc::Key<14> support_old_style_virtuals;
        typedef kc::Key<15> default_ebuild_distdir;
        typedef kc::Key<16> default_ebuild_write_cache;
        typedef kc::Key<17> default_ebuild_names_cache;
        typedef kc::Key<18> default_ebuild_builddir;
        typedef kc::Key<19> default_ebuild_layout;
        typedef kc::Key<20> default_ebuild_eapi_when_unknown;
        typedef kc::Key<21> default_ebuild_eapi_when_unspecified;
        typedef kc::Key<22> default_ebuild_profile_eapi;
        typedef kc::Key<23> default_vdb_provides_cache;
        typedef kc::Key<24> default_vdb_names_cache;
        typedef kc::Key<25> paludis_environment_use_conf_filename;
        typedef kc::Key<26> paludis_environment_keywords_conf_filename;
        typedef kc::Key<27> concept_use;
        typedef kc::Key<28> concept_keyword;
        typedef kc::Key<29> paludis_package;
        typedef kc::Key<30> kind;
        typedef kc::Key<31> value;
        typedef kc::Key<32> mask_file;
        typedef kc::Key<33> comment;
        typedef kc::Key<34> environment;
        typedef kc::Key<35> image;
        typedef kc::Key<36> root;
        typedef kc::Key<37> no_chown;
        typedef kc::Key<38> options;
        typedef kc::Key<39> contents_file;
        typedef kc::Key<40> config_protect;
        typedef kc::Key<41> config_protect_mask;
        typedef kc::Key<42> package_id;
        typedef kc::Key<43> sets_interface;
        typedef kc::Key<44> syncable_interface;
        typedef kc::Key<45> use_interface;
        typedef kc::Key<46> world_interface;
        typedef kc::Key<47> mirrors_interface;
        typedef kc::Key<48> environment_variable_interface;
        typedef kc::Key<49> provides_interface;
        typedef kc::Key<50> virtuals_interface;
        typedef kc::Key<51> make_virtuals_interface;
        typedef kc::Key<52> destination_interface;
        typedef kc::Key<53> e_interface;
        typedef kc::Key<54> hook_interface;
        typedef kc::Key<55> qa_interface;
        typedef kc::Key<56> manifest_interface;
        typedef kc::Key<57> path;
        typedef kc::Key<58> arch;
        typedef kc::Key<59> status;
        typedef kc::Key<60> profile;
        typedef kc::Key<61> virtual_name;
        typedef kc::Key<62> provided_by;
        typedef kc::Key<63> provided_by_spec;
        typedef kc::Key<64> image_dir;
        typedef kc::Key<65> environment_file;
        typedef kc::Key<66> filter_file;
        typedef kc::Key<67> output_prefix;
        typedef kc::Key<68> local;
        typedef kc::Key<69> remote;
        typedef kc::Key<69> ebuild_dir;
        typedef kc::Key<70> ebuild_file;
        typedef kc::Key<71> eclassdirs;
        typedef kc::Key<72> exlibsdirs;
        typedef kc::Key<73> portdir;
        typedef kc::Key<74> distdir;
        typedef kc::Key<75> builddir;
        typedef kc::Key<76> userpriv;
        typedef kc::Key<77> sandbox;
        typedef kc::Key<78> commands;
        typedef kc::Key<79> a;
        typedef kc::Key<80> aa;
        typedef kc::Key<81> use;
        typedef kc::Key<82> use_expand;
        typedef kc::Key<83> profiles;
        typedef kc::Key<84> expand_vars;
        typedef kc::Key<85> files_dir;
        typedef kc::Key<86> name;
        typedef kc::Key<87> exported_name;
        typedef kc::Key<88> supported;
        typedef kc::Key<89> package_dep_spec_parse_options;
        typedef kc::Key<90> dependency_spec_tree_parse_options;
        typedef kc::Key<91> iuse_flag_parse_options;
        typedef kc::Key<92> merger_options;
        typedef kc::Key<93> breaks_portage;
        typedef kc::Key<94> can_be_pbin;
        typedef kc::Key<95> ebuild_options;
        typedef kc::Key<96> ebuild_phases;
        typedef kc::Key<97> ebuild_metadata_variables;
        typedef kc::Key<98> ebuild_environment_variables;
        typedef kc::Key<99> uri_labels;
        typedef kc::Key<100> dependency_labels;
        typedef kc::Key<101> pipe_commands;
        typedef kc::Key<102> tools_options;
        typedef kc::Key<103> rewrite_virtuals;
        typedef kc::Key<104> no_slot_or_repo;
        typedef kc::Key<105> disable_cfgpro;
        typedef kc::Key<106> slot;
        typedef kc::Key<107> loadsaveenv_dir;
        typedef kc::Key<108> unmerge_only;
        typedef kc::Key<109> load_environment;
        typedef kc::Key<110> info_vars;
        typedef kc::Key<111> output_directory;
        typedef kc::Key<112> destination_repository;
        typedef kc::Key<113> binary_ebuild_location;
        typedef kc::Key<114> binary_distdir;
        typedef kc::Key<115> env_use;
        typedef kc::Key<116> env_use_expand;
        typedef kc::Key<117> env_use_expand_hidden;
        typedef kc::Key<118> env_aa;
        typedef kc::Key<119> env_arch;
        typedef kc::Key<120> env_kv;
        typedef kc::Key<121> env_accept_keywords;
        typedef kc::Key<122> env_portdir;
        typedef kc::Key<123> env_distdir;
        typedef kc::Key<124> description_use;
        typedef kc::Key<125> debug_dir;
        typedef kc::Key<126> use_ebuild_file;
        typedef kc::Key<127> install_under;
        typedef kc::Key<128> location;
        typedef kc::Key<129> version;
        typedef kc::Key<130> build_dependencies;
        typedef kc::Key<131> run_dependencies;
        typedef kc::Key<132> description;
        typedef kc::Key<133> fs_location;
        typedef kc::Key<134> magic;
        typedef kc::Key<135> mutex;
        typedef kc::Key<136> ndbam;
        typedef kc::Key<137> on_string;
        typedef kc::Key<138> on_arrow;
        typedef kc::Key<139> on_any;
        typedef kc::Key<140> on_use;
        typedef kc::Key<141> on_label;
        typedef kc::Key<142> on_pop;
        typedef kc::Key<143> on_should_be_empty;
        typedef kc::Key<144> on_all;
        typedef kc::Key<145> on_error;
        typedef kc::Key<146> add_handler;
        typedef kc::Key<147> item;
        typedef kc::Key<148> on_use_under_any;
    }
}

#endif
