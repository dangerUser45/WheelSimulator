#include <cstdlib>
#include <iostream>
#include <exception>

#include "application.hpp"

int main()
{
    using namespace whsim;
    
    try {
        Application app;
        app.RunLoop();
    }

    catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
