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
    void onToggleFileSelectionView();
    void onToggleSettingsView();
    void onVerifyFileFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onShowMachineCode();
    void onUpgradeQtSoftware();
    void onUpgrade7evEmmc();
    void onUpgrade7evSd();
    void onUpgradeKu5p();
    void onUpgradePatch();
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
    void onEnableSSHKey();
    void onDisableSSHKey(); // 新增：退出SSH密钥功能
    
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
    bool validateConnectionSettings();  // 只验证连接设置，不检查文件路径
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
    
    // 加密文件解密功能
    bool decryptPackageFile(const QString &encryptedPath, const QString &outputPath);
    QString createTempDecryptedFile(const QString &encryptedPath);
    void cleanupTempDecryptedFile();
    
    // SSH远程命令执行
    void executeRemoteCommand(const QString &command, const QString &workingDir = QString());
    void executeCustomRemoteCommand(const QString &command);
    void execute7evRemoteCommand(const QString &command);
    void executePreCheck7ev(const QString &devicePath, const QString &mountPath);
    void executePreCheck7evCommand(const QString &command);
    void executeActual7evUpgrade(const QString &devicePath, const QString &mountPath);
    void executeKu5pUpgrade();
    void executeKu5pRemoteCommand(const QString &command);
    void executePatchRemoteCommand(const QString &command);
    
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
    QPushButton *upgrade7evEmmcButton;
    QPushButton *upgrade7evSdButton;
    QPushButton *upgradeKu5pButton;
    QPushButton *upgradePatchButton;
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
    QAction *toggleFileSelectionAction;
    QAction *toggleSettingsAction;
    QAction *showMachineCodeAction;
    QAction *openSettingsAction;
    QAction *enableSSHKeyAction;
    QAction *disableSSHKeyAction; // 新增：退出SSH密钥菜单项
    
    // 上传相关
    QProcess *uploadProcess;
    QProcess *testProcess;
    QProcess *verifyProcess;
    QProcess *remoteCommandProcess;
    QProcess *customCommandProcess;
    QProcess *preCheck7evProcess;
    QProcess *upgrade7evProcess;
    QProcess *upgradeKu5pProcess;
    QProcess *upgradePatchProcess;
    QProcess *sshKeyGenProcess;
    QProcess *builtinCommandProcess;
    QTimer *progressTimer;
    QTimer *timeoutTimer;
    QString selectedFilePath;
    QString localFileMD5;
    QPushButton *cancelButton;
    QTemporaryFile *keyFile;
    QString tempDecryptedFilePath;  // 临时解密文件路径
    QString originalEncryptedPath;  // 原始加密文件路径
    
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
    QString currentDevicePath;  // 当前升级使用的设备路径
    QString currentMountPath;   // 当前升级使用的挂载路径
    
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
    bool sshKeyEnabled; // 新增：SSH密钥功能是否已启用
    // 标志位
    bool isQtUpgradeFile = false;  // 标记当前是否在处理Qt升级包
    bool is7evUpgradeFile = false; // 标记当前是否在处理7ev EMMC升级包
    bool is7evSdUpgradeFile = false; // 标记当前是否在处理7ev SD卡升级包
    bool isKu5pUpgradeFile = false; // 标记当前是否在处理KU5P升级包
    bool isPatchUpgradeFile = false; // 标记当前是否在处理升级补丁包
};

#endif // MAINWINDOW_H 