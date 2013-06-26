// Part of ssig -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#if !BOOST_PP_IS_ITERATING
#   error "Do not include this file! Include ssig.hpp instead."
#endif

#define NARGS BOOST_PP_ITERATION()

#define TYPES BOOST_PP_ENUM_PARAMS(NARGS, A)
#define TRAILING_TMPL_PARAMS BOOST_PP_ENUM_TRAILING_PARAMS(NARGS, typename A)

#define PRINT_TYPED_ARG(z, n, _) BOOST_PP_CAT(A, n) BOOST_PP_CAT(arg, n)
#define TYPED_ARGS BOOST_PP_ENUM(NARGS, PRINT_TYPED_ARG, BOOST_PP_EMPTY())
#define PRINT_TYPED_ARG_RREF(z, n, _) BOOST_PP_CAT(A, n)&& BOOST_PP_CAT(arg, n)
#define TRAILING_TYPED_ARGS_RREF BOOST_PP_ENUM_TRAILING(NARGS, PRINT_TYPED_ARG_RREF, BOOST_PP_EMPTY())
#define PRINT_FORWARD_ARG(z, n, _) std::forward<BOOST_PP_CAT(A, n)>(BOOST_PP_CAT(arg, n))
#define ARGS BOOST_PP_ENUM(NARGS, PRINT_FORWARD_ARG, BOOST_PP_EMPTY())

template<typename F TRAILING_TMPL_PARAMS>
void discard_result(F f TRAILING_TYPED_ARGS_RREF)
{
    f(ARGS);
}

template<typename R TRAILING_TMPL_PARAMS>
class Signal<R(TYPES)> {
public:
    typedef boost::function<R(TYPES)> function_type;
    typedef Connection<R(TYPES)> connection_type;

    Signal():
        m_calling(false)
    {
    }

    R const operator() (TYPED_ARGS)
    {
        detail::Calling lock(m_calling);
        while (!m_slots.empty() && m_slots.front()->empty())
            m_slots.pop_front();
        for (auto it = m_slots.begin(); it != m_slots.end(); ++it) {
            for (;;) {
                auto next = boost::next(it);
                if (next == m_slots.end())
                    return (**it)(ARGS); // return last result
                if ((*next)->empty())
                    m_slots.erase_after(it);
                else
                    break;
            }
            (**it)(ARGS); // discard all other results
        }
        return detail::default_result<R>();
    }


    connection_type connect(function_type const& slot);

    bool empty() const {
        for (auto const& slot: m_slots) {
            if (!slot->empty())
                return false;
        }
        return true;
    }


private:
    friend Connection<R(TYPES)>;

    typedef std::forward_list<boost::shared_ptr<function_type>> container_type;
    container_type m_slots;
    bool m_calling;
};

template<typename R TRAILING_TMPL_PARAMS>
class Connection<R(TYPES)>: public ConnectionBase {
public:
    typedef Signal<R(TYPES)> signal_type;
    static_assert(
        std::is_same<typename signal_type::connection_type, Connection>::value,
        "internal error: inconsistent typedef");

    Connection() { } // construct disconnected signal
    Connection(Connection&& rhs):
        m_slot(std::move(rhs.m_slot))
    {
    }

     Connection& operator=(Connection&& rhs)
     {
        m_slot = std::move(rhs.m_slot);
        return *this;
     }

    Connection(signal_type& signal, typename signal_type::function_type const& slot)
    {
        auto slotPointer = boost::make_shared<
            typename signal_type::function_type>(slot);
        m_slot = slotPointer;
        signal.m_slots.push_front(std::move(slotPointer));
    }

    bool isConnected() const
    {
        if (m_slot.expired())
            return false;
        if (m_slot.lock()->empty()) {
            const_cast<Connection<R(TYPES)>*>(this)->m_slot.reset();
            return false;
        }
        return true;
    }
    void disconnect() { checkConnection(); m_slot.lock()->clear(); }
    R invokeSlot(TYPED_ARGS) { checkConnection(); return (*m_slot.lock())(ARGS); }

private:
    void checkConnection() const
    {
        if (!isConnected())
            throw SsigError("attempt to use a disconnected signal");
    }

    boost::weak_ptr<typename signal_type::function_type> m_slot;
};

template<typename R TRAILING_TMPL_PARAMS>
typename Signal<R(TYPES)>::connection_type Signal<R(TYPES)>::connect(function_type const& slot)
{
    return connection_type(*this, slot);
}

template<typename R TRAILING_TMPL_PARAMS>
class ScopedConnection<R(TYPES)>: public Connection<R(TYPES)>
{
    typedef Connection<R(TYPES)> base_t;
    typedef typename base_t::signal_type signal_type;
public:
    ScopedConnection() { }

    ScopedConnection(signal_type& signal, typename signal_type::function_type const& slot):
      base_t(signal, slot)
    { }

    ScopedConnection(base_t const& rhs):
        base_t(rhs)
    { }

    ScopedConnection(base_t&& rhs):
        base_t(std::move(rhs))
    { }

    ScopedConnection(ScopedConnection&& rhs):
        base_t(std::move(rhs))
    { }

    ScopedConnection& operator=(base_t&& rhs)
    {
        base_t::operator=(std::move(rhs));
        return *this;
    }

    ScopedConnection& operator=(ScopedConnection&& rhs)
    {
        base_t::operator=(std::move(rhs));
        return *this;
    }

    ~ScopedConnection()
    {
        if (this->isConnected())
            this->disconnect();
    }
};
