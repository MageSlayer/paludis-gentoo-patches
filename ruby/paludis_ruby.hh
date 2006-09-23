/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PALUDIS_GUARD_RUBY_RUBY_PALUDIS_RUBY_HH
#define PALUDIS_GUARD_RUBY_RUBY_PALUDIS_RUBY_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/exception.hh>

namespace paludis
{
    namespace ruby
    {
        void exception_to_ruby_exception(const std::exception &) PALUDIS_ATTRIBUTE((noreturn));

        class RegisterRubyClass :
            public InstantiationPolicy<RegisterRubyClass, instantiation_method::SingletonAsNeededTag>,
            private PrivateImplementationPattern<RegisterRubyClass>
        {
            friend class InstantiationPolicy<RegisterRubyClass, instantiation_method::SingletonAsNeededTag>;

            private:
                RegisterRubyClass();
                ~RegisterRubyClass();

            public:
                class Register;
                friend class Register;

                void execute() const;
        };

        class RegisterRubyClass::Register :
            public InstantiationPolicy<RegisterRubyClass, instantiation_method::NonCopyableTag>
        {
            public:
                Register(void (* func)());
        };
    }
}

#endif
