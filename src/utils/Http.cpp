#include "utils/Http.hpp"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Web.Http.Headers.h>

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Web::Http;

void Http::post(const std::string& url, const std::string& apiKey, const std::string& body, Callback onDone) {
    Operation operation{nullptr};
    HttpClient client;
    try {
        HttpRequestMessage request(HttpMethod::Post(), Uri(winrt::to_hstring(url)));
        request.Content(HttpStringContent(winrt::to_hstring(body),
            winrt::Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));
        if (!apiKey.empty()) {
            request.Headers().Authorization(Headers::HttpCredentialsHeaderValue(L"Bearer", winrt::to_hstring(apiKey)));
        }
        operation = client.SendRequestAsync(request);
    } catch (const winrt::hresult_error& e) {
        return onDone(std::unexpected(winrt::to_string(e.message())));
    }

    replaceCurrent(operation);

    operation.Completed([client, onDone = std::move(onDone)](const Operation& op, AsyncStatus status) {
        if (status == AsyncStatus::Canceled) {
            return;
        }
        try {
            auto response = op.GetResults();
            if (!response.IsSuccessStatusCode()) {
                return onDone(std::unexpected(std::format("API Error: {}", static_cast<int>(response.StatusCode()))));
            }
            onDone(winrt::to_string(response.Content().ReadAsStringAsync().get()));
        } catch (const winrt::hresult_error& e) {
            onDone(std::unexpected(winrt::to_string(e.message())));
        }
    });
}

void Http::cancel() {
    replaceCurrent(nullptr);
}

void Http::replaceCurrent(const Operation& next) {
    Operation previous{nullptr};
    {
        std::lock_guard lock(m_mutex);
        previous = std::exchange(m_current, next);
    }
    if (previous) {
        previous.Cancel();
    }
}
