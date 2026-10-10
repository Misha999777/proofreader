#include "ProofReaderApp.hpp"

#include "TrayIcon.hpp"

ProofReaderApp::ProofReaderApp() : m_hMutex(NULL) {}

ProofReaderApp::~ProofReaderApp() {
    if (m_hMutex) {
        CloseHandle(m_hMutex);
    }
}

bool ProofReaderApp::enforceSingleInstance() {
    m_hMutex = CreateMutexW(NULL, TRUE, L"ProofReaderSingleInstanceMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        HWND existing = FindWindowW(L"ProofReaderTrayWindow", L"ProofReaderTrayWindow");
        if (existing) {
            PostMessageW(existing, WM_USER + 2, 0, 0);
        }
        return false;
    }
    return true;
}

int ProofReaderApp::run(const std::wstring& cmdLine) {
    if (!enforceSingleInstance()) {
        return 0;
    }

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    bool hiddenLaunch = cmdLine.find(L"--autostart") != std::wstring::npos;
    bool devMode = (cmdLine.find(L"--dev") != std::wstring::npos);
    
    auto app_result = saucer::application::create({.id = "ProofReader", .quit_on_last_window_closed = false});
    if (!app_result) {
        return 1;
    }

    return app_result.value().run([this, hiddenLaunch, devMode](saucer::application *app) -> coco::stray {
        TrayIcon::Callbacks callbacks = {
            .onToggleWindow = [this]() { this->toggleWindow(); },
            .onTextSelected = [this](const std::wstring& text) { this->onTextSelected(text); },
            .onQuit = [this]() { this->quit(); }
        };
        auto trayIcon = std::make_unique<TrayIcon>(std::move(callbacks));
        m_proofReaderWindow = std::make_unique<ProofReaderWindow>(app, devMode);
        m_app = app;

        if (!hiddenLaunch) {
            this->toggleWindow();
        }

        co_await app->finish();
        m_proofReaderWindow.reset();
        trayIcon.reset();
        m_app = nullptr;
    });
}

void ProofReaderApp::toggleWindow() {
    if (m_proofReaderWindow) {
        m_proofReaderWindow->show();
        m_proofReaderWindow->focus();
    }
}

void ProofReaderApp::onTextSelected(const std::wstring& text) {
    if (!text.empty()) {
        m_proofReaderWindow->sendText(text);
        this->toggleWindow();
    } else {
        MessageBeep(MB_ICONWARNING);
    }
}

void ProofReaderApp::quit() {
    if (!m_app) return;
    m_app->post([this]() {
        m_app->quit();
    });
}
