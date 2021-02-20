
#include "../src/immutable/common.hpp"

#include "../src/sha256IdGenerator.hpp"

void testSha256(std::string const &testScenario, std::string const &expectedResult) {
    Sha256IdGenerator generator;
    PageId result = generator.generateId(testScenario);
    ASSERT(result == PageId(expectedResult),
           "Incorrect SHA256, scenario=" << testScenario
                                         << ", result=" << result
                                         << ", expectedResult=" << expectedResult);
}

int main() {
    testSha256("Ala ma kota\n", "c51bc001db0206126e1681ba88497ce583f077a92e427e4f62da96b691d28813");
    testSha256("Zawartość naprawdę naprawdę naprawdę wielkiego pliku 1234567890\n", "69bddbdc52992ae9952d3368d48bfe0517ce346d6040e495de3542926294498b");
    testSha256("Hello", "185f8db32271fe25f561a6fc938b2e264306ec304eda518007d1764826381969");
    testSha256("Welcome", "0e2226b5235f0ff94a276eb4d07a3bfea74b7e3b8b85e9efca6c18430f041bf8");
    testSha256("Bye", "128901223aac8df3b89cd75d7ec644f9924ed9dcd01e0c65ae99334a3cf9273a");
    return 0;
}
