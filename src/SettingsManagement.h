#ifndef SETTINGSMANAGEMENT_H
#define SETTINGSMANAGEMENT_H

#include <QObject>
#include <QSettings>
#include <QFont>

class QTabWidget;
class QFontComboBox;
class QSpinBox;
class QCheckBox;
class QDialog;

class SettingsManagement : public QObject
{
    Q_OBJECT

public:
    explicit SettingsManagement(QTabWidget *tabWidget, QSettings *settings, QObject *parent = nullptr);

    void loadSettings();
    void saveSettings();
    void applySettings();
    void showSettingsDialog();

    QFont currentFont() const { return m_currentFont; }
    bool isSpellCheckEnabled() const { return m_spellCheckEnabled; }
    void setSpellCheckEnabled(bool enabled) { m_spellCheckEnabled = enabled; }
    bool isSyntaxHighlightingEnabled() const { return m_syntaxHighlightingEnabled; }
    void setSyntaxHighlightingEnabled(bool enabled) { m_syntaxHighlightingEnabled = enabled; }

    bool isTabBarVisible() const { return m_tabBarVisible; }
    void setTabBarVisible(bool visible) { m_tabBarVisible = visible; }

Q_SIGNALS:
    void settingsChanged();

private:
    QTabWidget *m_tabWidget;
    QSettings *m_settings;
    QFont m_currentFont;
    bool m_spellCheckEnabled;
    bool m_syntaxHighlightingEnabled;

    bool m_tabBarVisible;

    QDialog *m_dialog;
    QFontComboBox *m_fontComboBox;
    QSpinBox *m_fontSizeSpinBox;
    QCheckBox *m_syntaxHighlightingCheckBox;
    QCheckBox *m_tabBarVisibilityCheckBox;
};

#endif // SETTINGSMANAGEMENT_H