Title:  ssig README
Author: Christian Neumüller

ssig Simple Signal Library
==========================

ssig is a library which originated from the need for a simple signal library
which is fast enough for signals which are invoked each frame in a game.
Sadly, [Boost.Signals][] was far too slow for this application, so I decided
to write my own library. You may find the [results of a benchmark](#benchmark)
at the end of this README.


[Boost.Signals]: http://www.boost.org/libs/signals


Building
--------

ssig is header-only and requires [Boost][] and a C++11 compiler.

The [CMake][] files are provided to support CMake's `find_package()`, provide
install rules and run the unit test.

If you just want to use the library, all you need to do is making the files
in the `include` directory `#include`able (e.g. by adding them to your
compiler's include path or by simply copying them to the referencing project).


[Boost]: http://www.boost.org/
[CMake]: http://www.cmake.org/


Usage
-----

All functionality is provided through the [`<ssig.hpp>`][mainheader] header
in `namespace ssig`. Note: The `ssig_template.hpp` header must not be included
by user code.

You may want to look into the unit test at [`test/test.cpp`][test] for usage
examples.

[mainheader]: include/ssig.hpp
[test]: test/test.cpp

### `class Signal<Signature>`

    template<typename R, typename A0, typename A1, ...>
    class Signal<R(A0, A1, ...)>

This class provides means to connect callable entities and call them.

* `Connection<Signature> connect(boost::function<Signature> const& slot)`
   connects `slot` to the signal, meaning it will be called on each invocation
   of the Signal's `operator()`. The returned Connection can be used to
   disconnect the signal.

* `R operator(A0, A1, ...)` calls all connected slots in reverse order of
   connection (LIFO) with the given arguments.

    The return value is the one of the slot called last (i.e. connected first).
  For non-void return types, a `SsigError` is thrown when an attempt is made
  to invoke an empty Signal.

    It is allowed to disconnect any slots of the signal while (i.e. from a
  function called by the slot, not really concurrently from another thread) it
  or even the slot itself is invoked. What is not allowed, however, are
  recursive calls of the same Signal's `operator()` because then the previous
  would lead to undefined behavior. A `SsigError` is thrown in this case.

* `bool empty() const` returns true when no slots are connected to the signal.

* `~Signal()` destructor: Disconnects all slots (i.e. making their
   `isConnected()` property false) and destroys the object.


### `class Connection<Signature>`

    template<typename R, typename A0, typename A1, ...>
    class Connection<R(A0, A1, ...)>: public ConnectionBase

This class provides means to disconnect or invoke a particular connected
slot of a Signal.

* `Connection()` constructor: Constructs a disconnected connection.

* `Connection(Signal<Signature>& signal, boost::function<Signature> const&)`
  constructor:
  Writing `Connection<R(A0, A1, ...)> connection(signal, slot)` is equivalent
  to `auto connection = signal.connect(slot)`, i.e. this method connects
  `slot` to `signal` and then represents this connection.

* `~Connection()` destructor: destroys the Connection object *without*
  disconnecting the slot.

* `bool isConnected() const` returns if the Connection has not been
   disconnected, i.e. is currently connecting a slot to a signal.

* `void disconnect()` disconnects the slot. Requires `isConnected()`.

* `R invokeSlot(A0, A1, ...)` invokes the slot. Requires `isConnected()`.

Any methods requiring `isConnected()` will throw a `SsigError` if this
requirement is violated.

#### Copy semantics
If you copy a Connection, the slot is *not* connected again. This means
that the slot will not be called additional times when invoking the signal and
that disconnecting one copy of a Connection will disconnect all other copies.
Moving a Connection, however, causes only the moved-from object to be
disconnected (as you would expect).


### `class ScopedConnection<Signature>`

    template<typename R, typename A0, typename A1, ...>
    class ScopedConnection<R(A0, A1, ...)>: public Connection<R(A0, A1, ...)>

`ScopedConnection` is a wrapper around a `Connection` which disconnects it in
the destructor, if it is (still) connected. Note that copies are not disabled.
As soon as the first copy is destroyed the slot (if (still) connected) will
be disconnected. Moving, however is supported and does not cause the
moved-from `ScopedConnection` to disconnect the slot (still making the
moved-from object disconnected of course).


### `class ConnectionBase`

This abstract template-less class provides only a virtual destructor and pure
virtual `disconnect()` and `isConnected()` methods (as implemented by
`Connection<Signature>`. It can be used to store Connections of different
types.


### `class SsigError`

    class SsigError: public std::logic_error

This exception class is thrown by various functions of ssig.


### Macros
* `SSIG_MAX_ARGS` can be defined to a positive integer specifying the maximum
  number of slot arguments supported by ssig. It defaults to 5 and is subject
  to the [limitations of Boost.Preprocessor][Boost.PP.limits].
* `SSIG_DEFINE_MEMBERSIGNAL(name, signature)` convenience macro that must
  be used at class scope and defines a private
  `Signal<signature> m_sig_##name` member and a public `connect_##name()`
  which is a delegate of `m_sig_##name.connect()`.
* `SSIG_DEFINE_STATICSIGNAL(name, signature)` is similar to
  `SSIG_DEFINE_MEMBERSIGNAL()` but the public connect function is static and
  the private signal is provides in the form of a static function with the
  signature `Signal<signature>& sig_##name()` which contains the Signal as
  a local static variable.


[Boost.PP.limits]: http://www.boost.org/doc/libs/release/libs/preprocessor/doc/headers/config/limits.html


Benchmark
---------
Running the code from [`test/benchmark.cpp`][bmcode] at commit ef32b04edd
compiled with MSVC11 in release mode yields the following on my Intel Core i5
430M Windows 7 x64 notebook (with the arguments mentioned in the output):

    Benchmark with 100000 signals and 10000 runs:
    Test 1: std::vector of function pointers
     2.874066s wall, 2.854818s user + 0.000000s system = 2.854818s CPU (99.3%)
    Test 2: std::vector of std::function
     5.484764s wall, 5.366434s user + 0.000000s system = 5.366434s CPU (97.8%)
    Test 3: std::vector of boost::function
     6.032948s wall, 5.928038s user + 0.000000s system = 5.928038s CPU (98.3%)
    Test 4: Boost.Signals
     114.299385s wall, 112.554722s user + 0.078001s system = 112.632722s CPU (98.5%)
    Test 5: Boost.Signals2
     77.404713s wall, 76.596491s user + 0.000000s system = 76.596491s CPU (99.0%)
    Test 6: ssig
     14.829950s wall, 14.508093s user + 0.000000s system = 14.508093s CPU (97.8%)
    Test 6a: ssig+void
     14.184894s wall, 14.008890s user + 0.000000s system = 14.008890s CPU (98.8%)
    Test 7: virtual function calls
     8.451679s wall, 8.424054s user + 0.000000s system = 8.424054s CPU (99.7%)
    Total time (incl. preparation): 244.018221s wall, 240.615942s user + 0.093601s system = 240.709543s CPU (98.6%)

What immediately catches the eye is that Boost.Signals needs over 112 seconds,
being even far slower than Boost.Signals2, which is interesting since Signals
is, contrary to Signals2 not even thread safe. In the following table, the
total CPU times of each test are summarized, with factors normalized to ssig
and rounded to two places after the decimal point:

<table>
    <thead><tr><td>Test</td> <td>Time (s)</td> <td>Factor</td></tr></thead>
    <tbody>
    <tr><td>vector of function pointers</td> <td>2.854818</td> <td>0.2</td></tr>
    <tr><td>vector of <code>std::function</code></td> <td>5.366434</td> <td>0.37</td></tr>
    <tr><td>vector of <code>boost::function</code></td> <td>5.928038</td> <td>0.41</td></tr>
    <tr><td>Boost.Signals</td> <td>112.632722</td> <td>7.76</td></tr>
    <tr><td>Boost.Signals2</td> <td>76.596491</td> <td>5.28</td></tr>
    <tr><td>ssig</td> <td>14.508093</td> <td>1</td></tr>
    <tr><td>ssig+void</td> <td>14.008890</td> <td>0.97</td></tr>
    <tr><td>virtual functions</td> <td>8.424054</td> <td>0.58</td></tr>
    </tbody>
</table>



[bmcode]: test/benchmark.cpp


> ssig -- Copyright (c) Christian Neumüller 2012--2013
> This file is subject to the terms of the BSD 2-Clause License.
> See LICENSE.txt or <http://opensource.org/licenses/BSD-2-Clause>.
