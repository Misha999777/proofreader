import { useState, useEffect } from 'react'

import { FluentProvider } from '@fluentui/react-components'

import { loadSettings } from './services/api'
import useSystemTheme from './hooks/useSystemTheme'

import SettingsView from './components/SettingsView'
import ProofreadingView from './components/ProofreadingView'

import styles from './styles/App.module.css'

async function checkConfigured() {
  const { apiUrl, apiKey, model } = await loadSettings()
  return !!(apiUrl && apiKey && model)
}

export default function App() {
  const [isConfigured, setIsConfigured] = useState(null)
  const [showSettings, setShowSettings] = useState(false)
  const theme = useSystemTheme()

  useEffect(() => {
    checkConfigured().then(setIsConfigured)
  }, [])

  const handleSettingsClosed = () => {
    setShowSettings(false)
    checkConfigured().then(setIsConfigured)
  }

  const shouldShowSettings = showSettings || isConfigured === false

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
  )
}
