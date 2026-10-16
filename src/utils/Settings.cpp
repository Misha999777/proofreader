#include "utils/Settings.hpp"

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.h>

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;

Settings SettingsStore::load() {
    try {
        auto values = ApplicationData::Current().LocalSettings().Values();
        return {
            .apiUrl = winrt::to_string(winrt::unbox_value_or<winrt::hstring>(values.TryLookup(L"ApiUrl"), L"")),
            .apiKey = winrt::to_string(winrt::unbox_value_or<winrt::hstring>(values.TryLookup(L"ApiKey"), L"")),
            .model = winrt::to_string(winrt::unbox_value_or<winrt::hstring>(values.TryLookup(L"Model"), L"")),
        };
    } catch (const winrt::hresult_error&) {
        return {};
    }
}

bool SettingsStore::save(const Settings& settings) {
    try {
        auto values = ApplicationData::Current().LocalSettings().Values();
        values.Insert(L"ApiUrl", winrt::box_value(winrt::to_hstring(settings.apiUrl)));
        values.Insert(L"ApiKey", winrt::box_value(winrt::to_hstring(settings.apiKey)));
        values.Insert(L"Model", winrt::box_value(winrt::to_hstring(settings.model)));
        return true;
    } catch (const winrt::hresult_error&) {
        return false;
    }
}
