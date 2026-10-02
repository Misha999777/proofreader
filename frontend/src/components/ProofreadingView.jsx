import { useState, useEffect, useRef } from 'react';

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
import { Settings24Regular, Delete24Regular, Copy24Regular, Play24Regular, Send24Regular } from '@fluentui/react-icons';
import { diffWords } from 'diff';

import { proofread } from '../services/api';
import GOALS from '../config/goals.json';

import styles from '../styles/ProofreadingView.module.css';

function renderDiff(oldText, newText) {
  if (!oldText || !newText) return newText;
  
  const differences = diffWords(oldText, newText);
  
  return (
    <div className={styles.diffContainer}>
      {differences.map((part, index) => {
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
      })}
    </div>
  );
}

function ProofreadingView({ onShowSettings }) {
  const [originalText, setOriginalText] = useState('');
  const [proofreadText, setProofreadText] = useState('');
  const [selectedGoalId, setSelectedGoalId] = useState(GOALS[0].id);
  const [showDiff, setShowDiff] = useState(true);
  const [isLoading, setIsLoading] = useState(false);
  const [conversationHistory, setConversationHistory] = useState([]);
  const [hasCurrentResult, setHasCurrentResult] = useState(false);
  const [followUpText, setFollowUpText] = useState('');
  const [isFollowingUp, setIsFollowingUp] = useState(false);
  const pendingAutoRun = useRef(false);

  useEffect(() => {
    window.setOriginalText = (text) => {
      setOriginalText(text);
      setProofreadText('');
      setHasCurrentResult(false);
      setConversationHistory([]);
      pendingAutoRun.current = true;
    };

    return () => {
      delete window.setOriginalText;
    };
  }, []);

  useEffect(() => {
    if (window.saucer) {
      saucer.call("resize", [400, hasCurrentResult ? 635 : 585]);
    }
  }, [hasCurrentResult]);

  useEffect(() => {
    if (pendingAutoRun.current && originalText) {
      pendingAutoRun.current = false;
      handleRun();
    }
  }, [originalText]);

  const reset = () => {
    setOriginalText('');
    setProofreadText('');
    setConversationHistory([]);
    setHasCurrentResult(false);
    setFollowUpText('');
  };

  const handleRun = async () => {
    if (!originalText) return;
    setIsLoading(true);
    setHasCurrentResult(false);

    const selectedGoal = GOALS.find(g => g.id === selectedGoalId);
    const messages = [
      { role: 'system', content: selectedGoal.prompt },
      { role: 'user', content: originalText }
    ];

    setConversationHistory(messages);

    const { success, text } = await proofread(messages);
    setProofreadText(text);
    if (success) {
      setConversationHistory([...messages, { role: 'assistant', content: text }]);
      setHasCurrentResult(true);
    }
    setIsLoading(false);
  };

  const handleFollowUp = async () => {
    if (!followUpText || !hasCurrentResult) return;
    setIsFollowingUp(true);

    const updatedHistory = [...conversationHistory, { role: 'user', content: followUpText }];
    setConversationHistory(updatedHistory);

    const { success, text } = await proofread(updatedHistory);
    if (success) {
      setProofreadText(text);
      setConversationHistory([...updatedHistory, { role: 'assistant', content: text }]);
      setFollowUpText('');
    }
    setIsFollowingUp(false);
  };

  const handleCopy = () => {
    navigator.clipboard.writeText(proofreadText);
  };

  return (
    <div className={styles.container}>
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
            disabled={isLoading || isFollowingUp}
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
          onClick={handleRun}
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
        >
          Copy Result
        </Button>
      </div>
    </div>
  );
}

export default ProofreadingView;
