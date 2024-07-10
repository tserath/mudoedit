#include "SettingsManagement.h"
#include "SyntaxHighlighter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFontComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QDialog>
#include <QMdiSubWindow>
#include <QMdiArea>
#include <QSplitter>
#include <QTabWidget>
#include <QTabWidget>
#include <KTextEdit>


SettingsManagement::SettingsManagement(QTabWidget *tabWidget, QSettings *settings, QObject *parent)
    : QObject(parent), m_tabWidget(tabWidget), m_settings(settings),
      m_currentFont(QFont()), m_spellCheckEnabled(true), m_syntaxHighlightingEnabled(true),
      m_tabBarVisible(true)
{
    loadSettings();
}


void SettingsManagement::loadSettings()
{
    QString fontFamily = m_settings->value(QStringLiteral("fontFamily"), QStringLiteral("Monospace")).toString();
    int fontSize = m_settings->value(QStringLiteral("fontSize"), 12).toInt();
    m_currentFont = QFont(fontFamily, fontSize);
    m_spellCheckEnabled = m_settings->value(QStringLiteral("spellCheckEnabled"), true).toBool();
    m_syntaxHighlightingEnabled = m_settings->value(QStringLiteral("syntaxHighlightingEnabled"), true).toBool();
    m_tabBarVisible = m_settings->value(QStringLiteral("tabBarVisible"), true).toBool();
}

void SettingsManagement::saveSettings()
{
    m_settings->setValue(QStringLiteral("fontFamily"), m_currentFont.family());
    m_settings->setValue(QStringLiteral("fontSize"), m_currentFont.pointSize());
    m_settings->setValue(QStringLiteral("spellCheckEnabled"), m_spellCheckEnabled);
    m_settings->setValue(QStringLiteral("syntaxHighlightingEnabled"), m_syntaxHighlightingEnabled);
    m_settings->setValue(QStringLiteral("tabBarVisible"), m_tabBarVisible);
}

void SettingsManagement::applySettings()
{
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        QSplitter *splitter = qobject_cast<QSplitter*>(m_tabWidget->widget(i));
        if (!splitter) continue;

        QMdiArea *mdiArea = qobject_cast<QMdiArea*>(splitter->widget(0));
        if (!mdiArea) continue;

        for (QMdiSubWindow *window : mdiArea->subWindowList()) {
            if (KTextEdit *textEdit = qobject_cast<KTextEdit*>(window->widget())) {
                textEdit->setFont(m_currentFont);
                textEdit->setCheckSpellingEnabled(m_spellCheckEnabled);

                if (m_syntaxHighlightingEnabled) {
                    if (textEdit->findChild<SyntaxHighlighter*>()) {
                        delete textEdit->findChild<SyntaxHighlighter*>();
                    }
                    new SyntaxHighlighter(textEdit->document());
                } else {
                    if (SyntaxHighlighter *highlighter = textEdit->findChild<SyntaxHighlighter*>()) {
                        delete highlighter;
                    }
                }
            }
        }
    }

    // Apply tab bar visibility setting
    m_tabWidget->tabBar()->setVisible(m_tabBarVisible);
    if (!m_tabBarVisible) {
        m_tabWidget->setTabText(0, QLatin1String("mudoedit"));
    } else {
        m_tabWidget->setTabText(0, QLatin1String("Default"));
    }
}

// Show the settings dialog to the user
void SettingsManagement::showSettingsDialog()
{
    m_dialog = new QDialog(m_tabWidget);
    m_dialog->setWindowTitle(tr("Editor Settings"));

    QVBoxLayout *mainLayout = new QVBoxLayout(m_dialog);

    // Font settings
    QHBoxLayout *fontLayout = new QHBoxLayout;
    QLabel *fontLabel = new QLabel(tr("Font:"));
    m_fontComboBox = new QFontComboBox;
    m_fontSizeSpinBox = new QSpinBox;
    m_fontSizeSpinBox->setRange(8, 72);
    m_fontSizeSpinBox->setValue(m_currentFont.pointSize());
    m_fontComboBox->setCurrentFont(m_currentFont);
    fontLayout->addWidget(fontLabel);
    fontLayout->addWidget(m_fontComboBox);
    fontLayout->addWidget(m_fontSizeSpinBox);

    // Checkboxes for various settings
    QCheckBox *spellCheckBox = new QCheckBox(tr("Enable Spell Checking"));
    spellCheckBox->setChecked(m_spellCheckEnabled);

    m_syntaxHighlightingCheckBox = new QCheckBox(tr("Enable Syntax Highlighting"));
    m_syntaxHighlightingCheckBox->setChecked(m_syntaxHighlightingEnabled);

    m_tabBarVisibilityCheckBox = new QCheckBox(tr("Show Tab Bar"));
    m_tabBarVisibilityCheckBox->setChecked(m_tabBarVisible);

    // OK and Cancel buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *okButton = new QPushButton(tr("OK"));
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    
    connect(okButton, &QPushButton::clicked, this, [this, spellCheckBox]() {
        m_currentFont = m_fontComboBox->currentFont();
        m_currentFont.setPointSize(m_fontSizeSpinBox->value());
        m_spellCheckEnabled = spellCheckBox->isChecked();
        m_syntaxHighlightingEnabled = m_syntaxHighlightingCheckBox->isChecked();
        m_tabBarVisible = m_tabBarVisibilityCheckBox->isChecked();
        saveSettings();
        applySettings();
        m_dialog->accept();
    });
    
    connect(cancelButton, &QPushButton::clicked, m_dialog, &QDialog::reject);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    // Add all layouts and widgets to the main layout
    mainLayout->addLayout(fontLayout);
    mainLayout->addWidget(spellCheckBox);
    mainLayout->addWidget(m_syntaxHighlightingCheckBox);
    mainLayout->addWidget(m_tabBarVisibilityCheckBox);
    mainLayout->addLayout(buttonLayout);
    
    m_dialog->exec();
    delete m_dialog;
}