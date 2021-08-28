#include "headers/engine.hpp"

using namespace std;

int main() {
    GEngine app{800, 600};

#ifdef NDEBUG
    cout << "realse mode" << endl;
#else
    cout << "debug mode" << endl;
#endif

    cout << "test" << endl;
    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}