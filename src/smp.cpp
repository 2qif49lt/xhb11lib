#include "smp.h"
#include "reactor.h"

namespace xhb {

struct reactor_deleter {
    void operator()(reactor* p) {
        p->~reactor();
        free(p);
    }
};

thread_local std::unique_ptr<reactor, reactor_deleter> reactor_holder;


std::vector<posix_thread> smp::_threads;
std::vector<reactor*> smp::_reactors;
std::thread::id smp::_tmain;
unsigned smp::count = 1;


void smp::join_all()
{
    for (auto&& t: smp::_threads) {
        t.join();
    }
}

void smp::cleanup() {
    smp::_threads = std::vector<posix_thread>();
}

void smp::pin(unsigned cpu_id) {
    pin_this_thread(cpu_id);
}

void smp::allocate_reactor(unsigned id) {
    assert(!reactor_holder);

    // we cannot just write "local_engin = new reactor" since reactor's constructor
    // uses local_engine
    void *buf;
    int r = posix_memalign(&buf, 64, sizeof(reactor));
    assert(r == 0);
    local_engine = reinterpret_cast<reactor*>(buf);
    new (buf) reactor(id);
    reactor_holder.reset(local_engine);
}

void smp::create_thread(std::function<void ()> thread_loop) {
    _threads.emplace_back(std::move(thread_loop));
}

// Installs handler for Signal which ensures that Func is invoked only once
// in the whole program and that after it is invoked the default handler is restored.
template<int Signal, void(*Func)()>
void install_oneshot_signal_handler() {
    static bool handled = false;
    static spinlock lock;

    struct sigaction sa;
    sa.sa_sigaction = [](int sig, siginfo_t *info, void *p) {
        std::lock_guard<spinlock> g(lock);
        if (!handled) {
            handled = true;
            Func();
            signal(sig, SIG_DFL);
        }
    };
    sigfillset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    if (Signal == SIGSEGV) {
        sa.sa_flags |= SA_ONSTACK;
    }
    auto r = ::sigaction(Signal, &sa, nullptr);
    throw_system_error_on(r == -1);
}

static void sigsegv_action() noexcept {
    print_safe("Segmentation fault\n");
}

static void sigabrt_action() noexcept {
    print_safe("Aborting\n");
}

void smp::configure(resource_config& rc) {
    sigset_t sigs;
    sigfillset(&sigs);

    for (auto sig : {SIGHUP, SIGQUIT, SIGILL, SIGABRT, SIGFPE, SIGSEGV,
            SIGALRM, SIGCONT, SIGSTOP, SIGTSTP, SIGTTIN, SIGTTOU}) {
        sigdelset(&sigs, sig);
    }

    pthread_sigmask(SIG_BLOCK, &sigs, nullptr);

    install_oneshot_signal_handler<SIGSEGV, sigsegv_action>();
    install_oneshot_signal_handler<SIGABRT, sigabrt_action>();

    auto thread_affinity = true;

    smp::count = 1;
    smp::_tmain = std::this_thread::get_id();
    auto nr_cpus = get_pu_count();
    smp::count = nr_cpus;
    _reactors.resize(nr_cpus);

    // huge pages path
    // mlockall

    auto resources = allocate_resource(rc);
    std::vector<cpu_t> allocations = std::move(resources.cpus);
    if (thread_affinity) {
        smp::pin(allocations[0].cpu_id);
    }

    // cpu memory

    static boost::barrier inited(smp::count);
    
    unsigned int i;
    for (i = 1; i < smp::count; i++) {
        auto allocation = allocations[i % allocations.size()];
        create_thread([rc, i, allocation, thread_affinity] {
            stringstream ss;
            ss << "reactor-{" << i << "}";
            auto thread_name = ss.str();
            pthread_setname_np(pthread_self(), thread_name.c_str());
            if (thread_affinity) {
                smp::pin(allocation.cpu_id);
            }
            sigset_t mask;
            sigfillset(&mask);
            for (auto sig : { SIGSEGV }) {
                sigdelset(&mask, sig);
            }
            auto r = ::pthread_sigmask(SIG_BLOCK, &mask, NULL);
            throw_pthread_error(r);
            allocate_reactor(i);
            _reactors[i] = &engine();

            inited.wait();
            engine().configure(rc);
            engine().run();
        });
    }

     allocate_reactor(0);
    _reactors[0] = &engine();

    inited.wait();
    engine().configure(rc);

}

} // xhb namespace