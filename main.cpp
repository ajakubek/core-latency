#include <pthread.h>
#include <unistd.h>

#include <atomic>
#include <exception>
#include <thread>

#include <nonius/nonius.h++>

enum State
{
  Preparing,
  Ready,
  Ping,
  Pong,
  Finish,
};

class Sync
{
public:
  State wait_as_long_as(State wait_state)
  {
    State loaded_state = state.load();
    while (loaded_state == wait_state)
      loaded_state = state.load();
    return loaded_state;
  }

  void wait_until(State expected_state)
  {
    while (state.load() != expected_state)
    {
    }
  }

  void set(State new_state)
  {
    state.store(new_state);
  }

private:
  std::atomic<State> state{Preparing};
};

static void set_affinity(unsigned int cpu_num)
{
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpu_num, &cpuset);
  pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}

struct LatencyBench
{
  LatencyBench(long first_cpu_, long second_cpu_)
    : first_cpu{first_cpu_}
    , second_cpu{second_cpu_}
  {
  }

  void operator()(nonius::chronometer meter) const
  {
    Sync sync;

    set_affinity(first_cpu);

    std::thread t([&] {
      set_affinity(second_cpu);
      sync.set(Ready);

      State state = sync.wait_as_long_as(Ready);
      while (state != Finish)
      {
        if (state == Ping)
          sync.set(Pong);
        state = sync.wait_as_long_as(Pong);
      }
    });

    sync.wait_until(Ready);

    meter.measure([&] {
      sync.set(Ping);
      sync.wait_until(Pong);
    });

    sync.set(Finish);
    t.join();
  }

  const long first_cpu;
  const long second_cpu;
};

int main()
{
  const long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);

  for (long i = 0; i < num_cpus; ++i)
    for (long j = i + 1; j < num_cpus; ++j)
      nonius::global_benchmark_registry().emplace_back(
        "latency between CPU " + std::to_string(i) + " and " + std::to_string(j),
        LatencyBench(i, j));

  try
  {
    nonius::go(nonius::configuration{});
    return 0;
  }
  catch (const std::exception& exc)
  {
    std::cerr << "Error: " << exc.what() << '\n';
    return 1;
  }
  catch (...)
  {
    std::cerr << "Unknown error\n";
    return 1;
  }
}
