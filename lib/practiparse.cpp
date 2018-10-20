#include <iostream>

#include "parser.h"
#include "mmap.h"

int main(int argc, char *argv[]) {
    if( argc<2 ) {
        std::cerr << "Usage: practiparse filename\n";

        return 1;
    }

    try {
        Mmap<MapMode::ReadOnly> source(argv[1]);

        auto module = Parser::parse(source.getSlice<const char>());
    } catch(std::exception &error) {
        std::cerr << "Parsing failed: " << error.what() << "\n";

        return 1;
    }

    std::cout << argv[1] << " parsed successfully\n";
}
