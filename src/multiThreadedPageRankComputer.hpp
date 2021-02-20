#ifndef SRC_MULTITHREADEDPAGERANKCOMPUTER_HPP_
#define SRC_MULTITHREADEDPAGERANKCOMPUTER_HPP_

#include <atomic>
#include <mutex>
#include <thread>
#include <functional>
#include <future>

#ifndef DNDEBUG

#include <iostream>

using namespace std;
#endif

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "immutable/network.hpp"
#include "immutable/pageIdAndRank.hpp"
#include "immutable/pageRankComputer.hpp"
#include "sha256IdGenerator.hpp"

class MultiThreadedPageRankComputer : public PageRankComputer {

    using Nothing = struct {
    };

    using PageRankHashMap = std::unordered_map<PageId, PageRank, PageIdHash>;

public:
    MultiThreadedPageRankComputer(uint32_t numThreadsArg)
            : numThreads(numThreadsArg) {};

    // Distributes job(0),job(1),...,job(n) across multiple threads
    // and returns future results.
    template<typename T, typename S, typename Iterator>
    std::vector<std::future<S>> pool(Iterator begin, Iterator end, std::function<void(T, S &)> work, S init) const {

        auto it = make_shared<Iterator>(begin);
        auto itMutex = make_shared<std::mutex>();

        auto getJob = [=]() -> auto {
            const std::lock_guard<std::mutex> lockGuard(*itMutex);
            return (*it == end) ? end : ((*it)++);;
        };

        auto worker = [=]() -> S {
            S state = init;
            Iterator nextJob;
            while ((nextJob = getJob()) != end)
                work(*nextJob, state);
            return state;
        };

        std::vector<std::future<S>> results;
        results.reserve(numThreads);

        for (uint32_t i = 0; i < numThreads; i++)
            results.push_back(std::async(worker));

        return results;
    }

    template<typename T>
    void waitForAll(std::vector<std::future<T>> &futures) const {
        for (std::future<T> &future : futures)
            future.wait();
    }

    template<typename T, typename Iterator>
    void poolAndWait(Iterator begin, Iterator end, std::function<void(T)> work) const {
        std::function<void(T, Nothing &)> workAndReturnNothing = [work](T t, Nothing &) { work(t); };
        auto futures = pool(begin, end, workAndReturnNothing, {});
        waitForAll(futures);
    }

    std::vector<PageIdAndRank> computeForNetwork(Network const &network, double alpha, uint32_t iterations, double tolerance) const {

        auto &pages = network.getPages();
        Sha256IdGenerator generator{};

        std::function<void(Page const &)> generatingFun = [generator](Page const &page) { page.generateId(generator); };
        poolAndWait(pages.begin(), pages.end(), generatingFun);

        PageRankHashMap map1;
        PageRankHashMap map2;

        auto initialPageRank = 1.0 / network.getSize();
        for (auto const &page : pages) {
            map1[page.getId()] = initialPageRank;
            map2[page.getId()] = initialPageRank;
        }

        std::unordered_map<PageId, uint32_t, PageIdHash> numLinks;
        for (auto page : pages) {
            numLinks[page.getId()] = page.getLinks().size();
        }

        std::unordered_set<PageId, PageIdHash> danglingNodes;
        for (auto page : pages) {
            if (page.getLinks().size() == 0) {
                danglingNodes.insert(page.getId());
            }
        }

        std::unordered_map<PageId, std::vector<PageId>, PageIdHash> edges;
        for (auto page : network.getPages()) {
            for (auto link : page.getLinks()) {
                edges[link].push_back(page.getId());
            }
        }

        PageRankHashMap *previousPageHashMap = &map1;
        PageRankHashMap *pageHashMap = &map2;

        for (uint32_t i = 0; i < iterations; ++i) {
            swap(previousPageHashMap, pageHashMap);

            double dangleSum = 0;
            for (auto danglingNode : danglingNodes) {
                dangleSum += (*previousPageHashMap)[danglingNode];
            }
            dangleSum = dangleSum * alpha;

            double difference = 0;
            for (auto &pageMapElem : *pageHashMap) {
                PageId pageId = pageMapElem.first;

                double danglingWeight = 1.0 / network.getSize();
                pageMapElem.second = dangleSum * danglingWeight + (1.0 - alpha) / network.getSize();

                if (edges.count(pageId) > 0) {
                    for (auto link : edges[pageId]) {
                        pageMapElem.second += alpha * (*previousPageHashMap)[link] / numLinks[link];
                    }
                }
                difference += std::abs((*previousPageHashMap)[pageId] - (*pageHashMap)[pageId]);
            }

            std::vector<PageIdAndRank> result;
            for (auto iter : *pageHashMap) {
                result.push_back(PageIdAndRank(iter.first, iter.second));
            }

            ASSERT(result.size() == network.getSize(), "Invalid result size=" << result.size() << ", for network" << network);

            if (difference < tolerance) {
                return result;
            }
        }

        ASSERT(false, "Not able to find result in iterations=" << iterations);

    }

    std::string getName() const {
        return "MultiThreadedPageRankComputer[" + std::to_string(this->numThreads) + "]";
    }

private:
    uint32_t numThreads;
};

#endif /* SRC_MULTITHREADEDPAGERANKCOMPUTER_HPP_ */
