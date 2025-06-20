/**
 * @File Name: mainwindow.h
 * @brief  680图像机软件主窗口类头文件，定义UI控件和文件上传功能
 * @Author : chency email:121888719@qq.com
 * @Version : 1.0
 * @Creat Date : 2025
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QTextCursor>
#include <QFont>
#include <QListWidget>

#include <QGroupBox>
#include <QSpinBox>
#include <QProgressBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QTimer>
#include <QFileDialog>
#include <QProcess>
#include <QTcpSocket>
#include <QTemporaryFile>
#include <QDir>
#include <QStandardPaths>
#include <QSizePolicy>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>
#include <QCryptographicHash>
#include <QClipboard>

class SettingsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSelectFile();
    void onUploadFile();
    void onClearLog();
    void onTestConnection();
    void onCancelUpload();
    void onUploadFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onUploadProgress();
    void onUploadOutput();
    void onUploadTimeout();
    void onTestFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onMenuAction();
    void onToggleLogView();
    void onToggleCommandView();
    void onToggleBuiltinCommandView();
    void onVerifyFileFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onShowMachineCode();
    void onUpgradeQtSoftware();
    void onUpgrade7evFirmware();
    void onUpgradeKu5p();
    void onExecuteCustomCommand();
    void onClearCommandOutput();
    void onCommandInputEnterPressed();
    void onOpenSettings();
    
    // SSH密钥管理相关槽函数
    void onManageSSHKeys();
    void onGenerateSSHKey();
    void onCopyPublicKey();
    void onInstallPublicKey();
    void onDeleteSSHKey();
    void onSSHKeyGenFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onGenerateAndDeploySSHKey();
    
    // 内置命令窗口相关槽函数
    void onExecuteBuiltinCommand();
    void onClearBuiltinCommand();
    void onClearBuiltinOutput();
    void onBuiltinCommandInputEnterPressed();
    void onPasswordInputEnterPressed();
    void onPasswordInputFinished();
    void onPasswordInputCanceled();
    void onDeploySSHKey();

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void connectSignals();
    void logMessage(const QString &message);
    bool validateSettings();
    bool validateSSHSettings();  // SSH密钥功能专用验证函数
    void startUpload();
    
    // 设置保存和加载
    void saveSettingsToFile();
    void loadSettingsFromFile();
    QString getSettingsFilePath();
    
    // 日志管理
    void writeLogToFile(const QString &message);
    QString getLogFilePath();
    
    // 文件校验
    QString calculateFileMD5(const QString &filePath);
    void startFileVerification();
    
    // SSH远程命令执行
    void executeRemoteCommand(const QString &command, const QString &workingDir = QString());
    void executeCustomRemoteCommand(const QString &command);
    void execute7evRemoteCommand(const QString &command);
    void executePreCheck7ev();
    void executePreCheck7evCommand(const QString &command);
    void executeActual7evUpgrade();
    void executeKu5pUpgrade();
    void executeKu5pRemoteCommand(const QString &command);
    
    // 机器码验证相关函数
    QString getMachineCode();
    bool checkMachineAuthorization();
    QString getAuthorizationFilePath();
    
    // SSH密钥管理相关函数
    QString getSSHKeyPath();
    QString getSSHPublicKeyPath();
    bool checkSSHKeyExists();
    QString readPublicKey();
    void generateSSHKey();
    void installPublicKeyToServer();
    void executeSSHKeyInstallation();
    void executeSSHKeyInstallationDirect();
    void showSmartInstallationGuide();
    void showManualInstallationGuide();
    void showSSHKeyStatus();
    void showSSHTroubleshooting();
    
    // 内置命令窗口相关函数
    void executeBuiltinSystemCommand(const QString &command);
    void executeSSHCommandWithPassword(const QString &command);
    void executeSSHWithPassword(const QString &command, const QString &password);
    void executeSSHWithDirectPassword(const QString &command, const QString &password);
    bool validateBasicSettings();
    void setBuiltinCommand(const QString &command);
    QString generateReliableSSHInstallCommand(const QString &username, const QString &ip, int port, const QString &publicKey);
    void showPasswordInput(const QString &prompt);
    void hidePasswordInput();
    void processPasswordInput(const QString &password);
    void executeSSHWithSshpass(const QString &password);
    void executeDirectSSHCommand(const QString &password);
    void executeSSHKeyGenerationAndDeployment();
    
    // UI组件
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    
    // 连接设置组
    QGroupBox *connectionGroup;
    QGridLayout *connectionLayout;
    QLabel *ipLabel;
    QLineEdit *ipLineEdit;
    QLabel *portLabel;
    QSpinBox *portSpinBox;
    QLabel *usernameLabel;
    QLineEdit *usernameLineEdit;
    QLabel *passwordLabel;
    QLineEdit *passwordLineEdit;
    QPushButton *testConnectionButton;
    QPushButton *sshKeyManageButton;
    
    // 文件选择组
    QGroupBox *fileGroup;
    QHBoxLayout *fileLayout;
    QLabel *fileLabel;
    QLineEdit *filePathLineEdit;
    QPushButton *selectFileButton;
    
    // 上传控制组
    QGroupBox *uploadGroup;
    QVBoxLayout *uploadLayout;
    QHBoxLayout *uploadButtonLayout;
    QPushButton *uploadButton;
    QPushButton *clearLogButton;
    QPushButton *upgradeQtButton;
    QPushButton *upgrade7evButton;
    QPushButton *upgradeKu5pButton;
    QLabel *statusLabel;
    QProgressBar *transferProgressBar;
    
    // 日志显示
    QLabel *logLabel;
    QTextEdit *logTextEdit;
    
    // 远程命令执行组
    QGroupBox *commandGroup;
    QVBoxLayout *commandLayout;
    QHBoxLayout *commandInputLayout;
    QLabel *commandLabel;
    QLineEdit *commandLineEdit;
    QPushButton *executeCommandButton;
    QPushButton *clearOutputButton;
    QLabel *outputLabel;
    QTextEdit *commandOutputEdit;
    
    // 内置命令窗口组
    QGroupBox *builtinCommandGroup;
    QVBoxLayout *builtinCommandLayout;
    QHBoxLayout *builtinCommandInputLayout;
    QHBoxLayout *quickCommandLayout;
    QLabel *builtinCommandLabel;
    QLineEdit *builtinCommandLineEdit;
    QPushButton *executeBuiltinCommandButton;
    QPushButton *clearBuiltinCommandButton;
    QPushButton *clearBuiltinOutputButton;
    QPushButton *deploySSHKeyButton;
    QLabel *builtinOutputLabel;
    QTextEdit *builtinCommandOutputEdit;
    
    // 密码输入相关控件
    QWidget *passwordInputWidget;
    QHBoxLayout *passwordInputLayout;
    QLabel *passwordPromptLabel;
    QLineEdit *sshPasswordLineEdit;
    QPushButton *passwordConfirmButton;
    QPushButton *passwordCancelButton;
    
    // 菜单和状态栏
    QMenu *fileMenu;
    QMenu *settingsMenu;
    QMenu *helpMenu;
    QAction *saveSettingsAction;
    QAction *loadSettingsAction;
    QAction *exitAction;
    QAction *aboutAction;
    QAction *toggleLogAction;
    QAction *toggleCommandAction;
    QAction *toggleBuiltinCommandAction;
    QAction *showMachineCodeAction;
    QAction *openSettingsAction;
    
    // 上传相关
    QProcess *uploadProcess;
    QProcess *testProcess;
    QProcess *verifyProcess;
    QProcess *remoteCommandProcess;
    QProcess *customCommandProcess;
    QProcess *preCheck7evProcess;
    QProcess *upgrade7evProcess;
    QProcess *upgradeKu5pProcess;
    QProcess *sshKeyGenProcess;
    QProcess *builtinCommandProcess;
    QTimer *progressTimer;
    QTimer *timeoutTimer;
    QString selectedFilePath;
    QString localFileMD5;
    QPushButton *cancelButton;
    QTemporaryFile *keyFile;
    
    // 设置相关
    SettingsDialog *settingsDialog;
    QString remoteDirectory;
    QString logStoragePath;
    QString defaultLocalPath;
    bool autoSaveSettings;
    bool showLogByDefault;
    bool showCommandByDefault;
    bool showBuiltinCommandByDefault;
    bool autoCleanLog;
    int logRetentionDays;
    QString qtExtractPath;
    QString sevEvExtractPath;
    
    // 应用设置管理
    void loadApplicationSettings();
    void saveApplicationSettings();
    
    // 日志清理功能
    void cleanExpiredLogs();
    
    // 按钮状态管理
    void disableAllOperationButtons();
    void enableAllOperationButtons();
    
    QString pendingSSHCommand;  // 待执行的SSH命令
    bool waitingForPassword;    // 是否正在等待密码输入
    bool isGeneratingAndDeploying; // 是否正在执行一体化生成和部署
};

#endif // MAINWINDOW_H 