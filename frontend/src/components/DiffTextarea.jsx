import { useState, useCallback } from 'react'
import { createPortal } from 'react-dom'

import { Textarea, FluentProvider, RendererProvider, mergeClasses, createDOMRenderer } from '@fluentui/react-components'
import { diffWords } from 'diff'

import useSystemTheme from '../hooks/useSystemTheme'

import styles from '../styles/DiffTextarea.module.css'
import frameStyles from '../styles/DiffTextarea.module.css?inline'

export default function DiffTextarea({ oldText, newText, showDiff, className }) {
  const theme = useSystemTheme()
  const [frame, setFrame] = useState(null)

  const attachFrame = useCallback((node) => {
    const doc = node.contentDocument
    const view = doc.defaultView

    const onBlur = () => view.getSelection().removeAllRanges()
    const onDrop = () => node.blur()

    view.addEventListener('blur', onBlur)
    window.addEventListener('drop', onDrop)
    setFrame({ doc, renderer: createDOMRenderer(doc) })

    return () => {
      view.removeEventListener('blur', onBlur)
      window.removeEventListener('drop', onDrop)
      setFrame(null)
    }
  }, [])

  let offset = 0
  const parts = !showDiff || !newText ? newText : diffWords(oldText, newText).map((part) => {
    const key = offset
    offset += part.value.length

    if (part.added) {
      return <ins key={key} className={styles.added}>{part.value}</ins>
    }
    if (part.removed) {
      return <del key={key} className={styles.removed}>{part.value}</del>
    }
    return <span key={key}>{part.value}</span>
  })

  return (
    <>
      <iframe
        title="Result Box"
        ref={attachFrame}
        className={mergeClasses(className, styles.iframe)}
      />
      {frame && createPortal(
        <RendererProvider renderer={frame.renderer} targetDocument={frame.doc}>
          <style>{frameStyles}</style>
          <FluentProvider theme={theme} targetDocument={frame.doc}>
            <Textarea
              tabIndex={0}
              className={styles.textArea}
              textarea={{
                as: 'div',
                className: styles.container,
                children: parts,
              }}
            />
          </FluentProvider>
        </RendererProvider>,
        frame.doc.body,
      )}
    </>
  )
}
