import { useState, useEffect } from 'react'

import {
  Button,
  Input,
  Label,
  Title3,
  Spinner,
  Text,
  Divider,
} from '@fluentui/react-components'
import { Settings24Regular, Save24Regular, NetworkCheck24Regular } from '@fluentui/react-icons'

import { proofread, cancelRequest, loadSettings, saveSettings } from '../services/api'
import { settings } from '../config/defaults.json'

import styles from '../styles/SettingsView.module.css'

export default function SettingsView({ onClose, isConfigured }) {
  const [apiUrl, setApiUrl] = useState(settings.apiUrl)
  const [apiKey, setApiKey] = useState('')
  const [model, setModel] = useState(settings.model)

  const [testStatus, setTestStatus] = useState('unknown')

  useEffect(() => {
    if (window.saucer) {
      saucer.call('resize', [500, 470])
    }
    loadSettings().then((stored) => {
      setApiUrl(stored.apiUrl || settings.apiUrl)
      setApiKey(stored.apiKey)
      setModel(stored.model || settings.model)
    })
  }, [])

  const handleFieldChange = setter => (_, data) => {
    cancelRequest()
    setTestStatus('unknown')
    setter(data.value)
  }

  const handleSave = async () => {
    await saveSettings({ apiUrl, apiKey, model })
    onClose()
  }

  const handleCancel = () => {
    cancelRequest()
    onClose()
  }

  const handleTestConnection = () => {
    if (testStatus === 'progress') {
      cancelRequest()
      setTestStatus('unknown')
      return
    }

    setTestStatus('progress')
    const messages = [{ role: 'user', content: 'Hello' }]

    proofread(messages, { apiUrl, apiKey, model })
      .then(() => {
        setTestStatus('success')
      })
      .catch(() => {
        setTestStatus('failure')
      })
  }

  return (
    <div className={styles.container}>
      <div>
        <div className={styles.headerRow}>
          <Settings24Regular />
          <Title3>Configuration</Title3>
        </div>
        <Text size={200} className={styles.subtext}>
          ProofReader works with any Model Provider that supports an OpenAI-compatible API (e.g., OpenAI, Gemini, Ollama).
        </Text>
      </div>

      <div className={styles.formGroup}>
        <div className={styles.fieldGroup}>
          <Label>API URL</Label>
          <Input
            value={apiUrl}
            onChange={handleFieldChange(setApiUrl)}
          />
        </div>
        <div className={styles.fieldGroup}>
          <Label>API Key</Label>
          <Input
            type="password"
            value={apiKey}
            onChange={handleFieldChange(setApiKey)}
          />
        </div>
        <div className={styles.fieldGroup}>
          <Label>Model</Label>
          <Input
            value={model}
            onChange={handleFieldChange(setModel)}
          />
        </div>
      </div>

      <div className={styles.testRow}>
        <Button
          appearance="secondary"
          icon={testStatus === 'progress' ? <Spinner size="tiny" /> : <NetworkCheck24Regular />}
          onClick={handleTestConnection}
          disabled={testStatus !== 'progress' && (!apiUrl || !apiKey || !model)}
        >
          {testStatus === 'progress' ? 'Cancel' : 'Test Connection'}
        </Button>
        {testStatus === 'success' && <Text className={styles.successText}>Connection Successful</Text>}
        {testStatus === 'failure' && <Text className={styles.failureText}>Connection Failed</Text>}
      </div>

      <Divider />

      <div className={styles.footerRow}>
        {isConfigured && (
          <Button appearance="secondary" onClick={handleCancel}>Cancel</Button>
        )}
        <Button
          appearance="primary"
          icon={<Save24Regular />}
          onClick={handleSave}
          disabled={testStatus !== 'success'}
        >
          Save & Close
        </Button>
      </div>
    </div>
  )
}
