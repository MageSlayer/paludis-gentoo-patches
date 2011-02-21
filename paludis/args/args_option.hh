/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2006 Stephen Bennett
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

#ifndef PALUDIS_GUARD_ARGS_ARGS_OPTION_HH
#define PALUDIS_GUARD_ARGS_ARGS_OPTION_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/type_list.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/sequence-fwd.hh>
#include <memory>

/** \file
 * Declarations for ArgsOption.
 *
 * \ingroup g_args
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_description> description;
        typedef Name<struct name_long_name> long_name;
        typedef Name<struct name_short_name> short_name;
    }

    namespace args
    {
        class ArgsGroup;
        class StringArg;
        class AliasArg;
        class SwitchArg;
        class IntegerArg;
        class EnumArg;
        class StringSetArg;
        class StringSequenceArg;

        /**
         * Base class for a command line option.
         *
         * \ingroup g_args
         */
        class PALUDIS_VISIBLE ArgsOption :
            public virtual DeclareAbstractAcceptMethods<ArgsOption, MakeTypeList<
                    StringArg, AliasArg, SwitchArg, IntegerArg, EnumArg, StringSetArg, StringSequenceArg>::Type>
        {
            friend class ArgsHandler;

            private:
                ArgsGroup * const _group;

                const std::string _long_name;
                const char _short_name;
                const std::string _description;

                bool _specified;

                ArgsOption(const ArgsOption &);
                void operator= (const ArgsOption &);

            protected:
                /**
                 * Constructor.
                 */
                ArgsOption(ArgsGroup * const, const std::string & long_name,
                        const char short_name, const std::string & description);

                /**
                 * Destructor.
                 */
                virtual ~ArgsOption();

            public:
                /**
                 * Remove this option.  Removes our group from its
                 * section if the group would be left empty.
                 */
                void remove();

                /**
                 * Fetch our long name.
                 */
                const std::string & long_name() const
                {
                    return _long_name;
                }

                /**
                 * Fetch our short name (may be 0).
                 */
                char short_name() const
                {
                    return _short_name;
                }

                /**
                 * Fetch our description.
                 */
                const std::string & description() const
                {
                    return _description;
                }

                /**
                 * Fetch whether or not we were specified on the
                 * command line.
                 */
                virtual bool specified() const
                {
                    return _specified;
                }

                /**
                 * Set the value returned by specified().
                 */
                virtual void set_specified(const bool value)
                {
                    _specified = value;
                }

                /**
                 * Fetch our group.
                 */
                ArgsGroup * group()
                {
                    return _group;
                }

                /**
                 * Can we be negated?
                 *
                 * Needs to match up with ArgsVisitor logic.
                 */
                virtual bool can_be_negated() const = 0;

                /**
                 * Ourself as a forwardable string.
                 *
                 * For example, '--foo bar' or '--foo bar --foo baz' or '--foo', or
                 * if not specified, the empty string.
                 *
                 * \since 0.40
                 */
                virtual const std::string forwardable_string() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                /**
                 * Ourself as a sequence of strings.
                 *
                 * For example, { '--foo', 'bar' } or { '--foo', 'bar', '--foo', 'baz' }
                 * if not specified, an empty sequence.
                 *
                 * \since 0.46
                 */
                virtual const std::shared_ptr<Sequence<std::string> > forwardable_args() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
        };

        /**
         * A SwitchArg is an option that can either be specified or not
         * specified, and that takes no value (for example, --help).
         *
         * \ingroup g_args
         */
        class PALUDIS_VISIBLE SwitchArg :
            public ArgsOption,
            public ImplementAcceptMethods<ArgsOption, SwitchArg>
        {
            private:
                bool _can_be_negated;

            public:
                /**
                 * Constructor.
                 *
                 * \since 0.26
                 */
                SwitchArg(ArgsGroup * const group, const std::string & long_name, char short_name,
                        const std::string & description, const bool can_be_negated);

                ~SwitchArg();

                virtual bool can_be_negated() const;

                virtual const std::string forwardable_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::shared_ptr<Sequence<std::string> > forwardable_args() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        /**
         * An option that takes a string argument.
         *
         * \ingroup g_args
         */
        class PALUDIS_VISIBLE StringArg :
            public ArgsOption,
            public ImplementAcceptMethods<ArgsOption, StringArg>
        {
            private:
                std::string _argument;
                void (* _validator) (const std::string &);

            public:
                /**
                * Constructor
                */
                StringArg(ArgsGroup * const, const std::string & long_name,
                       const char short_name, const std::string & description);

                /**
                 * Constructor with validator.
                 */
                StringArg(ArgsGroup * const, const std::string & long_name,
                       const char short_name, const std::string & description,
                       void (* validator) (const std::string &));

                /**
                 * Fetch the argument that was given to this option.
                 */
                const std::string & argument() const { return _argument; }

                /**
                 * Set the argument returned by argument().
                 */
                void set_argument(const std::string & arg);

                virtual bool can_be_negated() const;

                virtual const std::string forwardable_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::shared_ptr<Sequence<std::string> > forwardable_args() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        /**
         * An option that takes a set of strings.
         *
         * \ingroup g_args
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE StringSetArg :
            public ArgsOption,
            public ImplementAcceptMethods<ArgsOption, StringSetArg>
        {
            private:
                Pimp<StringSetArg> _imp;

                void (* _validator) (const std::string &);

            public:
                /**
                 * Helper class for passing available options and associated descriptions
                 * to the StringSetArg constructor.
                 *
                 * \ingroup grplibpaludisargs
                 */
                class PALUDIS_VISIBLE StringSetArgOptions
                {
                    friend class StringSetArg;

                    private:
                        Pimp<StringSetArgOptions> _imp;

                    public:
                        /**
                         * Constructor
                         */
                        StringSetArgOptions(const std::string &, const std::string &);

                        /**
                         * Blank constructor
                         */
                        explicit StringSetArgOptions();

                        /**
                         * Copy constructor
                         */
                        StringSetArgOptions(const StringSetArgOptions &);

                        /**
                         * Destructor.
                         */
                        ~StringSetArgOptions();

                        /**
                         * Adds another (option, description) pair.
                         */
                        StringSetArgOptions & operator() (const std::string &, const std::string &);
                };

                ///\name Basic operations
                ///\{

                StringSetArg(ArgsGroup * const, const std::string & long_name,
                        const char short_name, const std::string & description,
                        const StringSetArgOptions & options = StringSetArgOptions());

                StringSetArg(ArgsGroup * const, const std::string & long_name,
                        const char short_name, const std::string & description,
                        const StringSetArgOptions & options,
                        void (* validator) (const std::string &));

                ~StringSetArg();

                ///\}

                ///\name Iterate over our args.
                ///\{

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag, const std::string> ConstIterator;

                ConstIterator begin_args() const;

                ConstIterator end_args() const;

                ///\}

                /**
                 * Add an argument to the set.
                 */
                void add_argument(const std::string & arg);

                ///\name Iterate over our allowed arguments and associated descriptions
                ///\{

                struct AllowedArgConstIteratorTag;
                typedef WrappedForwardIterator<AllowedArgConstIteratorTag,
                        const std::pair<std::string, std::string> > AllowedArgConstIterator;

                AllowedArgConstIterator begin_allowed_args() const;

                AllowedArgConstIterator end_allowed_args() const;

                ///\}

                virtual bool can_be_negated() const;

                virtual const std::string forwardable_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::shared_ptr<Sequence<std::string> > forwardable_args() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        /**
         * An option that takes a set of strings.
         *
         * \since 0.32
         * \ingroup g_args
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE StringSequenceArg :
            public ArgsOption,
            public ImplementAcceptMethods<ArgsOption, StringSequenceArg>
        {
            private:
                Pimp<StringSequenceArg> _imp;

            public:
                ///\name Basic operations
                ///\{

                StringSequenceArg(ArgsGroup * const, const std::string & long_name,
                        const char short_name, const std::string & description);

                ~StringSequenceArg();

                ///\}

                ///\name Iterate over our args.
                ///\{

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag, const std::string> ConstIterator;

                ConstIterator begin_args() const;

                ConstIterator end_args() const;

                ///\}

                /**
                 * Add an argument to the set.
                 */
                void add_argument(const std::string & arg);

                virtual bool can_be_negated() const;

                virtual const std::string forwardable_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::shared_ptr<Sequence<std::string> > forwardable_args() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };


        /**
         * An AliasArg is an alias for another argument.
         *
         * \ingroup g_args
         */
        class PALUDIS_VISIBLE AliasArg :
            public ArgsOption,
            public ImplementAcceptMethods<ArgsOption, AliasArg>
        {
            private:
                ArgsOption * const _other;
                bool _hidden;

            public:
                /**
                 * Constructor.
                 */
                AliasArg(ArgsOption * const other, const std::string & new_long_name, bool is_hidden = false);

                virtual bool specified() const
                {
                    return _other->specified();
                }

                virtual void set_specified(const bool value)
                {
                    _other->set_specified(value);
                }

                virtual bool hidden() const
                {
                    return _hidden;
                }

                virtual void set_hidden(const bool value)
                {
                    _hidden = value;
                }

                /**
                 * Fetch our associated option.
                 */
                ArgsOption * other() const
                {
                    return _other;
                }

                virtual bool can_be_negated() const;

                virtual const std::string forwardable_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::shared_ptr<Sequence<std::string> > forwardable_args() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        /**
         * An option that takes an integer argument.
         *
         * \ingroup grplibpaludisargs
         */
        class PALUDIS_VISIBLE IntegerArg :
            public ArgsOption,
            public ImplementAcceptMethods<ArgsOption, IntegerArg>
        {
            private:
                int _argument;

            public:
                /**
                 * Constructor
                 */
                IntegerArg(ArgsGroup * const, const std::string & long_name,
                        const char short_name, const std::string & description);
                /**
                 * Fetch the argument that was given to this option.
                 */
                int argument() const { return _argument; }

                /**
                 * Set the argument returned by argument().
                 */
                void set_argument(const int arg) { _argument = arg; }

                virtual bool can_be_negated() const;

                virtual const std::string forwardable_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::shared_ptr<Sequence<std::string> > forwardable_args() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        /**
         * An allowed argument for an EnumArg.
         *
         * \ingroup g_args
         * \since 0.40
         */
        struct AllowedEnumArg
        {
            NamedValue<n::description, std::string> description;
            NamedValue<n::long_name, std::string> long_name;

            /// Might be '\0', for none.
            NamedValue<n::short_name, char> short_name;
        };

        /**
         * An option that takes one of a predefined set of string arguments.
         *
         * \ingroup g_args
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE EnumArg :
            public ArgsOption,
            public ImplementAcceptMethods<ArgsOption, EnumArg>
        {
            private:
                Pimp<EnumArg> _imp;

                std::string _argument;
                std::string _default_arg;

            public:
                /**
                 * Helper class for passing available options and associated descriptions
                 * to the EnumArg constructor.
                 *
                 * \ingroup grplibpaludisargs
                 */
                class PALUDIS_VISIBLE EnumArgOptions
                {
                    friend class EnumArg;

                    private:
                        Pimp<EnumArgOptions> _imp;

                    public:
                        /**
                         * Constructor
                         */
                        EnumArgOptions(const std::string &, const std::string &);

                        /**
                         * Constructor, with short arg.
                         *
                         * \since 0.40
                         */
                        EnumArgOptions(const std::string &, const char, const std::string &);

                        /**
                         * Destructor.
                         */
                        ~EnumArgOptions();

                        /**
                         * Adds another (option, description).
                         */
                        EnumArgOptions & operator() (const std::string &, const std::string &);

                        /**
                         * Adds another (option, short-option, description).
                         *
                         * \since 0.40
                         */
                        EnumArgOptions & operator() (const std::string &, const char, const std::string &);
                };

                /**
                 * Constructor.
                 */
                EnumArg(ArgsGroup * const group, const std::string & long_name,
                        const char short_name, const std::string & description,
                        const EnumArgOptions & opts, const std::string & default_arg);

                ~EnumArg();

                /**
                 * Fetch the argument that was given to this option.
                 */
                const std::string & argument() const
                {
                    return _argument;
                }

                /**
                 * Set the argument returned by argument(), having verified that
                 * it is one of the arguments allowed for this option.
                 */
                void set_argument(const std::string & arg);

                /**
                 * Change the default option (should be called before
                 * set_argument()).
                 */
                void set_default_arg(const std::string & arg);

                /**
                 * Fetch the default option, as specified to the
                 * constructor or set_default_arg().
                 */
                const std::string & default_arg() const
                {
                    return _default_arg;
                }

                ///\name Iterate over our allowed arguments and associated descriptions
                ///\{

                struct AllowedArgConstIteratorTag;
                typedef WrappedForwardIterator<AllowedArgConstIteratorTag,
                        const AllowedEnumArg> AllowedArgConstIterator;

                AllowedArgConstIterator begin_allowed_args() const;

                AllowedArgConstIterator end_allowed_args() const;

                ///\}

                virtual bool can_be_negated() const;

                virtual const std::string forwardable_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::shared_ptr<Sequence<std::string> > forwardable_args() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

    extern template class WrappedForwardIterator<args::StringSetArg::ConstIteratorTag, const std::string>;
    extern template class WrappedForwardIterator<args::StringSetArg::AllowedArgConstIteratorTag,
                        const std::pair<std::string, std::string> >;
    extern template class WrappedForwardIterator<args::StringSequenceArg::ConstIteratorTag, const std::string>;
    extern template class WrappedForwardIterator<args::EnumArg::AllowedArgConstIteratorTag,
                        const args::AllowedEnumArg>;
}

#endif
