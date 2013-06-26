// Part of ssig -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "ssig.hpp"

#define BOOST_TEST_MODULE SsigTest
#include <boost/test/unit_test.hpp>

using namespace ssig; // Don't do this at home.

BOOST_AUTO_TEST_SUITE(nullary_signals)

namespace {

unsigned g_numFooCalls = 0;

unsigned long foo()
{
    return ++g_numFooCalls;
}

void discard_test()
{
    std::function<void()> f([](){ foo(); });
}

void prepare()
{
    g_numFooCalls = 0;
}

template<typename R>
void checkNullaryFooConnection(Connection<R()>& c, Signal<R()>& s)
{
    BOOST_CHECK(c.isConnected());
    bool lambdaCalled = false;
    prepare();
    Connection<R()> cl = s.connect([&lambdaCalled]()->R{ lambdaCalled = true; return R(); });
    BOOST_CHECK(!s.empty());
    BOOST_CHECK(c.isConnected());
    unsigned numFooCalls = c.invokeSlot();
    BOOST_CHECK_EQUAL(numFooCalls, 1);
    BOOST_CHECK_EQUAL(numFooCalls, g_numFooCalls);
    BOOST_CHECK(!lambdaCalled);
    numFooCalls = s();
    BOOST_CHECK_EQUAL(numFooCalls, 2);
    BOOST_CHECK_EQUAL(numFooCalls, g_numFooCalls);
    BOOST_CHECK(lambdaCalled);
    cl.disconnect();
    c.disconnect();
    BOOST_CHECK(s.empty());
    BOOST_CHECK_THROW(c.disconnect(), SsigError);
}

}

BOOST_AUTO_TEST_CASE(signal_construction)
{
    {
        Signal<double()> s;
        BOOST_CHECK(s.empty());
    }
    {
        Signal<void()> s;
        BOOST_CHECK(s.empty());
    }

}

BOOST_AUTO_TEST_CASE(connecting_and_invoking_void)
{
    unsigned test = 2;

    Signal<void()> s;
    s(); // void return type: should not throw
    s.connect([&test](){test += 4;});
    BOOST_CHECK(!s.empty());
    s.connect([&test](){test /= 2;});
    s();
    BOOST_CHECK_EQUAL(test, 2 / 2 + 4);
}

BOOST_AUTO_TEST_CASE(connecting_and_invoking_basic)
{
    prepare();

    Signal<unsigned long()> s;
    s.connect(&foo);
    BOOST_CHECK(!s.empty());
    s.connect(&foo);
    unsigned const numFooCalls = s();
    BOOST_CHECK_EQUAL(numFooCalls, 2);
    BOOST_CHECK_EQUAL(numFooCalls, g_numFooCalls);
}

BOOST_AUTO_TEST_CASE(connecting_and_invoking_illegal)
{
    Signal<double()> s;
    BOOST_CHECK_THROW(s(), SsigError);
}

BOOST_AUTO_TEST_CASE(connection)
{
    {
        Signal<short()> s;
        Connection<short()> c = s.connect(&foo);
        Connection<short()> c2 = c;
        Connection<short()> c3 = c2;
        c2.disconnect();
        BOOST_CHECK(!c2.isConnected());
        BOOST_CHECK(!c.isConnected());
        BOOST_CHECK(!c3.isConnected());
        BOOST_CHECK(s.empty());
        Connection<short()> c4 = s.connect(&foo);
        checkNullaryFooConnection(c4, s);
    }

    {
        Signal<wchar_t()> s;
        Connection<wchar_t()> c(s, &foo);
        checkNullaryFooConnection(c, s);
    }
}

BOOST_AUTO_TEST_CASE(scoped_connection)
{
    Signal<short()> s;

    {
        ScopedConnection<short()> c = s.connect(&foo);
        checkNullaryFooConnection(c, s);
    }
    BOOST_CHECK(s.empty());

    {
        ScopedConnection<short()> c = s.connect(&foo);
        c.disconnect();
        BOOST_CHECK(s.empty());
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(unary_signals)

namespace {

unsigned g_numFooCalls = 0;

unsigned long foo(int i)
{
    return ++g_numFooCalls + i;
}

void prepare()
{
    g_numFooCalls = 0;
}

template<typename R, typename A>
void checkUnaryFooConnection(Connection<R(A)>& c, Signal<R(A)>& s)
{
    static const A arg = 4;

    bool lambdaCalled = false;
    prepare();
    Connection<R(A)> cl = s.connect([&lambdaCalled](A)->R{ lambdaCalled = true; return R(); });
    BOOST_CHECK(!s.empty());
    BOOST_CHECK(c.isConnected());
    unsigned numFooCalls = c.invokeSlot(arg) - arg;
    BOOST_CHECK_EQUAL(numFooCalls, 1);
    BOOST_CHECK_EQUAL(numFooCalls, g_numFooCalls);
    BOOST_CHECK(!lambdaCalled);
    numFooCalls = s(arg) - arg;
    BOOST_CHECK_EQUAL(numFooCalls, 2);
    BOOST_CHECK_EQUAL(numFooCalls, g_numFooCalls);
    BOOST_CHECK(lambdaCalled);
    cl.disconnect();
    c.disconnect();
    BOOST_CHECK(s.empty());
    BOOST_CHECK_THROW(c.disconnect(), SsigError);
}

}

BOOST_AUTO_TEST_CASE(signal_construction_un)
{
    {
        Signal<double(float)> s;
        BOOST_CHECK(s.empty());
    }
    {
        Signal<void(int)> s;
        BOOST_CHECK(s.empty());
    }

}

BOOST_AUTO_TEST_CASE(connecting_and_invoking_void_un)
{
    double test = 16;

    Signal<void(int)> s;
    s.connect([&test](int arg){test += 4 + arg;});
    BOOST_CHECK(!s.empty());
    s.connect([&test](int arg){test /= (2 + arg);});
    static const int arg = 2;
    s(arg);
    BOOST_CHECK_EQUAL(test, 16 / (2 + arg) + (4 + arg));
}

BOOST_AUTO_TEST_CASE(connecting_and_invoking_basic_un)
{
    prepare();

    Signal<unsigned long(short)> s;
    s.connect(&foo);
    BOOST_CHECK(!s.empty());
    s.connect(&foo);
    static const unsigned arg = 3;
    unsigned const numFooCalls = s(arg) - arg;
    BOOST_CHECK_EQUAL(numFooCalls, 2);
    BOOST_CHECK_EQUAL(numFooCalls, g_numFooCalls);
}

BOOST_AUTO_TEST_CASE(connecting_and_invoking_illegal_un)
{
    Signal<double(std::logic_error)> s;
    BOOST_CHECK_THROW(s(std::logic_error("")), SsigError);
}

static bool disconnect_arg(ConnectionBase* c)
{
    c->disconnect();
    return false;
}

static bool nop(ConnectionBase*)
{
    return true;
}


BOOST_AUTO_TEST_CASE(disconnect_while_called)
{
    Signal<bool(ConnectionBase*)> s;
    {
        auto c = s.connect(&disconnect_arg);
        s(&c);
        BOOST_CHECK(!c.isConnected());
        BOOST_CHECK(s.empty());
    }
    {
        ScopedConnection<bool(ConnectionBase*)> c1 = s.connect(&nop);
        auto c = s.connect(&disconnect_arg);
        ScopedConnection<bool(ConnectionBase*)> c2 = s.connect(&nop);
        s(&c);
        BOOST_CHECK(!c.isConnected());
    }
    BOOST_CHECK(s.empty());
}

BOOST_AUTO_TEST_CASE(disconnect_next_called)
{
    Signal<bool(ConnectionBase*)> s;
    {
        auto c = s.connect(&nop);
        ScopedConnection<bool(ConnectionBase*)> c1 = s.connect(&disconnect_arg);
        BOOST_CHECK_EQUAL(s(&c), false);
        BOOST_CHECK(!c.isConnected());
    }
    BOOST_CHECK(s.empty());
}

static void checkList(std::forward_list<int> l)
{
    BOOST_CHECK(!l.empty());
}

BOOST_AUTO_TEST_CASE(move_vulnerability)
{
    Signal<void(std::forward_list<int>)> s;
    s.connect(&checkList);
    s.connect(&checkList);
    std::forward_list<int> l(1);
    s(l);
}

BOOST_AUTO_TEST_CASE(connection_un)
{
    {
        Signal<short(int)> s;
        Connection<short(int)> c = s.connect(&foo);
        checkUnaryFooConnection(c, s);
    }

    {
        Signal<wchar_t(short)> s;
        Connection<wchar_t(short)> c(s, &foo);
        checkUnaryFooConnection(c, s);
    }
}

BOOST_AUTO_TEST_CASE(scoped_connection_un)
{
    Signal<short(long)> s;

    {
        ScopedConnection<short(long)> c = s.connect(&foo);
        checkUnaryFooConnection(c, s);
    }
    BOOST_CHECK(s.empty());

    {
        ScopedConnection<short(long)> c = s.connect(&foo);
        c.disconnect();
        BOOST_CHECK(s.empty());
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(binary_signals)

namespace {

unsigned g_numFooCalls = 0;

unsigned long foo(int i, int j)
{
    return ++g_numFooCalls + i - j;
}

void prepare()
{
    g_numFooCalls = 0;
}

template<typename R, typename A1, typename A2>
void checkBinaryFooConnection(Connection<R(A1, A2)>& c, Signal<R(A1, A2)>& s)
{
    static const A1 arg = 4;

    bool lambdaCalled = false;
    prepare();
    Connection<R(A1, A2)> cl = s.connect([&lambdaCalled](A1, A2)->R{ lambdaCalled = true; return R(); });
    BOOST_CHECK(!s.empty());
    BOOST_CHECK(c.isConnected());
    unsigned numFooCalls = c.invokeSlot(arg, arg);
    BOOST_CHECK_EQUAL(numFooCalls, 1);
    BOOST_CHECK_EQUAL(numFooCalls, g_numFooCalls);
    BOOST_CHECK(!lambdaCalled);
    numFooCalls = s(arg, arg);
    BOOST_CHECK_EQUAL(numFooCalls, 2);
    BOOST_CHECK_EQUAL(numFooCalls, g_numFooCalls);
    BOOST_CHECK(lambdaCalled);
    cl.disconnect();
    c.disconnect();
    BOOST_CHECK(s.empty());
    BOOST_CHECK_THROW(c.disconnect(), SsigError);
}

}

BOOST_AUTO_TEST_CASE(signal_construction_bin)
{
    {
        Signal<double(float, wchar_t)> s;
        BOOST_CHECK(s.empty());
    }
    {
        Signal<void(int, std::div_t)> s;
        BOOST_CHECK(s.empty());
    }

}

BOOST_AUTO_TEST_CASE(connecting_and_invoking_void_bin)
{
    double test = 16;

    Signal<void(int, int)> s;
    s.connect([&test](int arg, int arg2){test += 4 + arg - arg2;});
    BOOST_CHECK(!s.empty());
    s.connect([&test](int arg, int arg2){test /= (2 + arg - arg2);});
    static const int arg = 2;
    s(arg, arg);
    BOOST_CHECK_EQUAL(test, 16 / 2 + 4);
}

BOOST_AUTO_TEST_CASE(connecting_and_invoking_basic_bin)
{
    prepare();

    Signal<unsigned long(short, int)> s;
    s.connect(&foo);
    BOOST_CHECK(!s.empty());
    s.connect(&foo);
    static const int arg = 3;
    unsigned const numFooCalls = s(arg, -arg) - 2 * arg;
    BOOST_CHECK_EQUAL(numFooCalls, 2);
    BOOST_CHECK_EQUAL(numFooCalls, g_numFooCalls);
}

BOOST_AUTO_TEST_CASE(connecting_and_invoking_illegal_un)
{
    Signal<double(std::logic_error, void*)> s;
    BOOST_CHECK_THROW(s(std::logic_error(""), nullptr), SsigError);
}

BOOST_AUTO_TEST_CASE(connection_bin)
{
    {
        Signal<short(int, int)> s;
        Connection<short(int, int)> c = s.connect(&foo);
        checkBinaryFooConnection(c, s);
    }

    {
        Signal<wchar_t(short, short)> s;
        Connection<wchar_t(short, short)> c(s, &foo);
        checkBinaryFooConnection(c, s);
    }
}

BOOST_AUTO_TEST_CASE(scoped_connection_bin)
{
    struct Foo { int f(bool){return int();} };
    typedef int (Foo::*fn)(bool);
    Signal<short(long, long)> s;

    {
        ScopedConnection<short(long, long)> c = s.connect(&foo);
        checkBinaryFooConnection(c, s);
    }
    BOOST_CHECK(s.empty());

    {
        ScopedConnection<short(long, long)> c = s.connect(&foo);
        c.disconnect();
        BOOST_CHECK(s.empty());
    }
}

BOOST_AUTO_TEST_SUITE_END()
