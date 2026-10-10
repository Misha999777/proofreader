import { useState, useEffect } from 'react';

import {
  Button,
  Input,
  Label,
  Title3,
  Spinner,
  Text,
  Divider
} from '@fluentui/react-components';
import { Settings24Regular, Save24Regular, NetworkCheck24Regular } from '@fluentui/react-icons';

import { proofread } from '../services/api';

import styles from '../styles/SettingsView.module.css';

function SettingsView({ onClose, isConfigured, isVisible }) {
  const [apiUrl, setApiUrl] = useState('');
  const [apiKey, setApiKey] = useState('');
  const [model, setModel] = useState('');
  const [testStatus, setTestStatus] = useState('unknown');

  useEffect(() => {
    setApiUrl(localStorage.getItem('apiUrl') || 'https://generativelanguage.googleapis.com/v1beta/openai/');
    setApiKey(localStorage.getItem('apiKey') || '');
    setModel(localStorage.getItem('model') || 'gemini-2.5-flash');
  }, []);

  useEffect(() => {
    if (isVisible && window.saucer) {
      saucer.call("resize", [500, 470]);
    }
  }, [isVisible]);

  useEffect(() => {
    setTestStatus("unknown");
  }, [apiUrl, apiKey, model])

  const handleSave = () => {
    localStorage.setItem('apiUrl', apiUrl);
    localStorage.setItem('apiKey', apiKey);
    localStorage.setItem('model', model);
    onClose();
  };

  const handleTestConnection = async () => {
    setTestStatus('progress');
    const messages = [{ role: 'user', content: 'Hello' }];
    const { success } = await proofread(messages, apiUrl, apiKey, model);
    setTestStatus(success ? 'success' : 'failure');
  };

  return (
    <div className={styles.container} style={{ display: isVisible ? 'flex' : 'none' }}>
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
            onChange={(e, data) => setApiUrl(data.value)}
          />
        </div>
        <div className={styles.fieldGroup}>
          <Label>API Key</Label>
          <Input
            type="password"
            value={apiKey}
            onChange={(e, data) => setApiKey(data.value)}
          />
        </div>
        <div className={styles.fieldGroup}>
          <Label>Model</Label>
          <Input
            value={model}
            onChange={(e, data) => setModel(data.value)}
          />
        </div>
      </div>

      <div className={styles.testRow}>
        <Button
          appearance="secondary"
          icon={testStatus === 'progress' ? <Spinner size="tiny" /> : <NetworkCheck24Regular />}
          onClick={handleTestConnection}
          disabled={testStatus === 'progress' || !apiUrl || !apiKey || !model}
        >
          {testStatus === 'progress' ? 'Testing...' : 'Test Connection'}
        </Button>
        {testStatus === 'success' && <Text className={styles.successText}>Connection Successful</Text>}
        {testStatus === 'failure' && <Text className={styles.failureText}>Connection Failed</Text>}
      </div>

      <Divider />

      <div className={styles.footerRow}>
        {isConfigured && (
          <Button appearance="secondary" onClick={onClose}>Cancel</Button>
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
  );
}

export default SettingsView;
