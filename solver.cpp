#include <iostream>
#include <unistd.h>
#include <vector>
#include <signal.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    std::vector<pid_t> kids;

    int solver_to_traffic[2];
    pipe(solver_to_traffic);
    int traffic_to_parser[2];
    pipe(traffic_to_parser);
    

    pid_t child_pid;
    child_pid = fork();
    if (child_pid == 0) {
        dup2(traffic_to_parser[1], STDOUT_FILENO);  // traffic out -> traffic_to_parser
        close(traffic_to_parser[0]);
        close(traffic_to_parser[1]);

        dup2(solver_to_traffic[0], STDIN_FILENO); // solver_to_traffic -> traffic in
        close(solver_to_traffic[0]);
        close(solver_to_traffic[1]);

        execlp("python3", "python3", "traffic.py", nullptr);
    } else if (child_pid < 0) {
        std::cerr << "Error: could not fork" << std::endl;
    }
    kids.push_back(child_pid);

    child_pid = fork();
    if (child_pid == 0) {
        dup2(traffic_to_parser[0], STDIN_FILENO);  // traffic_to_parser -> parser in
        close(traffic_to_parser[0]);
        close(traffic_to_parser[1]);

        close(solver_to_traffic[0]);
        close(solver_to_traffic[1]);

        execl("./parser", nullptr);
    } else if (child_pid < 0) {
        std::cerr << "Error: could not fork" << std::endl;
    }
    kids.push_back(child_pid);

    close(traffic_to_parser[0]);
    close(traffic_to_parser[1]);

    dup2(solver_to_traffic[1], STDOUT_FILENO);  // solver out -> solver_to_traffic
    close(solver_to_traffic[0]);
    close(solver_to_traffic[1]);

    std::string line;

    sleep(3);
    while (true) {
        if (std::getline(std::cin, line)) {
            std::cout << line << std::endl;
        } else if (std::cin.eof()) {
            break;
        } else {
            std::cerr << "Error: cin error" << std::endl;
            break;
        }
    }

    for (pid_t k: kids) {
        kill(k, SIGTERM);
    }
    return 0;
}
