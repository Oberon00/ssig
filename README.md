Title:  ssig README
Author: Christian Neumüller

ssig Simple Signal Library
==========================

ssig is a library which originated from the need for a simple signal library
which is fast enough for signals which are invoked each frame in a game.
Sadly, [Boost.Signals][] was far too slow for this application, so I decided
to write my own library.


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

All functionality is provided through the [`<ssig.hpp>`][mainheader] header.
The `ssig_template.hpp` header must not be included by user code.

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
  which is a delegate of `m_sig##name.connect()`.
* `SSIG_DEFINE_STATICSIGNAL(name, signature)` is similar to
  `SSIG_DEFINE_MEMBERSIGNAL()` but the public connect function is static and
  the private signal is provides in the form of a static function with the
  signature `Signal<signature>& sig_##name()` which contains the Signal as
  a local static variable.

[Boost.PP.limits]: http://www.boost.org/doc/libs/release/libs/preprocessor/doc/headers/config/limits.html


> ssig -- Copyright (c) Christian Neumüller 2012--2013
> This file is subject to the terms of the BSD 2-Clause License.
> See LICENSE.txt or <http://opensource.org/licenses/BSD-2-Clause>.
