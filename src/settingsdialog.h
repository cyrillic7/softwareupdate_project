#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QTabWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QTextEdit>
#include <QSizePolicy>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    // 获取设置值
    QString getRemoteDirectory() const;
    int getConnectionTimeout() const;
    bool getAutoSaveSettings() const;
    bool getShowLogByDefault() const;
    QString getDefaultLocalPath() const;
    QString getLogStoragePath() const;
    bool getAutoCleanLog() const;
    int getLogRetentionDays() const;
    int getMaxLogLines() const;
    QString getQtExtractPath() const;
    QString get7evExtractPath() const;
    
    // 设置值
    void setRemoteDirectory(const QString &path);
    void setConnectionTimeout(int timeout);
    void setAutoSaveSettings(bool autoSave);
    void setShowLogByDefault(bool showLog);
    void setDefaultLocalPath(const QString &path);
    void setLogStoragePath(const QString &path);
    void setAutoCleanLog(bool autoClean);
    void setLogRetentionDays(int days);
    void setMaxLogLines(int maxLines);
    void setQtExtractPath(const QString &path);
    void set7evExtractPath(const QString &path);

private slots:
    void onAccept();
    void onReject();
    void onRestoreDefaults();
    void onSelectDefaultPath();
    void onSelectLogPath();
    void onTestRemoteDirectory();
    void onOpenLogDirectory();
    void onSelectQtExtractPath();
    void onSelect7evExtractPath();

private:
    void setupUI();
    void setupConnectionSettings();
    void setupApplicationSettings();
    void setupAdvancedSettings();
    void connectSignals();
    void loadDefaults();

    // UI组件
    QVBoxLayout *mainLayout;
    QTabWidget *tabWidget;
    
    // 连接设置标签页
    QWidget *connectionTab;
    QVBoxLayout *connectionLayout;
    QGroupBox *remoteGroup;
    QGridLayout *remoteLayout;
    QLabel *remoteDirLabel;
    QLineEdit *remoteDirLineEdit;
    QPushButton *testRemoteDirButton;
    QLabel *timeoutLabel;
    QSpinBox *timeoutSpinBox;
    
    // 升级路径设置组
    QGroupBox *upgradePathGroup;
    QGridLayout *upgradePathLayout;
    QLabel *qtExtractPathLabel;
    QLineEdit *qtExtractPathLineEdit;
    QPushButton *selectQtExtractPathButton;
    QLabel *sevEvExtractPathLabel;
    QLineEdit *sevEvExtractPathLineEdit;
    QPushButton *select7evExtractPathButton;
    
    // 应用程序设置标签页
    QWidget *applicationTab;
    QVBoxLayout *applicationLayout;
    QGroupBox *generalGroup;
    QGridLayout *generalLayout;
    QCheckBox *autoSaveCheckBox;
    QCheckBox *showLogCheckBox;
    QLabel *defaultPathLabel;
    QLineEdit *defaultPathLineEdit;
    QPushButton *selectPathButton;
    QLabel *logPathLabel;
    QLineEdit *logPathLineEdit;
    QPushButton *selectLogPathButton;
    QPushButton *openLogDirButton;
    QCheckBox *autoCleanLogCheckBox;
    QLabel *retentionDaysLabel;
    QSpinBox *retentionDaysSpinBox;
    
    // 高级设置标签页
    QWidget *advancedTab;
    QVBoxLayout *advancedLayout;
    QGroupBox *logGroup;
    QGridLayout *logLayout;
    QLabel *maxLogLinesLabel;
    QSpinBox *maxLogLinesSpinBox;
    QGroupBox *securityGroup;
    QGridLayout *securityLayout;
    QCheckBox *verifyHostCheckBox;
    QCheckBox *savePasswordCheckBox;
    
    // 按钮区域
    QHBoxLayout *buttonLayout;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QPushButton *defaultsButton;
};

#endif // SETTINGSDIALOG_H 