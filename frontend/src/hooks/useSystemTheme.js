import { useState, useEffect } from 'react'

import { createLightTheme, createDarkTheme } from '@fluentui/react-components'

import { brand } from '../config/theme.json'

const lightTheme = createLightTheme(brand)
const darkTheme = createDarkTheme(brand)

export default function useSystemTheme() {
  const [theme, setTheme] = useState(
    () => window.matchMedia('(prefers-color-scheme: dark)').matches ? darkTheme : lightTheme,
  )

  useEffect(() => {
    const mq = window.matchMedia('(prefers-color-scheme: dark)')
    const handleChange = e => setTheme(e.matches ? darkTheme : lightTheme)
    mq.addEventListener('change', handleChange)
    return () => mq.removeEventListener('change', handleChange)
  }, [])

  return theme
}
