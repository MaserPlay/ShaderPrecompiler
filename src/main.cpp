#include <iostream>

#include <argparse/argparse.hpp>

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program(APPNAME, APPVERSION);
    
    program.add_argument("files").remaining();

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    try {
        std::cout << program.get<std::string>("files") << std::endl;
    }
    catch (std::logic_error& e) {
        std::cout << "No files provided" << std::endl;
    }

    return 0;
}