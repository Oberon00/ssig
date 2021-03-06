// Part of ssig -- Copyright (c) Christian Neum�ller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef SSIG_HPP_INCLUDED
#define SSIG_HPP_INCLUDED SSIG_HPP_INCLUDED

#ifndef SSIG_MAX_ARGS
#   define SSIG_MAX_ARGS 5
#endif

#include <boost/function.hpp>
#include <boost/next_prior.hpp>
#include <boost/preprocessor/enum.hpp>
#include <boost/preprocessor/iteration/iterate.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/control/expr_if.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>

#include <forward_list>
#include <stdexcept>


namespace ssig {

class SsigError: public std::logic_error
{
public:
    SsigError(char const* msg): std::logic_error(msg) { }
    SsigError(std::string const& msg): std::logic_error(msg) { }
};

namespace detail {

template <typename Signature>
struct SignalInvoker;

struct Calling {
    Calling(bool& b): b(b)
    {
        if (b)
            throw SsigError("recursive signal invocation is not supported");
        b = true;
    }
    ~Calling() { b = false; }
private:
    bool& b;

    Calling& operator=(Calling const&); // silence warning
};

} // namespace detail

class ConnectionBase
{
public:
    virtual ~ConnectionBase() {}
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
};


template<class Signature>
class Signal;

template<class Signature>
class Connection;

template<class Signature>
class ScopedConnection;

#define BOOST_PP_ITERATION_LIMITS (0, SSIG_MAX_ARGS)
#define BOOST_PP_FILENAME_1 "ssig_template.hpp"
#include BOOST_PP_ITERATE()

#undef TYPES
#undef TMPL_PARAMS
#undef TRAILING_TMPL_PARAMS

#undef TYPED_ARGS
#undef TRAILING_TYPED_ARGS
#undef ARGS

#undef PRINT_RREF_ARG
#undef TYPED_RREF_ARGS
#undef RREF_TMPL
#undef PRINT_FORWARD_ARG
#undef TRAILING_FWD_ARGS

#define SSIG_DEFINE_MEMBERSIGNAL(name, signature) \
    public:                                                            \
        ssig::Signal<signature>::connection_type const connect_##name( \
            ssig::Signal<signature>::function_type const& slot)        \
        {                                                              \
            return m_sig_##name.connect(slot);                         \
        }                                                              \
    private: ssig::Signal<signature> m_sig_##name;


#define SSIG_DEFINE_STATICSIGNAL(name, signature) \
    public:                                                                   \
        static ssig::Signal<signature>::connection_type const connect_##name( \
            ssig::Signal<signature>::function_type const& slot)               \
        {                                                                     \
            return sig_##name().connect(slot);                                \
        }                                                                     \
    private:                                                                  \
        static ssig::Signal<signature>& sig_##name()                          \
        {                                                                     \
            static ssig::Signal<signature> sig;                               \
            return sig;                                                       \
        }

} // namespace ssig
#endif
