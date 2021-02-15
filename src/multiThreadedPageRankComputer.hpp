#ifndef SRC_MULTITHREADEDPAGERANKCOMPUTER_HPP_
#define SRC_MULTITHREADEDPAGERANKCOMPUTER_HPP_

#include <atomic>
#include <mutex>
#include <thread>
#include <functional>
#include <future>

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "immutable/network.hpp"
#include "immutable/pageIdAndRank.hpp"
#include "immutable/pageRankComputer.hpp"

class MultiThreadedPageRankComputer : public PageRankComputer {
public:
    MultiThreadedPageRankComputer(uint32_t numThreadsArg)
            : numThreads(numThreadsArg) {};

    // Distributes job(0),job(1),...,job(n) across multiple threads
    // and returns AND of the results.
    bool poolAnd(uint32_t n, std::function<bool(int)> job) const {

        std::atomic_int64_t nn(n);

        auto worker = [&]() -> bool {
            bool result = true;
            int64_t jobIndex;
            while ((jobIndex = nn--) >= 0)
                result = result && job(jobIndex);
            return result;
        };

        std::future<bool> results[numThreads];

        for (uint32_t i = 0; i < numThreads; i++)
            results[i] = std::async(worker);

        bool finalResult = true;
        for (std::future<bool> &result : results)
            finalResult = finalResult && result.get();

        return finalResult;
    }

    std::vector<PageIdAndRank> computeForNetwork(Network const & /*network*/, double /*alpha*/, uint32_t /*iterations*/, double /*tolerance*/) const {
        std::vector<PageIdAndRank> result;
        return result;
    }

    std::string getName() const {
        return "MultiThreadedPageRankComputer[" + std::to_string(this->numThreads) + "]";
    }

private:
    uint32_t numThreads;
};

#endif /* SRC_MULTITHREADEDPAGERANKCOMPUTER_HPP_ */
