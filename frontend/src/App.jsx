import { useState, useEffect } from 'react';

import { FluentProvider, createLightTheme, createDarkTheme } from '@fluentui/react-components';

import SettingsView from './components/SettingsView';
import ProofreadingView from './components/ProofreadingView';
import { loadSettings } from './services/api';
import { brand } from './config/theme.json';

import styles from './styles/App.module.css';

const lightTheme = createLightTheme(brand);
const darkTheme = createDarkTheme(brand);

async function checkConfigured() {
  const { apiUrl, apiKey, model } = await loadSettings();
  return !!(apiUrl && apiKey && model);
}

export default function App() {
  const [isConfigured, setIsConfigured] = useState(null);
  const [showSettings, setShowSettings] = useState(false);
  const [theme, setTheme] = useState(
    window.matchMedia('(prefers-color-scheme: dark)').matches ? darkTheme : lightTheme
  );

  useEffect(() => {
    checkConfigured().then(setIsConfigured);
  }, []);

  useEffect(() => {
    const mq = window.matchMedia('(prefers-color-scheme: dark)');
    const handleChange = (e) => setTheme(e.matches ? darkTheme : lightTheme);
    mq.addEventListener('change', handleChange);
    return () => mq.removeEventListener('change', handleChange);
  }, []);

  const handleSettingsClosed = () => {
    setShowSettings(false);
    checkConfigured().then(setIsConfigured);
  };

  const shouldShowSettings = showSettings || isConfigured === false;

  return (
    <FluentProvider theme={theme} className={styles.root}>
      {shouldShowSettings && (
        <SettingsView
          onClose={handleSettingsClosed}
          isConfigured={isConfigured}
        />
      )}
      <ProofreadingView 
        onShowSettings={() => setShowSettings(true)}
        onHideSettings={handleSettingsClosed}
        isVisible={isConfigured !== null && !shouldShowSettings}
      />
    </FluentProvider>
  );
}
