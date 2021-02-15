#ifndef SRC_SHA256IDGENERATOR_HPP_
#define SRC_SHA256IDGENERATOR_HPP_

#include "immutable/idGenerator.hpp"
#include "immutable/pageId.hpp"
#include <unistd.h>
#include <sys/wait.h>

#define BUF_SIZE 64

class Sha256IdGenerator : public IdGenerator {
public:
    PageId generateId(std::string const &content) const {
        int shaIn[2];
        pipe(shaIn);

        int shaOut[2];
        pipe(shaOut);

        int shaPID;

        switch (shaPID = fork()) {
            case -1:
                break;
            case 0:
                close(0);
                close(1);

                dup2(shaIn[0], 0);
                dup2(shaOut[1], 1);

                close(shaIn[0]);
                close(shaIn[1]);
                close(shaOut[0]);
                close(shaOut[1]);

                execlp("sha256sum", "sha256sum", NULL);
        }
        close(shaOut[1]);
        close(shaIn[0]);

        write(shaIn[1], content.c_str(), content.size());
        close(shaIn[1]);

        char buf[BUF_SIZE + 1];
        int r = read(shaOut[0], buf, BUF_SIZE);
        close(shaOut[0]);
        buf[r] = '\0';

        waitpid(shaPID, nullptr, 0);

        return std::string(buf);
    }
};

#endif /* SRC_SHA256IDGENERATOR_HPP_ */
