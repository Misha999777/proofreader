#include "core/ProofReaderApp.hpp"

#include <string>
#include <Windows.h>

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    ProofReaderApp app;
    return app.run(pCmdLine);
}
