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
    // and returns future results.
    template<template<typename> class container, typename T, typename S>
    std::vector<std::future<T>> pool(typename container<T>::iterator begin, typename container<T>::iterator end, std::function<void(T &, S &)> job, S init) const {

        auto it = begin;
        std::mutex itMutex;

        auto getJob = [&]() -> auto {
            const std::lock_guard<std::mutex> lockGuard(itMutex);
            return (it == end) ? end : (it++);
        };

        auto worker = [&]() -> S {
            S state = init;
            int64_t jobIndex;
            while ((jobIndex = getJob()) != end)
                job(jobIndex, state);
            return state;
        };

        std::vector<std::future<T>> results;
        results.reserve(numThreads);

        for (uint32_t i = 0; i < numThreads; i++)
            results.push_back(std::async(worker));

        return results;
    }

    template<typename T>
    void waitForAll(std::vector<std::future<T>> futures) {
        for (std::future<T> &future : futures)
            future.wait();
    }

    std::vector<PageIdAndRank> computeForNetwork(Network const &network, double alpha, uint32_t iterations, double tolerance) const {

        //std::vector<PageIdAndRank> result;
        std::unordered_map<PageId, PageRank, PageIdHash> pageHashMap;
        std::vector<Page> const &pages = network.getPages();
        IdGenerator const &generator = network.getGenerator();

        pageHashMap[page.getId()] = 1.0 / network.getSize();


        std::unordered_map<PageId, uint32_t, PageIdHash> numLinks;
        for (auto page : network.getPages()) {
            numLinks[page.getId()] = page.getLinks().size();
        }

        std::unordered_set<PageId, PageIdHash> danglingNodes;
        for (auto page : network.getPages()) {
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

        for (uint32_t i = 0; i < iterations; ++i) {
            std::unordered_map<PageId, PageRank, PageIdHash> previousPageHashMap = pageHashMap;

            double dangleSum = 0;
            for (auto danglingNode : danglingNodes) {
                dangleSum += previousPageHashMap[danglingNode];
            }
            dangleSum = dangleSum * alpha;

            double difference = 0;
            for (auto &pageMapElem : pageHashMap) {
                PageId pageId = pageMapElem.first;

                double danglingWeight = 1.0 / network.getSize();
                pageMapElem.second = dangleSum * danglingWeight + (1.0 - alpha) / network.getSize();

                if (edges.count(pageId) > 0) {
                    for (auto link : edges[pageId]) {
                        pageMapElem.second += alpha * previousPageHashMap[link] / numLinks[link];
                    }
                }
                difference += std::abs(previousPageHashMap[pageId] - pageHashMap[pageId]);
            }

            std::vector<PageIdAndRank> result;
            for (auto iter : pageHashMap) {
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
