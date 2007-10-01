/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 * Copyright (c) 2006 Stephen Bennett <spb@gentoo.org>
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

#include <paludis/args/args_visitor.hh>
#include <paludis/util/private_implementation_pattern.hh>

/** \file
 * Declaration for ArgsOption.
 *
 * \ingroup grplibpaludisargs
 */

namespace paludis
{
    namespace args
    {
        class ArgsGroup;

        /**
         * Base class for a command line option.
         *
         * \ingroup grplibpaludisargs
         */
        class PALUDIS_VISIBLE ArgsOption :
            public virtual AcceptInterface<ArgsVisitorTypes>
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
                 * Remove this option.
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
        };

        /**
         * A SwitchArg is an option that can either be specified or not
         * specified, and that takes no value (for example, --help).
         *
         * \ingroup grplibpaludisargs
         */
        class PALUDIS_VISIBLE SwitchArg :
            public ArgsOption,
            public AcceptInterfaceVisitsThis<ArgsVisitorTypes, SwitchArg>
        {
            public:
                /**
                 * Constructor.
                 */
                SwitchArg(ArgsGroup * const group, std::string long_name, char short_name,
                        std::string description);

                ~SwitchArg();
        };

        /**
         * An option that takes a string argument.
         *
         * \ingroup grplibpaludisargs
         */
        class PALUDIS_VISIBLE StringArg :
            public ArgsOption,
            public AcceptInterfaceVisitsThis<ArgsVisitorTypes, StringArg>
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
                const std::string& argument() const { return _argument; }

                /**
                 * Set the argument returned by argument().
                 */
                void set_argument(const std::string& arg);
        };

        /**
         * An option that takes a set of strings.
         *
         * \ingroup grplibpaludisargs
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE StringSetArg :
            public ArgsOption,
            public AcceptInterfaceVisitsThis<ArgsVisitorTypes, StringSetArg>,
            private PrivateImplementationPattern<StringSetArg>
        {
            private:
                void (* _validator) (const std::string &);

            public:
                /**
                 * Helper class for passing available options and associated descriptions
                 * to the StringSetArg constructor.
                 *
                 * \ingroup grplibpaludisargs
                 */
                class PALUDIS_VISIBLE StringSetArgOptions :
                    private PrivateImplementationPattern<StringSetArgOptions>
                {
                    friend class StringSetArg;

                    public:
                        /**
                         * Constructor
                         */
                        StringSetArgOptions(const std::string, const std::string);

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
                        StringSetArgOptions & operator() (const std::string, const std::string);
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

                typedef libwrapiter::ForwardIterator<StringArg, const std::string> ConstIterator;

                ConstIterator begin_args() const;

                ConstIterator end_args() const;

                ///\}

                /**
                 * Add an argument to the set.
                 */
                void add_argument(const std::string & arg);

                ///\name Iterate over our allowed arguments and associated descriptions
                ///\{

                typedef libwrapiter::ForwardIterator<StringSetArg,
                        const std::pair<std::string, std::string> > AllowedArgConstIterator;

                AllowedArgConstIterator begin_allowed_args() const;

                AllowedArgConstIterator end_allowed_args() const;

                ///\}
        };


        /**
         * An AliasArg is an alias for another argument.
         *
         * \ingroup grplibpaludisargs
         */
        class PALUDIS_VISIBLE AliasArg :
            public ArgsOption,
            public AcceptInterfaceVisitsThis<ArgsVisitorTypes, AliasArg>
        {
            private:
                ArgsOption * const _other;

            public:
                /**
                 * Constructor.
                 */
                AliasArg(ArgsOption * const other, const std::string & new_long_name);

                virtual bool specified() const
                {
                    return _other->specified();
                }

                virtual void set_specified(const bool value)
                {
                    _other->set_specified(value);
                }

                /**
                 * Fetch our associated option.
                 */
                ArgsOption * other() const
                {
                    return _other;
                }
        };

        /**
         * An option that takes an integer argument.
         *
         * \ingroup grplibpaludisargs
         */
        class PALUDIS_VISIBLE IntegerArg :
            public ArgsOption,
            public AcceptInterfaceVisitsThis<ArgsVisitorTypes, IntegerArg>
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
        };

        /**
         * An option that takes one of a predefined set of string arguments.
         *
         * \ingroup grplibpaludisargs
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE EnumArg :
            public ArgsOption,
            public AcceptInterfaceVisitsThis<ArgsVisitorTypes, EnumArg>,
            private PrivateImplementationPattern<EnumArg>
        {
            private:
                std::string _argument;
                std::string _default_arg;

            public:
                /**
                 * Helper class for passing available options and associated descriptions
                 * to the EnumArg constructor.
                 *
                 * \ingroup grplibpaludisargs
                 */
                class PALUDIS_VISIBLE EnumArgOptions :
                    private PrivateImplementationPattern<EnumArgOptions>
                {
                    friend class EnumArg;

                    public:
                        /**
                         * Constructor
                         */
                        EnumArgOptions(const std::string, const std::string);

                        /**
                         * Destructor.
                         */
                        ~EnumArgOptions();

                        /**
                         * Adds another (option, description) pair.
                         */
                        EnumArgOptions & operator() (const std::string, const std::string);
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

                typedef libwrapiter::ForwardIterator<EnumArg,
                        const std::pair<std::string, std::string> > AllowedArgConstIterator;

                AllowedArgConstIterator begin_allowed_args() const;

                AllowedArgConstIterator end_allowed_args() const;

                ///\}
        };
    }
}

#endif
