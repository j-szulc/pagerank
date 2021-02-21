#ifndef SRC_SHA256IDGENERATOR_HPP_
#define SRC_SHA256IDGENERATOR_HPP_

#include "immutable/idGenerator.hpp"
#include "immutable/pageId.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <fstream>
#include <cstdio>


class Sha256IdGenerator : public IdGenerator {
public:

    PageId generateId(std::string const &content) const {
        char shaIn[L_tmpnam];
        char shaOut[L_tmpnam];

        tmpnam(shaIn);
        std::ofstream shaInFile(shaIn);
        tmpnam(shaOut);

        shaInFile << content;
        shaInFile.close();

        std::string command = "sha256sum " + std::string{shaIn} + " | head -c 64 > " + std::string{shaOut};
        std::system(command.c_str());

        std::ifstream shaOutFile(shaOut);
        std::string output;
        shaOutFile >> output;
        shaOutFile.close();

        remove(shaIn);
        remove(shaOut);

        return output;
    }
};

#endif /* SRC_SHA256IDGENERATOR_HPP_ */
