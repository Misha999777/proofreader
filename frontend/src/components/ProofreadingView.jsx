import { useState, useEffect, useCallback } from 'react';

import {
  Button,
  Textarea,
  Select,
  Switch,
  Spinner,
  Label,
  Input,
  Divider
} from '@fluentui/react-components';
import {
  Settings24Regular,
  Delete24Regular,
  Copy24Regular,
  Play24Regular,
  Send24Regular
} from '@fluentui/react-icons';
import { diffWords } from 'diff';

import { proofread, cancelRequest, hideWindow } from '../services/api';
import GOALS from '../config/goals.json';

import styles from '../styles/ProofreadingView.module.css';

function renderDiff(oldText, newText) {
  if (!oldText || !newText) return newText;
  
  const differences = diffWords(oldText, newText);
  
  return (
    <Textarea
      readOnly
      resize="none"
      className={styles.textareaRoot}
      textarea={{
        as: 'div',
        style: { whiteSpace: 'pre-wrap', overflowY: 'auto' },
        children: differences.map((part, index) => {
          let className = '';

          if (part.added) {
            className = styles.diffAdded;
          } else if (part.removed) {
            className = styles.diffRemoved;
          }

          return (
            <span key={index} className={className || undefined}>
              {part.value}
            </span>
          );
        })
      }}
    />
  );
}

export default function ProofreadingView({ onShowSettings, onHideSettings, isVisible }) {
  const [selectedGoalId, setSelectedGoalId] = useState(GOALS[0].id);
  const [originalText, setOriginalText] = useState('');
  const [proofreadText, setProofreadText] = useState('');
  const [showDiff, setShowDiff] = useState(true);

  const [isLoading, setIsLoading] = useState(false);
  const [isFollowingUp, setIsFollowingUp] = useState(false);
  const [hasCurrentResult, setHasCurrentResult] = useState(false);
  const [followUpText, setFollowUpText] = useState('');
  const [conversationHistory, setConversationHistory] = useState([]);

  useEffect(() => {
    if (isVisible && window.saucer) {
      saucer.call("resize", [400, hasCurrentResult ? 635 : 585]);
    }
  }, [hasCurrentResult, isVisible]);

  const reset = useCallback(() => {
    setOriginalText('');
    setProofreadText('');
    setFollowUpText('');

    setIsLoading(false);
    setIsFollowingUp(false);
    setHasCurrentResult(false);

    cancelRequest();
    setConversationHistory([]);
  }, []);

  const handleRun = useCallback(textToRun => {
    reset();
    setOriginalText(textToRun);
    setIsLoading(true);

    const selectedGoal = GOALS.find(g => g.id === selectedGoalId);
    const messages = [
      { role: 'system', content: selectedGoal.prompt },
      { role: 'user', content: textToRun }
    ];

    proofread(messages)
      .then(text => {
        setProofreadText(text);
        setConversationHistory([...messages, { role: 'assistant', content: text }]);
        setHasCurrentResult(true);
      })
      .catch(error => {
        setProofreadText(error?.message || 'API Error');
      })
      .finally(() => {
        setIsLoading(false);
      });
  }, [selectedGoalId, reset]);

  const handleCopy = useCallback(() => {
    return navigator.clipboard.writeText(proofreadText);
  }, [proofreadText]);

  useEffect(() => {
    const handleKeyDown = e => {
      if (!isVisible || !e.ctrlKey || !e.altKey || e.code !== 'KeyC' || !proofreadText) return;

      e.preventDefault();
      handleCopy().then(hideWindow);
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [isVisible, proofreadText, handleCopy]);

  useEffect(() => {
    window.setOriginalText = text => {
      onHideSettings();
      handleRun(text);
    };

    return () => {
      delete window.setOriginalText;
    };
  }, [onHideSettings, handleRun]);

  const handleFollowUp = async () => {
    setIsFollowingUp(true);

    const updatedHistory = [...conversationHistory, { role: 'user', content: followUpText }];
    setConversationHistory(updatedHistory);

    proofread(updatedHistory)
      .then(text => {
        setProofreadText(text);
        setConversationHistory([...updatedHistory, { role: 'assistant', content: text }]);
        setFollowUpText('');
      })
      .catch(error => {
        setProofreadText(error?.message || 'API Error');
      })
      .finally(() => {
        setIsFollowingUp(false);
      });
  };

  return (
    <div className={styles.container} style={{ display: isVisible ? 'flex' : 'none' }}>
      <div className={styles.headerRow}>
        <Label weight="semibold">Goal</Label>
        <Button appearance="subtle" icon={<Settings24Regular />} onClick={onShowSettings} title="Settings" />
      </div>

      <Select
        value={selectedGoalId}
        onChange={(e, data) => {
          setSelectedGoalId(data.value);
          setShowDiff(data.value === 'fixGrammar');
        }}
      >
        {GOALS.map(g => (
          <option key={g.id} value={g.id}>{g.title}</option>
        ))}
      </Select>

      <div className={styles.fieldGroup}>
        <div className={styles.headerRow}>
          <Label weight="semibold">Original Text</Label>
          <Button
            appearance="subtle"
            size="small"
            icon={<Delete24Regular />}
            onClick={reset}
          >
            Clear
          </Button>
        </div>
        <Textarea
          value={originalText}
          onChange={(e, data) => setOriginalText(data.value)}
          resize="none"
          className={styles.textareaRoot}
        />
      </div>

      <div className={styles.buttonRow}>
        <Button
          appearance="primary"
          icon={isLoading ? <Spinner size="tiny" /> : <Play24Regular />}
          onClick={() => handleRun(originalText)}
          disabled={!originalText || isLoading || isFollowingUp}
        >
          Run Proofreading
        </Button>
      </div>

      <div className={styles.fieldGroup}>
        <div className={styles.resultHeaderRow}>
          <Label weight="semibold">Result</Label>
          <Switch
            checked={showDiff}
            onChange={(e, data) => setShowDiff(data.checked)}
            label="Show Diff"
            className={selectedGoalId === 'fixGrammar' ? styles.switchVisible : styles.switchHidden}
          />
        </div>
        {selectedGoalId === 'fixGrammar' && showDiff && proofreadText ? (
          renderDiff(originalText, proofreadText)
        ) : (
          <Textarea
            value={proofreadText}
            readOnly
            resize="none"
            className={styles.textareaRoot}
          />
        )}
      </div>

      {hasCurrentResult && (
        <div className={styles.followUpRow}>
          <Input
            placeholder="Follow up..."
            value={followUpText}
            onChange={(e, data) => setFollowUpText(data.value)}
            onKeyDown={(e) => e.key === 'Enter' && handleFollowUp()}
            disabled={isLoading || isFollowingUp}
            className={styles.followUpInput}
          />
          <Button
            appearance="secondary"
            icon={isFollowingUp ? <Spinner size="tiny" /> : <Send24Regular />}
            onClick={handleFollowUp}
            disabled={!followUpText || isLoading || isFollowingUp}
          >
            Send
          </Button>
        </div>
      )}

      <Divider />

      <div className={styles.footerRow}>
        <Button
          appearance="secondary"
          icon={<Copy24Regular />}
          onClick={handleCopy}
          disabled={!proofreadText}
          title="Ctrl+Alt+C"
        >
          Copy Result
        </Button>
      </div>
    </div>
  );
}
