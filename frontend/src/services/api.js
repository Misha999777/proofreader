export async function loadSettings() {
  return await saucer.exposed.getSettings();
}

export async function saveSettings(settings) {
  if (!await saucer.exposed.setSettings(settings)) {
    throw new Error('Failed to save settings');
  }
}

export function cancelRequest() {
  saucer.exposed.cancel();
}

export function hideWindow() {
  saucer.exposed.hide();
}

export async function proofread(messages, customSettings) {
    const { apiUrl, apiKey, model } = customSettings ?? await loadSettings();

    let urlString = apiUrl;
    if (!urlString.endsWith('/chat/completions')) {
      if (!urlString.endsWith('/')) {
        urlString += '/';
      }
      urlString += 'chat/completions';
    }

    const body = await saucer.exposed.post(urlString, apiKey, JSON.stringify({ model, messages }));

    const data = JSON.parse(body);
    return data.choices?.[0]?.message?.content ?? '';
}
