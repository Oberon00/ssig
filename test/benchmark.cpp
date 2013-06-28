#include <iostream> // cout, cerr
#include <boost/timer/timer.hpp>
#include <functional> // std::function
#include <boost/function.hpp>
#include <boost/signal.hpp>
#include <boost/signals2/signal.hpp>
#include <vector>
#include <cstdlib>  // atol
#include <ssig.hpp>

static int dummy = 0;

using std::cout;

namespace {

struct B {
    virtual ~B() {}
    virtual int operator() (int i) const = 0;
};

struct D1: public B {
    virtual int operator() (int i) const { return ++i; }
};

struct D2: public B {
    virtual int operator() (int i) const { return i *= 2; }
};

} // anonymous namespace

static int foo(int i)
{
    return ++i;
}

static void voidfoo(int i)
{
    ++i;
}

template <typename ContainerT>
void test_container(unsigned num_sigs, unsigned num_runs)
{
    cout << "Preparing...";
    ContainerT signals(num_sigs, &foo);

    cout << "\rRunning...     \r";
    boost::timer::auto_cpu_timer t;
    for (unsigned r = 0; r < num_runs; ++r)
        for (auto it = signals.cbegin(), end = signals.cend(); it != end; ++it)
            (*it)(2);
}

template <typename SignalT>
void test_signal(unsigned num_sigs, unsigned num_runs)
{
    cout << "Preparing...";
    SignalT sig;
    for (unsigned i = 0; i < num_sigs; ++i)
        sig.connect(&foo);
    cout << "\rRunning...     \r";
    boost::timer::auto_cpu_timer t;
    for (unsigned r = 0; r < num_runs; ++r)
        sig(2);
}


int main(int argc, char* argv[])
{
#ifndef NDEBUG
    cout << "DEBUG version!\n";
#endif
    if (argc < 3 || argc > 4)
    {
        std::cerr << "Arguments: number_of_signals number_of_runs [skip Boost.Signals* 0|1].\n";
        return EXIT_FAILURE;
    }
    const unsigned num_sigs = atol(argv[1]), num_runs = atol(argv[2]);
    bool skip_boost = false;
    if (argc == 4)
    {
        skip_boost = atol(argv[3]) ? true : false;
    }
    std::cout << "Benchmark with " << num_sigs << " signals and " << num_runs << " runs:\n";
    boost::timer::auto_cpu_timer total;
    try
    {
        cout << "Test 1: std::vector of function pointers\n";
        typedef int(*fn_t)(int);
        test_container<std::vector<fn_t> >(num_sigs, num_runs);

        cout << "Test 2: std::vector of std::function\n";
        test_container<std::vector<std::function<int(int)> > >(num_sigs, num_runs);

        cout << "Test 3: std::vector of boost::function\n";
        test_container<std::vector<boost::function<int(int)> > >(num_sigs, num_runs);

        if (!skip_boost)
        {
            cout << "Test 4: Boost.Signals\n";
            test_signal<boost::signal<int(int)>>(num_sigs, num_runs);
            cout << "Test 5: Boost.Signals2\n";
            test_signal<boost::signals2::signal<int(int)>>(num_sigs, num_runs);
        }


        cout << "Test 6: ssig\n";
        test_signal<ssig::Signal<int(int)>>(num_sigs, num_runs);

        {
            cout << "Test 6a: ssig+void\n";
            cout << "Preparing...";
            ssig::Signal<void(int)> sig;
            for (unsigned i = 0; i < num_sigs; ++i)
                sig.connect(&voidfoo);
            cout << "\rRunning...     \r";
            boost::timer::auto_cpu_timer t;
            for (unsigned r = 0; r < num_runs; ++r)
                sig(2);
        }

        {
            cout << "Test 7: virtual function calls\n";
            cout << "Preparing...";
            B const& d1 = D1();
            B const& d2 = D2();
            std::vector<B const*> signals;
            signals.reserve(num_sigs);
            for (unsigned i = 0; i < num_sigs; ++i) {
                signals.push_back(rand() % 2 ? &d1 : &d2);
            }
            cout << "\rRunning...     \r";
            boost::timer::auto_cpu_timer t;
            for (unsigned r = 0; r < num_runs; ++r)
                for (auto it = signals.cbegin(), end = signals.cend(); it != end; ++it)
                    (**it)(2);
        }
    }
    catch (const std::bad_alloc& e)
    {
        std::cerr << e.what() << "; try decreasing number_of_signals\n";
    }
    std::cout << "Total time (incl. preparation):"; // destruction of total timer prints space + results
}
