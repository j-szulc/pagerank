
#include <iostream>
#include "lib/networkGenerator.hpp"
#include "../src/multiThreadedPageRankComputer.hpp"

using namespace std;

int main() {

    Page p1("Hello");
    Page p2("Welcome");
    Page p3("Bye");

    const PageId id1("185f8db32271fe25f561a6fc938b2e264306ec304eda518007d1764826381969");
    const PageId id2("0e2226b5235f0ff94a276eb4d07a3bfea74b7e3b8b85e9efca6c18430f041bf8");
    const PageId id3("128901223aac8df3b89cd75d7ec644f9924ed9dcd01e0c65ae99334a3cf9273a");

    p1.addLink(id2);
    p2.addLink(id3);

    const Sha256IdGenerator g;
    Network n{g};
    n.addPage(p1);
    n.addPage(p2);
    n.addPage(p3);
    MultiThreadedPageRankComputer m(2);

    for (auto x : m.computeForNetwork(n, 0.85, 100, 0.0000001)) {
        cout << x << endl;
    };
}