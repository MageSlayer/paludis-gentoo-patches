/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Richard Brown <mynamewasgone@gmail.com>
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

#include <paludis_ruby.hh>
#include <paludis/dep_list/dep_list.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_dep_list_target_type;
    static VALUE c_dep_list_reinstall_option;
    static VALUE c_dep_list_fall_back_option;
    static VALUE c_dep_list_reinstall_scm_option;
    static VALUE c_dep_list_upgrade_option;
    static VALUE c_dep_list_new_slots_option;
    static VALUE c_dep_list_deps_option;
    static VALUE c_dep_list_circular_option;
    static VALUE c_dep_list_blocks_option;
    static VALUE c_dep_list_entry_state;
    static VALUE c_dep_list_entry_kind;
    static VALUE c_dep_list_options;
    static VALUE c_dep_list;
    static VALUE c_dep_list_entry;

    DepListOptions
    value_to_dep_list_options(VALUE v)
    {
        if (rb_obj_is_kind_of(v, c_dep_list_options))
        {
            DepListOptions * v_ptr;
            Data_Get_Struct(v, DepListOptions, v_ptr);
            return *v_ptr;
        }
        else
        {
            rb_raise(rb_eTypeError, "Can't convert %s into DepListOptions", rb_obj_classname(v));
        }
    }

    VALUE
    dep_list_entry_to_value(const DepListEntry & v)
    {
        DepListEntry * vv(new DepListEntry(v));
        return Data_Wrap_Struct(c_dep_list_entry, 0, &Common<DepListEntry>::free, vv);
    }

    VALUE
    dep_list_options_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    /*
     * call-seq:
     *     DepListOptions.new(reinstall, reinstall_scm, target_type, upgrade, new_slots, fall_back, installed_deps_prem installed_deps_runtime, installed_deps_post, uninstalled_deps_pre, uninstalled_deps_runtime, uninstalled_deps_post, circular, blocks, dependency_tags) -> DepListOptions
     *     DepListOptions.new(Hash) -> DepListOptions
     *
     * DepListOptions.new can either be called with all parameters in order, or with one hash parameter, where the hash keys are symbols with the names above.
     */
    VALUE
    dep_list_options_new(int argc, VALUE *argv, VALUE self)
    {
        DepListOptions * ptr(0);
        try
        {
            int value_for_reinstall;
            int value_for_reinstall_scm;
            int value_for_target_type;
            int value_for_upgrade;
            int value_for_new_slots;
            int value_for_fall_back;
            int value_for_installed_deps_pre;
            int value_for_installed_deps_runtime;
            int value_for_installed_deps_post;
            int value_for_uninstalled_deps_pre;
            int value_for_uninstalled_deps_runtime;
            int value_for_uninstalled_deps_post;
            int value_for_circular;
            int value_for_blocks;
            MaskReasons value_for_override_mask_reasons;
            bool value_for_dependency_tags;

            if (1 == argc && rb_obj_is_kind_of(argv[0], rb_cHash))
            {
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("reinstall"))))
                    rb_raise(rb_eArgError, "Missing Parameter: reinstall");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("reinstall_scm"))))
                    rb_raise(rb_eArgError, "Missing Parameter: reinstall_scm");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("target_type"))))
                    rb_raise(rb_eArgError, "Missing Parameter: target_type");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("upgrade"))))
                    rb_raise(rb_eArgError, "Missing Parameter: upgrade");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("new_slots"))))
                    rb_raise(rb_eArgError, "Missing Parameter: new_slots");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("fall_back"))))
                    rb_raise(rb_eArgError, "Missing Parameter: fall_back");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("installed_deps_pre"))))
                    rb_raise(rb_eArgError, "Missing Parameter: installed_deps_pre");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("installed_deps_runtime"))))
                    rb_raise(rb_eArgError, "Missing Parameter: installed_deps_runtime");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("installed_deps_post"))))
                    rb_raise(rb_eArgError, "Missing Parameter: installed_deps_post");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("uninstalled_deps_pre"))))
                    rb_raise(rb_eArgError, "Missing Parameter: uninstalled_deps_pre");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("installed_deps_runtime"))))
                    rb_raise(rb_eArgError, "Missing Parameter: uninstalled_deps_runtime");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("uninstalled_deps_post"))))
                    rb_raise(rb_eArgError, "Missing Parameter: uninstalled_deps_post");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("circular"))))
                    rb_raise(rb_eArgError, "Missing Parameter: circular");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("blocks"))))
                    rb_raise(rb_eArgError, "Missing Parameter: blocks");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("override_mask_reasons"))))
                    rb_raise(rb_eArgError, "Missing Parameter: override_mask_reasons");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("dependency_tags"))))
                    rb_raise(rb_eArgError, "Missing Parameter: dependency_tags");
                value_for_reinstall =
                    NUM2INT(rb_hash_aref(argv[0], ID2SYM(rb_intern("reinstall"))));
                value_for_reinstall_scm =
                    NUM2INT(rb_hash_aref(argv[0], ID2SYM(rb_intern("reinstall_scm"))));
                value_for_target_type =
                    NUM2INT(rb_hash_aref(argv[0], ID2SYM(rb_intern("target_type"))));
                value_for_upgrade =
                    NUM2INT(rb_hash_aref(argv[0], ID2SYM(rb_intern("upgrade"))));
                value_for_new_slots =
                    NUM2INT(rb_hash_aref(argv[0], ID2SYM(rb_intern("new_slots"))));
                value_for_fall_back =
                    NUM2INT(rb_hash_aref(argv[0], ID2SYM(rb_intern("fall_back"))));
                value_for_installed_deps_pre =
                    NUM2INT(rb_hash_aref(argv[0], ID2SYM(rb_intern("installed_deps_pre"))));
                value_for_installed_deps_runtime =
                    NUM2INT(rb_hash_aref(argv[0], ID2SYM(rb_intern("installed_deps_runtime"))));
                value_for_installed_deps_post =
                    NUM2INT(rb_hash_aref(argv[0], ID2SYM(rb_intern("installed_deps_post"))));
                value_for_uninstalled_deps_pre =
                    NUM2INT(rb_hash_aref(argv[0], ID2SYM(rb_intern("uninstalled_deps_pre"))));
                value_for_uninstalled_deps_runtime =
                    NUM2INT(rb_hash_aref(argv[0], ID2SYM(rb_intern("uninstalled_deps_runtime"))));
                value_for_uninstalled_deps_post =
                    NUM2INT(rb_hash_aref(argv[0], ID2SYM(rb_intern("uninstalled_deps_post"))));
                value_for_circular =
                    NUM2INT(rb_hash_aref(argv[0], ID2SYM(rb_intern("circular"))));
                value_for_blocks =
                    NUM2INT(rb_hash_aref(argv[0], ID2SYM(rb_intern("blocks"))));
                value_for_override_mask_reasons = value_to_mask_reasons(
                    rb_hash_aref(argv[0], ID2SYM(rb_intern("override_mask_reasons")))
                        );
                value_for_dependency_tags =
                    Qtrue == (rb_hash_aref(argv[0], ID2SYM(rb_intern("dependency_tags")))) ? true : false;
            }
            else if (16 == argc)
            {
                value_for_reinstall                = NUM2INT(argv[0]);
                value_for_reinstall_scm            = NUM2INT(argv[1]);
                value_for_target_type              = NUM2INT(argv[2]);
                value_for_upgrade                  = NUM2INT(argv[3]);
                value_for_new_slots                = NUM2INT(argv[4]);
                value_for_fall_back                = NUM2INT(argv[5]);
                value_for_installed_deps_pre       = NUM2INT(argv[6]);
                value_for_installed_deps_runtime   = NUM2INT(argv[7]);
                value_for_installed_deps_post      = NUM2INT(argv[8]);
                value_for_uninstalled_deps_pre     = NUM2INT(argv[9]);
                value_for_uninstalled_deps_runtime = NUM2INT(argv[10]);
                value_for_uninstalled_deps_post    = NUM2INT(argv[11]);
                value_for_circular                 = NUM2INT(argv[12]);
                value_for_blocks                   = NUM2INT(argv[13]);
                value_for_override_mask_reasons    = value_to_mask_reasons(argv[14]);
                value_for_dependency_tags = Qtrue == argv[15] ? true : false;
            }
            else
            {
                rb_raise(rb_eArgError, "DepListOptions expects fourteen arguments, but got %d",argc);
            }

            if (value_for_reinstall < 0 ||  value_for_reinstall >= last_dl_reinstall)
                rb_raise(rb_eArgError, "reinstall out of range");
            if (value_for_reinstall_scm < 0 ||  value_for_reinstall_scm >= last_dl_reinstall_scm)
                rb_raise(rb_eArgError, "reinstall_scm out of range");
            if (value_for_target_type < 0 ||  value_for_target_type >= last_dl_target)
                rb_raise(rb_eArgError, "target_type out of range");
            if (value_for_upgrade < 0 ||  value_for_upgrade >= last_dl_upgrade)
                rb_raise(rb_eArgError, "upgrade out of range");
            if (value_for_new_slots < 0 ||  value_for_new_slots >= last_dl_new_slots)
                rb_raise(rb_eArgError, "new_slots out of range");
            if (value_for_fall_back < 0 ||  value_for_fall_back >= last_dl_fall_back)
                rb_raise(rb_eArgError, "fall_back out of range");
            if (value_for_installed_deps_pre < 0 ||  value_for_installed_deps_pre >= last_dl_deps)
                rb_raise(rb_eArgError, "installed_deps_pre out of range");
            if (value_for_installed_deps_runtime < 0 ||  value_for_installed_deps_runtime >= last_dl_deps)
                rb_raise(rb_eArgError, "installed_deps_runtime out of range");
            if (value_for_installed_deps_post < 0 ||  value_for_installed_deps_post >= last_dl_deps)
                rb_raise(rb_eArgError, "installed_deps_post out of range");
            if (value_for_uninstalled_deps_pre < 0 ||  value_for_uninstalled_deps_pre >= last_dl_deps)
                rb_raise(rb_eArgError, "uninstalled_deps_pre out of range");
            if (value_for_uninstalled_deps_runtime < 0 ||  value_for_uninstalled_deps_runtime >= last_dl_deps)
                rb_raise(rb_eArgError, "uninstalled_deps_runtime out of range");
            if (value_for_uninstalled_deps_post < 0 ||  value_for_uninstalled_deps_post >= last_dl_deps)
                rb_raise(rb_eArgError, "uninstalled_deps_post out of range");
            if (value_for_circular < 0 ||  value_for_circular >= last_dl_circular)
                rb_raise(rb_eArgError, "circular out of range");
            if (value_for_circular < 0 ||  value_for_blocks >= last_dl_blocks)
                rb_raise(rb_eArgError, "blocks out of range");

            ptr = new DepListOptions(static_cast<DepListReinstallOption>(value_for_reinstall),
                    static_cast<DepListReinstallScmOption>(value_for_reinstall_scm),
                    static_cast<DepListTargetType>(value_for_target_type),
                    static_cast<DepListUpgradeOption>(value_for_upgrade),
                    static_cast<DepListNewSlotsOption>(value_for_new_slots),
                    static_cast<DepListFallBackOption>(value_for_fall_back),
                    static_cast<DepListDepsOption>(value_for_installed_deps_pre),
                    static_cast<DepListDepsOption>(value_for_installed_deps_runtime),
                    static_cast<DepListDepsOption>(value_for_installed_deps_post),
                    static_cast<DepListDepsOption>(value_for_uninstalled_deps_pre),
                    static_cast<DepListDepsOption>(value_for_uninstalled_deps_runtime),
                    static_cast<DepListDepsOption>(value_for_uninstalled_deps_post),
                    static_cast<DepListCircularOption>(value_for_circular),
                    static_cast<DepListBlocksOption>(value_for_blocks),
                    value_for_override_mask_reasons,
                    value_for_dependency_tags
                    );

            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<DepListOptions>::free, ptr));
            rb_obj_call_init(tdata, argc, argv);
            return tdata;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    /*
     * Document-method: reinstall
     *
     * call-seq:
     *     reinstall -> DepListReinstallOption
     *
     * Our DepListReinstallOption
     */
    /*
     * Document-method: reinstall_scm
     *
     * call-seq:
     *     reinstall_scm -> DepListReinstallScmOption
     *
     * Our DepListReinstallScmOption
     */
    /*
     * Document-method: target
     *
     * call-seq:
     *     target -> DepListTargetType
     *
     * Our DepListTargetType
     */
    /*
     * Document-method: upgrade
     *
     * call-seq:
     *     upgrade -> DepListUpgradeOption
     *
     * Our DepListUpgradeOption
     */
    /*
     * Document-method: new_slots
     *
     * call-seq:
     *     new_slots -> DepListNewSlotsOption
     *
     * Our DepListNewSlotsOption
     */
    /*
     * Document-method: fall_back
     *
     * call-seq:
     *     fall_back -> DepListFallBackOption
     *
     * Our DepListFallBackOption
     */
    /*
     * Document-method: installed_deps_pre
     *
     * call-seq:
     *     installed_deps_pre -> DepListDepsOption
     *
     * Our installed_deps_runtime
     */
    /*
     * Document-method: installed_deps_runtime
     *
     * call-seq:
     *     installed_deps_runtime -> DepListDepsOption
     *
     * Our installed_deps_runtime
     */
    /*
     * Document-method: installed_deps_post
     *
     * call-seq:
     *     installed_deps_post -> DepListDepsOption
     *
     * Our installed_deps_post
     */
    /*
     * Document-method: uninstalled_deps_pre
     *
     * call-seq:
     *     uninstalled_deps_pre -> DepListDepsOption
     *
     * Our uninstalled_deps_runtime
     */
    /*
     * Document-method: uninstalled_deps_runtime
     *
     * call-seq:
     *     uninstalled_deps_runtime -> DepListDepsOption
     *
     * Our uninstalled_deps_runtime
     */
    /*
     * Document-method: uninstalled_deps_post
     *
     * call-seq:
     *     uninstalled_deps_post -> DepListDepsOption
     *
     * Our uninstalled_deps_post
     */
    /*
     * Document-method: circular
     *
     * call-seq:
     *     circular -> DepListCircularOption
     *
     * Our DepListCircularOption
     */
    template <typename T_, T_ DepListOptions::* m_>
    struct OptionsMember
    {
        static VALUE
        fetch(VALUE self)
        {
            DepListOptions * p;
            Data_Get_Struct(self, DepListOptions, p);
            return INT2FIX(p->*m_);
        }
    };

    /*
     * call-seq:
     *     override_mask_reasons -> MaskReasons
     *
     * Our override MaskReasons.
     */
    VALUE
    dep_list_options_mask_reasons(VALUE self)
    {
        DepListOptions * p;
        Data_Get_Struct(self, DepListOptions, p);
        return mask_reasons_to_value(p->override_mask_reasons);
    }

    /*
     * call-seq:
     *     dependancy_tags -> true or false
     *
     * Out dependency_tags
     */
    VALUE
    dep_list_options_dependency_tags(VALUE self)
    {
        DepListOptions * p;
        Data_Get_Struct(self, DepListOptions, p);
        return p->dependency_tags ? Qtrue : Qfalse;
    }

    VALUE
    dep_list_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    /*
     * call-seq:
     *     new(environment, dep_list_options) -> DepList
     *
     * Create a new DepList
     */
    VALUE
    dep_list_new(int argc, VALUE *argv, VALUE self)
    {
        DepList * ptr(0);
        try
        {
            if (2 == argc)
            {
                ptr = new DepList(value_to_environment_data(argv[0])->env_ptr,
                        value_to_dep_list_options(argv[1]));
            }
            else
            {
                rb_raise(rb_eArgError, "DepList expects two arguments, but got %d",argc);
            }
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<DepList>::free, ptr));
            rb_obj_call_init(tdata, argc, argv);
            return tdata;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     add(dep_atom)
     *
     * Add the packages required to resolve an additional dependency atom.
     */
    VALUE
    dep_list_add(VALUE self, VALUE da)
    {
        try
        {
            DepList * p;
            Data_Get_Struct(self, DepList, p);
            p->add(value_to_dep_atom(da));
            return self;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     clear
     *
     * Clear the list.
     */
    VALUE
    dep_list_clear(VALUE self)
    {
        DepList * p;
        Data_Get_Struct(self, DepList, p);
        p->clear();
        return self;
    }

    /*
     * call-seq:
     *     already_installed?(dep_atom)
     *
     * Is an atom structure already installed?
     */
    VALUE
    dep_list_already_installed(VALUE self, VALUE da)
    {
        DepList * p;
        Data_Get_Struct(self, DepList, p);
        return p->already_installed(value_to_dep_atom(da)) ? Qtrue : Qfalse;
    }

    /*
     * call-seq:
     *     each {|dep_list_entry| block}
     *
     * Iterate over our DepListEntrys
     */
    VALUE
    dep_list_each(VALUE self)
    {
        DepList * p;
        Data_Get_Struct(self, DepList, p);
        for (DepList::Iterator i(p->begin()), i_end(p->end()) ; i != i_end ; ++i)
            rb_yield(dep_list_entry_to_value(*i));
        return self;
    }

    /*
     * call-seq:
     *     entry_state -> DepListEntryState
     *
     * Our DepListEntryState.
     */
    VALUE
    dep_list_entry_kind(VALUE self)
    {
        try
        {
            DepListEntry * p;
            Data_Get_Struct(self, DepListEntry, p);
            return INT2FIX(p->kind);
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     package -> PackageDatabaseEntry
     *
     * Our PackageDatabaseEntry.
     */
    VALUE
    dep_list_entry_package(VALUE self)
    {
        try
        {
            DepListEntry * p;
            Data_Get_Struct(self, DepListEntry, p);
            return(package_database_entry_to_value(p->package));
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     metadata -> VersionMetadata
     *
     * Our VersionMetadata.
     */
    VALUE
    dep_list_entry_metadata(VALUE self)
    {
        try
        {
            DepListEntry * p;
            Data_Get_Struct(self, DepListEntry, p);
            return(version_metadata_to_value(p->metadata));
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     destinations -> Array
     *
     * Our destinations repostiories.
     */
    VALUE
    dep_list_entry_destinations(VALUE self)
    {
        try
        {
            DepListEntry * p;
            Data_Get_Struct(self, DepListEntry, p);

            VALUE result(rb_ary_new());
            for (RepositoryNameCollection::Iterator r(p->destinations->begin()),
                    r_end(p->destinations->end()) ; r != r_end ; ++r)
                rb_ary_push(result, rb_str_new2(stringify(*r).c_str()));

            return result;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     entry_state -> DepListEntryState
     *
     * Our DepListEntryState.
     */
    VALUE
    dep_list_entry_state(VALUE self)
    {
        try
        {
            DepListEntry * p;
            Data_Get_Struct(self, DepListEntry, p);
            return INT2FIX(p->state);
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    void do_register_dep_list()
    {
        /*
         * Document-module: Paludis::DepListTargetType
         *
         * What type of target are we handling at the top level.
         *
         */
        c_dep_list_target_type = rb_define_module_under(paludis_module(), "DepListTargetType");
        for (DepListTargetType l(static_cast<DepListTargetType>(0)), l_end(last_dl_target) ; l != l_end ;
                l = static_cast<DepListTargetType>(static_cast<int>(l) + 1))
            rb_define_const(c_dep_list_target_type, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/dep_list/options.hh, DepListTargetType, c_dep_list_target_type>

        /*
         * Document-module: Paludis::DepListReinstallOption
         *
         * What should we reinstall.
         *
         */
        c_dep_list_reinstall_option = rb_define_module_under(paludis_module(), "DepListReinstallOption");
        for (DepListReinstallOption l(static_cast<DepListReinstallOption>(0)), l_end(last_dl_reinstall) ; l != l_end ;
                l = static_cast<DepListReinstallOption>(static_cast<int>(l) + 1))
            rb_define_const(c_dep_list_reinstall_option, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/dep_list/options.hh, DepListReinstallOption, c_dep_list_reinstall_option>

        /*
         * Document-module: Paludis::DepListFallBackOption
         *
         * When can we fall back to installed?
         *
         */
        c_dep_list_fall_back_option = rb_define_module_under(paludis_module(), "DepListFallBackOption");
        for (DepListFallBackOption l(static_cast<DepListFallBackOption>(0)), l_end(last_dl_fall_back) ; l != l_end ;
                l = static_cast<DepListFallBackOption>(static_cast<int>(l) + 1))
            rb_define_const(c_dep_list_fall_back_option, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/dep_list/options.hh, DepListFallBackOption, c_dep_list_fall_back_option>

        /*
         * Document-module: Paludis::DepListReinstallScmOption
         *
         * When should we reinstall scm.
         *
         */
        c_dep_list_reinstall_scm_option = rb_define_module_under(paludis_module(), "DepListReinstallScmOption");
        for (DepListReinstallScmOption l(static_cast<DepListReinstallScmOption>(0)), l_end(last_dl_reinstall_scm) ; l != l_end ;
                l = static_cast<DepListReinstallScmOption>(static_cast<int>(l) + 1))
            rb_define_const(c_dep_list_reinstall_scm_option, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/dep_list/options.hh, DepListReinstalScmlOption, c_dep_list_reinstall_scm_option>

        /*
         * Document-module: Paludis::DepListUpgradeOption
         *
         * When should we upgrade.
         *
         */
        c_dep_list_upgrade_option = rb_define_module_under(paludis_module(), "DepListUpgradeOption");
        for (DepListUpgradeOption l(static_cast<DepListUpgradeOption>(0)), l_end(last_dl_upgrade) ; l != l_end ;
                l = static_cast<DepListUpgradeOption>(static_cast<int>(l) + 1))
            rb_define_const(c_dep_list_upgrade_option, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/dep_list/options.hh, DepListUpgradeOption, c_dep_list_upgrade_option>

        /*
         * Document-module: Paludis::DepListNewSlotsOption
         *
         * When should we pull in a new slot.
         *
         */
        c_dep_list_new_slots_option = rb_define_module_under(paludis_module(), "DepListNewSlotsOption");
        for (DepListNewSlotsOption l(static_cast<DepListNewSlotsOption>(0)), l_end(last_dl_new_slots) ; l != l_end ;
                l = static_cast<DepListNewSlotsOption>(static_cast<int>(l) + 1))
            rb_define_const(c_dep_list_new_slots_option, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/dep_list/options.hh, DepListNewSlotsOption, c_dep_list_new_slots_option>

        /*
         * Document-module: Paludis::DepListDepsOption
         *
         * How should we handle a dep class.
         *
         */
        c_dep_list_deps_option = rb_define_module_under(paludis_module(), "DepListDepsOption");
        for (DepListDepsOption l(static_cast<DepListDepsOption>(0)), l_end(last_dl_deps) ; l != l_end ;
                l = static_cast<DepListDepsOption>(static_cast<int>(l) + 1))
            rb_define_const(c_dep_list_deps_option, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/dep_list/options.hh, DepListDepsOption, c_dep_list_deps_option>

        /*
         * Document-module: Paludis::DepListCircularOption
         *
         * How we handle circular deps.
         *
         */
        c_dep_list_circular_option = rb_define_module_under(paludis_module(), "DepListCircularOption");
        for (DepListCircularOption l(static_cast<DepListCircularOption>(0)), l_end(last_dl_circular) ; l != l_end ;
                l = static_cast<DepListCircularOption>(static_cast<int>(l) + 1))
            rb_define_const(c_dep_list_circular_option, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/dep_list/options.hh, DepListCircularOption, c_dep_list_circular_option>

        /*
         * Document-module: Paludis::DepListBlocksOption
         *
         * How we handle blocks.
         *
         */
        c_dep_list_blocks_option = rb_define_module_under(paludis_module(), "DepListBlocksOption");
        for (DepListBlocksOption l(static_cast<DepListBlocksOption>(0)), l_end(last_dl_blocks) ; l != l_end ;
                l = static_cast<DepListBlocksOption>(static_cast<int>(l) + 1))
            rb_define_const(c_dep_list_blocks_option, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/dep_list/options.hh, DepListBlocksOption, c_dep_list_blocks_option>

        /*
         * Document-module: Paludis::DepListEntryState
         *
         * State of a DepListEntry.
         *
         */
        c_dep_list_entry_state = rb_define_module_under(paludis_module(), "DepListEntryState");
        for (DepListEntryState l(static_cast<DepListEntryState>(0)), l_end(last_dle) ; l != l_end ;
                l = static_cast<DepListEntryState>(static_cast<int>(l) + 1))
            rb_define_const(c_dep_list_entry_state, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/dep_list/options.hh, DepListEntryState, c_dep_list_entry_state>

        /*
         * Document-module: Paludis::DepListEntryKind
         *
         * Kind of a DepListEntry.
         *
         */
        c_dep_list_entry_kind = rb_define_module_under(paludis_module(), "DepListEntryKind");
        for (DepListEntryKind l(static_cast<DepListEntryKind>(0)), l_end(last_dlk) ; l != l_end ;
                l = static_cast<DepListEntryKind>(static_cast<int>(l) + 1))
            rb_define_const(c_dep_list_entry_kind, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/dep_list/options.hh, DepListEntryKind, c_dep_list_entry_kind>

        /*
         * Document-class: Paludis::DepListOptions
         *
         * Parameters for a DepList.
         */
        c_dep_list_options = rb_define_class_under(paludis_module(), "DepListOptions", rb_cObject);
        rb_define_singleton_method(c_dep_list_options, "new", RUBY_FUNC_CAST(&dep_list_options_new), -1);
        rb_define_method(c_dep_list_options, "initialize", RUBY_FUNC_CAST(&dep_list_options_init), -1);
        rb_define_method(c_dep_list_options, "reinstall",
                RUBY_FUNC_CAST((&OptionsMember<DepListReinstallOption, &DepListOptions::reinstall>::fetch)),0);
        rb_define_method(c_dep_list_options, "reinstall_scm",
                RUBY_FUNC_CAST((&OptionsMember<DepListReinstallScmOption, &DepListOptions::reinstall_scm>::fetch)),0);
        rb_define_method(c_dep_list_options, "target_type",
                RUBY_FUNC_CAST((&OptionsMember<DepListTargetType, &DepListOptions::target_type>::fetch)),0);
        rb_define_method(c_dep_list_options, "upgrade",
                RUBY_FUNC_CAST((&OptionsMember<DepListUpgradeOption, &DepListOptions::upgrade>::fetch)),0);
        rb_define_method(c_dep_list_options, "new_slots",
                RUBY_FUNC_CAST((&OptionsMember<DepListNewSlotsOption, &DepListOptions::new_slots>::fetch)),0);
        rb_define_method(c_dep_list_options, "fall_back",
                RUBY_FUNC_CAST((&OptionsMember<DepListFallBackOption, &DepListOptions::fall_back>::fetch)),0);
        rb_define_method(c_dep_list_options, "installed_deps_pre",
                RUBY_FUNC_CAST((&OptionsMember<DepListDepsOption, &DepListOptions::installed_deps_pre>::fetch)),0);
        rb_define_method(c_dep_list_options, "installed_deps_runtime",
                RUBY_FUNC_CAST((&OptionsMember<DepListDepsOption, &DepListOptions::installed_deps_runtime>::fetch)),0);
        rb_define_method(c_dep_list_options, "installed_deps_post",
                RUBY_FUNC_CAST((&OptionsMember<DepListDepsOption, &DepListOptions::installed_deps_post>::fetch)),0);
        rb_define_method(c_dep_list_options, "uninstalled_deps_pre",
                RUBY_FUNC_CAST((&OptionsMember<DepListDepsOption, &DepListOptions::uninstalled_deps_pre>::fetch)),0);
        rb_define_method(c_dep_list_options, "uninstalled_deps_runtime",
                RUBY_FUNC_CAST((&OptionsMember<DepListDepsOption, &DepListOptions::uninstalled_deps_runtime>::fetch)),0);
        rb_define_method(c_dep_list_options, "uninstalled_deps_post",
                RUBY_FUNC_CAST((&OptionsMember<DepListDepsOption, &DepListOptions::installed_deps_post>::fetch)),0);
        rb_define_method(c_dep_list_options, "circular",
                RUBY_FUNC_CAST((&OptionsMember<DepListCircularOption, &DepListOptions::circular>::fetch)),0);
        rb_define_method(c_dep_list_options, "blocks",
                RUBY_FUNC_CAST((&OptionsMember<DepListBlocksOption, &DepListOptions::blocks>::fetch)),0);
        rb_define_method(c_dep_list_options, "ovveride_mask_reasons",
                RUBY_FUNC_CAST((&OptionsMember<DepListBlocksOption, &DepListOptions::blocks>::fetch)),0);

        rb_define_method(c_dep_list_options, "override_mask_reasons", RUBY_FUNC_CAST(&dep_list_options_mask_reasons),0);
        rb_define_method(c_dep_list_options, "dependency_tags", RUBY_FUNC_CAST(&dep_list_options_dependency_tags),0);

        /*
         * Document-class: Paludis::DepList
         *
         * Holds a list of dependencies in merge order. Includes Enumerable[http://www.ruby-doc.org/core/classes/Enumerable.html],
         * but not Comparable.
         */
        c_dep_list= rb_define_class_under(paludis_module(), "DepList", rb_cObject);
        rb_define_singleton_method(c_dep_list, "new", RUBY_FUNC_CAST(&dep_list_new), -1);
        rb_define_method(c_dep_list, "initialize", RUBY_FUNC_CAST(&dep_list_init), -1);
        rb_define_method(c_dep_list, "add", RUBY_FUNC_CAST(&dep_list_add), 1);
        rb_define_method(c_dep_list, "clear", RUBY_FUNC_CAST(&dep_list_clear), 0);
        rb_define_method(c_dep_list, "already_installed?", RUBY_FUNC_CAST(&dep_list_already_installed), 1);
        rb_define_method(c_dep_list, "each", RUBY_FUNC_CAST(&dep_list_each), 0);
        rb_include_module(c_dep_list, rb_mEnumerable);

        /*
         * Document-class Paludis::DepListEntry
         *
         * An entry in a DepList.
         */
        c_dep_list_entry = rb_define_class_under(paludis_module(), "DepListEntry", rb_cObject);
        rb_funcall(c_dep_list_entry, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_dep_list_entry, "kind", RUBY_FUNC_CAST(&dep_list_entry_kind),0);
        rb_define_method(c_dep_list_entry, "package", RUBY_FUNC_CAST(&dep_list_entry_package),0);
        rb_define_method(c_dep_list_entry, "metadata", RUBY_FUNC_CAST(&dep_list_entry_metadata),0);
        rb_define_method(c_dep_list_entry, "destinations", RUBY_FUNC_CAST(&dep_list_entry_destinations),0);
        rb_define_method(c_dep_list_entry, "state", RUBY_FUNC_CAST(&dep_list_entry_state),0);
    }
}


RegisterRubyClass::Register paludis_ruby_register_dep_list PALUDIS_ATTRIBUTE((used))
    (&do_register_dep_list);

