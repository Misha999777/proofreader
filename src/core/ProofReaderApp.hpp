#pragma once

#include <string>
#include <Windows.h>
#include <saucer/app.hpp>

#include "ProofReaderWindow.hpp"

class ProofReaderApp {
public:
    ProofReaderApp();
    ~ProofReaderApp();

    int run(const std::wstring& cmdLine);

    void toggleWindow();
    void onTextSelected(const std::wstring& text);
    void quit();

private:
    bool enforceSingleInstance();

    HANDLE m_hMutex;
    saucer::application* m_app = nullptr;
    std::unique_ptr<ProofReaderWindow> m_proofReaderWindow;
};
