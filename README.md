# ProofReader

A Windows proofreading application.
Select text anywhere, press **Ctrl+Alt+P**, and get AI-powered corrections, rewrites, and summaries.

## Features

- **Global Hotkey** — `Ctrl+Alt+P` captures selected text from any application via Windows UI Automation
- **OpenAI-compatible API** — Works with any API that implements the `/chat/completions` endpoint
- **Multiple Modes** — Fix grammar, make text professional/friendly/concise, or summarize
- **Diff View** — Side-by-side word-level diff highlighting for grammar corrections
- **Follow-up Prompts** — Refine results with additional instructions

## Architecture

| Layer    | Tech                                                            |
|----------|-----------------------------------------------------------------|
| Backend  | C++, Win32, [Saucer](https://github.com/saucer/saucer)          |
| Frontend | Vite, React, [Fluent UI](https://github.com/microsoft/fluentui) |

## Download

<a href="https://get.microsoft.com/installer/download/9NHN4G3HQR3P?referrer=appbadge" target="_self">
	<img src="https://get.microsoft.com/images/en-us%20dark.svg" width="200"/>
</a>
