export function proofread(messages, customApiUrl, customApiKey, customModel) {
  const apiUrl = customApiUrl || localStorage.getItem('apiUrl');
  const apiKey = customApiKey || localStorage.getItem('apiKey');
  const model = customModel || localStorage.getItem('model');

  let urlString = apiUrl;
  if (!urlString.endsWith('/chat/completions')) {
    if (!urlString.endsWith('/')) {
      urlString += '/';
    }
    urlString += 'chat/completions';
  }

  const controller = new AbortController();
  const promise = (async () => {
    const response = await fetch(urlString, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        'Authorization': `Bearer ${apiKey}`
      },
      body: JSON.stringify({
        model,
        messages
      }),
      signal: controller.signal,
    });

    if (!response.ok) {
      throw new Error(`API Error: ${response.status} ${response.statusText}`);
    }

    const data = await response.json();
    return data.choices?.[0]?.message?.content ?? '';
  })();

  return { controller, promise };
}

export function trackRequest(ref, { controller, promise }) {
  ref.current?.abort();
  ref.current = controller;

  return new Promise((resolve, reject) => {
    promise
      .then(result => {
        if (ref.current === controller) {
          resolve(result);
        }
      })
      .catch(err => {
        if (ref.current === controller && err?.name !== 'AbortError') {
          reject(err);
        }
      });
  });
}
