#pragma once

#include <mutex>
#include <expected>
#include <functional>
#include <winrt/Windows.Web.Http.h>

class Http {
public:
    using Callback = std::function<void(std::expected<std::string, std::string>)>;

    void post(const std::string& url, const std::string& apiKey, const std::string& body, Callback onDone);
    void cancel();

private:
    using Operation = winrt::Windows::Foundation::IAsyncOperationWithProgress<
        winrt::Windows::Web::Http::HttpResponseMessage, winrt::Windows::Web::Http::HttpProgress>;

    void replaceCurrent(const Operation& next);

    std::mutex m_mutex;
    Operation m_current{nullptr};
};
