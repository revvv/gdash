#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>


std::string to_identifier(std::string const & fn) {
    return fn.substr(0, fn.find('.'));
}


std::string to_name(std::string const & fn) {
    std::string to_name = to_identifier(fn);
    for (auto & c : to_name)
        if (c == '_')
            c = ' ';
    return to_name;
}


void print_file(std::string file_to_name) {
    std::ifstream is(file_to_name, std::ios_base::binary);
    if (!is) {
        std::cerr << "Error opening file " << file_to_name << std::endl;
        exit(2);
    }
    std::cout << "static unsigned char const " << to_identifier(file_to_name) << "[] = {";
    int pos = 0;
    char c;
    while (is.get(c)) {
        if (pos == 0)
            std::cout << "\n  ";
        std::cout << static_cast<unsigned int>(static_cast<unsigned char>(c)) << ",";
        pos += 1;
        if (pos > 20)
            pos = 0;
    }
    std::cout << std::endl << "};" << std::endl;
}


void print_files(std::vector<std::string> filenames) {
    for (auto const & filename : filenames)
        print_file(filename);
}


void print_arrays(std::string arrayname_prefix, std::vector<std::string> filenames) {
    if (arrayname_prefix == "")
        return;
    std::cout << "static unsigned char const *" << arrayname_prefix << "_pointers[] = { ";
    for (auto const & filename : filenames)
        std::cout << to_identifier(filename) << ", ";
    std::cout << "nullptr };" << std::endl;
    std::cout << "static char const *" << arrayname_prefix << "_names[] = { ";
    for (auto const & filename : filenames)
        std::cout << "\"" << to_name(filename) << "\", ";
    std::cout << "nullptr };" << std::endl;
}


int main(int argc, char *argv[]) {
    if (argc-1 < 2) {
        std::cerr << "Usage: " << argv[0] << " <arrayto_name> <files...>" << std::endl;
        exit(1);
    }
    
    std::string arrayname_prefix = argv[1];
    std::vector<std::string> filenames(argv + 2, argv + argc);
    
    print_files(filenames);
    print_arrays(arrayname_prefix, filenames);
}
