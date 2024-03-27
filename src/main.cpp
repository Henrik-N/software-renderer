#include "_application.h"

int main() {
    Application application;
    if (!Application::init(application)) {
        return EXIT_FAILURE;
    }
    return application.run();
}
