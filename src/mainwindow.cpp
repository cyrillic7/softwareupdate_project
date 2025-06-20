/**
 * @File Name: mainwindow.cpp
 * @brief  680图像机软件主窗口实现，包含文件选择、上传进度显示和授权验证
 * @Author : chency email:121888719@qq.com
 * @Version : 1.0
 * @Creat Date : 2025
 *
 */

#include "mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QSysInfo>
#include <QNetworkInterface>
#include "settingsdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), uploadProcess(nullptr), testProcess(nullptr), verifyProcess(nullptr),
              remoteCommandProcess(nullptr), customCommandProcess(nullptr), preCheck7evProcess(nullptr), upgrade7evProcess(nullptr),
        upgradeKu5pProcess(nullptr), sshKeyGenProcess(nullptr), builtinCommandProcess(nullptr), progressTimer(nullptr), timeoutTimer(nullptr), keyFile(nullptr),
        settingsDialog(nullptr), remoteDirectory("/media/sata/ue_data/"), waitingForPassword(false), isGeneratingAndDeploying(false)
{
    // 设置应用程序信息
    QApplication::setOrganizationName("680SoftwareUpdate");
    QApplication::setApplicationName("680SoftwareUpdate");
    QApplication::setApplicationVersion("1.0");
    
    setupUI();
    setupMenuBar();
    setupStatusBar();
    connectSignals();
    
    // 设置窗口属性
    setWindowTitle("680图像机软件升级工具");
    
    // 设置窗口大小策略，允许自适应调整
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    
    // 确保所有控件都已正确设置后，让窗口自动调整到合适的大小
    QTimer::singleShot(0, this, [this](){
        adjustSize();
        // 设置合理的最小尺寸，但允许窗口扩展
        int minWidth = qMax(700, sizeHint().width());
        int minHeight = qMax(500, sizeHint().height());
        setMinimumSize(minWidth, minHeight);
    });
    
    // 初始化定时器
    progressTimer = new QTimer(this);
    connect(progressTimer, &QTimer::timeout, this, &MainWindow::onUploadProgress);
    
    timeoutTimer = new QTimer(this);
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, this, &MainWindow::onUploadTimeout);
    
    // 设置默认值（SCP配置）
    ipLineEdit->setText("172.16.10.161");
    portSpinBox->setValue(22);  // SSH端口
    usernameLineEdit->setText("root");
    
    logMessage("应用程序启动完成，请配置SSH设置并选择要上传的文件。");
    logMessage("使用SCP协议通过SSH安全传输文件，请确保服务器已开启SSH服务。");
    logMessage("✨ SSH密钥认证配置简化：");
    logMessage("1. 点击 'SSH密钥' 按钮打开密钥管理");
    logMessage("2. 选择 '生成密钥' 自动创建SSH密钥对");
    logMessage("3. 选择 '安装到服务器' 自动配置免密码认证");
    logMessage("4. 完成后即可享受免密码连接体验！");
    logMessage("💻 新增功能：内置命令窗口");
    logMessage("• 可以直接在应用内执行SSH命令");
    logMessage("• 支持一键部署SSH密钥，实现免密码登录");
    logMessage("• 实时显示命令执行结果");
    
    // 检查SSH密钥状态并显示提示
    if (checkSSHKeyExists()) {
        logMessage("✅ 检测到已有SSH密钥，可直接使用密钥认证");
    } else {
        logMessage("💡 建议使用SSH密钥认证替代密码认证，更安全便捷");
    }
    
    // 设置进度条样式
    transferProgressBar->setStyleSheet(
        "QProgressBar {"
        "    border: 1px solid #ccc;"
        "    border-radius: 10px;"
        "    background-color: #f0f0f0;"
        "    text-align: center;"
        "}"
        "QProgressBar::chunk {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4CAF50, stop:1 #45a049);"
        "    border-radius: 9px;"
        "    margin: 1px;"
        "}"
    );
    
    // 初始化应用设置默认值
    QString appDir = QApplication::applicationDirPath();
    logStoragePath = appDir;        // 日志保存在可执行程序目录
    defaultLocalPath = appDir;      // 默认文件路径为可执行程序目录
    autoSaveSettings = true;
    showLogByDefault = false;
    showCommandByDefault = false;
    showBuiltinCommandByDefault = false;
    autoCleanLog = false;
    logRetentionDays = 30;
    qtExtractPath = "/mnt/qtfs";    // Qt软件升级默认解压路径
    sevEvExtractPath = "/mnt/mmcblk0p1";  // 7ev固件升级默认解压路径
    
    // 加载应用设置
    loadApplicationSettings();
    
    // 根据设置初始化界面
    if (showLogByDefault) {
        logLabel->setVisible(true);
        logTextEdit->setVisible(true);
        toggleLogAction->setChecked(true);
        toggleLogAction->setText("隐藏日志(&L)");
    } else {
        logLabel->setVisible(false);
        logTextEdit->setVisible(false);
        toggleLogAction->setChecked(false);
        toggleLogAction->setText("显示日志(&L)");
    }
    
    if (showCommandByDefault) {
        commandGroup->setVisible(true);
        toggleCommandAction->setChecked(true);
        toggleCommandAction->setText("隐藏远程命令执行(&R)");
    } else {
        commandGroup->setVisible(false);
        toggleCommandAction->setChecked(false);
        toggleCommandAction->setText("显示远程命令执行(&R)");
    }
    
    if (showBuiltinCommandByDefault) {
        builtinCommandGroup->setVisible(true);
        toggleBuiltinCommandAction->setChecked(true);
        toggleBuiltinCommandAction->setText("隐藏内置命令窗口(&B)");
    } else {
        builtinCommandGroup->setVisible(false);
        toggleBuiltinCommandAction->setChecked(false);
        toggleBuiltinCommandAction->setText("显示内置命令窗口(&B)");
    }
    
    // 如果启用自动清理日志，启动时执行一次清理
    if (autoCleanLog && logRetentionDays > 0) {
        cleanExpiredLogs();
    }
    
    // 启动时尝试加载之前保存的设置
    QString settingsPath = getSettingsFilePath();
    if (QFile::exists(settingsPath)) {
        logMessage("检测到设置文件，自动加载上次保存的设置...");
        loadSettingsFromFile();
    } else {
        logMessage(QString("设置文件不存在: %1").arg(settingsPath));
        logMessage("使用默认设置，您可以通过菜单保存当前设置。");
    }
}

MainWindow::~MainWindow()
{
    if (uploadProcess) {
        uploadProcess->kill();
        uploadProcess->waitForFinished(3000);
        uploadProcess->deleteLater();
    }
    if (testProcess) {
        testProcess->kill();
        testProcess->waitForFinished(1000);
        testProcess->deleteLater();
    }
    if (verifyProcess) {
        verifyProcess->kill();
        verifyProcess->waitForFinished(1000);
        verifyProcess->deleteLater();
    }
    if (remoteCommandProcess) {
        remoteCommandProcess->kill();
        remoteCommandProcess->waitForFinished(1000);
        remoteCommandProcess->deleteLater();
    }
    if (customCommandProcess) {
        customCommandProcess->kill();
        customCommandProcess->waitForFinished(1000);
        customCommandProcess->deleteLater();
    }
    if (preCheck7evProcess) {
        preCheck7evProcess->kill();
        preCheck7evProcess->waitForFinished(1000);
        preCheck7evProcess->deleteLater();
    }
    if (upgrade7evProcess) {
        upgrade7evProcess->kill();
        upgrade7evProcess->waitForFinished(1000);
        upgrade7evProcess->deleteLater();
    }
    if (upgradeKu5pProcess) {
        upgradeKu5pProcess->kill();
        upgradeKu5pProcess->waitForFinished(1000);
        upgradeKu5pProcess->deleteLater();
    }
    if (sshKeyGenProcess) {
        sshKeyGenProcess->kill();
        sshKeyGenProcess->waitForFinished(1000);
        sshKeyGenProcess->deleteLater();
    }
    if (builtinCommandProcess) {
        builtinCommandProcess->kill();
        builtinCommandProcess->waitForFinished(1000);
        builtinCommandProcess->deleteLater();
    }
    if (keyFile) {
        keyFile->close();
        delete keyFile;
    }
}

void MainWindow::setupUI()
{
    // 创建中心窗口部件
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 创建主布局
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // 连接设置组
    connectionGroup = new QGroupBox("服务器连接设置", this);
    connectionGroup->setObjectName("connectionGroup");
    connectionGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    connectionLayout = new QGridLayout(connectionGroup);
    connectionLayout->setSpacing(10);
    connectionLayout->setContentsMargins(15, 20, 15, 15);
    
    // IP地址
    ipLabel = new QLabel("服务器IP:", this);
    ipLineEdit = new QLineEdit(this);
    ipLineEdit->setObjectName("ipLineEdit");
    ipLineEdit->setPlaceholderText("例如: 192.168.1.100");
    ipLineEdit->setMinimumWidth(150);
    
    // 端口
    portLabel = new QLabel("端口:", this);
    portSpinBox = new QSpinBox(this);
    portSpinBox->setObjectName("portSpinBox");
    portSpinBox->setRange(1, 65535);
    portSpinBox->setValue(22);
    portSpinBox->setMinimumWidth(80);
    
    // 用户名
    usernameLabel = new QLabel("用户名:", this);
    usernameLineEdit = new QLineEdit(this);
    usernameLineEdit->setObjectName("usernameLineEdit");
    usernameLineEdit->setPlaceholderText("Linux用户名");
    usernameLineEdit->setMinimumWidth(120);
    
    // 密码
    passwordLabel = new QLabel("密码:", this);
    passwordLineEdit = new QLineEdit(this);
    passwordLineEdit->setObjectName("passwordLineEdit");
    passwordLineEdit->setEchoMode(QLineEdit::Password);
    passwordLineEdit->setPlaceholderText("登录密码");
    passwordLineEdit->setMinimumWidth(120);
    
    // 测试连接按钮
    testConnectionButton = new QPushButton("测试连接", this);
    testConnectionButton->setObjectName("testButton");
    testConnectionButton->setMinimumWidth(100);
    testConnectionButton->setMaximumWidth(120);
    
    // SSH密钥管理按钮
    sshKeyManageButton = new QPushButton("SSH密钥", this);
    sshKeyManageButton->setObjectName("sshKeyManageButton");
    sshKeyManageButton->setMinimumWidth(100);
    sshKeyManageButton->setMaximumWidth(120);
    sshKeyManageButton->setToolTip("生成和部署SSH密钥，一键配置免密码连接");
    
    // 布局连接设置
    connectionLayout->addWidget(ipLabel, 0, 0);
    connectionLayout->addWidget(ipLineEdit, 0, 1);
    connectionLayout->addWidget(portLabel, 0, 2);
    connectionLayout->addWidget(portSpinBox, 0, 3);
    connectionLayout->addWidget(usernameLabel, 1, 0);
    connectionLayout->addWidget(usernameLineEdit, 1, 1);
    connectionLayout->addWidget(passwordLabel, 1, 2);
    connectionLayout->addWidget(passwordLineEdit, 1, 3);
    connectionLayout->addWidget(testConnectionButton, 1, 4);
    connectionLayout->addWidget(sshKeyManageButton, 1, 5);
    
    // 设置列拉伸比例，使输入框能够自适应宽度
    connectionLayout->setColumnStretch(1, 2); // 第二列（输入框）拉伸比例为2
    connectionLayout->setColumnStretch(3, 1); // 第四列（按钮）拉伸比例为1
    
    mainLayout->addWidget(connectionGroup);
    
    // 文件选择组
    fileGroup = new QGroupBox("文件选择", this);
    fileGroup->setObjectName("fileGroup");
    fileGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    fileLayout = new QHBoxLayout(fileGroup);
    
    fileLabel = new QLabel("选择文件:", this);
    filePathLineEdit = new QLineEdit(this);
    filePathLineEdit->setObjectName("filePathLineEdit");
    filePathLineEdit->setPlaceholderText("点击浏览按钮选择要上传的文件");
    filePathLineEdit->setReadOnly(true);
    
    selectFileButton = new QPushButton("浏览文件", this);
    selectFileButton->setObjectName("selectFileButton");
    selectFileButton->setFixedWidth(100); // 固定按钮宽度
    
    // 设置文件路径输入框的拉伸策略
    filePathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    fileLayout->addWidget(fileLabel);
    fileLayout->addWidget(filePathLineEdit, 1); // 设置拉伸因子，使其占用更多空间
    fileLayout->addWidget(selectFileButton);
    
    mainLayout->addWidget(fileGroup);
    
    // 上传控制组
    uploadGroup = new QGroupBox("上传控制", this);
    uploadGroup->setObjectName("uploadGroup");
    uploadGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    uploadLayout = new QVBoxLayout(uploadGroup);
    
    // 状态显示区域
    QWidget *statusWidget = new QWidget(this);
    QHBoxLayout *statusLayout = new QHBoxLayout(statusWidget);
    statusLayout->setContentsMargins(8, 8, 8, 8);
    statusLayout->setSpacing(10);
    
    // 状态文字标签
    statusLabel = new QLabel("准备就绪", this);
    statusLabel->setObjectName("statusLabel");
    statusLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    statusLabel->setMinimumWidth(150);
    
    // 传输进度条（横向滚动条）
    transferProgressBar = new QProgressBar(this);
    transferProgressBar->setObjectName("transferProgressBar");
    transferProgressBar->setMinimum(0);
    transferProgressBar->setMaximum(0);  // 设置为不确定模式，显示滚动动画
    transferProgressBar->setVisible(false);  // 默认隐藏
    transferProgressBar->setMaximumHeight(20);
    transferProgressBar->setTextVisible(false);  // 不显示百分比文字
    
    // 将标签和进度条添加到布局
    statusLayout->addWidget(statusLabel);
    statusLayout->addWidget(transferProgressBar, 1);  // 进度条占用剩余空间
    
    // 设置状态区域的样式
    statusWidget->setStyleSheet("QWidget { background-color: #f0f0f0; border: 1px solid #ccc; border-radius: 4px; }");
    statusWidget->setMinimumHeight(40);
    
    // 按钮布局
    uploadButtonLayout = new QHBoxLayout();
    uploadButton = new QPushButton("开始上传", this);
    uploadButton->setObjectName("uploadButton");
    
    cancelButton = new QPushButton("取消上传", this);
    cancelButton->setObjectName("cancelButton");
    cancelButton->setVisible(false);
    
    clearLogButton = new QPushButton("清空日志", this);
    clearLogButton->setObjectName("clearLogButton");
    
    upgradeQtButton = new QPushButton("升级qt软件", this);
    upgradeQtButton->setObjectName("upgradeQtButton");
    
    upgrade7evButton = new QPushButton("7ev固件升级", this);
    upgrade7evButton->setObjectName("upgrade7evButton");
    
    // 设置7ev固件升级按钮的红色样式
    upgrade7evButton->setStyleSheet(
        "QPushButton#upgrade7evButton {"
        "    background-color: #e74c3c;"
        "    color: white;"
        "    border: 2px solid #c0392b;"
        "    border-radius: 6px;"
        "    font-weight: bold;"
        "    padding: 8px 16px;"
        "}"
        "QPushButton#upgrade7evButton:hover {"
        "    background-color: #c0392b;"
        "    border-color: #a93226;"
        "}"
        "QPushButton#upgrade7evButton:pressed {"
        "    background-color: #a93226;"
        "    border-color: #922b21;"
        "}"
        "QPushButton#upgrade7evButton:disabled {"
        "    background-color: #bdc3c7;"
        "    color: #7f8c8d;"
        "    border-color: #95a5a6;"
        "}"
    );
    
    upgradeKu5pButton = new QPushButton("升级ku5p", this);
    upgradeKu5pButton->setObjectName("upgradeKu5pButton");
    
    // 设置ku5p升级按钮的橙色样式
    upgradeKu5pButton->setStyleSheet(
        "QPushButton#upgradeKu5pButton {"
        "    background-color: #f39c12;"
        "    color: white;"
        "    border: 2px solid #e67e22;"
        "    border-radius: 6px;"
        "    font-weight: bold;"
        "    padding: 8px 16px;"
        "}"
        "QPushButton#upgradeKu5pButton:hover {"
        "    background-color: #e67e22;"
        "    border-color: #d35400;"
        "}"
        "QPushButton#upgradeKu5pButton:pressed {"
        "    background-color: #d35400;"
        "    border-color: #ba4a00;"
        "}"
        "QPushButton#upgradeKu5pButton:disabled {"
        "    background-color: #bdc3c7;"
        "    color: #7f8c8d;"
        "    border-color: #95a5a6;"
        "}"
    );
    
    // 设置按钮的最小宽度，保持一致的外观
    uploadButton->setMinimumWidth(100);
    cancelButton->setMinimumWidth(100);
    clearLogButton->setMinimumWidth(100);
    upgradeQtButton->setMinimumWidth(100);
    upgrade7evButton->setMinimumWidth(100);
    upgradeKu5pButton->setMinimumWidth(100);
    
    uploadButtonLayout->addWidget(uploadButton);
    uploadButtonLayout->addWidget(cancelButton);
    uploadButtonLayout->addWidget(clearLogButton);
    uploadButtonLayout->addWidget(upgradeQtButton);
    uploadButtonLayout->addWidget(upgrade7evButton);
    uploadButtonLayout->addWidget(upgradeKu5pButton);
    uploadButtonLayout->addStretch(); // 添加弹性空间，使按钮左对齐
    
    uploadLayout->addWidget(statusWidget);
    uploadLayout->addLayout(uploadButtonLayout);
    
    mainLayout->addWidget(uploadGroup);
    
    // 远程命令执行组
    commandGroup = new QGroupBox("远程命令执行", this);
    commandGroup->setObjectName("commandGroup");
    commandGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    commandLayout = new QVBoxLayout(commandGroup);
    commandLayout->setSpacing(10);
    commandLayout->setContentsMargins(15, 20, 15, 15);
    
    // 命令输入区域
    QWidget *commandInputWidget = new QWidget(this);
    commandInputLayout = new QHBoxLayout(commandInputWidget);
    commandInputLayout->setContentsMargins(0, 0, 0, 0);
    
    commandLabel = new QLabel("命令:", this);
    commandLineEdit = new QLineEdit(this);
    commandLineEdit->setObjectName("commandLineEdit");
    commandLineEdit->setPlaceholderText("输入要在远程服务器执行的命令（例如: ls -la, ps aux 等）");
    
    executeCommandButton = new QPushButton("执行命令", this);
    executeCommandButton->setObjectName("executeCommandButton");
    executeCommandButton->setMinimumWidth(100);
    
    clearOutputButton = new QPushButton("清空输出", this);
    clearOutputButton->setObjectName("clearOutputButton");
    clearOutputButton->setMinimumWidth(100);
    
    commandInputLayout->addWidget(commandLabel);
    commandInputLayout->addWidget(commandLineEdit, 1);
    commandInputLayout->addWidget(executeCommandButton);
    commandInputLayout->addWidget(clearOutputButton);
    
    // 输出显示区域
    outputLabel = new QLabel("命令输出:", this);
    commandOutputEdit = new QTextEdit(this);
    commandOutputEdit->setObjectName("commandOutputEdit");
    commandOutputEdit->setReadOnly(true);
    commandOutputEdit->setMinimumHeight(200);
    commandOutputEdit->setMaximumHeight(300);
    commandOutputEdit->setFont(QFont("Consolas", 9)); // 使用等宽字体
    
    // 设置类似终端的样式
    commandOutputEdit->setStyleSheet(
        "QTextEdit#commandOutputEdit {"
        "    background-color: #2b2b2b;"
        "    color: #ffffff;"
        "    border: 2px solid #555555;"
        "    border-radius: 6px;"
        "    font-family: 'Consolas', 'Monaco', monospace;"
        "    font-size: 9pt;"
        "    line-height: 1.2;"
        "}"
    );
    
    commandLayout->addWidget(commandInputWidget);
    commandLayout->addWidget(outputLabel);
    commandLayout->addWidget(commandOutputEdit, 1);
    
    mainLayout->addWidget(commandGroup);
    
    // 默认隐藏远程命令执行窗口
    commandGroup->setVisible(false);
    
    // 内置命令窗口组
    builtinCommandGroup = new QGroupBox("内置命令窗口", this);
    builtinCommandGroup->setObjectName("builtinCommandGroup");
    builtinCommandGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    builtinCommandLayout = new QVBoxLayout(builtinCommandGroup);
    builtinCommandLayout->setSpacing(10);
    builtinCommandLayout->setContentsMargins(15, 20, 15, 15);
    
    // 命令输入区域
    QWidget *builtinCommandInputWidget = new QWidget(this);
    builtinCommandInputLayout = new QHBoxLayout(builtinCommandInputWidget);
    builtinCommandInputLayout->setContentsMargins(0, 0, 0, 0);
    
    builtinCommandLabel = new QLabel("命令:", this);
    builtinCommandLineEdit = new QLineEdit(this);
    builtinCommandLineEdit->setObjectName("builtinCommandLineEdit");
    builtinCommandLineEdit->setPlaceholderText("输入系统命令（如SSH命令）或点击下方按钮自动填入");
    
    executeBuiltinCommandButton = new QPushButton("执行命令", this);
    executeBuiltinCommandButton->setObjectName("executeBuiltinCommandButton");
    executeBuiltinCommandButton->setMinimumWidth(100);
    
    clearBuiltinCommandButton = new QPushButton("清空命令", this);
    clearBuiltinCommandButton->setObjectName("clearBuiltinCommandButton");
    clearBuiltinCommandButton->setMinimumWidth(100);
    
    clearBuiltinOutputButton = new QPushButton("清空输出", this);
    clearBuiltinOutputButton->setObjectName("clearBuiltinOutputButton");
    clearBuiltinOutputButton->setMinimumWidth(100);
    
    builtinCommandInputLayout->addWidget(builtinCommandLabel);
    builtinCommandInputLayout->addWidget(builtinCommandLineEdit, 1);
    builtinCommandInputLayout->addWidget(executeBuiltinCommandButton);
    builtinCommandInputLayout->addWidget(clearBuiltinCommandButton);
    builtinCommandInputLayout->addWidget(clearBuiltinOutputButton);
    
    // 快捷命令按钮区域
    QWidget *quickCommandWidget = new QWidget(this);
    quickCommandLayout = new QHBoxLayout(quickCommandWidget);
    quickCommandLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel *quickLabel = new QLabel("快捷操作:", this);
    deploySSHKeyButton = new QPushButton("部署SSH Key", this);
    deploySSHKeyButton->setObjectName("deploySSHKeyButton");
    deploySSHKeyButton->setMinimumWidth(150);
    deploySSHKeyButton->setToolTip("直接部署SSH密钥到服务器，实现免密码登录");
    
    quickCommandLayout->addWidget(quickLabel);
    quickCommandLayout->addWidget(deploySSHKeyButton);
    quickCommandLayout->addStretch();
    
    // 密码输入区域（默认隐藏）
    passwordInputWidget = new QWidget(this);
    passwordInputWidget->setVisible(false);
    passwordInputLayout = new QHBoxLayout(passwordInputWidget);
    passwordInputLayout->setContentsMargins(0, 0, 0, 0);
    
    passwordPromptLabel = new QLabel("请输入服务器密码:", this);
    passwordPromptLabel->setStyleSheet("QLabel { color: #ff7f00; font-weight: bold; }");
    
    sshPasswordLineEdit = new QLineEdit(this);
    sshPasswordLineEdit->setObjectName("sshPasswordLineEdit");
    sshPasswordLineEdit->setEchoMode(QLineEdit::Password);
    sshPasswordLineEdit->setPlaceholderText("输入SSH密码后按回车或点击确认");
    sshPasswordLineEdit->setMinimumWidth(200);
    
    passwordConfirmButton = new QPushButton("确认", this);
    passwordConfirmButton->setObjectName("passwordConfirmButton");
    passwordConfirmButton->setMinimumWidth(80);
    passwordConfirmButton->setStyleSheet("QPushButton { background-color: #28a745; color: white; font-weight: bold; }");
    
    passwordCancelButton = new QPushButton("取消", this);
    passwordCancelButton->setObjectName("passwordCancelButton");
    passwordCancelButton->setMinimumWidth(80);
    passwordCancelButton->setStyleSheet("QPushButton { background-color: #dc3545; color: white; }");
    
    passwordInputLayout->addWidget(passwordPromptLabel);
    passwordInputLayout->addWidget(sshPasswordLineEdit, 1);
    passwordInputLayout->addWidget(passwordConfirmButton);
    passwordInputLayout->addWidget(passwordCancelButton);
    
    // 输出显示区域
    builtinOutputLabel = new QLabel("命令输出:", this);
    builtinCommandOutputEdit = new QTextEdit(this);
    builtinCommandOutputEdit->setObjectName("builtinCommandOutputEdit");
    builtinCommandOutputEdit->setReadOnly(true);
    builtinCommandOutputEdit->setMinimumHeight(150);
    builtinCommandOutputEdit->setMaximumHeight(250);
    builtinCommandOutputEdit->setFont(QFont("Consolas", 9)); // 使用等宽字体
    
    // 设置类似终端的样式
    builtinCommandOutputEdit->setStyleSheet(
        "QTextEdit#builtinCommandOutputEdit {"
        "    background-color: #1e1e1e;"
        "    color: #ffffff;"
        "    border: 2px solid #555555;"
        "    border-radius: 6px;"
        "    font-family: 'Consolas', 'Monaco', monospace;"
        "    font-size: 9pt;"
        "    line-height: 1.2;"
        "}"
    );
    
    builtinCommandLayout->addWidget(builtinCommandInputWidget);
    builtinCommandLayout->addWidget(quickCommandWidget);
    builtinCommandLayout->addWidget(passwordInputWidget);
    builtinCommandLayout->addWidget(builtinOutputLabel);
    builtinCommandLayout->addWidget(builtinCommandOutputEdit, 1);
    
    mainLayout->addWidget(builtinCommandGroup);
    
    // 默认隐藏内置命令窗口
    builtinCommandGroup->setVisible(false);
    
    // 日志显示
    logLabel = new QLabel("操作日志:", this);
    logTextEdit = new QTextEdit(this);
    logTextEdit->setObjectName("logTextEdit");
    logTextEdit->setMinimumHeight(120);
    logTextEdit->setMaximumHeight(250);
    logTextEdit->setReadOnly(true);
    
    // 设置日志区域的大小策略，允许垂直拉伸，但有合理的大小提示
    logTextEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    logTextEdit->setFixedHeight(180); // 设置一个合适的初始高度
    
    mainLayout->addWidget(logLabel);
    mainLayout->addWidget(logTextEdit, 1); // 设置拉伸因子为1，使其占用剩余空间
    
    // 默认隐藏日志控件
    logLabel->setVisible(false);
    logTextEdit->setVisible(false);
}

void MainWindow::setupMenuBar()
{
    // 文件菜单
    fileMenu = menuBar()->addMenu("文件(&F)");
    
    exitAction = new QAction("退出(&X)", this);
    exitAction->setShortcut(QKeySequence::Quit);
    
    fileMenu->addAction(exitAction);
    
    // 设置菜单
    settingsMenu = menuBar()->addMenu("设置(&S)");
    
    openSettingsAction = new QAction("设置选项(&O)", this);
    openSettingsAction->setShortcut(QKeySequence("Ctrl+O"));
    
    saveSettingsAction = new QAction("保存设置(&S)", this);
    saveSettingsAction->setShortcut(QKeySequence("Ctrl+S"));
    
    loadSettingsAction = new QAction("加载设置(&L)", this);
    loadSettingsAction->setShortcut(QKeySequence("Ctrl+L"));
    
    settingsMenu->addAction(openSettingsAction);
    settingsMenu->addSeparator();
    settingsMenu->addAction(saveSettingsAction);
    settingsMenu->addAction(loadSettingsAction);
    
    // 查看菜单
    QMenu *viewMenu = menuBar()->addMenu("查看(&V)");
    
    toggleLogAction = new QAction("显示日志(&L)", this);
    toggleLogAction->setShortcut(QKeySequence("Ctrl+L"));
    toggleLogAction->setCheckable(true);
    toggleLogAction->setChecked(false); // 默认隐藏日志
    viewMenu->addAction(toggleLogAction);
    
    toggleCommandAction = new QAction("显示远程命令执行(&R)", this);
    toggleCommandAction->setShortcut(QKeySequence("Ctrl+R"));
    toggleCommandAction->setCheckable(true);
    toggleCommandAction->setChecked(false); // 默认隐藏远程命令执行窗口
    viewMenu->addAction(toggleCommandAction);
    
    toggleBuiltinCommandAction = new QAction("显示内置命令窗口(&B)", this);
    toggleBuiltinCommandAction->setShortcut(QKeySequence("Ctrl+B"));
    toggleBuiltinCommandAction->setCheckable(true);
    toggleBuiltinCommandAction->setChecked(false); // 默认隐藏内置命令窗口
    viewMenu->addAction(toggleBuiltinCommandAction);
    
    // 帮助菜单
    helpMenu = menuBar()->addMenu("帮助(&H)");
    
    showMachineCodeAction = new QAction("显示机器码(&M)", this);
    aboutAction = new QAction("关于(&A)", this);
    
    helpMenu->addAction(showMachineCodeAction);
    helpMenu->addSeparator();
    helpMenu->addAction(aboutAction);
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("就绪", 3000);
}

void MainWindow::connectSignals()
{
    // 连接按钮信号
    connect(selectFileButton, &QPushButton::clicked, this, &MainWindow::onSelectFile);
    connect(uploadButton, &QPushButton::clicked, this, &MainWindow::onUploadFile);
    connect(cancelButton, &QPushButton::clicked, this, &MainWindow::onCancelUpload);
    connect(clearLogButton, &QPushButton::clicked, this, &MainWindow::onClearLog);
    connect(testConnectionButton, &QPushButton::clicked, this, &MainWindow::onTestConnection);
    connect(upgradeQtButton, &QPushButton::clicked, this, &MainWindow::onUpgradeQtSoftware);
    connect(upgrade7evButton, &QPushButton::clicked, this, &MainWindow::onUpgrade7evFirmware);
    connect(upgradeKu5pButton, &QPushButton::clicked, this, &MainWindow::onUpgradeKu5p);
    connect(executeCommandButton, &QPushButton::clicked, this, &MainWindow::onExecuteCustomCommand);
    connect(clearOutputButton, &QPushButton::clicked, this, &MainWindow::onClearCommandOutput);
    connect(commandLineEdit, &QLineEdit::returnPressed, this, &MainWindow::onCommandInputEnterPressed);
    connect(sshKeyManageButton, &QPushButton::clicked, this, &MainWindow::onManageSSHKeys);
    
    // 内置命令窗口信号连接
    connect(executeBuiltinCommandButton, &QPushButton::clicked, this, &MainWindow::onExecuteBuiltinCommand);
    connect(clearBuiltinCommandButton, &QPushButton::clicked, this, &MainWindow::onClearBuiltinCommand);
    connect(clearBuiltinOutputButton, &QPushButton::clicked, this, &MainWindow::onClearBuiltinOutput);
    connect(builtinCommandLineEdit, &QLineEdit::returnPressed, this, &MainWindow::onBuiltinCommandInputEnterPressed);
    connect(deploySSHKeyButton, &QPushButton::clicked, this, &MainWindow::onDeploySSHKey);
    
    // 密码输入相关信号连接
    connect(sshPasswordLineEdit, &QLineEdit::returnPressed, this, &MainWindow::onPasswordInputEnterPressed);
    connect(passwordConfirmButton, &QPushButton::clicked, this, &MainWindow::onPasswordInputFinished);
    connect(passwordCancelButton, &QPushButton::clicked, this, &MainWindow::onPasswordInputCanceled);
    
    // 连接菜单动作
    connect(openSettingsAction, &QAction::triggered, this, &MainWindow::onOpenSettings);
    connect(saveSettingsAction, &QAction::triggered, this, &MainWindow::onMenuAction);
    connect(loadSettingsAction, &QAction::triggered, this, &MainWindow::onMenuAction);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onMenuAction);
    connect(toggleLogAction, &QAction::triggered, this, &MainWindow::onToggleLogView);
    connect(toggleCommandAction, &QAction::triggered, this, &MainWindow::onToggleCommandView);
    connect(toggleBuiltinCommandAction, &QAction::triggered, this, &MainWindow::onToggleBuiltinCommandView);
    connect(showMachineCodeAction, &QAction::triggered, this, &MainWindow::onShowMachineCode);
}

void MainWindow::onSelectFile()
{
    // 使用设置中的默认文件路径，如果不存在则使用当前目录
    QString startPath = defaultLocalPath;
    if (!QDir(startPath).exists()) {
        startPath = QDir::currentPath();
    }
    
    QString fileName = QFileDialog::getOpenFileName(this, 
        "选择要上传的文件", 
        startPath, 
        "所有文件 (*)");
    
    if (!fileName.isEmpty()) {
        selectedFilePath = fileName;
        filePathLineEdit->setText(fileName);
        logMessage(QString("已选择文件: %1").arg(QFileInfo(fileName).fileName()));
        statusBar()->showMessage("文件选择完成", 2000);
        
        // 如果启用自动保存，更新默认路径为选择文件的目录
        if (autoSaveSettings) {
            QString newDefaultPath = QFileInfo(fileName).absolutePath();
            if (newDefaultPath != defaultLocalPath) {
                defaultLocalPath = newDefaultPath;
                saveApplicationSettings();
                logMessage(QString("默认文件路径已更新为: %1").arg(defaultLocalPath));
            }
        }
    }
}

void MainWindow::onUploadFile()
{
    if (!validateSettings()) {
        return;
    }
    
    startUpload();
}

void MainWindow::onClearLog()
{
    logTextEdit->clear();
    logMessage("日志已清空");
}

void MainWindow::onTestConnection()
{    
    if (testProcess) {
        testProcess->kill();
        testProcess->waitForFinished(1000);
        testProcess->deleteLater();
        testProcess = nullptr;
    }
    
    QString ip = ipLineEdit->text().trimmed();
    int port = portSpinBox->value();
    QString username = usernameLineEdit->text().trimmed();
    QString password = passwordLineEdit->text();
    
    if (ip.isEmpty() || username.isEmpty()) {
        logMessage("[错误] 请输入服务器IP地址和用户名");
        return;
    }
    
    logMessage(QString("[测试] 正在测试SSH连接到 %1@%2:%3")
               .arg(username).arg(ip).arg(port));
    
    testConnectionButton->setText("连接中...");
    testConnectionButton->setEnabled(false);
    
    // 创建SSH连接测试进程
    testProcess = new QProcess(this);
    
    connect(testProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::onTestFinished);
    
    // 设置环境变量
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    
    QString program;
    QStringList arguments;
    
    // 检查是否有SSH密钥
    bool hasSSHKey = checkSSHKeyExists();
    
    if (hasSSHKey) {
        logMessage("[测试] 检测到SSH密钥，优先使用密钥认证");
        
        program = "ssh";
        arguments << "-o" << "ConnectTimeout=10"
                  << "-o" << "StrictHostKeyChecking=no"
                  << "-o" << "UserKnownHostsFile=/dev/null"
                  << "-o" << "PreferredAuthentications=publickey"
                  << "-o" << "PubkeyAuthentication=yes"
                  << "-o" << "PasswordAuthentication=no"
                  << "-o" << "BatchMode=yes"
                  << "-p" << QString::number(port)
                  << QString("%1@%2").arg(username).arg(ip)
                  << "echo 'SSH连接测试成功'";
        
        logMessage("[提示] 使用SSH密钥认证，无需密码");
    } else if (!password.isEmpty()) {
        logMessage("[测试] 未检测到SSH密钥，但提供了密码");
        logMessage("[说明] 需要SSH密钥才能进行自动化认证");
        logMessage("[建议] 请点击'SSH密钥'按钮生成并配置SSH密钥");
        
        // 重置按钮状态
        testConnectionButton->setText("测试连接");
        testConnectionButton->setEnabled(true);
        return;
    } else {
        logMessage("[错误] 既没有SSH密钥，也没有输入密码");
        logMessage("[建议] 请选择以下任一方式：");
        logMessage("1. 点击'SSH密钥'按钮生成和配置SSH密钥（推荐）");
        logMessage("2. 或在密码字段输入服务器密码（临时方案）");
        
        // 重置按钮状态
        testConnectionButton->setText("测试连接");
        testConnectionButton->setEnabled(true);
        return;
    }
    
    testProcess->setProcessEnvironment(env);
    
    // 设置10秒超时
    QTimer::singleShot(10000, this, [this](){
        if (testProcess && testProcess->state() == QProcess::Running) {
            testProcess->kill();
            logMessage("[错误] 连接测试超时，请检查网络和服务器设置");
            testConnectionButton->setText("测试连接");
            testConnectionButton->setEnabled(true);
        }
    });
    
    testProcess->start(program, arguments);
}

void MainWindow::onUploadFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    timeoutTimer->stop();
    progressTimer->stop();
    uploadButton->setEnabled(true);
    upgradeQtButton->setEnabled(true);
    upgrade7evButton->setEnabled(true);
    upgradeKu5pButton->setEnabled(true);
    cancelButton->setVisible(false);
    transferProgressBar->setVisible(false);  // 隐藏传输进度条
    
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        logMessage("文件上传成功！开始校验文件完整性...");
        statusLabel->setText("正在校验文件完整性...");
        statusBar()->showMessage("正在校验文件...", 0);
        
        // 读取进程输出
        QString output = uploadProcess->readAllStandardOutput();
        if (!output.isEmpty()) {
            logMessage(QString("SCP输出: %1").arg(output.trimmed()));
        }
        
        // 开始MD5校验
        startFileVerification();
    } else {
        QString error = uploadProcess->readAllStandardError();
        QString output = uploadProcess->readAllStandardOutput();
        
        logMessage(QString("文件上传失败！退出码: %1").arg(exitCode));
        statusLabel->setText("上传失败");
        statusBar()->showMessage("上传失败", 3000);
        
        if (!error.isEmpty()) {
            logMessage(QString("错误信息: %1").arg(error.trimmed()));
        }
        if (!output.isEmpty()) {
            logMessage(QString("输出信息: %1").arg(output.trimmed()));
        }
        
        QMessageBox::warning(this, "上传失败", 
            QString("SCP上传失败\n错误代码: %1\n%2")
            .arg(exitCode)
            .arg(error.isEmpty() ? "请检查网络连接和服务器设置" : error.trimmed()));
    }
    
    if (uploadProcess) {
        uploadProcess->deleteLater();
        uploadProcess = nullptr;
    }
}

void MainWindow::onUploadProgress()
{
    // 使用滚动点来显示上传状态
    static int dotCount = 0;
    static QString baseStatus = "正在上传文件";
    
    if (uploadProcess && uploadProcess->state() == QProcess::Running) {
        dotCount = (dotCount + 1) % 4; // 0-3循环
        QString dots = QString(".").repeated(dotCount);
        QString spaces = QString(" ").repeated(3 - dotCount);
        
        QString displayStatus = baseStatus + dots + spaces;
        statusLabel->setText(displayStatus);
        statusBar()->showMessage(displayStatus, 0);
        
        // 每10秒记录一次状态
        static int logCounter = 0;
        logCounter++;
        if (logCounter % 10 == 0) {
            logMessage("文件上传中，请等待...");
        }
    } else {
        dotCount = 0; // 重置
        statusLabel->setText("准备就绪");
    }
}

void MainWindow::onUploadOutput()
{
    if (uploadProcess) {
        QByteArray data = uploadProcess->readAllStandardOutput();
        if (!data.isEmpty()) {
            QString output = QString::fromUtf8(data).trimmed();
            logMessage(QString("SCP输出: %1").arg(output));
        }
    }
}

void MainWindow::onCancelUpload()
{
    if (uploadProcess) {
        logMessage("用户取消上传操作...");
        
        timeoutTimer->stop();
        progressTimer->stop();
        uploadProcess->kill();
        
        statusLabel->setText("上传已取消");
        uploadButton->setEnabled(true);
        upgradeQtButton->setEnabled(true);
        upgrade7evButton->setEnabled(true);
        upgradeKu5pButton->setEnabled(true);
        cancelButton->setVisible(false);
        transferProgressBar->setVisible(false);  // 隐藏传输进度条
        
        logMessage("上传已取消");
        statusBar()->showMessage("上传已取消", 3000);
        
        uploadProcess->waitForFinished(1000);
        uploadProcess->deleteLater();
        uploadProcess = nullptr;
    }
}


void MainWindow::onUploadTimeout()
{
    if (uploadProcess) {
        logMessage("上传超时！强制终止上传...");
        
        progressTimer->stop();
        uploadProcess->kill();
        statusLabel->setText("上传超时");
        uploadButton->setEnabled(true);
        upgradeQtButton->setEnabled(true);
        upgrade7evButton->setEnabled(true);
        upgradeKu5pButton->setEnabled(true);
        cancelButton->setVisible(false);
        transferProgressBar->setVisible(false);  // 隐藏传输进度条
        
        logMessage("上传超时失败！请检查网络连接和服务器设置。");
        statusBar()->showMessage("上传超时", 3000);
        
        QMessageBox::warning(this, "上传超时", 
            "SCP上传操作超时（5分钟），请检查：\n"
            "1. 网络连接是否正常\n"
            "2. 服务器是否可达\n"
            "3. SSH服务是否正常\n"
            "4. 用户名密码是否正确");
            
        uploadProcess->waitForFinished(1000);
        uploadProcess->deleteLater();
        uploadProcess = nullptr;
    }
}

void MainWindow::onTestFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    testConnectionButton->setText("测试连接");
    testConnectionButton->setEnabled(true);
    
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        QString output = testProcess->readAllStandardOutput();
        logMessage("[成功] SSH连接测试成功，服务器可访问");
        statusBar()->showMessage("连接测试成功", 3000);
        if (!output.isEmpty()) {
            logMessage(QString("[响应] %1").arg(output.trimmed()));
        }
        
        // 如果使用SSH密钥认证成功，清空密码字段避免混淆
        bool hasSSHKey = checkSSHKeyExists();
        if (hasSSHKey) {
            passwordLineEdit->clear();
            logMessage("[安全] 已清空密码字段，当前使用SSH密钥认证");
        }
    } else {
        QString error = testProcess->readAllStandardError();
        QString output = testProcess->readAllStandardOutput();
        
        logMessage(QString("[错误] SSH连接测试失败 (退出码: %1)").arg(exitCode));
        statusBar()->showMessage("连接测试失败", 3000);
        
        if (!error.isEmpty()) {
            logMessage(QString("[错误信息] %1").arg(error.trimmed()));
        }
        if (!output.isEmpty()) {
            logMessage(QString("[输出] %1").arg(output.trimmed()));
        }
        
        // 提供详细的解决方案
        if (error.contains("Permission denied") || error.contains("Authentication failed")) {
            bool hasSSHKey = checkSSHKeyExists();
            QString password = passwordLineEdit->text();
            
            if (hasSSHKey) {
                logMessage("[分析] SSH密钥认证失败，可能的原因：");
                logMessage("1. 公钥未正确安装到服务器");
                logMessage("2. 服务器 ~/.ssh/authorized_keys 权限不正确");
                logMessage("3. 服务器SSH配置禁用了公钥认证");
                logMessage("4. 公钥格式不正确或损坏");
                logMessage("[解决方案] 请确认公钥已正确安装：");
                logMessage("💡 点击 'SSH密钥' 按钮，查看公钥内容并重新安装到服务器");
                logMessage("📋 确保服务器端文件权限正确：");
                logMessage("   chmod 700 ~/.ssh");
                logMessage("   chmod 600 ~/.ssh/authorized_keys");
            } else if (!password.isEmpty()) {
                logMessage("[分析] 密码认证失败，可能的原因：");
                logMessage("1. 密码不正确");
                logMessage("2. 服务器禁用了密码认证");
                logMessage("3. 用户账户被锁定");
                logMessage("[建议] 检查密码或改用SSH密钥认证：");
                logMessage("💡 点击 'SSH密钥' 按钮，选择 '生成密钥' 和 '安装到服务器'");
            } else {
                logMessage("[分析] 认证失败，既没有SSH密钥也没有密码：");
                logMessage("1. 未配置任何认证方式");
                logMessage("2. 服务器拒绝连接");
                logMessage("[建议] 配置SSH密钥认证：");
                logMessage("💡 点击 'SSH密钥' 按钮，选择 '生成密钥' 和 '安装到服务器'");
            }
            
            logMessage("✅ SSH密钥认证是最安全便捷的连接方式");
            logMessage("[说明] SSH密钥认证比密码认证更安全且适合自动化操作");
        } else if (error.contains("Connection refused") || error.contains("No route to host")) {
            logMessage("[提示] 网络连接问题，请检查：");
            logMessage("1. 服务器IP地址是否正确");
            logMessage("2. 服务器SSH服务是否运行");
            logMessage("3. 防火墙是否阻止SSH连接");
            logMessage("4. 网络是否可达");
        } else {
            logMessage("[提示] 请检查：1)服务器IP和端口 2)SSH服务状态 3)防火墙设置 4)网络连接");
            logMessage("[建议] 优先配置SSH密钥认证以避免密码认证问题");
        }
    }
    
    if (testProcess) {
        testProcess->deleteLater();
        testProcess = nullptr;
    }
}

void MainWindow::onMenuAction()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (action) {
        if (action == saveSettingsAction) {
            saveSettingsToFile();
        } else if (action == loadSettingsAction) {
            loadSettingsFromFile();
        } else if (action == aboutAction) {
            QMessageBox::about(this, "关于", 
                "文件上传工具\n\n"
                "开发者：Chency\n"
                "邮箱：121888719@qq.com\n\n"
                "版本：V1.0");
        }
    }
}

void MainWindow::logMessage(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString fullMessage = QString("[%1] %2").arg(timestamp).arg(message);
    
    // 显示到界面（如果日志控件可见）
    logTextEdit->append(fullMessage);
    
    // 写入到日志文件
    writeLogToFile(fullMessage);
}

bool MainWindow::validateSettings()
{
    if (ipLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, "设置错误", "请输入服务器IP地址");
        ipLineEdit->setFocus();
        return false;
    }
    
    if (usernameLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, "设置错误", "请输入用户名");
        usernameLineEdit->setFocus();
        return false;
    }
    
    // 检查认证方式：SSH密钥或密码
    bool hasSSHKey = checkSSHKeyExists();
    QString password = passwordLineEdit->text();
    
    if (!hasSSHKey && password.isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("认证设置");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("需要配置认证方式才能连接服务器。");
        msgBox.setInformativeText("请选择以下任一认证方式：");
        
        QPushButton *sshKeyButton = msgBox.addButton("配置SSH密钥（推荐）", QMessageBox::ActionRole);
        QPushButton *passwordButton = msgBox.addButton("输入密码", QMessageBox::ActionRole);
        msgBox.addButton("取消", QMessageBox::RejectRole);
        
        msgBox.exec();
        
        if (msgBox.clickedButton() == sshKeyButton) {
            // 打开SSH密钥管理
            onManageSSHKeys();
            return false; // 需要用户先配置SSH密钥
        } else if (msgBox.clickedButton() == passwordButton) {
            passwordLineEdit->setFocus();
            return false; // 需要用户输入密码
        } else {
            return false; // 用户取消
        }
    }
    
    if (selectedFilePath.isEmpty()) {
        QMessageBox::warning(this, "文件错误", "请先选择要上传的文件");
        return false;
    }
    
    if (!QFile::exists(selectedFilePath)) {
        QMessageBox::warning(this, "文件错误", "选择的文件不存在");
        return false;
    }
    
    // 显示当前使用的认证方式
    if (hasSSHKey) {
        logMessage("[认证] 使用SSH密钥认证，无需密码");
    } else {
        logMessage("[认证] 使用密码认证");
    }
    
    return true;
}

void MainWindow::startUpload()
{
    logMessage(QString("开始上传文件: %1").arg(QFileInfo(selectedFilePath).fileName()));
    logMessage(QString("文件大小: %1 字节").arg(QFileInfo(selectedFilePath).size()));
    
    // 计算本地文件MD5值
    logMessage("正在计算本地文件MD5值...");
    localFileMD5 = calculateFileMD5(selectedFilePath);
    if (localFileMD5.isEmpty()) {
        logMessage("[错误] 无法计算本地文件MD5值");
        QMessageBox::warning(this, "错误", "无法计算本地文件MD5值，上传取消");
        return;
    }
    logMessage(QString("本地文件MD5: %1").arg(localFileMD5));
    
    if (uploadProcess) {
        uploadProcess->kill();
        uploadProcess->waitForFinished(1000);
        uploadProcess->deleteLater();
        uploadProcess = nullptr;
    }
    
    uploadButton->setEnabled(false);
    upgradeQtButton->setEnabled(false);
    upgrade7evButton->setEnabled(false);
    upgradeKu5pButton->setEnabled(false);
    cancelButton->setVisible(true);
    statusLabel->setText("正在连接服务器...");
    transferProgressBar->setVisible(true);  // 显示传输进度条
    timeoutTimer->start(300000); // 5分钟超时
    
    // 准备SCP命令
    QString ip = ipLineEdit->text().trimmed();
    int port = portSpinBox->value();
    QString username = usernameLineEdit->text().trimmed();
    QString remotePath = remoteDirectory.trimmed();
    
    // 确保远程路径以/结尾
    if (!remotePath.endsWith('/')) {
        remotePath += '/';
    }
    
    // 构建远程文件路径
    QString remoteFile = QString("%1@%2:%3%4")
                        .arg(username)
                        .arg(ip)
                        .arg(remotePath)
                        .arg(QFileInfo(selectedFilePath).fileName());
    
    uploadProcess = new QProcess(this);
    
    // 连接信号
    connect(uploadProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::onUploadFinished);
    connect(uploadProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::onUploadOutput);
    
    // 设置环境变量
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    uploadProcess->setProcessEnvironment(env);
    
    // 构建SCP命令，使用SSH密钥认证
    QString program = "scp";
    QStringList arguments;
    arguments << "-o" << "ConnectTimeout=30"
              << "-o" << "StrictHostKeyChecking=no"
              << "-o" << "UserKnownHostsFile=/dev/null"
              << "-o" << "PreferredAuthentications=publickey,password"
              << "-o" << "PubkeyAuthentication=yes"
              << "-o" << "PasswordAuthentication=yes"
              << "-o" << "BatchMode=yes"  // 非交互模式
              << "-P" << QString::number(port)  // 注意SCP用大写P
              << selectedFilePath
              << remoteFile;
    
    logMessage(QString("SCP命令: %1 %2").arg(program).arg(arguments.join(" ")));
    logMessage("开始SCP上传...");
    
    // 启动进程
    uploadProcess->start(program, arguments);
    
    // 启动进度模拟定时器
    progressTimer->start(1000); // 每秒更新一次进度
    
    statusBar()->showMessage("正在上传文件...", 0);
}

QString MainWindow::getSettingsFilePath()
{
    // 获取可执行程序所在目录
    QString appDir = QApplication::applicationDirPath();
    QString settingsFile = appDir + "/upload_settings.json";
    return settingsFile;
}

void MainWindow::saveSettingsToFile()
{
    QString filePath = getSettingsFilePath();
    
    // 创建JSON对象保存设置
    QJsonObject settingsObj;
    settingsObj["serverIP"] = ipLineEdit->text();
    settingsObj["serverPort"] = portSpinBox->value();
    settingsObj["username"] = usernameLineEdit->text();
    settingsObj["remoteDir"] = remoteDirectory;
    settingsObj["saveTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // 创建JSON文档
    QJsonDocument doc(settingsObj);
    
    // 写入文件
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        
        logMessage(QString("设置已保存到: %1").arg(filePath));
        statusBar()->showMessage("设置保存成功", 2000);
        
        QMessageBox::information(this, "保存设置", 
            QString("设置已成功保存"));
    } else {
        QString errorMsg = QString("无法保存设置文件: %1").arg(file.errorString());
        logMessage(QString("[错误] %1").arg(errorMsg));
        statusBar()->showMessage("设置保存失败", 3000);
        
        QMessageBox::warning(this, "保存失败", errorMsg);
    }
}

void MainWindow::loadSettingsFromFile()
{
    QString filePath = getSettingsFilePath();
    
    QFile file(filePath);
    if (!file.exists()) {
        QString msg = QString("设置文件不存在: %1").arg(filePath);
        logMessage(QString("[提示] %1").arg(msg));
        statusBar()->showMessage("设置文件不存在", 3000);
        
        QMessageBox::information(this, "加载设置", 
            "设置文件不存在，请先保存设置。\n" + msg);
        return;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        QString errorMsg = QString("无法读取设置文件: %1").arg(file.errorString());
        logMessage(QString("[错误] %1").arg(errorMsg));
        statusBar()->showMessage("设置加载失败", 3000);
        
        QMessageBox::warning(this, "加载失败", errorMsg);
        return;
    }
    
    // 读取并解析JSON
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        QString errorMsg = QString("设置文件格式错误: %1").arg(parseError.errorString());
        logMessage(QString("[错误] %1").arg(errorMsg));
        statusBar()->showMessage("设置文件格式错误", 3000);
        
        QMessageBox::warning(this, "加载失败", errorMsg);
        return;
    }
    
    // 从JSON对象读取设置
    QJsonObject settingsObj = doc.object();
    
    // 应用设置到界面
    ipLineEdit->setText(settingsObj.value("serverIP").toString("172.16.10.161"));
    portSpinBox->setValue(settingsObj.value("serverPort").toInt(22));
    usernameLineEdit->setText(settingsObj.value("username").toString("root"));
    remoteDirectory = settingsObj.value("remoteDir").toString("/media/sata/ue_data/");
    
    QString saveTime = settingsObj.value("saveTime").toString();
    
    logMessage(QString("设置已从文件加载: %1").arg(filePath));
    if (!saveTime.isEmpty()) {
        logMessage(QString("设置保存时间: %1").arg(saveTime));
    }
    statusBar()->showMessage("设置加载成功", 2000);
    
    // QMessageBox::information(this, "加载设置", 
    //              QString("设置已成功从以下文件加载：\n%1\n\n保存时间: %2")
    //      .arg(filePath)
    //      .arg(saveTime.isEmpty() ? "未知" : saveTime));
}

void MainWindow::onToggleLogView()
{
    bool isVisible = logLabel->isVisible();
    
    // 切换日志控件的显示状态
    logLabel->setVisible(!isVisible);
    logTextEdit->setVisible(!isVisible);
    
    // 更新菜单项文本
    if (!isVisible) {
        toggleLogAction->setText("隐藏日志(&L)");
        toggleLogAction->setChecked(true);
        logMessage("日志面板已显示");
        showLogByDefault = true;
    } else {
        toggleLogAction->setText("显示日志(&L)");
        toggleLogAction->setChecked(false);
        showLogByDefault = false;
    }
    
    // 如果启用自动保存，保存设置
    if (autoSaveSettings) {
        saveApplicationSettings();
    }
    
    // 调整窗口大小以适应新布局
    adjustSize();
}

void MainWindow::onToggleCommandView()
{
    bool isVisible = commandGroup->isVisible();
    
    // 切换远程命令执行窗口的显示状态
    commandGroup->setVisible(!isVisible);
    
    // 更新菜单项文本和状态
    if (!isVisible) {
        toggleCommandAction->setText("隐藏远程命令执行(&R)");
        toggleCommandAction->setChecked(true);
        logMessage("远程命令执行窗口已显示");
        showCommandByDefault = true;
    } else {
        toggleCommandAction->setText("显示远程命令执行(&R)");
        toggleCommandAction->setChecked(false);
        logMessage("远程命令执行窗口已隐藏");
        showCommandByDefault = false;
    }
    
    // 如果启用自动保存，保存设置
    if (autoSaveSettings) {
        saveApplicationSettings();
    }
    
    // 调整窗口大小以适应新布局
    adjustSize();
}

void MainWindow::onToggleBuiltinCommandView()
{
    bool isVisible = builtinCommandGroup->isVisible();
    
    // 切换内置命令窗口的显示状态
    builtinCommandGroup->setVisible(!isVisible);
    
    // 更新菜单项文本和状态
    if (!isVisible) {
        toggleBuiltinCommandAction->setText("隐藏内置命令窗口(&B)");
        toggleBuiltinCommandAction->setChecked(true);
        logMessage("内置命令窗口已显示");
        showBuiltinCommandByDefault = true;
    } else {
        toggleBuiltinCommandAction->setText("显示内置命令窗口(&B)");  
        toggleBuiltinCommandAction->setChecked(false);
        logMessage("内置命令窗口已隐藏");
        showBuiltinCommandByDefault = false;
    }
    
    // 如果启用自动保存，保存设置
    if (autoSaveSettings) {
        saveApplicationSettings();
    }
    
    // 调整窗口大小以适应新布局
    adjustSize();
}

QString MainWindow::getLogFilePath()
{
    // 使用设置中的日志存储路径，如果为空则使用可执行程序目录
    QString logDir = logStoragePath;
    if (logDir.isEmpty()) {
        logDir = QApplication::applicationDirPath();
    }
    
    // 确保目录存在
    QDir dir(logDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QString logFile = logDir + "/upload_log.txt";
    return logFile;
}

void MainWindow::writeLogToFile(const QString &message)
{
    QString filePath = getLogFilePath();
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8"); // 设置UTF-8编码以支持中文
        
        // 添加日期信息（仅在每天第一次写入时）
        static QString lastDate;
        QString currentDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");
        if (lastDate != currentDate) {
            stream << "\n========== " << currentDate << " ==========\n";
            lastDate = currentDate;
        }
        
        stream << message << "\n";
        file.close();
    }
}

QString MainWindow::calculateFileMD5(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    
    QCryptographicHash hash(QCryptographicHash::Md5);
    if (hash.addData(&file)) {
        return hash.result().toHex();
    }
    
    return QString();
}

void MainWindow::startFileVerification()
{
    if (verifyProcess) {
        verifyProcess->kill();
        verifyProcess->waitForFinished(1000);
        verifyProcess->deleteLater();
        verifyProcess = nullptr;
    }
    
    QString ip = ipLineEdit->text().trimmed();
    int port = portSpinBox->value();
    QString username = usernameLineEdit->text().trimmed();
    QString remotePath = remoteDirectory.trimmed();
    
    // 确保远程路径以/结尾
    if (!remotePath.endsWith('/')) {
        remotePath += '/';
    }
    
    // 构建远程文件路径
    QString remoteFilePath = remotePath + QFileInfo(selectedFilePath).fileName();
    
    // 创建SSH进程来计算远程文件MD5
    verifyProcess = new QProcess(this);
    
    connect(verifyProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::onVerifyFileFinished);
    
    // 设置环境变量
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    verifyProcess->setProcessEnvironment(env);
    
    // 构建SSH命令来计算远程文件MD5
    QString program = "ssh";
    QStringList arguments;
    arguments << "-o" << "ConnectTimeout=30"
              << "-o" << "StrictHostKeyChecking=no"
              << "-o" << "UserKnownHostsFile=/dev/null"
              << "-o" << "PreferredAuthentications=publickey,password"
              << "-o" << "PubkeyAuthentication=yes"
              << "-o" << "PasswordAuthentication=yes"
              << "-o" << "BatchMode=yes"  // 非交互模式
              << "-p" << QString::number(port)
              << QString("%1@%2").arg(username).arg(ip)
              << QString("md5sum '%1'").arg(remoteFilePath);
    
    logMessage(QString("执行远程MD5计算: md5sum '%1'").arg(remoteFilePath));
    
    // 启动进程
    verifyProcess->start(program, arguments);
}

void MainWindow::onVerifyFileFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    timeoutTimer->stop();
    progressTimer->stop();
    uploadButton->setEnabled(true);
    upgradeQtButton->setEnabled(true);
    upgrade7evButton->setEnabled(true);
    upgradeKu5pButton->setEnabled(true);
    cancelButton->setVisible(false);
    transferProgressBar->setVisible(false);  // 隐藏传输进度条
    
    if (verifyProcess) {
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QString output = verifyProcess->readAllStandardOutput().trimmed();
            
            if (!output.isEmpty()) {
                // md5sum的输出格式: "MD5值 文件名"
                QStringList parts = output.split(" ", QString::SkipEmptyParts);
                if (!parts.isEmpty()) {
                    QString remoteMD5 = parts[0].toLower();
                    QString localMD5Lower = localFileMD5.toLower();
                    
                    logMessage(QString("远程文件MD5: %1").arg(remoteMD5));
                    logMessage(QString("本地文件MD5: %1").arg(localMD5Lower));
                    
                    if (remoteMD5 == localMD5Lower) {
                        logMessage("[成功] 文件校验通过，上传完整无误！");
                        statusLabel->setText("上传并校验成功");
                        statusBar()->showMessage("上传并校验成功", 3000);
                        
                        QMessageBox::information(this, "上传成功", 
                            QString("文件 %1 已成功上传到服务器并通过MD5校验\n"
                                   "目标路径: %2\n"
                                   "本地MD5: %3\n"
                                   "远程MD5: %4")
                            .arg(QFileInfo(selectedFilePath).fileName())
                            .arg(remoteDirectory)
                            .arg(localMD5Lower)
                            .arg(remoteMD5));
                    } else {
                        logMessage("[错误] MD5校验失败！文件可能损坏或不完整");
                        statusLabel->setText("MD5校验失败");
                        statusBar()->showMessage("校验失败", 3000);
                        
                        QMessageBox::warning(this, "校验失败", 
                            QString("文件上传成功但MD5校验失败！\n"
                                   "本地MD5: %1\n"
                                   "远程MD5: %2\n"
                                   "建议重新上传文件")
                            .arg(localMD5Lower)
                            .arg(remoteMD5));
                    }
                } else {
                    logMessage("[错误] 无法解析远程MD5值");
                    statusLabel->setText("校验失败");
                    statusBar()->showMessage("校验失败", 3000);
                    
                    QMessageBox::warning(this, "校验失败", 
                        "无法获取远程文件MD5值，但文件已上传成功");
                }
            } else {
                logMessage("[错误] 远程MD5计算无输出");
                statusBar()->showMessage("校验失败", 3000);
                
                QMessageBox::warning(this, "校验失败", 
                    "无法计算远程文件MD5值，但文件已上传成功");
            }
        } else {
            QString error = verifyProcess->readAllStandardError();
            logMessage(QString("[错误] 远程MD5计算失败 (退出码: %1)").arg(exitCode));
            
            if (!error.isEmpty()) {
                logMessage(QString("[错误信息] %1").arg(error.trimmed()));
            }
            
            statusBar()->showMessage("校验失败", 3000);
            
            QMessageBox::warning(this, "校验失败", 
                QString("无法验证文件完整性，但文件已上传成功\n错误: %1")
                .arg(error.isEmpty() ? "远程命令执行失败" : error.trimmed()));
        }
        
        verifyProcess->deleteLater();
        verifyProcess = nullptr;
    }
}

QString MainWindow::getMachineCode()
{
    QString machineCode;
    
    // 获取CPU信息
    QString cpuInfo = QSysInfo::currentCpuArchitecture();
    
    // 获取机器名
    QString machineName = QSysInfo::machineHostName();
    
    // 获取MAC地址
    QString macAddress;
    foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces()) {
        if (!(interface.flags() & QNetworkInterface::IsLoopBack)) {
            macAddress = interface.hardwareAddress();
            if (!macAddress.isEmpty()) {
                break;
            }
        }
    }
    
    // 组合机器码信息
    QString combinedInfo = QString("%1-%2-%3")
                          .arg(cpuInfo)
                          .arg(machineName)
                          .arg(macAddress);
    
    // 计算MD5哈希作为机器码
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(combinedInfo.toUtf8());
    machineCode = hash.result().toHex().toUpper();
    
    return machineCode;
}

bool MainWindow::checkMachineAuthorization()
{
    QString currentMachineCode = getMachineCode();
    QString authFile = getAuthorizationFilePath();
    
    // 如果授权文件不存在，拒绝访问
    if (!QFile::exists(authFile)) {
        logMessage(QString("授权文件不存在: %1").arg(authFile));
        logMessage(QString("当前机器码: %1").arg(currentMachineCode));
        logMessage("请使用授权工具生成授权文件后再运行此软件。");
        return false;
    }
    
    // 读取授权文件中的机器码
    QFile file(authFile);
    if (!file.open(QIODevice::ReadOnly)) {
        logMessage(QString("无法读取授权文件: %1").arg(authFile));
        return false;
    }
    
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QString authorizedMachineCode = stream.readAll().trimmed();
    file.close();
    
    // 验证机器码是否匹配
    bool isAuthorized = (currentMachineCode == authorizedMachineCode);
    
    if (isAuthorized) {
        logMessage(QString("机器授权验证成功，机器码: %1").arg(currentMachineCode));
    } else {
        logMessage(QString("机器授权验证失败！"));
        logMessage(QString("当前机器码: %1").arg(currentMachineCode));
        logMessage(QString("授权机器码: %1").arg(authorizedMachineCode));
        logMessage("请使用正确的授权文件或重新生成授权文件。");
    }
    
    return isAuthorized;
}

QString MainWindow::getAuthorizationFilePath()
{
    // 获取可执行程序所在目录
    QString appDir = QApplication::applicationDirPath();
    QString authFile = appDir + "/machine_auth.key";
    return authFile;
}

void MainWindow::onShowMachineCode()
{
    QString machineCode = getMachineCode();
    QString authFile = getAuthorizationFilePath();
    
    QMessageBox::information(this, "机器码信息", 
        QString("当前机器码：%1\n\n"
               "授权文件路径：%2\n\n"
               "注意：机器码用于软件授权验证，请妥善保管。")
        .arg(machineCode)
        .arg(authFile));
    
    logMessage(QString("用户查看机器码：%1").arg(machineCode));
}

void MainWindow::onUpgradeQtSoftware()
{
    if (!validateSettings()) {
        return;
    }
    
    // 检查是否有进程正在运行
    if (uploadProcess && uploadProcess->state() != QProcess::NotRunning) {
        QMessageBox::warning(this, "操作进行中", "请等待当前操作完成后再执行升级操作！");
        return;
    }
    
    if (remoteCommandProcess && remoteCommandProcess->state() != QProcess::NotRunning) {
        QMessageBox::warning(this, "操作进行中", "远程命令正在执行中，请稍等...");
        return;
    }
    
    // 构建源文件路径
    QString sourceDir = remoteDirectory.trimmed();
    if (!sourceDir.endsWith('/')) {
        sourceDir += '/';
    }
    QString sourceFile = sourceDir + "qt_update.tar.gz";
    
    // 确认对话框
    int ret = QMessageBox::question(this, "确认升级", 
        QString("即将在远程服务器上执行qt软件升级操作：\n\n"
        "工作目录：%1\n"
        "执行命令：tar -xzvf qt_update.tar.gz -C %2 && sync\n\n"
        "说明：\n"
        "1. 从 %3 解压qt_update.tar.gz到%2目录\n"
        "2. 执行sync命令同步数据到磁盘\n\n"
        "注意：此操作将解压并覆盖目标目录中的文件，请确认无误后继续。\n\n"
        "是否继续执行升级操作？")
        .arg(sourceDir).arg(qtExtractPath).arg(sourceFile),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    
    if (ret != QMessageBox::Yes) {
        logMessage("用户取消了qt软件升级操作");
        return;
    }
    
    logMessage("开始执行qt软件升级操作...");
    logMessage("操作步骤：1. 解压qt软件包  2. 同步数据到磁盘");
    statusLabel->setText("正在升级qt软件");
    transferProgressBar->setVisible(true);
    
    // 禁用所有操作按钮
    disableAllOperationButtons();
    
    // 执行远程命令（升级后自动同步磁盘）
    QString qtCommand = QString("tar -xzvf qt_update.tar.gz -C %1 && sync").arg(qtExtractPath);
    executeRemoteCommand(qtCommand, sourceDir.trimmed());
}

void MainWindow::onUpgrade7evFirmware()
{
    if (!validateSettings()) {
        return;
    }
    
    // 检查是否有进程正在运行
    if (uploadProcess && uploadProcess->state() != QProcess::NotRunning) {
        QMessageBox::warning(this, "操作进行中", "请等待当前操作完成后再执行7ev固件升级操作！");
        return;
    }
    
    if (remoteCommandProcess && remoteCommandProcess->state() != QProcess::NotRunning) {
        QMessageBox::warning(this, "操作进行中", "远程命令正在执行中，请稍等...");
        return;
    }
    
    if (preCheck7evProcess && preCheck7evProcess->state() != QProcess::NotRunning) {
        QMessageBox::warning(this, "操作进行中", "7ev固件升级预检查正在执行中，请稍等...");
        return;
    }
    
    if (upgrade7evProcess && upgrade7evProcess->state() != QProcess::NotRunning) {
        QMessageBox::warning(this, "操作进行中", "7ev固件升级正在执行中，请稍等...");
        return;
    }
    
    // 构建源文件路径
    QString sourceDir = remoteDirectory.trimmed();
    if (!sourceDir.endsWith('/')) {
        sourceDir += '/';
    }
    QString sourceFile = sourceDir + "boots.tar.gz";
    
    // 确认对话框
    int ret = QMessageBox::question(this, "确认7ev固件升级", 
        QString("即将在远程服务器上执行7ev固件升级操作：\n\n"
        "执行步骤：\n"
        "1. 检查并处理已有挂载状态\n"
        "2. 挂载 /dev/mmcblk0p1 到 %1\n"
        "3. 检查 %2 文件是否存在\n"
        "4. 如果存在，解压到 %1（忽略权限问题）\n"
        "5. 验证解压结果\n"
        "6. 执行 sync 同步数据到磁盘\n\n"
        "注意：此操作将替换系统固件文件，请确认：\n"
        "• 已备份重要数据\n"
        "• boots.tar.gz 文件完整有效\n"
        "• 升级过程中不要断电\n\n"
        "是否继续执行7ev固件升级操作？").arg(sevEvExtractPath).arg(sourceFile),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    
    if (ret != QMessageBox::Yes) {
        logMessage("用户取消了7ev固件升级操作");
        return;
    }
    
    logMessage("开始执行7ev固件升级操作...");
    logMessage("执行前检查：验证系统环境和文件完整性");
    logMessage("步骤1：检查并处理已有挂载状态");
    logMessage("步骤2：挂载分区 /dev/mmcblk0p1 到 /mnt/mmcblk0p1");
    logMessage(QString("步骤3：检查固件文件 %1").arg(sourceFile));
    logMessage("步骤4：解压固件到目标分区");
    logMessage("步骤5：验证解压结果");
    logMessage("步骤6：同步数据到磁盘并卸载分区");
    
    statusLabel->setText("正在执行7ev固件升级");
    transferProgressBar->setVisible(true);
    
    // 禁用所有操作按钮
    disableAllOperationButtons();
    
    // 先执行预检查
    executePreCheck7ev();
}

void MainWindow::executePreCheck7ev()
{
    logMessage("[预检查] 正在验证升级环境...");
    
    // 构建源文件路径
    QString sourceDir = remoteDirectory.trimmed();
    if (!sourceDir.endsWith('/')) {
        sourceDir += '/';
    }
    QString sourceFile = sourceDir + "boots.tar.gz";
    
    // 构建预检查命令
    QString preCheckCommand = 
        "echo 'Pre-check: Verifying system environment...' && "
        "echo 'Checking device /dev/mmcblk0p1...' && "
        "ls -la /dev/mmcblk0p1 && "
        "echo 'Checking firmware directory...' && "
        "ls -la " + sourceDir + " && "
        "echo 'Checking firmware file...' && "
        "if [ -f " + sourceFile + " ]; then "
        "  ls -la " + sourceFile + " && "
        "  echo 'File size:' && du -h " + sourceFile + " && "
        "  echo 'Testing file integrity...' && "
        "  tar -tzf " + sourceFile + " > /dev/null && "
        "  echo 'Testing extraction compatibility...' && "
        "  tar -tf " + sourceFile + " | head -5 && "
        "  echo 'Firmware file verification passed'; "
        "else "
        "  echo 'ERROR: Firmware file not found'; exit 1; "
        "fi && "
        "echo 'Checking available space...' && "
        "df -h " + sourceDir + " && "
        "echo 'Pre-check completed successfully'";
    
    executePreCheck7evCommand(preCheckCommand);
}

void MainWindow::executePreCheck7evCommand(const QString &command)
{
    if (preCheck7evProcess) {
        preCheck7evProcess->kill();
        preCheck7evProcess->waitForFinished(1000);
        preCheck7evProcess->deleteLater();
        preCheck7evProcess = nullptr;
    }
    
    preCheck7evProcess = new QProcess(this);
    
    // 连接信号
    connect(preCheck7evProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
                
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QString output = preCheck7evProcess->readAllStandardOutput();
            logMessage("[预检查] 系统环境验证通过，开始正式升级...");
            if (!output.isEmpty()) {
                logMessage(QString("[预检查输出] %1").arg(output.trimmed()));
            }
            
            // 预检查通过，执行正式升级
            executeActual7evUpgrade();
        } else {
            QString error = preCheck7evProcess->readAllStandardError();
            QString output = preCheck7evProcess->readAllStandardOutput();
            
            logMessage("[错误] 7ev固件升级预检查失败");
            if (!error.isEmpty()) {
                logMessage(QString("[预检查错误] %1").arg(error.trimmed()));
            }
            if (!output.isEmpty()) {
                logMessage(QString("[预检查输出] %1").arg(output.trimmed()));
            }
            
            transferProgressBar->setVisible(false);
            statusLabel->setText("预检查失败");
            
            // 恢复所有操作按钮
            enableAllOperationButtons();
            statusBar()->showMessage("升级预检查失败", 3000);
            
            QMessageBox::warning(this, "预检查失败", 
                "7ev固件升级预检查失败！\n\n"
                "在执行实际升级前发现以下问题，升级已中止：\n\n" + 
                (output.isEmpty() ? "无法获取详细信息" : output.trimmed()) + "\n\n"
                "请解决以上问题后重新尝试升级。");
        }
        
        if (preCheck7evProcess) {
            preCheck7evProcess->deleteLater();
            preCheck7evProcess = nullptr;
        }
    });
    
    // 连接输出信号
    connect(preCheck7evProcess, &QProcess::readyReadStandardOutput, 
            this, [this]() {
        QString output = preCheck7evProcess->readAllStandardOutput();
        if (!output.isEmpty()) {
            logMessage(QString("[预检查] %1").arg(output.trimmed()));
        }
    });
    
    connect(preCheck7evProcess, &QProcess::readyReadStandardError, 
            this, [this]() {
        QString error = preCheck7evProcess->readAllStandardError();
        if (!error.isEmpty()) {
            logMessage(QString("[预检查警告] %1").arg(error.trimmed()));
        }
    });
    
    // 设置进程环境变量
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    preCheck7evProcess->setProcessEnvironment(env);
    
    // 构建SSH命令
    QString program = "ssh";
    QStringList arguments;
    arguments << "-o" << "ConnectTimeout=30"
              << "-o" << "StrictHostKeyChecking=no"
              << "-o" << "UserKnownHostsFile=/dev/null"
              << "-o" << "PreferredAuthentications=publickey,password"
              << "-o" << "PubkeyAuthentication=yes"
              << "-o" << "PasswordAuthentication=yes"
              << "-o" << "BatchMode=yes"  // 非交互模式
              << "-p" << QString::number(portSpinBox->value())
              << QString("%1@%2").arg(usernameLineEdit->text()).arg(ipLineEdit->text())
              << command;
    
    logMessage("开始执行7ev固件升级预检查...");
    
    // 启动进程
    preCheck7evProcess->start(program, arguments);
    
            if (!preCheck7evProcess->waitForStarted(5000)) {
        logMessage("[错误] 无法启动SSH进程进行预检查");
        logMessage("[提示] 请确保SSH密钥认证已正确配置");
        transferProgressBar->setVisible(false);
        statusLabel->setText("预检查失败");
        
        // 恢复所有操作按钮
        enableAllOperationButtons();
        
        QMessageBox::critical(this, "预检查失败", 
            "无法启动SSH进程进行预检查。\n建议配置SSH密钥认证后重试。");
        
        if (preCheck7evProcess) {
            preCheck7evProcess->deleteLater();
            preCheck7evProcess = nullptr;
        }
    }
}

void MainWindow::executeActual7evUpgrade()
{
    logMessage("[正式升级] 预检查通过，开始执行7ev固件升级...");
    
    // 构建源文件路径
    QString sourceDir = remoteDirectory.trimmed();
    if (!sourceDir.endsWith('/')) {
        sourceDir += '/';
    }
    QString sourceFile = sourceDir + "boots.tar.gz";
    
    // 构建复合命令：检查挂载状态->挂载->检查文件->解压->同步
    QString command = 
        QString("echo 'Step 1: Creating mount point...' && "
        "mkdir -p %1 && "                                               // 确保挂载点存在
        "echo 'Step 2: Checking device /dev/mmcblk0p1...' && "
        "ls -la /dev/mmcblk0p1 && "                                     // 检查设备是否存在
        "echo 'Step 3: Checking mount status...' && "
        "if mountpoint -q %1; then "                                   // 检查挂载点是否已挂载
        "  echo 'Mount point already in use, unmounting...'; "
        "  umount %1; "                                                 // 如果已挂载则卸载
        "fi && "
        "if mount | grep -q /dev/mmcblk0p1; then "                     // 检查设备是否在其他地方被挂载
        "  echo 'Device mounted elsewhere, unmounting...'; "
        "  umount /dev/mmcblk0p1; "                                     // 卸载设备
        "fi && "
        "echo 'Step 4: Mounting partition...' && "
        "mount /dev/mmcblk0p1 %1 && "                                  // 挂载分区
        "echo 'Mount successful, checking mount point...' && "
        "df -h %1 && "                                                  // 显示挂载信息
        "echo 'Step 5: Checking firmware file...' && "
        "ls -la %2 && "                                                 // 列出目录内容
        "if [ ! -f %3 ]; then "                                        // 检查文件是否存在
        "  echo 'ERROR: boots.tar.gz not found in %2'; "
        "  echo 'Directory contents:'; ls -la %2; "
        "  umount %1; "                                                 // 如果文件不存在，卸载并退出
        "  exit 1; "
        "fi && "
        "echo 'Found boots.tar.gz, file info:' && "
        "ls -la %3 && "                                                 // 显示文件信息
        "echo 'Step 6: Starting extraction...' && "
        "tar -xzvf %3 -C %1 --no-same-owner --no-same-permissions && " // 解压到目标分区，忽略所有权和权限
        "echo 'Step 7: Verifying extracted files...' && "
        "ls -la %1/ && "                                               // 显示解压后的文件
        "echo 'Step 8: Syncing data...' && "
        "sync && "                                                      // 同步数据
        "echo 'Step 9: Unmounting partition...' && "
        "umount %1 && "                                                // 卸载分区
        "echo '7ev firmware upgrade completed successfully'")           // 完成提示
        .arg(sevEvExtractPath).arg(sourceDir).arg(sourceFile);
    
    execute7evRemoteCommand(command);
}

void MainWindow::execute7evRemoteCommand(const QString &command)
{
    if (upgrade7evProcess) {
        upgrade7evProcess->kill();
        upgrade7evProcess->waitForFinished(1000);
        upgrade7evProcess->deleteLater();
        upgrade7evProcess = nullptr;
    }
    
    upgrade7evProcess = new QProcess(this);
    
    // 连接信号
    connect(upgrade7evProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
                
        transferProgressBar->setVisible(false);
        
        // 恢复所有操作按钮
        enableAllOperationButtons();
        
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QString output = upgrade7evProcess->readAllStandardOutput();
            QString error = upgrade7evProcess->readAllStandardError();
            
            logMessage("[成功] 7ev固件升级操作执行完成！");
            if (!output.isEmpty()) {
                logMessage(QString("[输出] %1").arg(output.trimmed()));
            }
            if (!error.isEmpty()) {
                logMessage(QString("[信息] %1").arg(error.trimmed()));
            }
            
            statusLabel->setText("7ev固件升级完成");
            statusBar()->showMessage("7ev固件升级操作成功完成", 3000);
            
            QString sourceDir = remoteDirectory.trimmed();
            if (!sourceDir.endsWith('/')) {
                sourceDir += '/';
            }
            QString sourceFile = sourceDir + "boots.tar.gz";
            
            QMessageBox::information(this, "升级成功", 
                QString("7ev固件升级操作已成功完成！\n\n"
                "完成的操作：\n"
                "1. ✓ 检查并处理已有挂载状态\n"
                "2. ✓ 挂载分区 /dev/mmcblk0p1 到 %1\n"
                "3. ✓ 检查固件文件 %2\n"
                "4. ✓ 解压固件到目标分区（忽略权限问题）\n"
                "5. ✓ 验证解压结果\n"
                "6. ✓ 同步数据到磁盘并卸载分区\n\n"
                "固件升级详情请查看操作日志。\n"
                "建议重启设备以应用新固件。").arg(sevEvExtractPath).arg(sourceFile));
        } else {
            QString error = upgrade7evProcess->readAllStandardError();
            QString output = upgrade7evProcess->readAllStandardOutput();
            
            // 详细分析错误原因
            QString errorDetails = "";
            QString solutionDetails = "";
            
            if (output.contains("boots.tar.gz not found") || error.contains("boots.tar.gz not found")) {
                QString sourceDir = remoteDirectory.trimmed();
                if (!sourceDir.endsWith('/')) {
                    sourceDir += '/';
                }
                QString sourceFile = sourceDir + "boots.tar.gz";
                
                logMessage(QString("[错误] 7ev固件升级失败：%1 文件不存在").arg(sourceFile));
                statusLabel->setText("固件文件不存在");
                statusBar()->showMessage("固件文件不存在", 3000);
                errorDetails = QString("在 %1 目录下找不到 boots.tar.gz 固件文件").arg(sourceDir);
                solutionDetails = QString("1. 请先上传 boots.tar.gz 固件文件到 %1 目录\n"
                                "2. 确认文件名为 boots.tar.gz（区分大小写）\n"
                                "3. 确认文件完整且未损坏\n"
                                "4. 重新执行升级操作").arg(sourceDir);
                
                QMessageBox::critical(this, "固件文件不存在", 
                    QString("7ev固件升级失败！\n\n错误原因：%1\n\n解决方案：\n%2")
                    .arg(errorDetails).arg(solutionDetails));
            } else if (output.contains("No such file or directory") && output.contains("/dev/mmcblk0p1")) {
                logMessage("[错误] 7ev固件升级失败：设备 /dev/mmcblk0p1 不存在");
                statusLabel->setText("存储设备不存在");
                statusBar()->showMessage("存储设备不存在", 3000);
                errorDetails = "设备 /dev/mmcblk0p1 不存在，可能是存储设备未连接或驱动问题";
                solutionDetails = "1. 检查存储设备是否正确连接\n"
                                "2. 确认设备路径是否正确\n"
                                "3. 检查系统是否识别到存储设备\n"
                                "4. 可能需要重启设备或重新插拔存储设备";
                
                QMessageBox::critical(this, "存储设备不存在", 
                    QString("7ev固件升级失败！\n\n错误原因：%1\n\n解决方案：\n%2")
                    .arg(errorDetails).arg(solutionDetails));
            } else if (output.contains("mount:") || error.contains("mount:")) {
                // 检查是否是设备忙碌错误
                if (output.contains("Device or resource busy") || error.contains("Device or resource busy")) {
                    logMessage("[错误] 7ev固件升级失败：设备正在使用中");
                    statusLabel->setText("设备使用中");
                    statusBar()->showMessage("设备使用中", 3000);
                    errorDetails = "设备 /dev/mmcblk0p1 正在使用中，无法挂载";
                    solutionDetails = "1. 设备可能已经被其他程序挂载\n"
                                    "2. 检查是否有其他升级操作正在进行\n"
                                    "3. 尝试手动卸载：umount /dev/mmcblk0p1\n"
                                    "4. 重启设备后重新尝试升级\n"
                                    "5. 检查是否有程序正在访问该分区";
                } else {
                    logMessage("[错误] 7ev固件升级失败：分区挂载失败");
                    statusLabel->setText("分区挂载失败");
                    statusBar()->showMessage("分区挂载失败", 3000);
                    errorDetails = "无法挂载分区 /dev/mmcblk0p1，可能是分区损坏或权限不足";
                    solutionDetails = "1. 检查分区是否已经被挂载\n"
                                    "2. 确认当前用户是否有挂载权限\n"
                                    "3. 检查分区文件系统是否正常\n"
                                    "4. 尝试手动卸载后重新挂载";
                }
                
                QMessageBox::critical(this, "挂载失败", 
                    QString("7ev固件升级失败！\n\n错误原因：%1\n\n解决方案：\n%2")
                    .arg(errorDetails).arg(solutionDetails));
            } else if (output.contains("tar:") || error.contains("tar:")) {
                // 检查是否是权限相关错误
                if (output.contains("Cannot change ownership") || error.contains("Cannot change ownership") ||
                    output.contains("Operation not permitted") || error.contains("Operation not permitted")) {
                    logMessage("[错误] 7ev固件升级失败：解压时权限问题");
                    statusLabel->setText("解压权限错误");
                    statusBar()->showMessage("解压权限错误", 3000);
                    errorDetails = "tar解压时无法设置文件所有权，这通常是文件系统权限限制";
                    solutionDetails = "1. 这是正常现象，固件文件已解压但权限可能不同\n"
                                    "2. 检查目标分区中的文件是否已正确解压\n"
                                    "3. 如果文件存在且完整，升级可能已成功\n"
                                    "4. 可以忽略所有权警告，重点检查文件内容";
                } else {
                    logMessage("[错误] 7ev固件升级失败：压缩包解压失败");
                    statusLabel->setText("解压失败");
                    statusBar()->showMessage("解压失败", 3000);
                    errorDetails = "boots.tar.gz 文件解压失败，可能是文件损坏或格式错误";
                    solutionDetails = "1. 检查 boots.tar.gz 文件是否完整\n"
                                    "2. 确认文件是否为有效的 tar.gz 格式\n"
                                    "3. 检查目标分区是否有足够空间\n"
                                    "4. 重新下载或重新生成固件文件";
                }
                
                QMessageBox::critical(this, "解压问题", 
                    QString("7ev固件升级遇到解压问题！\n\n错误原因：%1\n\n解决方案：\n%2")
                    .arg(errorDetails).arg(solutionDetails));
            } else if (output.contains("Input/output error") || error.contains("Input/output error") ||
                       output.contains("I/O error") || error.contains("I/O error")) {
                logMessage("[错误] 7ev固件升级失败：硬件I/O错误，可能是存储设备损坏或断电");
                statusLabel->setText("硬件I/O错误");
                statusBar()->showMessage("硬件I/O错误", 3000);
                errorDetails = "检测到硬件输入/输出错误，通常是存储设备损坏、断电或连接问题导致";
                solutionDetails = "1. 检查设备是否正常供电\n"
                                "2. 检查存储设备连接是否牢固\n"
                                "3. 重启设备后重新尝试\n"
                                "4. 如果问题持续，可能需要更换存储设备\n"
                                "5. 联系技术支持进行硬件检测";
                
                QMessageBox::critical(this, "硬件I/O错误", 
                    QString("7ev固件升级遇到严重硬件错误！\n\n错误原因：%1\n\n紧急处理方案：\n%2\n\n"
                           "⚠️ 警告：此错误可能导致设备损坏，请立即停止操作并联系技术支持！")
                    .arg(errorDetails).arg(solutionDetails));
            } else {
                logMessage(QString("[错误] 7ev固件升级失败 (退出码: %1)").arg(exitCode));
                
                if (!error.isEmpty()) {
                    logMessage(QString("[错误信息] %1").arg(error.trimmed()));
                }
                if (!output.isEmpty()) {
                    logMessage(QString("[输出信息] %1").arg(output.trimmed()));
                }
                
                statusLabel->setText("7ev固件升级失败");
                statusBar()->showMessage("升级操作失败", 3000);
                
                QString sourceDir = remoteDirectory.trimmed();
                if (!sourceDir.endsWith('/')) {
                    sourceDir += '/';
                }
                
                QMessageBox::warning(this, "升级失败", 
                    QString("7ev固件升级操作执行失败！\n\n"
                           "错误信息：%1\n\n"
                           "请检查：\n"
                           "1. 服务器连接是否正常\n"
                           "2. boots.tar.gz 文件是否存在于 %2 目录\n"
                           "3. 分区 /dev/mmcblk0p1 是否可用\n"
                           "4. 目标目录权限是否足够\n"
                           "5. 网络连接是否稳定")
                    .arg(error.isEmpty() ? "命令执行失败" : error.trimmed())
                    .arg(sourceDir));
            }
        }
        
        if (upgrade7evProcess) {
            upgrade7evProcess->deleteLater();
            upgrade7evProcess = nullptr;
        }
    });
    
    // 连接输出信号，实时显示命令执行过程
    connect(upgrade7evProcess, &QProcess::readyReadStandardOutput, 
            this, [this]() {
        QString output = upgrade7evProcess->readAllStandardOutput();
        if (!output.isEmpty()) {
            logMessage(QString("[7ev升级] %1").arg(output.trimmed()));
        }
    });
    
    connect(upgrade7evProcess, &QProcess::readyReadStandardError, 
            this, [this]() {
        QString error = upgrade7evProcess->readAllStandardError();
        if (!error.isEmpty()) {
            logMessage(QString("[7ev信息] %1").arg(error.trimmed()));
            
            // 检测严重的硬件错误
            if (error.contains("Input/output error") || 
                error.contains("I/O error") ||
                error.contains("Device or resource busy") ||
                error.contains("No such device") ||
                error.contains("Operation not permitted") ||
                error.contains("Permission denied") ||
                error.contains("Read-only file system")) {
                
                logMessage(QString("[严重错误] 检测到硬件I/O错误，立即终止7ev升级操作"));
                logMessage(QString("[错误详情] %1").arg(error.trimmed()));
                
                // 立即终止进程
                if (upgrade7evProcess && upgrade7evProcess->state() == QProcess::Running) {
                    upgrade7evProcess->kill();
                    logMessage("[系统] 已强制终止7ev升级进程");
                }
            }
        }
    });
    
    // 设置进程环境变量
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    upgrade7evProcess->setProcessEnvironment(env);
    
    // 构建SSH命令
    QString program = "ssh";
    QStringList arguments;
    arguments << "-o" << "ConnectTimeout=30"
              << "-o" << "StrictHostKeyChecking=no"
              << "-o" << "UserKnownHostsFile=/dev/null"
              << "-o" << "PreferredAuthentications=publickey,password"
              << "-o" << "PubkeyAuthentication=yes"
              << "-o" << "PasswordAuthentication=yes"
              << "-o" << "BatchMode=yes"  // 非交互模式
              << "-p" << QString::number(portSpinBox->value())
              << QString("%1@%2").arg(usernameLineEdit->text()).arg(ipLineEdit->text())
              << command;
    
    logMessage("开始执行7ev固件升级命令...");
    
    // 启动进程
    upgrade7evProcess->start(program, arguments);
    
    // 设置升级超时检测（15分钟，7ev升级可能需要更长时间）
    QTimer::singleShot(900000, this, [this](){
        if (upgrade7evProcess && upgrade7evProcess->state() == QProcess::Running) {
            logMessage("[警告] 7ev固件升级操作超时（15分钟），可能遇到问题");
            logMessage("[系统] 正在强制终止升级进程...");
            
            upgrade7evProcess->kill();
            upgrade7evProcess->waitForFinished(3000);
            
            transferProgressBar->setVisible(false);
            statusLabel->setText("升级超时");
            statusBar()->showMessage("升级操作超时", 3000);
            
            // 恢复所有操作按钮
            enableAllOperationButtons();
            
            QMessageBox::critical(this, "升级超时", 
                "7ev固件升级操作超时（15分钟）！\n\n"
                "可能的原因：\n"
                "1. 网络连接中断\n"
                "2. 远程设备无响应\n"
                "3. 升级过程中遇到硬件错误\n"
                "4. 存储设备I/O错误\n\n"
                "建议：\n"
                "1. 检查网络连接\n"
                "2. 检查远程设备状态\n"
                "3. 重启设备后重新尝试\n"
                "4. 如果问题持续，请联系技术支持");
            
            if (upgrade7evProcess) {
                upgrade7evProcess->deleteLater();
                upgrade7evProcess = nullptr;
            }
        }
    });
    
    if (!upgrade7evProcess->waitForStarted(5000)) {
        logMessage("[错误] 无法启动SSH进程");
        logMessage("[提示] 请确保SSH密钥认证已正确配置");
        transferProgressBar->setVisible(false);
        statusLabel->setText("命令执行失败");
        
        // 恢复所有操作按钮
        enableAllOperationButtons();
        
        QMessageBox::critical(this, "执行失败", 
            "无法启动SSH进程。\n建议配置SSH密钥认证后重试。");
        
        if (upgrade7evProcess) {
            upgrade7evProcess->deleteLater();
            upgrade7evProcess = nullptr;
        }
    }
}

void MainWindow::executeRemoteCommand(const QString &command, const QString &workingDir)
{
    if (remoteCommandProcess) {
        remoteCommandProcess->deleteLater();
        remoteCommandProcess = nullptr;
    }
    
    remoteCommandProcess = new QProcess(this);
    
    // 连接信号
    connect(remoteCommandProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
                
        transferProgressBar->setVisible(false);
        
        // 恢复所有操作按钮
        enableAllOperationButtons();
        
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QString output = remoteCommandProcess->readAllStandardOutput();
            QString error = remoteCommandProcess->readAllStandardError();
            
            logMessage("[成功] qt软件升级操作执行完成！");
            if (!output.isEmpty()) {
                logMessage(QString("[输出] %1").arg(output.trimmed()));
            }
            if (!error.isEmpty()) {
                logMessage(QString("[信息] %1").arg(error.trimmed()));
            }
            
            statusLabel->setText("qt软件升级完成");
            statusBar()->showMessage("升级操作成功完成", 3000);
            
            QString sourceDir = remoteDirectory.trimmed();
            if (!sourceDir.endsWith('/')) {
                sourceDir += '/';
            }
            QString sourceFile = sourceDir + "qt_update.tar.gz";
            
            QMessageBox::information(this, "升级成功", 
                QString("qt软件升级操作已成功完成！\n\n"
                "完成的操作：\n"
                "1. ✓ 从 %1 解压qt_update.tar.gz到%2目录\n"
                "2. ✓ 执行sync命令同步数据到磁盘\n\n"
                "升级详情请查看操作日志。").arg(sourceFile).arg(qtExtractPath));
        } else {
            QString error = remoteCommandProcess->readAllStandardError();
            logMessage(QString("[错误] qt软件升级失败 (退出码: %1)").arg(exitCode));
            
            if (!error.isEmpty()) {
                logMessage(QString("[错误信息] %1").arg(error.trimmed()));
            }
            
            statusLabel->setText("qt软件升级失败");
            statusBar()->showMessage("升级操作失败", 3000);
            
            QString sourceDir = remoteDirectory.trimmed();
            if (!sourceDir.endsWith('/')) {
                sourceDir += '/';
            }
            
            QMessageBox::warning(this, "升级失败", 
                QString("qt软件升级操作执行失败！\n\n"
                       "错误信息：%1\n\n"
                       "请检查：\n"
                       "1. 服务器连接是否正常\n"
                       "2. qt_update.tar.gz文件是否存在于 %2 目录\n"
                       "3. 目标目录权限是否足够\n"
                       "4. 网络连接是否稳定")
                .arg(error.isEmpty() ? "命令执行失败" : error.trimmed())
                .arg(sourceDir));
        }
        
        if (remoteCommandProcess) {
            remoteCommandProcess->deleteLater();
            remoteCommandProcess = nullptr;
        }
    });
    
    // 连接输出信号，实时显示命令执行过程
    connect(remoteCommandProcess, &QProcess::readyReadStandardOutput, 
            this, [this]() {
        QString output = remoteCommandProcess->readAllStandardOutput();
        if (!output.isEmpty()) {
            logMessage(QString("[命令输出] %1").arg(output.trimmed()));
        }
    });
    
    connect(remoteCommandProcess, &QProcess::readyReadStandardError, 
            this, [this]() {
        QString error = remoteCommandProcess->readAllStandardError();
        if (!error.isEmpty()) {
            logMessage(QString("[命令信息] %1").arg(error.trimmed()));
        }
    });
    
    // 设置进程环境变量
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    remoteCommandProcess->setProcessEnvironment(env);
    
    // 构建SSH命令
    QString program = "ssh";
    QStringList arguments;
    arguments << "-o" << "ConnectTimeout=30"
              << "-o" << "StrictHostKeyChecking=no"
              << "-o" << "UserKnownHostsFile=/dev/null"
              << "-o" << "PreferredAuthentications=publickey,password"
              << "-o" << "PubkeyAuthentication=yes"
              << "-o" << "PasswordAuthentication=yes"
              << "-o" << "BatchMode=yes"  // 非交互模式
              << "-p" << QString::number(portSpinBox->value())
              << QString("%1@%2").arg(usernameLineEdit->text()).arg(ipLineEdit->text());
    
    // 如果指定了工作目录，先切换目录再执行命令
    QString fullCommand;
    if (!workingDir.isEmpty()) {
        fullCommand = QString("cd '%1' && %2").arg(workingDir).arg(command);
    } else {
        fullCommand = command;
    }
    
    arguments << fullCommand;
    
    logMessage(QString("执行远程命令: %1").arg(fullCommand));
    if (!workingDir.isEmpty()) {
        logMessage(QString("工作目录: %1").arg(workingDir));
    }
    
    // 启动进程
    remoteCommandProcess->start(program, arguments);
    
    if (!remoteCommandProcess->waitForStarted(5000)) {
        logMessage("[错误] 无法启动SSH进程");
        logMessage("[提示] 请确保SSH密钥认证已正确配置");
        transferProgressBar->setVisible(false);
        statusLabel->setText("命令执行失败");
        
        // 恢复所有操作按钮
        enableAllOperationButtons();
        
        QMessageBox::critical(this, "执行失败", 
            "无法启动SSH进程。\n建议配置SSH密钥认证后重试。");
        
        if (remoteCommandProcess) {
            remoteCommandProcess->deleteLater();
            remoteCommandProcess = nullptr;
        }
    }
}

void MainWindow::onExecuteCustomCommand()
{
    QString command = commandLineEdit->text().trimmed();
    if (command.isEmpty()) {
        commandOutputEdit->append("<span style='color: #ff6b6b;'>[错误] 请输入要执行的命令</span>");
        return;
    }
    
    if (!validateSettings()) {
        commandOutputEdit->append("<span style='color: #ff6b6b;'>[错误] 请先配置服务器连接信息</span>");
        return;
    }
    
    // 检查是否有进程正在运行
    if (customCommandProcess && customCommandProcess->state() != QProcess::NotRunning) {
        commandOutputEdit->append("<span style='color: #ffa500;'>[警告] 有命令正在执行中，请稍等...</span>");
        return;
    }
    
    executeCustomRemoteCommand(command);
}

void MainWindow::onClearCommandOutput()
{
    commandOutputEdit->clear();
    commandOutputEdit->append("<span style='color: #4ecdc4;'>[系统] 输出已清空</span>");
}

void MainWindow::onCommandInputEnterPressed()
{
    onExecuteCustomCommand();
}

void MainWindow::executeCustomRemoteCommand(const QString &command)
{
    if (customCommandProcess) {
        customCommandProcess->deleteLater();
        customCommandProcess = nullptr;
    }
    
    customCommandProcess = new QProcess(this);
    
    // 在输出区域显示执行的命令
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    commandOutputEdit->append(QString("<span style='color: #74b9ff;'>[%1] $ %2</span>").arg(timestamp).arg(command));
    
    // 连接信号处理实时输出
    connect(customCommandProcess, &QProcess::readyReadStandardOutput, 
            this, [this]() {
        QString output = customCommandProcess->readAllStandardOutput();
        if (!output.isEmpty()) {
            // 处理输出格式，保持原始格式
            output = output.replace("\n", "<br/>").replace(" ", "&nbsp;");
            commandOutputEdit->append(QString("<span style='color: #ffffff;'>%1</span>").arg(output));
        }
        // 自动滚动到底部
        QTextCursor cursor = commandOutputEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        commandOutputEdit->setTextCursor(cursor);
    });
    
    connect(customCommandProcess, &QProcess::readyReadStandardError, 
            this, [this]() {
        QString error = customCommandProcess->readAllStandardError();
        if (!error.isEmpty()) {
            error = error.replace("\n", "<br/>").replace(" ", "&nbsp;");
            commandOutputEdit->append(QString("<span style='color: #ff7675;'>%1</span>").arg(error));
        }
        QTextCursor cursor = commandOutputEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        commandOutputEdit->setTextCursor(cursor);
    });
    
    // 连接完成信号
    connect(customCommandProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, command](int exitCode, QProcess::ExitStatus exitStatus) {
                
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
        
        if (exitStatus == QProcess::NormalExit) {
            if (exitCode == 0) {
                commandOutputEdit->append(QString("<span style='color: #00b894;'>[%1] 命令执行完成 (退出码: %2)</span>")
                                        .arg(timestamp).arg(exitCode));
            } else {
                commandOutputEdit->append(QString("<span style='color: #e17055;'>[%1] 命令执行完成但有错误 (退出码: %2)</span>")
                                        .arg(timestamp).arg(exitCode));
            }
        } else {
            commandOutputEdit->append(QString("<span style='color: #ff6b6b;'>[%1] 命令执行异常终止</span>").arg(timestamp));
        }
        
        commandOutputEdit->append("<span style='color: #74b9ff;'>---</span>");
        
        // 自动滚动到底部
        QTextCursor cursor = commandOutputEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        commandOutputEdit->setTextCursor(cursor);
        
        executeCommandButton->setEnabled(true);
        
        if (customCommandProcess) {
            customCommandProcess->deleteLater();
            customCommandProcess = nullptr;
        }
    });
    
    // 设置进程环境变量
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    customCommandProcess->setProcessEnvironment(env);
    
    // 构建SSH命令
    QString program = "ssh";
    QStringList arguments;
    arguments << "-o" << "ConnectTimeout=30"
              << "-o" << "StrictHostKeyChecking=no"
              << "-o" << "UserKnownHostsFile=/dev/null"
              << "-o" << "PreferredAuthentications=publickey,password"
              << "-o" << "PubkeyAuthentication=yes"
              << "-o" << "PasswordAuthentication=yes"
              << "-o" << "BatchMode=yes"  // 非交互模式
              << "-p" << QString::number(portSpinBox->value())
              << QString("%1@%2").arg(usernameLineEdit->text()).arg(ipLineEdit->text())
              << command;
    
    // 禁用执行按钮防止重复执行
    executeCommandButton->setEnabled(false);
    
    // 启动进程
    customCommandProcess->start(program, arguments);
    
    if (!customCommandProcess->waitForStarted(5000)) {
        commandOutputEdit->append("<span style='color: #ff6b6b;'>[错误] 无法启动SSH进程，建议配置SSH密钥认证</span>");
        executeCommandButton->setEnabled(true);
        
        if (customCommandProcess) {
            customCommandProcess->deleteLater();
            customCommandProcess = nullptr;
        }
    }
}

void MainWindow::onOpenSettings()
{
    if (!settingsDialog) {
        settingsDialog = new SettingsDialog(this);
    }
    
    // 将当前设置加载到对话框
    settingsDialog->setRemoteDirectory(remoteDirectory);
    settingsDialog->setAutoSaveSettings(autoSaveSettings);
    settingsDialog->setShowLogByDefault(showLogByDefault);
    settingsDialog->setDefaultLocalPath(defaultLocalPath);
    settingsDialog->setLogStoragePath(logStoragePath);
    settingsDialog->setAutoCleanLog(autoCleanLog);
    settingsDialog->setLogRetentionDays(logRetentionDays);
    settingsDialog->setQtExtractPath(qtExtractPath);
    settingsDialog->set7evExtractPath(sevEvExtractPath);
    
    logMessage("打开设置对话框");
    logMessage(QString("当前设置 - 自动保存: %1, 显示日志: %2, 自动清理: %3")
              .arg(autoSaveSettings ? "是" : "否")
              .arg(showLogByDefault ? "是" : "否")
              .arg(autoCleanLog ? "是" : "否"));
    
    if (settingsDialog->exec() == QDialog::Accepted) {
        // 用户点击了确定，保存设置
        bool oldAutoSave = autoSaveSettings;
        bool oldShowLog = showLogByDefault;
        bool oldAutoClean = autoCleanLog;
        
        remoteDirectory = settingsDialog->getRemoteDirectory();
        autoSaveSettings = settingsDialog->getAutoSaveSettings();
        showLogByDefault = settingsDialog->getShowLogByDefault();
        defaultLocalPath = settingsDialog->getDefaultLocalPath();
        logStoragePath = settingsDialog->getLogStoragePath();
        autoCleanLog = settingsDialog->getAutoCleanLog();
        logRetentionDays = settingsDialog->getLogRetentionDays();
        qtExtractPath = settingsDialog->getQtExtractPath();
        sevEvExtractPath = settingsDialog->get7evExtractPath();
        
        logMessage("设置已更新");
        logMessage(QString("远程目录: %1").arg(remoteDirectory));
        logMessage(QString("自动保存设置: %1 %2").arg(autoSaveSettings ? "启用" : "禁用")
                  .arg(oldAutoSave != autoSaveSettings ? "(已更改)" : ""));
        logMessage(QString("启动时显示日志: %1 %2").arg(showLogByDefault ? "是" : "否")
                  .arg(oldShowLog != showLogByDefault ? "(已更改)" : ""));
        logMessage(QString("默认文件路径: %1").arg(defaultLocalPath));
        logMessage(QString("日志存储路径: %1").arg(logStoragePath));
        logMessage(QString("Qt软件解压路径: %1").arg(qtExtractPath));
        logMessage(QString("7ev固件解压路径: %1").arg(sevEvExtractPath));
        
        if (autoCleanLog) {
            logMessage(QString("自动清理日志已启用，保留 %1 天 %2")
                      .arg(logRetentionDays)
                      .arg(oldAutoClean != autoCleanLog ? "(已更改)" : ""));
            // 立即执行一次日志清理
            cleanExpiredLogs();
        } else {
            logMessage(QString("自动清理日志已禁用 %1")
                      .arg(oldAutoClean != autoCleanLog ? "(已更改)" : ""));
        }
        
        // 保存应用设置
        saveApplicationSettings();
        
        // 如果启用自动保存，也保存连接设置到文件
        if (autoSaveSettings) {
            saveSettingsToFile();
            logMessage("连接设置已自动保存到文件");
        }
        
        logMessage("所有设置更新完成");
    } else {
        logMessage("用户取消了设置更改");
    }
}

void MainWindow::loadApplicationSettings()
{
    QSettings settings; // 使用默认构造函数，自动使用应用程序信息
    
    // 获取当前可执行程序目录作为默认值
    QString appDir = QApplication::applicationDirPath();
    
    settings.beginGroup("Application");
    autoSaveSettings = settings.value("autoSave", true).toBool();
    showLogByDefault = settings.value("showLogByDefault", false).toBool();
    showCommandByDefault = settings.value("showCommandByDefault", false).toBool();
    showBuiltinCommandByDefault = settings.value("showBuiltinCommandByDefault", false).toBool();
    defaultLocalPath = settings.value("defaultLocalPath", appDir).toString();
    logStoragePath = settings.value("logStoragePath", appDir).toString();
    autoCleanLog = settings.value("autoCleanLog", false).toBool();
    logRetentionDays = settings.value("logRetentionDays", 30).toInt();
    qtExtractPath = settings.value("qtExtractPath", "/mnt/qtfs").toString();
    sevEvExtractPath = settings.value("sevEvExtractPath", "/mnt/mmcblk0p1").toString();
    settings.endGroup();
    
    // 确保日志目录存在
    QDir logDir(logStoragePath);
    if (!logDir.exists()) {
        if (logDir.mkpath(".")) {
            logMessage(QString("创建日志目录: %1").arg(logStoragePath));
        } else {
            logMessage(QString("无法创建日志目录: %1，使用默认目录").arg(logStoragePath));
            logStoragePath = appDir;
        }
    }
    
    logMessage("应用设置已加载");
    logMessage(QString("设置存储位置: %1").arg(settings.fileName()));
    logMessage(QString("组织名称: %1").arg(QApplication::organizationName()));
    logMessage(QString("应用程序名称: %1").arg(QApplication::applicationName()));
    logMessage(QString("日志存储路径: %1").arg(logStoragePath));
    logMessage(QString("默认文件路径: %1").arg(defaultLocalPath));
}

void MainWindow::saveApplicationSettings()
{
    QSettings settings; // 使用默认构造函数，自动使用应用程序信息
    
    settings.beginGroup("Application");
    settings.setValue("autoSave", autoSaveSettings);
    settings.setValue("showLogByDefault", showLogByDefault);
    settings.setValue("showCommandByDefault", showCommandByDefault);
    settings.setValue("showBuiltinCommandByDefault", showBuiltinCommandByDefault);
    settings.setValue("defaultLocalPath", defaultLocalPath);
    settings.setValue("logStoragePath", logStoragePath);
    settings.setValue("autoCleanLog", autoCleanLog);
    settings.setValue("logRetentionDays", logRetentionDays);
    settings.setValue("qtExtractPath", qtExtractPath);
    settings.setValue("sevEvExtractPath", sevEvExtractPath);
    settings.endGroup();
    
    settings.sync();
    
    logMessage("应用设置已保存");
}

void MainWindow::cleanExpiredLogs()
{
    if (!autoCleanLog || logRetentionDays <= 0) {
        return;
    }
    
    QDir logDir(logStoragePath);
    if (!logDir.exists()) {
        return;
    }
    
    // 计算过期日期
    QDateTime expireDate = QDateTime::currentDateTime().addDays(-logRetentionDays);
    
    // 查找所有日志文件（假设以.log结尾）
    QStringList filters;
    filters << "*.log" << "*.txt";
    QFileInfoList logFiles = logDir.entryInfoList(filters, QDir::Files);
    
    int deletedCount = 0;
    qint64 deletedSize = 0;
    
    foreach (const QFileInfo &fileInfo, logFiles) {
        if (fileInfo.lastModified() < expireDate) {
            deletedSize += fileInfo.size();
            if (QFile::remove(fileInfo.absoluteFilePath())) {
                deletedCount++;
            }
        }
    }
    
    if (deletedCount > 0) {
        double sizeInMB = deletedSize / (1024.0 * 1024.0);
        logMessage(QString("日志清理完成，删除了 %1 个过期文件，释放空间 %.2f MB")
                  .arg(deletedCount).arg(sizeInMB));
    } else {
        logMessage("日志清理完成，没有找到过期文件");
    }
}

void MainWindow::onUpgradeKu5p()
{
    if (!validateSettings()) {
        return;
    }
    
    // 检查是否有进程正在运行
    if (uploadProcess && uploadProcess->state() != QProcess::NotRunning) {
        QMessageBox::warning(this, "操作进行中", "请等待当前操作完成后再执行ku5p升级操作！");
        return;
    }
    
    if (remoteCommandProcess && remoteCommandProcess->state() != QProcess::NotRunning) {
        QMessageBox::warning(this, "操作进行中", "远程命令正在执行中，请稍等...");
        return;
    }
    
    if (upgradeKu5pProcess && upgradeKu5pProcess->state() != QProcess::NotRunning) {
        QMessageBox::warning(this, "操作进行中", "ku5p升级正在执行中，请稍等...");
        return;
    }
    
    // 构建源文件路径和目标目录路径
    QString sourceDir = remoteDirectory.trimmed();
    if (!sourceDir.endsWith('/')) {
        sourceDir += '/';
    }
    QString sourceFile = sourceDir + "ku5p_package.tar.gz";
    QString targetDir = sourceDir + "updatepackage";
    
    // 确认对话框
    int ret = QMessageBox::question(this, "确认ku5p升级", 
        QString("即将在远程服务器上执行ku5p升级操作：\n\n"
        "执行步骤：\n"
        "1. 检查 %1 文件是否存在\n"
        "2. 创建目标目录 %2\n"
        "3. 解压 ku5p_package.tar.gz 到 %2 目录\n"
        "4. 进入 %2 目录\n"
        "5. 执行 ./ku5pupgrade ku5p_package.bit 进行升级\n"
        "6. 同步数据到磁盘\n\n"
        "⚠️ 重要警告：\n"
        "• 升级过程中绝对不能断电或重启设备\n"
        "• 升级过程可能需要5-10分钟，请耐心等待\n"
        "• 如遇到I/O错误，系统会自动终止升级\n"
        "• 已备份重要数据\n"
        "• ku5p_package.tar.gz 文件完整有效\n\n"
        "升级过程中会显示擦除进度，请勿中断操作！\n\n"
        "是否继续执行ku5p升级操作？")
        .arg(sourceFile).arg(targetDir),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    
    if (ret != QMessageBox::Yes) {
        logMessage("用户取消了ku5p升级操作");
        return;
    }
    
    logMessage("开始执行ku5p升级操作...");
    logMessage(QString("执行步骤：1. 检查软件包  2. 创建目标目录  3. 解压到%1  4. 执行升级程序").arg(targetDir));
    statusLabel->setText("正在执行ku5p升级");
    transferProgressBar->setVisible(true);
    
    // 禁用所有操作按钮
    disableAllOperationButtons();
    
    // 执行ku5p升级
    executeKu5pUpgrade();
}

void MainWindow::executeKu5pUpgrade()
{
    logMessage("[ku5p升级] 开始执行ku5p升级操作...");
    
    // 构建源文件路径和目标目录路径
    QString sourceDir = remoteDirectory.trimmed();
    if (!sourceDir.endsWith('/')) {
        sourceDir += '/';
    }
    QString sourceFile = sourceDir + "ku5p_package.tar.gz";
    QString targetDir = sourceDir + "updatepackage";
    
    // 构建升级命令
    QString command = 
        QString("echo 'Step 1: Checking ku5p package file...' && "
        "if [ ! -f %1 ]; then "                                               // 检查文件是否存在
        "  echo 'ERROR: ku5p_package.tar.gz not found in %2'; "
        "  echo 'Available files in directory:'; "
        "  ls -la %2; "
        "  exit 1; "
        "fi && "
        "echo 'Found ku5p_package.tar.gz, file info:' && "
        "ls -la %1 && "                                                       // 显示文件信息
        "echo 'Step 2: Creating target directory...' && "
        "mkdir -p %3 && "                                                     // 创建目标目录
        "echo 'Step 3: Extracting ku5p package to %3...' && "
        "cd %3 && "                                                           // 切换到目标目录
        "tar -xzvf %1 && "                                                    // 解压文件到当前目录
        "echo 'Step 4: Verifying extracted files...' && "
        "ls -la %3 && "                                                       // 显示解压后的文件
        "echo 'Step 5: Checking for upgrade script...' && "
        "if [ ! -f ku5pupgrade ]; then "                                      // 检查升级脚本是否存在
        "  echo 'ERROR: ku5pupgrade script not found'; "
        "  ls -la; "
        "  exit 1; "
        "fi && "
        "if [ ! -f ku5p_package.bit ]; then "                                 // 检查bit文件是否存在
        "  echo 'ERROR: ku5p_package.bit not found'; "
        "  ls -la; "
        "  exit 1; "
        "fi && "
        "echo 'Making upgrade script executable...' && "
        "chmod +x ku5pupgrade && "                                            // 设置可执行权限
        "echo 'Step 6: Starting ku5p upgrade...' && "
        "./ku5pupgrade ku5p_package.bit && "                                  // 执行升级
        "echo 'Step 7: Syncing data...' && "
        "sync && "                                                            // 同步数据
        "echo 'ku5p upgrade completed successfully'")                         // 完成提示
        .arg(sourceFile).arg(sourceDir).arg(targetDir);
    
    executeKu5pRemoteCommand(command);
}

void MainWindow::executeKu5pRemoteCommand(const QString &command)
{
    if (upgradeKu5pProcess) {
        upgradeKu5pProcess->kill();
        upgradeKu5pProcess->waitForFinished(1000);
        upgradeKu5pProcess->deleteLater();
        upgradeKu5pProcess = nullptr;
    }
    
    upgradeKu5pProcess = new QProcess(this);
    
    // 连接信号
    connect(upgradeKu5pProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
                
        transferProgressBar->setVisible(false);
        
        // 恢复所有操作按钮
        enableAllOperationButtons();
        
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QString output = upgradeKu5pProcess->readAllStandardOutput();
            QString error = upgradeKu5pProcess->readAllStandardError();
            
            logMessage("[成功] ku5p升级操作执行完成！");
            if (!output.isEmpty()) {
                logMessage(QString("[输出] %1").arg(output.trimmed()));
            }
            if (!error.isEmpty()) {
                logMessage(QString("[信息] %1").arg(error.trimmed()));
            }
            
            statusLabel->setText("ku5p升级完成");
            statusBar()->showMessage("ku5p升级操作成功完成", 3000);
            
            QString sourceDir = remoteDirectory.trimmed();
            if (!sourceDir.endsWith('/')) {
                sourceDir += '/';
            }
            QString targetDir = sourceDir + "updatepackage";
            
            QMessageBox::information(this, "升级成功", 
                QString("ku5p升级操作已成功完成！\n\n"
                "完成的操作：\n"
                "1. ✓ 检查 ku5p_package.tar.gz 文件\n"
                "2. ✓ 创建目标目录 %1\n"
                "3. ✓ 解压 ku5p_package.tar.gz 到 %1 目录\n"
                "4. ✓ 执行 ./ku5pupgrade ku5p_package.bit 升级程序\n"
                "5. ✓ 同步数据到磁盘\n\n"
                "ku5p升级详情请查看操作日志。").arg(targetDir));
        } else {
            QString error = upgradeKu5pProcess->readAllStandardError();
            QString output = upgradeKu5pProcess->readAllStandardOutput();
            
            // 详细分析错误原因
            QString errorDetails = "";
            QString solutionDetails = "";
            
            if (output.contains("ku5p_package.tar.gz not found") || error.contains("ku5p_package.tar.gz not found")) {
                QString sourceDir = remoteDirectory.trimmed();
                if (!sourceDir.endsWith('/')) {
                    sourceDir += '/';
                }
                QString sourceFile = sourceDir + "ku5p_package.tar.gz";
                
                logMessage(QString("[错误] ku5p升级失败：%1 文件不存在").arg(sourceFile));
                statusLabel->setText("ku5p软件包不存在");
                statusBar()->showMessage("ku5p软件包不存在", 3000);
                errorDetails = QString("在 %1 目录下找不到 ku5p_package.tar.gz 软件包文件").arg(sourceDir);
                solutionDetails = QString("1. 请先上传 ku5p_package.tar.gz 软件包文件到 %1 目录\n"
                                "2. 确认文件名为 ku5p_package.tar.gz（区分大小写）\n"
                                "3. 确认文件完整且未损坏\n"
                                "4. 重新执行升级操作").arg(sourceDir);
                
                QMessageBox::critical(this, "软件包文件不存在", 
                    QString("ku5p升级失败！\n\n错误原因：%1\n\n解决方案：\n%2")
                    .arg(errorDetails).arg(solutionDetails));
            } else if (output.contains("ku5pupgrade script not found") || error.contains("ku5pupgrade script not found")) {
                logMessage("[错误] ku5p升级失败：升级脚本 ku5pupgrade 不存在");
                statusLabel->setText("升级脚本不存在");
                statusBar()->showMessage("升级脚本不存在", 3000);
                errorDetails = "解压后的软件包中找不到 ku5pupgrade 升级脚本";
                solutionDetails = "1. 检查 ku5p_package.tar.gz 软件包是否完整\n"
                                "2. 确认软件包中包含 ku5pupgrade 脚本文件\n"
                                "3. 重新生成或下载正确的软件包\n"
                                "4. 重新执行升级操作";
                
                QMessageBox::critical(this, "升级脚本缺失", 
                    QString("ku5p升级失败！\n\n错误原因：%1\n\n解决方案：\n%2")
                    .arg(errorDetails).arg(solutionDetails));
            } else if (output.contains("ku5p_package.bit not found") || error.contains("ku5p_package.bit not found")) {
                logMessage("[错误] ku5p升级失败：bit文件 ku5p_package.bit 不存在");
                statusLabel->setText("bit文件不存在");
                statusBar()->showMessage("bit文件不存在", 3000);
                errorDetails = "解压后的软件包中找不到 ku5p_package.bit 文件";
                solutionDetails = "1. 检查 ku5p_package.tar.gz 软件包是否完整\n"
                                "2. 确认软件包中包含 ku5p_package.bit 文件\n"
                                "3. 重新生成或下载正确的软件包\n"
                                "4. 重新执行升级操作";
                
                QMessageBox::critical(this, "bit文件缺失", 
                    QString("ku5p升级失败！\n\n错误原因：%1\n\n解决方案：\n%2")
                    .arg(errorDetails).arg(solutionDetails));
            } else if (output.contains("Input/output error") || error.contains("Input/output error") ||
                       output.contains("I/O error") || error.contains("I/O error")) {
                logMessage("[错误] ku5p升级失败：硬件I/O错误，可能是存储设备损坏或断电");
                statusLabel->setText("硬件I/O错误");
                statusBar()->showMessage("硬件I/O错误", 3000);
                errorDetails = "检测到硬件输入/输出错误，通常是存储设备损坏、断电或连接问题导致";
                solutionDetails = "1. 检查设备是否正常供电\n"
                                "2. 检查存储设备连接是否牢固\n"
                                "3. 重启设备后重新尝试\n"
                                "4. 如果问题持续，可能需要更换存储设备\n"
                                "5. 联系技术支持进行硬件检测";
                
                QMessageBox::critical(this, "硬件I/O错误", 
                    QString("ku5p升级遇到严重硬件错误！\n\n错误原因：%1\n\n紧急处理方案：\n%2\n\n"
                           "⚠️ 警告：此错误可能导致设备损坏，请立即停止操作并联系技术支持！")
                    .arg(errorDetails).arg(solutionDetails));
            } else if (output.contains("tar:") || error.contains("tar:")) {
                logMessage("[错误] ku5p升级失败：软件包解压失败");
                statusLabel->setText("解压失败");
                statusBar()->showMessage("解压失败", 3000);
                errorDetails = "ku5p_package.tar.gz 文件解压失败，可能是文件损坏或格式错误";
                solutionDetails = "1. 检查 ku5p_package.tar.gz 文件是否完整\n"
                                "2. 确认文件是否为有效的 tar.gz 格式\n"
                                "3. 检查目标目录是否有足够空间\n"
                                "4. 重新下载或重新生成软件包文件";
                
                QMessageBox::critical(this, "解压失败", 
                    QString("ku5p升级失败！\n\n错误原因：%1\n\n解决方案：\n%2")
                    .arg(errorDetails).arg(solutionDetails));
            } else {
                logMessage(QString("[错误] ku5p升级失败 (退出码: %1)").arg(exitCode));
                
                if (!error.isEmpty()) {
                    logMessage(QString("[错误信息] %1").arg(error.trimmed()));
                }
                if (!output.isEmpty()) {
                    logMessage(QString("[输出信息] %1").arg(output.trimmed()));
                }
                
                statusLabel->setText("ku5p升级失败");
                statusBar()->showMessage("升级操作失败", 3000);
                
                QString sourceDir = remoteDirectory.trimmed();
                if (!sourceDir.endsWith('/')) {
                    sourceDir += '/';
                }
                
                QMessageBox::warning(this, "升级失败", 
                    QString("ku5p升级操作执行失败！\n\n"
                           "错误信息：%1\n\n"
                           "请检查：\n"
                           "1. 服务器连接是否正常\n"
                           "2. ku5p_package.tar.gz 文件是否存在于 %2 目录\n"
                           "3. 软件包是否包含完整的升级文件\n"
                           "4. 目标目录权限是否足够\n"
                           "5. 网络连接是否稳定")
                    .arg(error.isEmpty() ? "命令执行失败" : error.trimmed())
                    .arg(sourceDir));
            }
        }
        
        if (upgradeKu5pProcess) {
            upgradeKu5pProcess->deleteLater();
            upgradeKu5pProcess = nullptr;
        }
    });
    
    // 连接输出信号，实时显示命令执行过程
    connect(upgradeKu5pProcess, &QProcess::readyReadStandardOutput, 
            this, [this]() {
        QString output = upgradeKu5pProcess->readAllStandardOutput();
        if (!output.isEmpty()) {
            logMessage(QString("[ku5p升级] %1").arg(output.trimmed()));
            
            // 监控擦除进度
            if (output.contains("Erasing blocks:")) {
                // 提取进度信息，格式如：Erasing blocks: 788/3302 (23%)
                QRegExp progressRegex("Erasing blocks: (\\d+)/(\\d+) \\((\\d+)%\\)");
                if (progressRegex.indexIn(output) != -1) {
                    QString current = progressRegex.cap(1);
                    QString total = progressRegex.cap(2);
                    QString percent = progressRegex.cap(3);
                    
                    statusLabel->setText(QString("ku5p升级中 - 擦除进度: %1/%2 (%3%)")
                                       .arg(current).arg(total).arg(percent));
                    
                    // 每10%记录一次进度
                    int percentInt = percent.toInt();
                    static int lastLoggedPercent = -1;
                    if (percentInt % 10 == 0 && percentInt != lastLoggedPercent) {
                        logMessage(QString("[进度] ku5p擦除进度: %1% (%2/%3)")
                                 .arg(percent).arg(current).arg(total));
                        lastLoggedPercent = percentInt;
                    }
                }
            }
            
            // 监控写入进度
            if (output.contains("Writing data:")) {
                statusLabel->setText("ku5p升级中 - 正在写入数据...");
            }
            
            // 监控验证进度
            if (output.contains("Verifying:")) {
                statusLabel->setText("ku5p升级中 - 正在验证数据...");
            }
        }
    });
    
    connect(upgradeKu5pProcess, &QProcess::readyReadStandardError, 
            this, [this]() {
        QString error = upgradeKu5pProcess->readAllStandardError();
        if (!error.isEmpty()) {
            logMessage(QString("[ku5p信息] %1").arg(error.trimmed()));
            
            // 检测严重的硬件错误
            if (error.contains("Input/output error") || 
                error.contains("I/O error") ||
                error.contains("Device or resource busy") ||
                error.contains("No such device") ||
                error.contains("Operation not permitted") ||
                error.contains("Permission denied") ||
                error.contains("Read-only file system")) {
                
                logMessage(QString("[严重错误] 检测到硬件I/O错误，立即终止升级操作"));
                logMessage(QString("[错误详情] %1").arg(error.trimmed()));
                
                // 立即终止进程
                if (upgradeKu5pProcess && upgradeKu5pProcess->state() == QProcess::Running) {
                    upgradeKu5pProcess->kill();
                    logMessage("[系统] 已强制终止ku5p升级进程");
                }
            }
        }
    });
    
    // 设置进程环境变量
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    upgradeKu5pProcess->setProcessEnvironment(env);
    
    // 构建SSH命令
    QString program = "ssh";
    QStringList arguments;
    arguments << "-o" << "ConnectTimeout=30"
              << "-o" << "StrictHostKeyChecking=no"
              << "-o" << "UserKnownHostsFile=/dev/null"
              << "-o" << "PreferredAuthentications=publickey,password"
              << "-o" << "PubkeyAuthentication=yes"
              << "-o" << "PasswordAuthentication=yes"
              << "-o" << "BatchMode=yes"  // 非交互模式
              << "-p" << QString::number(portSpinBox->value())
              << QString("%1@%2").arg(usernameLineEdit->text()).arg(ipLineEdit->text())
              << command;
    
    logMessage("开始执行ku5p升级命令...");
    
    // 启动进程
    upgradeKu5pProcess->start(program, arguments);
    
    // 设置升级超时检测（10分钟）
    QTimer::singleShot(600000, this, [this](){
        if (upgradeKu5pProcess && upgradeKu5pProcess->state() == QProcess::Running) {
            logMessage("[警告] ku5p升级操作超时（10分钟），可能遇到问题");
            logMessage("[系统] 正在强制终止升级进程...");
            
            upgradeKu5pProcess->kill();
            upgradeKu5pProcess->waitForFinished(3000);
            
            transferProgressBar->setVisible(false);
            statusLabel->setText("升级超时");
            statusBar()->showMessage("升级操作超时", 3000);
            
            // 恢复所有操作按钮
            enableAllOperationButtons();
            
            QMessageBox::critical(this, "升级超时", 
                "ku5p升级操作超时（10分钟）！\n\n"
                "可能的原因：\n"
                "1. 网络连接中断\n"
                "2. 远程设备无响应\n"
                "3. 升级过程中遇到硬件错误\n"
                "4. 存储设备I/O错误\n\n"
                "建议：\n"
                "1. 检查网络连接\n"
                "2. 检查远程设备状态\n"
                "3. 重启设备后重新尝试\n"
                "4. 如果问题持续，请联系技术支持");
            
            if (upgradeKu5pProcess) {
                upgradeKu5pProcess->deleteLater();
                upgradeKu5pProcess = nullptr;
            }
        }
    });
    
    if (!upgradeKu5pProcess->waitForStarted(5000)) {
        logMessage("[错误] 无法启动SSH进程");
        logMessage("[提示] 请确保SSH密钥认证已正确配置");
        transferProgressBar->setVisible(false);
        statusLabel->setText("命令执行失败");
        
        // 恢复所有操作按钮
        enableAllOperationButtons();
        
        QMessageBox::critical(this, "执行失败", 
            "无法启动SSH进程。\n建议配置SSH密钥认证后重试。");
        
        if (upgradeKu5pProcess) {
            upgradeKu5pProcess->deleteLater();
            upgradeKu5pProcess = nullptr;
        }
    }
}

void MainWindow::disableAllOperationButtons()
{
    // 禁用所有操作按钮
    uploadButton->setEnabled(false);
    testConnectionButton->setEnabled(false);
    selectFileButton->setEnabled(false);
    upgradeQtButton->setEnabled(false);
    upgrade7evButton->setEnabled(false);
    upgradeKu5pButton->setEnabled(false);
    executeCommandButton->setEnabled(false);
    clearLogButton->setEnabled(false);
    clearOutputButton->setEnabled(false);
    sshKeyManageButton->setEnabled(false);
    
    // 禁用内置命令窗口控件
    executeBuiltinCommandButton->setEnabled(false);
    clearBuiltinCommandButton->setEnabled(false);
    clearBuiltinOutputButton->setEnabled(false);
    deploySSHKeyButton->setEnabled(false);
    builtinCommandLineEdit->setEnabled(false);
    
    // 禁用输入控件
    ipLineEdit->setEnabled(false);
    portSpinBox->setEnabled(false);
    usernameLineEdit->setEnabled(false);
    passwordLineEdit->setEnabled(false);
    commandLineEdit->setEnabled(false);
    filePathLineEdit->setEnabled(false);
    
    logMessage("[系统] 升级操作进行中，已禁用所有操作按钮");
}

void MainWindow::enableAllOperationButtons()
{
    // 恢复所有操作按钮
    uploadButton->setEnabled(true);
    testConnectionButton->setEnabled(true);
    selectFileButton->setEnabled(true);
    upgradeQtButton->setEnabled(true);
    upgrade7evButton->setEnabled(true);
    upgradeKu5pButton->setEnabled(true);
    executeCommandButton->setEnabled(true);
    clearLogButton->setEnabled(true);
    clearOutputButton->setEnabled(true);
    sshKeyManageButton->setEnabled(true);
    
    // 恢复内置命令窗口控件
    executeBuiltinCommandButton->setEnabled(true);
    clearBuiltinCommandButton->setEnabled(true);
    clearBuiltinOutputButton->setEnabled(true);
    deploySSHKeyButton->setEnabled(true);
    builtinCommandLineEdit->setEnabled(true);
    
    // 恢复输入控件
    ipLineEdit->setEnabled(true);
    portSpinBox->setEnabled(true);
    usernameLineEdit->setEnabled(true);
    passwordLineEdit->setEnabled(true);
    commandLineEdit->setEnabled(true);
    filePathLineEdit->setEnabled(true);
    
    logMessage("[系统] 升级操作完成，已恢复所有操作按钮");
}

// ==================== SSH密钥管理功能实现 ====================

QString MainWindow::getSSHKeyPath()
{
    // 获取用户主目录下的.ssh目录路径
    QString homeDir = QDir::homePath();
    QString sshDir = homeDir + "/.ssh";
    
    // 确保.ssh目录存在
    QDir dir(sshDir);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            logMessage(QString("[警告] 无法创建SSH目录: %1").arg(sshDir));
        }
    }
    
    QString keyPath = sshDir + "/id_rsa";
    
    // 在Windows下，转换为原生路径格式
    if (QSysInfo::productType() == "windows") {
        keyPath = QDir::toNativeSeparators(keyPath);
    }
    
    return keyPath;
}

QString MainWindow::getSSHPublicKeyPath()
{
    return getSSHKeyPath() + ".pub";
}

bool MainWindow::checkSSHKeyExists()
{
    QString keyPath = getSSHKeyPath();
    QString pubKeyPath = getSSHPublicKeyPath();
    
    return QFile::exists(keyPath) && QFile::exists(pubKeyPath);
}

QString MainWindow::readPublicKey()
{
    QString pubKeyPath = getSSHPublicKeyPath();
    QFile file(pubKeyPath);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    
    QTextStream in(&file);
    QString publicKey = in.readAll().trimmed();
    file.close();
    
    return publicKey;
}

void MainWindow::onManageSSHKeys()
{
    logMessage("打开SSH密钥管理");
    
    // 检查SSH密钥状态
    bool keyExists = checkSSHKeyExists();
    QString keyPath = getSSHKeyPath();
    QString pubKeyPath = getSSHPublicKeyPath();
    
    QString statusText;
    if (keyExists) {
        QString publicKey = readPublicKey();
        statusText = QString("SSH密钥已存在\n\n"
                           "私钥位置: %1\n"
                           "公钥位置: %2\n\n"
                           "公钥内容:\n%3\n\n"
                           "您可以选择以下操作:")
                    .arg(keyPath)
                    .arg(pubKeyPath)
                    .arg(publicKey.left(100) + (publicKey.length() > 100 ? "..." : ""));
    } else {
        statusText = QString("SSH密钥不存在\n\n"
                           "将在以下位置生成新的SSH密钥:\n"
                           "私钥: %1\n"
                           "公钥: %2\n\n"
                           "请选择操作:")
                    .arg(keyPath)
                    .arg(pubKeyPath);
    }
    
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("SSH密钥管理");
    msgBox.setText(statusText);
    msgBox.setIcon(QMessageBox::Information);
    
    // 根据密钥存在状态显示不同的按钮
    QPushButton *generateButton = nullptr;
    QPushButton *viewButton = nullptr;
    QPushButton *copyButton = nullptr;
    QPushButton *installButton = nullptr;
    QPushButton *deleteButton = nullptr;
    QPushButton *autoConfigButton = nullptr; // 新增：一体化配置按钮
    msgBox.addButton("取消", QMessageBox::RejectRole);
    
    if (keyExists) {
        viewButton = msgBox.addButton("查看公钥", QMessageBox::ActionRole);
        copyButton = msgBox.addButton("复制公钥", QMessageBox::ActionRole);
        installButton = msgBox.addButton("安装到服务器", QMessageBox::ActionRole);
        generateButton = msgBox.addButton("重新生成", QMessageBox::ActionRole);
        autoConfigButton = msgBox.addButton("一体化配置", QMessageBox::ActionRole);
        deleteButton = msgBox.addButton("删除密钥", QMessageBox::DestructiveRole);
    } else {
        autoConfigButton = msgBox.addButton("一体化配置（推荐）", QMessageBox::ActionRole);
        generateButton = msgBox.addButton("仅生成密钥", QMessageBox::ActionRole);
    }
    
    msgBox.exec();
    
    QAbstractButton *clickedButton = msgBox.clickedButton();
    
    if (clickedButton == autoConfigButton) {
        onGenerateAndDeploySSHKey();
    } else if (clickedButton == generateButton) {
        onGenerateSSHKey();
    } else if (clickedButton == viewButton) {
        onCopyPublicKey();
    } else if (clickedButton == copyButton) {
        // 复制公钥到剪贴板
        QString publicKey = readPublicKey();
        if (!publicKey.isEmpty()) {
            QApplication::clipboard()->setText(publicKey);
            logMessage("公钥已复制到剪贴板");
            QMessageBox::information(this, "复制成功", "SSH公钥已复制到剪贴板，您可以手动粘贴到服务器的 ~/.ssh/authorized_keys 文件中。");
        } else {
            logMessage("[错误] 无法读取公钥文件");
            QMessageBox::warning(this, "读取失败", "无法读取SSH公钥文件。");
        }
    } else if (clickedButton == installButton) {
        installPublicKeyToServer();
    } else if (clickedButton == deleteButton) {
        onDeleteSSHKey();
    }
}

void MainWindow::onGenerateSSHKey()
{
    if (!validateSSHSettings()) {
        QMessageBox::warning(this, "设置错误", "请先配置服务器连接信息后再生成SSH密钥。");
        return;
    }
    
    QString keyPath = getSSHKeyPath();
    bool keyExists = checkSSHKeyExists();
    
    logMessage(QString("[调试] SSH密钥路径: %1").arg(keyPath));
    logMessage(QString("[调试] 密钥是否存在: %1").arg(keyExists ? "是" : "否"));
    logMessage(QString("[调试] 操作系统: %1").arg(QSysInfo::productType()));
    
    if (keyExists) {
        int ret = QMessageBox::question(this, "确认重新生成", 
            QString("SSH密钥已存在，重新生成将覆盖现有密钥！\n\n"
                   "现有密钥位置: %1\n\n"
                   "⚠️ 警告：重新生成密钥将导致:\n"
                   "1. 现有密钥失效\n"
                   "2. 需要重新安装公钥到所有服务器\n"
                   "3. 无法恢复旧密钥\n\n"
                   "确定要重新生成SSH密钥吗？").arg(keyPath),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        
        if (ret != QMessageBox::Yes) {
            logMessage("用户取消了SSH密钥重新生成");
            return;
        }
        
        // 删除现有密钥文件
        QString pubKeyPath = getSSHPublicKeyPath();
        if (QFile::exists(keyPath)) {
            if (QFile::remove(keyPath)) {
                logMessage(QString("[调试] 已删除现有私钥: %1").arg(keyPath));
            } else {
                logMessage(QString("[警告] 无法删除现有私钥: %1").arg(keyPath));
            }
        }
        if (QFile::exists(pubKeyPath)) {
            if (QFile::remove(pubKeyPath)) {
                logMessage(QString("[调试] 已删除现有公钥: %1").arg(pubKeyPath));
            } else {
                logMessage(QString("[警告] 无法删除现有公钥: %1").arg(pubKeyPath));
            }
        }
    }
    
    logMessage("开始生成SSH密钥...");
    logMessage(QString("生成位置: %1").arg(keyPath));
    
    generateSSHKey();
}

void MainWindow::generateSSHKey()
{
    if (sshKeyGenProcess) {
        sshKeyGenProcess->kill();
        sshKeyGenProcess->waitForFinished(1000);
        sshKeyGenProcess->deleteLater();
        sshKeyGenProcess = nullptr;
    }
    
    // 如果是重新生成密钥，先删除现有的密钥文件
    QString keyPath = getSSHKeyPath();
    QString pubKeyPath = getSSHPublicKeyPath();
    
    if (QFile::exists(keyPath)) {
        if (!QFile::remove(keyPath)) {
            logMessage(QString("[警告] 无法删除现有私钥文件: %1").arg(keyPath));
        } else {
            logMessage(QString("[信息] 已删除现有私钥文件: %1").arg(keyPath));
        }
    }
    
    if (QFile::exists(pubKeyPath)) {
        if (!QFile::remove(pubKeyPath)) {
            logMessage(QString("[警告] 无法删除现有公钥文件: %1").arg(pubKeyPath));
        } else {
            logMessage(QString("[信息] 已删除现有公钥文件: %1").arg(pubKeyPath));
        }
    }
    
    sshKeyGenProcess = new QProcess(this);
    
    // 连接进程完成信号
    connect(sshKeyGenProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::onSSHKeyGenFinished);
    
    // 连接错误处理信号
    connect(sshKeyGenProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        QString errorString;
        switch (error) {
            case QProcess::FailedToStart:
                errorString = "无法启动ssh-keygen程序";
                break;
            case QProcess::Crashed:
                errorString = "ssh-keygen程序崩溃";
                break;
            case QProcess::Timedout:
                errorString = "ssh-keygen程序超时";
                break;
            case QProcess::WriteError:
                errorString = "写入错误";
                break;
            case QProcess::ReadError:
                errorString = "读取错误";
                break;
            default:
                errorString = "未知错误";
                break;
        }
        
        logMessage(QString("[错误] SSH密钥生成失败: %1").arg(errorString));
        
        // 恢复按钮状态和重置标志
        sshKeyManageButton->setEnabled(true);
        sshKeyManageButton->setText("SSH密钥");
        isGeneratingAndDeploying = false; // 重置一体化流程标志
        
        QMessageBox::critical(this, "生成失败", 
            QString("SSH密钥生成失败！\n\n错误: %1\n\n"
                   "请检查:\n"
                   "1. 是否已安装OpenSSH客户端\n"
                   "2. ssh-keygen是否在系统PATH中\n"
                   "3. 目标目录是否有写入权限").arg(errorString));
        
        if (sshKeyGenProcess) {
            sshKeyGenProcess->deleteLater();
            sshKeyGenProcess = nullptr;
        }
    });
    
    QString comment = QString("%1@%2").arg(usernameLineEdit->text()).arg(ipLineEdit->text());
    
    // 在Windows下使用原生路径分隔符
    if (QSysInfo::productType() == "windows") {
        keyPath = QDir::toNativeSeparators(keyPath);
    }
    
    // 构建ssh-keygen命令
    QString program = "ssh-keygen";
    QStringList arguments;
    arguments << "-t" << "rsa"                    // RSA类型
              << "-b" << "4096"                   // 4096位长度
              << "-f" << keyPath                  // 输出文件路径
              << "-N" << ""                       // 空密码短语
              << "-q"                             // 静默模式，减少输出
              << "-C" << comment;                 // 注释
    
    logMessage(QString("执行命令: ssh-keygen -t rsa -b 4096 -f \"%1\" -N \"\" -C \"%2\"").arg(keyPath).arg(comment));
    
    // 设置按钮状态
    sshKeyManageButton->setEnabled(false);
    sshKeyManageButton->setText("生成中...");
    
    // 设置进程超时定时器（30秒）
    QTimer::singleShot(30000, this, [this]() {
        if (sshKeyGenProcess && sshKeyGenProcess->state() == QProcess::Running) {
            logMessage("[警告] SSH密钥生成超时，终止进程");
            sshKeyGenProcess->kill();
            sshKeyGenProcess->waitForFinished(3000);
            
            sshKeyManageButton->setEnabled(true);
            sshKeyManageButton->setText("SSH密钥");
            isGeneratingAndDeploying = false; // 重置一体化流程标志
            
            QMessageBox::warning(this, "生成超时", 
                "SSH密钥生成超时！\n\n"
                "可能原因：\n"
                "1. 系统资源不足\n"
                "2. 防病毒软件阻止\n"
                "3. 磁盘空间不足\n\n"
                "请重试或检查系统状态。");
            
            if (sshKeyGenProcess) {
                sshKeyGenProcess->deleteLater();
                sshKeyGenProcess = nullptr;
            }
        }
    });
    
    // 启动进程
    sshKeyGenProcess->start(program, arguments);
    
    // 检查进程是否成功启动
    if (!sshKeyGenProcess->waitForStarted(10000)) {
        logMessage("[错误] 无法启动ssh-keygen进程");
        
        // 尝试获取详细错误信息
        QString errorDetails;
        if (sshKeyGenProcess->error() == QProcess::FailedToStart) {
            errorDetails = "ssh-keygen程序未找到或无法启动。";
        } else {
            errorDetails = QString("进程错误: %1").arg(sshKeyGenProcess->errorString());
        }
        
        QMessageBox::critical(this, "生成失败", 
            QString("无法启动ssh-keygen程序！\n\n%1\n\n"
                   "解决方案：\n"
                   "Windows用户:\n"
                   "1. 安装OpenSSH客户端: 设置 -> 应用 -> 可选功能 -> 添加功能 -> OpenSSH客户端\n"
                   "2. 或安装Git for Windows (包含SSH工具)\n"
                   "3. 确保ssh-keygen在系统PATH中\n\n"
                   "Linux/macOS用户:\n"
                   "1. 安装openssh-client包\n"
                   "2. 确保ssh-keygen命令可用").arg(errorDetails));
        
        sshKeyManageButton->setEnabled(true);
        sshKeyManageButton->setText("SSH密钥");
        isGeneratingAndDeploying = false; // 重置一体化流程标志
        
        if (sshKeyGenProcess) {
            sshKeyGenProcess->deleteLater();
            sshKeyGenProcess = nullptr;
        }
    } else {
        logMessage("[信息] SSH密钥生成进程已启动，请稍候...");
    }
}

void MainWindow::onSSHKeyGenFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // 恢复按钮状态
    sshKeyManageButton->setEnabled(true);
    sshKeyManageButton->setText("SSH密钥");
    
    // 获取进程输出
    QString output, error;
    if (sshKeyGenProcess) {
        output = sshKeyGenProcess->readAllStandardOutput();
        error = sshKeyGenProcess->readAllStandardError();
    }
    
    logMessage(QString("[进程完成] 退出码: %1, 状态: %2").arg(exitCode).arg(exitStatus == QProcess::NormalExit ? "正常退出" : "异常退出"));
    
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        logMessage("[成功] SSH密钥生成完成！");
        if (!output.isEmpty()) {
            logMessage(QString("[输出] %1").arg(output.trimmed()));
        }
        if (!error.isEmpty()) {
            logMessage(QString("[信息] %1").arg(error.trimmed()));
        }
        
        QString keyPath = getSSHKeyPath();
        QString pubKeyPath = getSSHPublicKeyPath();
        
        // 验证密钥文件是否真的生成了
        if (!QFile::exists(keyPath) || !QFile::exists(pubKeyPath)) {
            logMessage("[警告] 密钥文件未找到，可能生成失败");
            QMessageBox::warning(this, "文件验证失败", 
                QString("SSH密钥生成可能失败，未找到密钥文件：\n\n"
                       "私钥: %1 %2\n"
                       "公钥: %3 %4\n\n"
                       "请检查目录权限或重试。")
                .arg(keyPath).arg(QFile::exists(keyPath) ? "✓" : "✗")
                .arg(pubKeyPath).arg(QFile::exists(pubKeyPath) ? "✓" : "✗"));
            
            if (sshKeyGenProcess) {
                sshKeyGenProcess->deleteLater();
                sshKeyGenProcess = nullptr;
            }
            return;
        }
        
        QString publicKey = readPublicKey();
        
        logMessage(QString("私钥位置: %1").arg(keyPath));
        logMessage(QString("公钥位置: %1").arg(pubKeyPath));
        logMessage(QString("公钥长度: %1 字符").arg(publicKey.length()));
        
        // 如果是一体化生成和部署流程，直接开始部署
        if (isGeneratingAndDeploying) {
            logMessage("[自动部署] SSH密钥生成完成，开始自动部署到服务器...");
            
            // 验证连接信息
            if (!validateSSHSettings()) {
                QMessageBox::warning(this, "连接信息不完整", 
                    "无法自动部署SSH密钥，请检查服务器连接信息是否完整。\n\n"
                    "您可以手动复制公钥内容并部署到服务器。");
                isGeneratingAndDeploying = false; // 重置标志
                if (sshKeyGenProcess) {
                    sshKeyGenProcess->deleteLater();
                    sshKeyGenProcess = nullptr;
                }
                return;
            }
            
            // 检查是否有密码，如果没有密码需要用户输入
            QString password = passwordLineEdit->text();
            if (password.isEmpty()) {
                isGeneratingAndDeploying = false; // 重置标志，因为需要用户交互
                
                QMessageBox::information(this, "需要服务器密码", 
                    "SSH密钥已生成完成！\n\n"
                    "现在需要您输入服务器密码来完成自动部署。\n"
                    "这将是最后一次需要输入密码，\n"
                    "部署完成后即可使用SSH密钥免密码登录。\n\n"
                    "请在连接设置中填入服务器密码，然后重新点击一体化配置。");
                
                if (sshKeyGenProcess) {
                    sshKeyGenProcess->deleteLater();
                    sshKeyGenProcess = nullptr;
                }
                return;
            }
            
            logMessage("[自动部署] 开始使用现有密码自动部署SSH密钥...");
            isGeneratingAndDeploying = false; // 重置标志
            
            // 自动开始部署
            installPublicKeyToServer();
        } else {
            // 传统流程：显示成功信息并询问是否部署
            QMessageBox::information(this, "密钥生成成功", 
                QString("SSH密钥已成功生成！\n\n"
                       "私钥位置: %1\n"
                       "公钥位置: %2\n\n"
                       "公钥内容:\n%3\n\n"
                       "下一步：\n"
                       "1. 点击 \"安装到服务器\" 自动安装公钥\n"
                       "2. 或手动复制公钥到服务器的 ~/.ssh/authorized_keys 文件")
                .arg(keyPath)
                .arg(pubKeyPath)
                .arg(publicKey.left(100) + (publicKey.length() > 100 ? "..." : "")));
            
            // 询问是否立即安装到服务器
            int ret = QMessageBox::question(this, "安装公钥", 
                "是否立即将公钥安装到服务器？\n\n"
                "这将自动执行安装命令将公钥添加到服务器的授权密钥列表中。",
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::Yes);
            
            if (ret == QMessageBox::Yes) {
                installPublicKeyToServer();
            }
        }
    } else {
        // 生成失败的处理
        logMessage(QString("[错误] SSH密钥生成失败 (退出码: %1, 状态: %2)").arg(exitCode).arg(exitStatus == QProcess::NormalExit ? "正常退出" : "异常退出"));
        
        if (!error.isEmpty()) {
            logMessage(QString("[错误信息] %1").arg(error.trimmed()));
        }
        if (!output.isEmpty()) {
            logMessage(QString("[输出] %1").arg(output.trimmed()));
        }
        
        QString errorMessage = "SSH密钥生成失败！\n\n";
        
        if (exitStatus == QProcess::CrashExit) {
            errorMessage += "进程崩溃或被终止。\n\n";
        } else if (exitCode != 0) {
            errorMessage += QString("进程返回错误码: %1\n\n").arg(exitCode);
        }
        
        if (!error.isEmpty()) {
            errorMessage += QString("错误信息: %1\n\n").arg(error.trimmed());
        }
        
        errorMessage += "可能的解决方案:\n"
                       "1. 检查ssh-keygen程序是否正确安装\n"
                       "2. 检查目标目录是否有写入权限\n"
                       "3. 检查磁盘空间是否充足\n"
                       "4. 检查防病毒软件是否阻止了操作\n"
                       "5. 尝试以管理员权限运行程序";
        
        QMessageBox::critical(this, "生成失败", errorMessage);
    }
    
    // 清理进程对象
    if (sshKeyGenProcess) {
        sshKeyGenProcess->deleteLater();
        sshKeyGenProcess = nullptr;
    }
}

void MainWindow::onCopyPublicKey()
{
    QString publicKey = readPublicKey();
    if (publicKey.isEmpty()) {
        logMessage("[错误] 无法读取SSH公钥");
        QMessageBox::warning(this, "读取失败", "无法读取SSH公钥文件。");
        return;
    }
    
    // 显示公钥内容的详细对话框
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("SSH公钥内容");
    msgBox.setText("SSH公钥内容如下，您可以复制此内容到服务器：");
    msgBox.setDetailedText(publicKey);
    msgBox.setIcon(QMessageBox::Information);
    
    QPushButton *copyButton = msgBox.addButton("复制到剪贴板", QMessageBox::ActionRole);
    QPushButton *installButton = msgBox.addButton("安装到服务器", QMessageBox::ActionRole);
    msgBox.addButton("关闭", QMessageBox::RejectRole);
    
    msgBox.exec();
    
    QAbstractButton *clickedButton = msgBox.clickedButton();
    if (clickedButton == copyButton) {
        QApplication::clipboard()->setText(publicKey);
        logMessage("SSH公钥已复制到剪贴板");
        QMessageBox::information(this, "复制成功", 
            "SSH公钥已复制到剪贴板。\n\n"
            "手动安装步骤：\n"
            "1. 登录到服务器\n"
            "2. 编辑 ~/.ssh/authorized_keys 文件\n"
            "3. 将公钥内容粘贴到文件末尾\n"
            "4. 保存文件并设置正确权限: chmod 600 ~/.ssh/authorized_keys");
    } else if (clickedButton == installButton) {
        onInstallPublicKey();
    }
}

void MainWindow::onInstallPublicKey()
{
    if (!validateSSHSettings()) {
        QMessageBox::warning(this, "设置错误", "请先配置完整的服务器连接信息。");
        return;
    }
    
    if (!checkSSHKeyExists()) {
        QMessageBox::warning(this, "密钥不存在", "SSH密钥不存在，请先生成SSH密钥。");
        return;
    }
    
    QString ip = ipLineEdit->text().trimmed();
    int port = portSpinBox->value();
    QString username = usernameLineEdit->text().trimmed();
    QString password = passwordLineEdit->text();
    
    if (password.isEmpty()) {
        QMessageBox::warning(this, "密码必需", 
            "安装SSH公钥需要服务器密码。\n\n"
            "注意：这是最后一次需要输入密码，安装完成后即可使用密钥认证。");
        return;
    }
    
    int ret = QMessageBox::question(this, "确认安装公钥", 
        QString("即将将SSH公钥安装到服务器:\n\n"
               "服务器: %1@%2:%3\n"
               "操作: 执行 ssh-copy-id 命令\n\n"
               "这将把您的公钥添加到服务器的 ~/.ssh/authorized_keys 文件中。\n"
               "安装成功后，您就可以使用密钥认证免密码登录了。\n\n"
               "是否继续？")
        .arg(username).arg(ip).arg(port),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes);
    
    if (ret != QMessageBox::Yes) {
        logMessage("用户取消了SSH公钥安装");
        return;
    }
    
    logMessage("开始安装SSH公钥到服务器...");
    installPublicKeyToServer();
}

void MainWindow::onDeleteSSHKey()
{
    if (!checkSSHKeyExists()) {
        QMessageBox::information(this, "无密钥", "SSH密钥不存在，无需删除。");
        return;
    }
    
    QString keyPath = getSSHKeyPath();
    QString pubKeyPath = getSSHPublicKeyPath();
    QString publicKey = readPublicKey();
    
    // 显示详细确认对话框
    QMessageBox confirmBox(this);
    confirmBox.setWindowTitle("确认删除SSH密钥");
    confirmBox.setIcon(QMessageBox::Warning);
    
    QString confirmText = QString(
        "⚠️ 您即将删除SSH密钥！\n\n"
        "将要删除的文件：\n"
        "• 私钥文件：%1\n"
        "• 公钥文件：%2\n\n"
        "公钥内容预览：\n%3\n\n"
        "❗ 重要警告：\n"
        "• 删除后将无法恢复\n"
        "• 已部署到服务器的公钥需要手动清理\n"
        "• 删除后需要重新生成密钥才能使用免密码登录\n\n"
        "是否确定删除这些SSH密钥文件？"
    ).arg(keyPath)
     .arg(pubKeyPath)
     .arg(publicKey.left(80) + (publicKey.length() > 80 ? "..." : ""));
    
    confirmBox.setText(confirmText);
    
    QPushButton *deleteButton = confirmBox.addButton("删除密钥", QMessageBox::DestructiveRole);
    QPushButton *cancelButton = confirmBox.addButton("取消", QMessageBox::RejectRole);
    confirmBox.setDefaultButton(cancelButton);
    
    confirmBox.exec();
    
    QAbstractButton *clickedButton = confirmBox.clickedButton();
    
    if (clickedButton == deleteButton) {
        // 执行删除操作
        bool privateKeyDeleted = false;
        bool publicKeyDeleted = false;
        QString errorMessage;
        
        // 删除私钥文件
        if (QFile::exists(keyPath)) {
            if (QFile::remove(keyPath)) {
                privateKeyDeleted = true;
                logMessage(QString("[成功] 已删除私钥文件: %1").arg(keyPath));
            } else {
                errorMessage += QString("• 无法删除私钥文件: %1\n").arg(keyPath);
                logMessage(QString("[错误] 无法删除私钥文件: %1").arg(keyPath));
            }
        } else {
            privateKeyDeleted = true; // 文件不存在，视为删除成功
            logMessage(QString("[信息] 私钥文件不存在: %1").arg(keyPath));
        }
        
        // 删除公钥文件
        if (QFile::exists(pubKeyPath)) {
            if (QFile::remove(pubKeyPath)) {
                publicKeyDeleted = true;
                logMessage(QString("[成功] 已删除公钥文件: %1").arg(pubKeyPath));
            } else {
                errorMessage += QString("• 无法删除公钥文件: %1\n").arg(pubKeyPath);
                logMessage(QString("[错误] 无法删除公钥文件: %1").arg(pubKeyPath));
            }
        } else {
            publicKeyDeleted = true; // 文件不存在，视为删除成功
            logMessage(QString("[信息] 公钥文件不存在: %1").arg(pubKeyPath));
        }
        
        // 显示删除结果
        if (privateKeyDeleted && publicKeyDeleted) {
            QMessageBox::information(this, "删除成功", 
                QString("SSH密钥已成功删除！\n\n"
                       "已删除的文件：\n"
                       "• %1\n"
                       "• %2\n\n"
                       "💡 后续操作建议：\n"
                       "• 如需继续使用SSH免密码登录，请重新生成密钥\n"
                       "• 如果已将公钥部署到服务器，建议手动清理服务器上的 ~/.ssh/authorized_keys 文件")
                .arg(keyPath).arg(pubKeyPath));
            
            logMessage("[完成] SSH密钥删除操作成功完成");
        } else {
            QMessageBox::critical(this, "删除失败", 
                QString("SSH密钥删除失败！\n\n"
                       "错误详情：\n%1\n"
                       "可能原因：\n"
                       "• 文件正在被其他程序使用\n"
                       "• 权限不足\n"
                       "• 文件被保护或只读\n\n"
                       "建议：\n"
                       "• 关闭可能使用SSH的程序\n"
                       "• 以管理员权限运行本程序\n"
                       "• 手动删除文件")
                .arg(errorMessage));
            
            logMessage("[失败] SSH密钥删除操作失败");
        }
    } else {
        logMessage("用户取消了SSH密钥删除操作");
    }
}

void MainWindow::installPublicKeyToServer()
{
    QString ip = ipLineEdit->text().trimmed();
    int port = portSpinBox->value();
    QString username = usernameLineEdit->text().trimmed();
    QString password = passwordLineEdit->text();
    QString publicKey = readPublicKey();
    
    if (publicKey.isEmpty()) {
        QMessageBox::warning(this, "读取失败", "无法读取SSH公钥文件。");
        return;
    }
    
    // 检查连接设置是否完整
    if (ip.isEmpty() || username.isEmpty()) {
        QMessageBox::warning(this, "连接设置不完整", 
            "请先填写完整的服务器连接信息（IP地址和用户名）。");
        return;
    }
    
    // 检查是否有密码
    if (password.isEmpty()) {
        QMessageBox::warning(this, "需要密码", 
            "部署SSH密钥需要服务器密码。\n\n"
            "请在服务器连接设置中填写密码后再进行部署。");
        return;
    }
    
    logMessage("[智能部署] 开始部署SSH公钥到服务器...");
    logMessage(QString("目标服务器: %1@%2:%3").arg(username).arg(ip).arg(port));
    logMessage("[智能部署] 使用服务器连接设置中的密码进行部署");
    
    // 智能部署：直接使用SSH命令进行安装
    QString installCommand = generateReliableSSHInstallCommand(username, ip, port, publicKey);
    
    if (installCommand.contains("ERROR")) {
        QMessageBox::critical(this, "部署失败", 
            "生成SSH安装命令时发生错误。\n\n"
            "请检查SSH密钥文件是否存在。");
        logMessage("[错误] 生成SSH安装命令失败");
        return;
    }
    
    // 直接使用服务器连接设置中的密码进行部署
    executeSSHWithPassword(installCommand, password);
}

void MainWindow::showSSHKeyStatus()
{
    bool keyExists = checkSSHKeyExists();
    QString keyPath = getSSHKeyPath();
    QString pubKeyPath = getSSHPublicKeyPath();
    
    if (keyExists) {
        QString publicKey = readPublicKey();
        QFileInfo keyInfo(keyPath);
        QFileInfo pubKeyInfo(pubKeyPath);
        
        QString statusText = QString("SSH密钥状态: ✅ 已配置\n\n"
                                   "私钥文件: %1\n"
                                   "创建时间: %2\n"
                                   "文件大小: %3 字节\n\n"
                                   "公钥文件: %4\n"
                                   "创建时间: %5\n"
                                   "文件大小: %6 字节\n\n"
                                   "公钥内容:\n%7")
                            .arg(keyPath)
                            .arg(keyInfo.lastModified().toString())
                            .arg(keyInfo.size())
                            .arg(pubKeyPath)
                            .arg(pubKeyInfo.lastModified().toString())
                            .arg(pubKeyInfo.size())
                            .arg(publicKey);
        
        logMessage("SSH密钥状态: 已配置");
        logMessage(QString("私钥位置: %1").arg(keyPath));
        logMessage(QString("公钥位置: %1").arg(pubKeyPath));
    } else {
        QString statusText = QString("SSH密钥状态: ❌ 未配置\n\n"
                                   "预期位置:\n"
                                   "私钥: %1\n"
                                   "公钥: %2\n\n"
                                   "建议点击 \"生成密钥\" 创建SSH密钥以启用免密码认证。")
                            .arg(keyPath)
                            .arg(pubKeyPath);
        
                 logMessage("SSH密钥状态: 未配置");
         logMessage(QString("可在以下位置生成密钥: %1").arg(keyPath));
     }
}

void MainWindow::showSSHTroubleshooting()
{
    QString ip = ipLineEdit->text().trimmed();
    int port = portSpinBox->value();
    QString username = usernameLineEdit->text().trimmed();
    
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("SSH连接故障排除");
    msgBox.setIcon(QMessageBox::Information);
    
    QString troubleshootingText = QString(
        "🔧 SSH连接故障排除指南\n\n"
        "如果SSH连接测试失败，请按照以下步骤检查：\n\n"
        "📋 第一步：验证公钥是否正确安装\n"
        "1. 登录服务器：ssh -p %2 %1@%3\n"
        "2. 检查授权文件：cat ~/.ssh/authorized_keys\n"
        "3. 确认公钥存在且格式正确（应该是一行完整内容）\n\n"
        "🔒 第二步：检查文件权限\n"
        "在服务器上执行以下命令：\n"
        "chmod 700 ~/.ssh\n"
        "chmod 600 ~/.ssh/authorized_keys\n"
        "chown %1:%1 ~/.ssh/authorized_keys\n\n"
        "⚙️ 第三步：检查SSH服务器配置\n"
        "1. 确认SSH服务运行：sudo systemctl status ssh\n"
        "2. 检查配置文件：sudo nano /etc/ssh/sshd_config\n"
        "3. 确认以下设置：\n"
        "   PubkeyAuthentication yes\n"
        "   AuthorizedKeysFile .ssh/authorized_keys\n"
        "4. 重启SSH服务：sudo systemctl restart ssh\n\n"
        "🚀 第四步：测试验证\n"
        "1. 退出服务器\n"
        "2. 重新连接：ssh -p %2 %1@%3\n"
        "3. 如果成功免密码登录，配置完成！\n\n"
        "🆘 如果仍然失败，请尝试：\n"
        "1. 重新生成SSH密钥\n"
        "2. 使用密码认证作为临时方案\n"
        "3. 检查防火墙和网络设置"
    ).arg(username).arg(port).arg(ip);
    
    msgBox.setText(troubleshootingText);
    
    QPushButton *copyCommandsButton = msgBox.addButton("复制检查命令", QMessageBox::ActionRole);
    QPushButton *testAgainButton = msgBox.addButton("重新测试", QMessageBox::ActionRole);
    msgBox.addButton("关闭", QMessageBox::RejectRole);
    
    msgBox.exec();
    
    QAbstractButton *clickedButton = msgBox.clickedButton();
    
    if (clickedButton == copyCommandsButton) {
        // 复制常用检查命令到剪贴板
        QString commands = QString(
            "# SSH故障排除命令集合\n"
            "# 1. 登录服务器\n"
            "ssh -p %2 %1@%3\n\n"
            "# 2. 检查授权文件\n"
            "cat ~/.ssh/authorized_keys\n\n"
            "# 3. 设置正确权限\n"
            "chmod 700 ~/.ssh\n"
            "chmod 600 ~/.ssh/authorized_keys\n"
            "chown %1:%1 ~/.ssh/authorized_keys\n\n"
            "# 4. 检查SSH服务状态\n"
            "sudo systemctl status ssh\n\n"
            "# 5. 重启SSH服务\n"
            "sudo systemctl restart ssh\n\n"
            "# 6. 测试连接\n"
            "ssh -p %2 %1@%3\n"
        ).arg(username).arg(port).arg(ip);
        
        QApplication::clipboard()->setText(commands);
        logMessage("SSH故障排除命令已复制到剪贴板");
        QMessageBox::information(this, "复制成功", "SSH故障排除命令已复制到剪贴板，您可以在终端中执行这些命令。");
    } else if (clickedButton == testAgainButton) {
        logMessage("[故障排除] 用户选择重新测试连接");
        onTestConnection();
    }
    
    logMessage("用户查看了SSH故障排除指南");
}

void MainWindow::executeSSHKeyInstallation()
{
    QString publicKey = readPublicKey();
    
    if (publicKey.isEmpty()) {
        QMessageBox::warning(this, "读取失败", "无法读取SSH公钥文件。");
        return;
    }
    
    logMessage("开始SSH公钥安装流程...");
    logMessage("使用标准SSH命令进行安装");
    
    // 显示智能安装指导
    showSmartInstallationGuide();
}

void MainWindow::executeSSHKeyInstallationDirect()
{
    logMessage("[智能安装] 提供详细安装指导和验证");
    
    // 直接显示智能安装指导
    showSmartInstallationGuide();
}

void MainWindow::showSmartInstallationGuide()
{
    QString ip = ipLineEdit->text().trimmed();
    int port = portSpinBox->value();
    QString username = usernameLineEdit->text().trimmed();
    QString publicKey = readPublicKey();
    
    if (publicKey.isEmpty()) {
        QMessageBox::warning(this, "读取失败", "无法读取SSH公钥文件。");
        return;
    }
    
    // 检测操作系统
    bool isWindows = QSysInfo::productType() == "windows";
    
    // 显示智能安装指导对话框
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("SSH公钥安装指导");
    msgBox.setIcon(QMessageBox::Information);
    
    QString instructions;
    
    if (isWindows) {
        // Windows系统专用指导
        instructions = QString(
            "🔧 SSH公钥安装指导（Windows系统）\n\n"
            "📋 连接信息：\n"
            "服务器：%1@%2:%3\n\n"
            "💡 Windows推荐方法（使用SSH命令）：\n"
            "1. 打开PowerShell或命令提示符\n"
            "2. 执行以下命令：\n"
            "   type \"%4\" | ssh -p %3 %1@%2 \"mkdir -p ~/.ssh && cat >> ~/.ssh/authorized_keys && chmod 700 ~/.ssh && chmod 600 ~/.ssh/authorized_keys\"\n"
            "3. 输入服务器密码（仅此一次）\n"
            "4. 看到成功提示即完成安装\n\n"
            "🔧 备用方法（手动复制粘贴）：\n"
            "1. 复制下方公钥内容（已自动复制到剪贴板）\n"
            "2. 登录服务器：ssh -p %3 %1@%2\n"
            "3. 创建目录：mkdir -p ~/.ssh\n"
            "4. 编辑文件：echo 'YOUR_PUBLIC_KEY' >> ~/.ssh/authorized_keys\n"
            "5. 设置权限：chmod 700 ~/.ssh && chmod 600 ~/.ssh/authorized_keys\n\n"
            "✅ 验证安装：\n"
            "重新连接：ssh -p %3 %1@%2\n"
            "如果无需密码即可登录，说明配置成功！\n\n"
            "💾 公钥内容（已复制到剪贴板）："
        ).arg(username).arg(ip).arg(port).arg(getSSHPublicKeyPath().replace("/", "\\"));
    } else {
        // Linux/macOS系统指导
        instructions = QString(
            "🔧 SSH公钥安装指导（Linux/macOS系统）\n\n"
            "📋 连接信息：\n"
            "服务器：%1@%2:%3\n\n"
            "💡 推荐方法（使用ssh-copy-id）：\n"
            "1. 打开终端\n"
            "2. 执行以下命令：\n"
            "   ssh-copy-id -p %3 %1@%2\n"
            "3. 输入服务器密码（仅此一次）\n"
            "4. 看到成功提示即完成安装\n\n"
            "🔧 备用方法（使用SSH命令）：\n"
            "1. 执行以下命令：\n"
            "   cat %4 | ssh -p %3 %1@%2 \"mkdir -p ~/.ssh && cat >> ~/.ssh/authorized_keys && chmod 700 ~/.ssh && chmod 600 ~/.ssh/authorized_keys\"\n"
            "2. 输入服务器密码\n\n"
            "🔧 手动方法：\n"
            "1. 复制下方公钥内容（已自动复制到剪贴板）\n"
            "2. 登录服务器：ssh -p %3 %1@%2\n"
            "3. 创建目录：mkdir -p ~/.ssh\n"
            "4. 编辑文件：nano ~/.ssh/authorized_keys\n"
            "5. 粘贴公钥到文件末尾（确保为一行）\n"
            "6. 保存并设置权限：chmod 700 ~/.ssh && chmod 600 ~/.ssh/authorized_keys\n\n"
            "✅ 验证安装：\n"
            "重新连接：ssh -p %3 %1@%2\n"
            "如果无需密码即可登录，说明配置成功！\n\n"
            "💾 公钥内容（已复制到剪贴板）："
        ).arg(username).arg(ip).arg(port).arg(getSSHPublicKeyPath());
    }
    
    msgBox.setText(instructions);
    msgBox.setDetailedText(publicKey);
    
    // 自动复制公钥到剪贴板
    QApplication::clipboard()->setText(publicKey);
    
    QPushButton *copyCommandButton;
    if (isWindows) {
        copyCommandButton = msgBox.addButton("复制Windows SSH命令", QMessageBox::ActionRole);
    } else {
        copyCommandButton = msgBox.addButton("复制ssh-copy-id命令", QMessageBox::ActionRole);
    }
    
    QPushButton *copySSHButton = msgBox.addButton("复制通用SSH命令", QMessageBox::ActionRole);
    QPushButton *copyToBuiltinButton = msgBox.addButton("复制到命令窗口", QMessageBox::ActionRole);
    QPushButton *testButton = msgBox.addButton("测试连接", QMessageBox::ActionRole);
    QPushButton *helpButton = msgBox.addButton("故障排除", QMessageBox::HelpRole);
    msgBox.addButton("关闭", QMessageBox::AcceptRole);
    
    msgBox.exec();
    
    QAbstractButton *clickedButton = msgBox.clickedButton();
    
    if (clickedButton == copyCommandButton) {
        QString command;
        if (isWindows) {
            QString windowsKeyPath = getSSHPublicKeyPath().replace("/", "\\");
            command = QString("type \"%4\" | ssh -p %3 %1@%2 \"mkdir -p ~/.ssh && cat >> ~/.ssh/authorized_keys && chmod 700 ~/.ssh && chmod 600 ~/.ssh/authorized_keys\"")
                     .arg(username).arg(ip).arg(port).arg(windowsKeyPath);
        } else {
            command = QString("ssh-copy-id -p %1 %2@%3").arg(port).arg(username).arg(ip);
        }
        
        QApplication::clipboard()->setText(command);
        logMessage(QString("SSH安装命令已复制到剪贴板: %1").arg(command));
        QMessageBox::information(this, "命令复制成功", 
            QString("SSH安装命令已复制到剪贴板：\n\n%1\n\n"
                   "请在终端中粘贴并执行此命令，然后输入服务器密码完成安装。").arg(command));
    } else if (clickedButton == copySSHButton) {
        QString sshCommand = QString("cat %4 | ssh -p %3 %1@%2 \"mkdir -p ~/.ssh && cat >> ~/.ssh/authorized_keys && chmod 700 ~/.ssh && chmod 600 ~/.ssh/authorized_keys\"")
                            .arg(username).arg(ip).arg(port).arg(isWindows ? getSSHPublicKeyPath().replace("/", "\\") : getSSHPublicKeyPath());
        
        if (isWindows) {
            sshCommand = QString("type \"%4\" | ssh -p %3 %1@%2 \"mkdir -p ~/.ssh && cat >> ~/.ssh/authorized_keys && chmod 700 ~/.ssh && chmod 600 ~/.ssh/authorized_keys\"")
                        .arg(username).arg(ip).arg(port).arg(getSSHPublicKeyPath().replace("/", "\\"));
        }
        
        QApplication::clipboard()->setText(sshCommand);
        logMessage(QString("通用SSH命令已复制到剪贴板: %1").arg(sshCommand));
        QMessageBox::information(this, "命令复制成功", 
            QString("通用SSH命令已复制到剪贴板：\n\n%1\n\n"
                   "此命令适用于所有支持SSH的系统。").arg(sshCommand));
    } else if (clickedButton == copyToBuiltinButton) {
        QString sshCommand = QString("cat %4 | ssh -p %3 %1@%2 \"mkdir -p ~/.ssh && cat >> ~/.ssh/authorized_keys && chmod 700 ~/.ssh && chmod 600 ~/.ssh/authorized_keys\"")
                            .arg(username).arg(ip).arg(port).arg(isWindows ? getSSHPublicKeyPath().replace("/", "\\") : getSSHPublicKeyPath());
        
        if (isWindows) {
            sshCommand = QString("type \"%4\" | ssh -p %3 %1@%2 \"mkdir -p ~/.ssh && cat >> ~/.ssh/authorized_keys && chmod 700 ~/.ssh && chmod 600 ~/.ssh/authorized_keys\"")
                        .arg(username).arg(ip).arg(port).arg(getSSHPublicKeyPath().replace("/", "\\"));
        }
        
        setBuiltinCommand(sshCommand);
        logMessage(QString("SSH命令已复制到内置命令窗口: %1").arg(sshCommand));
        QMessageBox::information(this, "命令复制成功", 
            QString("SSH安装命令已复制到内置命令窗口！\n\n%1\n\n"
                   "您可以在应用下方的\"内置命令窗口\"中看到该命令，\n"
                   "点击\"执行命令\"按钮或按Enter键运行。").arg(sshCommand));
    } else if (clickedButton == testButton) {
        logMessage("[智能安装] 用户请求测试连接");
        onTestConnection();
    } else if (clickedButton == helpButton) {
        showSSHTroubleshooting();
    }
    
    logMessage("已显示SSH公钥智能安装指导");
}

// ==================== 内置命令窗口功能实现 ====================

void MainWindow::onExecuteBuiltinCommand()
{
    QString command = builtinCommandLineEdit->text().trimmed();
    if (command.isEmpty()) {
        builtinCommandOutputEdit->append("<span style='color: #ff6b6b;'>[错误] 请输入要执行的命令</span>");
        return;
    }
    
    executeBuiltinSystemCommand(command);
}

void MainWindow::onClearBuiltinCommand()
{
    builtinCommandLineEdit->clear();
    builtinCommandOutputEdit->append("<span style='color: #4ecdc4;'>[系统] 命令输入已清空</span>");
}

void MainWindow::onClearBuiltinOutput()
{
    builtinCommandOutputEdit->clear();
    builtinCommandOutputEdit->append("<span style='color: #4ecdc4;'>[系统] 输出已清空</span>");
}

void MainWindow::onBuiltinCommandInputEnterPressed()
{
    onExecuteBuiltinCommand();
}


void MainWindow::onDeploySSHKey()
{
    if (!validateSSHSettings()) {
        QMessageBox::warning(this, "设置错误", "请先配置完整的服务器连接信息（IP地址、用户名、端口）");
        return;
    }
    
    // 检查SSH密钥是否存在
    bool keyExists = checkSSHKeyExists();
    
    if (!keyExists) {
        // 提示用户SSH密钥不存在，询问是否要生成并部署
        int ret = QMessageBox::question(this, "SSH密钥不存在", 
            "检测到SSH密钥不存在。\n\n"
            "是否要自动生成SSH密钥并部署到服务器？\n"
            "这将实现一键配置免密码登录。",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes);
        
        if (ret == QMessageBox::Yes) {
            // 执行一体化生成和部署
            onGenerateAndDeploySSHKey();
        } else {
            logMessage("用户取消了SSH密钥生成和部署");
        }
        return;
    }
    
    // SSH密钥存在，直接部署
    QString ip = ipLineEdit->text().trimmed();
    int port = portSpinBox->value();
    QString username = usernameLineEdit->text().trimmed();
    
    int ret = QMessageBox::question(this, "部署SSH密钥", 
        QString("即将部署SSH密钥到服务器：\n\n"
               "服务器：%1@%2:%3\n\n"
               "这将把您的SSH公钥安装到服务器的 ~/.ssh/authorized_keys 文件中，\n"
               "完成后即可使用SSH密钥免密码登录。\n\n"
               "是否继续？")
        .arg(username).arg(ip).arg(port),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes);
    
    if (ret == QMessageBox::Yes) {
        logMessage("开始部署SSH密钥到服务器...");
        installPublicKeyToServer();
    } else {
        logMessage("用户取消了SSH密钥部署");
    }
}

void MainWindow::executeBuiltinSystemCommand(const QString &command)
{
    if (builtinCommandProcess && builtinCommandProcess->state() != QProcess::NotRunning) {
        builtinCommandOutputEdit->append("<span style='color: #ffa500;'>[警告] 有命令正在执行中，请稍等...</span>");
        return;
    }
    
    // 检查是否是SSH命令且需要密码输入
    if ((command.contains("ssh") && !command.contains("ssh-keygen") && !command.contains("-i")) ||
        command.endsWith(".bat")) {
        executeSSHCommandWithPassword(command);
        return;
    }
    
    if (builtinCommandProcess) {
        builtinCommandProcess->deleteLater();
        builtinCommandProcess = nullptr;
    }
    
    builtinCommandProcess = new QProcess(this);
    
    // 在输出区域显示执行的命令
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    builtinCommandOutputEdit->append(QString("<span style='color: #74b9ff;'>[%1] $ %2</span>").arg(timestamp).arg(command));
    
    // 连接信号处理实时输出
    connect(builtinCommandProcess, &QProcess::readyReadStandardOutput, 
            this, [this]() {
        QByteArray data = builtinCommandProcess->readAllStandardOutput();
        if (!data.isEmpty()) {
            // 处理编码问题：在Windows下尝试使用本地编码
            QString output;
            if (QSysInfo::productType() == "windows") {
                // Windows下先尝试UTF-8，如果失败则使用本地编码
                output = QString::fromUtf8(data);
                if (output.contains(QChar::ReplacementCharacter)) {
                    output = QString::fromLocal8Bit(data);
                }
            } else {
                output = QString::fromUtf8(data);
            }
            
            if (!output.isEmpty()) {
                // 处理输出格式，保持原始格式
                output = output.replace("\n", "<br/>").replace(" ", "&nbsp;");
                builtinCommandOutputEdit->append(QString("<span style='color: #ffffff;'>%1</span>").arg(output));
            }
        }
        // 自动滚动到底部
        QTextCursor cursor = builtinCommandOutputEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        builtinCommandOutputEdit->setTextCursor(cursor);
    });
    
    connect(builtinCommandProcess, &QProcess::readyReadStandardError, 
            this, [this]() {
        QByteArray data = builtinCommandProcess->readAllStandardError();
        if (!data.isEmpty()) {
            // 处理编码问题：在Windows下尝试使用本地编码
            QString error;
            if (QSysInfo::productType() == "windows") {
                // Windows下先尝试UTF-8，如果失败则使用本地编码
                error = QString::fromUtf8(data);
                if (error.contains(QChar::ReplacementCharacter)) {
                    error = QString::fromLocal8Bit(data);
                }
            } else {
                error = QString::fromUtf8(data);
            }
            
            if (!error.isEmpty()) {
                error = error.replace("\n", "<br/>").replace(" ", "&nbsp;");
                builtinCommandOutputEdit->append(QString("<span style='color: #ff7675;'>%1</span>").arg(error));
            }
        }
        QTextCursor cursor = builtinCommandOutputEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        builtinCommandOutputEdit->setTextCursor(cursor);
    });
    
    // 连接完成信号
    connect(builtinCommandProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, command](int exitCode, QProcess::ExitStatus exitStatus) {
                
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
        bool isInteractiveSSH = command.contains("ssh") && !command.contains("ssh-keygen");
        bool isWindowsBatchFile = command.endsWith(".bat");
        
        if (isInteractiveSSH || isWindowsBatchFile) {
            // 交互式命令（SSH）的处理
            builtinCommandOutputEdit->append(QString("<span style='color: #74b9ff;'>[%1] SSH命令已在新终端窗口中启动</span>").arg(timestamp));
            builtinCommandOutputEdit->append("<span style='color: #00b894;'>[提示] 请在弹出的终端窗口中输入密码完成SSH操作</span>");
            
            // 询问用户是否要测试连接
            QTimer::singleShot(2000, this, [this, command]() {
                if (command.contains("authorized_keys")) {
                    int ret = QMessageBox::question(this, "SSH操作", 
                        "SSH公钥安装命令已在新终端中执行。\n\n"
                        "如果您已成功输入密码并完成安装，\n"
                        "是否立即测试SSH连接验证安装结果？",
                        QMessageBox::Yes | QMessageBox::No,
                        QMessageBox::Yes);
                    
                    if (ret == QMessageBox::Yes) {
                        logMessage("[内置命令] 用户选择测试SSH连接");
                        onTestConnection();
                    }
                }
            });
        } else {
            // 非交互式命令的正常处理
            if (exitStatus == QProcess::NormalExit) {
                if (exitCode == 0) {
                    builtinCommandOutputEdit->append(QString("<span style='color: #00b894;'>[%1] 命令执行完成 (退出码: %2)</span>")
                                                   .arg(timestamp).arg(exitCode));
                } else {
                    builtinCommandOutputEdit->append(QString("<span style='color: #e17055;'>[%1] 命令执行完成但有错误 (退出码: %2)</span>")
                                                   .arg(timestamp).arg(exitCode));
                }
            } else {
                builtinCommandOutputEdit->append(QString("<span style='color: #ff6b6b;'>[%1] 命令执行异常终止</span>").arg(timestamp));
            }
        }
        
        builtinCommandOutputEdit->append("<span style='color: #74b9ff;'>---</span>");
        
        // 自动滚动到底部
        QTextCursor cursor = builtinCommandOutputEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        builtinCommandOutputEdit->setTextCursor(cursor);
        
        executeBuiltinCommandButton->setEnabled(true);
        
        if (builtinCommandProcess) {
            builtinCommandProcess->deleteLater();
            builtinCommandProcess = nullptr;
        }
    });
    
    // 禁用执行按钮防止重复执行
    executeBuiltinCommandButton->setEnabled(false);
    
    // 设置进程环境
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    builtinCommandProcess->setProcessEnvironment(env);
    
    // 在Windows下使用cmd执行，在Linux/macOS下使用sh执行
    QString program;
    QStringList arguments;
    
    // 检测命令类型
    bool needsInteraction = command.contains("ssh") && !command.contains("ssh-keygen");
    bool isWindowsBatchFile = command.endsWith(".bat");
    
    if (QSysInfo::productType() == "windows") {
        if (isWindowsBatchFile) {
            // 直接执行批处理文件 - 使用Windows路径格式
            QString windowsPath = QDir::toNativeSeparators(command);
            program = "cmd";
            arguments << "/c" << "start" << "cmd" << "/k" << windowsPath;
        } else if (needsInteraction) {
            // 在新的cmd窗口中运行SSH命令，允许用户交互
            program = "cmd";
            arguments << "/c" << "start" << "cmd" << "/k" << QString("chcp 65001 && echo [SSH命令执行] && echo 请输入服务器密码： && %1 && pause").arg(command);
        } else {
            program = "cmd";
            arguments << "/c" << "chcp 65001 >nul && " + command;  // 设置UTF-8编码
        }
    } else {
        if (needsInteraction) {
            // 在新的终端窗口中运行SSH命令
            program = "gnome-terminal";
            arguments << "--" << "bash" << "-c" << QString("echo '[SSH命令执行]' && echo '请输入服务器密码：' && %1; read -p '按Enter键继续...'").arg(command);
        } else {
            program = "sh";
            arguments << "-c" << command;
        }
    }
    
    // 启动进程
    builtinCommandProcess->start(program, arguments);
    
    if (!builtinCommandProcess->waitForStarted(5000)) {
        builtinCommandOutputEdit->append("<span style='color: #ff6b6b;'>[错误] 无法启动命令执行进程</span>");
        executeBuiltinCommandButton->setEnabled(true);
        
        if (builtinCommandProcess) {
            builtinCommandProcess->deleteLater();
            builtinCommandProcess = nullptr;
        }
    }
}

bool MainWindow::validateBasicSettings()
{
    if (ipLineEdit->text().isEmpty()) {
        return false;
    }
    
    if (usernameLineEdit->text().isEmpty()) {
        return false;
    }
    
    return true;
}

void MainWindow::setBuiltinCommand(const QString &command)
{
    builtinCommandLineEdit->setText(command);
    
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    builtinCommandOutputEdit->append(QString("<span style='color: #74b9ff;'>[%1] 命令已填入命令框</span>").arg(timestamp));
    builtinCommandOutputEdit->append(QString("<span style='color: #00b894;'>[提示] 点击'执行命令'按钮运行，或按Enter键执行</span>"));
    
    // 自动滚动到底部
    QTextCursor cursor = builtinCommandOutputEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    builtinCommandOutputEdit->setTextCursor(cursor);
    
    // 聚焦到命令输入框
    builtinCommandLineEdit->setFocus();
}

void MainWindow::showManualInstallationGuide()
{
    QString ip = ipLineEdit->text().trimmed();
    int port = portSpinBox->value();
    QString username = usernameLineEdit->text().trimmed();
    QString publicKey = readPublicKey();
    
    if (publicKey.isEmpty()) {
        QMessageBox::warning(this, "读取失败", "无法读取SSH公钥文件。");
        return;
    }
    
    // 显示手动安装指导
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("SSH公钥手动安装指导");
    msgBox.setIcon(QMessageBox::Information);
    
    QString instructions = QString(
        "自动安装失败，请按照以下步骤手动安装SSH公钥：\n\n"
        "🔧 手动安装步骤：\n"
        "1. 复制下方的公钥内容（已自动复制到剪贴板）\n"
        "2. 登录服务器：ssh -p %3 %1@%2\n"
        "3. 创建SSH目录：mkdir -p ~/.ssh\n"
        "4. 编辑授权文件：nano ~/.ssh/authorized_keys\n"
        "5. 将公钥内容粘贴到文件末尾（一行完整内容）\n"
        "6. 保存文件并退出编辑器\n"
        "7. 设置正确权限：\n"
        "   chmod 700 ~/.ssh\n"
        "   chmod 600 ~/.ssh/authorized_keys\n\n"
        "📋 验证安装：\n"
        "退出服务器后，执行：ssh -p %3 %1@%2\n"
        "如果不需要密码直接登录，说明配置成功！\n\n"
        "💾 公钥内容（已复制到剪贴板）："
    ).arg(username).arg(ip).arg(port);
    
    msgBox.setText(instructions);
    msgBox.setDetailedText(publicKey);
    
    // 自动复制公钥到剪贴板
    QApplication::clipboard()->setText(publicKey);
    
    QPushButton *testButton = msgBox.addButton("测试连接", QMessageBox::ActionRole);
    QPushButton *helpButton = msgBox.addButton("故障排除", QMessageBox::HelpRole);
    msgBox.addButton("关闭", QMessageBox::AcceptRole);
    
    msgBox.exec();
    
    QAbstractButton *clickedButton = msgBox.clickedButton();
    
    if (clickedButton == testButton) {
        logMessage("[手动安装后] 用户请求测试连接");
        onTestConnection();
    } else if (clickedButton == helpButton) {
        showSSHTroubleshooting();
    }
    
    logMessage("已显示SSH公钥手动安装指导");
}

QString MainWindow::generateReliableSSHInstallCommand(const QString &username, const QString &ip, int port, const QString &publicKey)
{
    // 使用SSH生成的公钥文件路径，而不是临时文件
    Q_UNUSED(publicKey); // 标记publicKey参数为未使用，避免编译警告
    QString pubKeyPath = getSSHPublicKeyPath();
    
    // 检查SSH公钥文件是否存在
    if (!QFile::exists(pubKeyPath)) {
        qDebug() << "[错误] SSH公钥文件不存在:" << pubKeyPath;
        return QString("echo ERROR: SSH public key file not found: %1").arg(pubKeyPath);
    }
    
    // 验证公钥文件内容
    QFile keyFile(pubKeyPath);
    if (keyFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = keyFile.readAll();
        keyFile.close();
        
        qDebug() << "[调试] 使用SSH公钥文件:" << pubKeyPath;
        qDebug() << "[调试] 文件大小:" << content.length() << "字节";
        qDebug() << "[调试] 公钥内容前50字符:" << content.left(50);
        
        if (content.isEmpty()) {
            qDebug() << "[警告] SSH公钥文件内容为空！";
            return QString("echo ERROR: Empty SSH public key file");
        }
    } else {
        qDebug() << "[错误] 无法读取SSH公钥文件:" << pubKeyPath;
        return QString("echo ERROR: Cannot read SSH public key file");
    }
    
    // 获取Python脚本路径 - 使用程序同级目录
    QString execDir = QCoreApplication::applicationDirPath();
    QString scriptPath = QDir(execDir).absoluteFilePath("install_ssh_key.py");
    
    // 检查Python脚本是否存在
    if (!QFile::exists(scriptPath)) {
        qDebug() << "[错误] Python脚本不存在:" << scriptPath;
        return QString("echo ERROR: Python script not found: %1").arg(scriptPath);
    }
    
    qDebug() << "[信息] 找到Python脚本:" << scriptPath;
    
    // 生成Python命令 - 使用SSH生成的公钥文件路径
    QString pythonCommand;
    if (QSysInfo::productType() == "windows") {
        // Windows下使用python，使用原生路径分隔符
        // 注意：不在这里添加引号，让executeSSHWithSshpass函数中的cmd处理路径转义
        pythonCommand = QString("python install_ssh_key.py --host %1 --port %2 --user %3 --key-file %4")
                       .arg(ip).arg(port).arg(username).arg(QDir::toNativeSeparators(pubKeyPath));
    } else {
        // Linux/macOS下使用python3
        pythonCommand = QString("python3 install_ssh_key.py --host %1 --port %2 --user %3 --key-file \"%4\"")
                       .arg(ip).arg(port).arg(username).arg(pubKeyPath);
    }
    
    return pythonCommand;
}

// ==================== SSH密码输入功能实现 ====================

void MainWindow::executeSSHCommandWithPassword(const QString &command)
{
    if (waitingForPassword) {
        builtinCommandOutputEdit->append("<span style='color: #ffa500;'>[警告] 正在等待密码输入，请先完成当前操作</span>");
        return;
    }
    
    // 保存待执行的命令
    pendingSSHCommand = command;
    waitingForPassword = true;
    
    // 在输出区域显示执行的命令
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    builtinCommandOutputEdit->append(QString("<span style='color: #74b9ff;'>[%1] $ %2</span>").arg(timestamp).arg(command));
    builtinCommandOutputEdit->append("<span style='color: #00b894;'>[SSH] 检测到SSH命令，请在下方输入服务器密码</span>");
    
    // 显示密码输入界面
    showPasswordInput("请输入SSH服务器密码：");
    
    // 禁用命令执行按钮
    executeBuiltinCommandButton->setEnabled(false);
}

void MainWindow::showPasswordInput(const QString &prompt)
{
    passwordPromptLabel->setText(prompt);
    sshPasswordLineEdit->clear();
    passwordInputWidget->setVisible(true);
    sshPasswordLineEdit->setFocus();
    
    // 自动滚动到底部以确保密码输入框可见
    QTextCursor cursor = builtinCommandOutputEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    builtinCommandOutputEdit->setTextCursor(cursor);
}

void MainWindow::hidePasswordInput()
{
    passwordInputWidget->setVisible(false);
    sshPasswordLineEdit->clear();
    waitingForPassword = false;
    executeBuiltinCommandButton->setEnabled(true);
}

void MainWindow::processPasswordInput(const QString &password)
{
    if (password.isEmpty()) {
        builtinCommandOutputEdit->append("<span style='color: #ff6b6b;'>[错误] 密码不能为空</span>");
        return;
    }
    
    if (builtinCommandProcess) {
        builtinCommandProcess->deleteLater();
        builtinCommandProcess = nullptr;
    }
    
    // 显示正在连接的消息和调试信息
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    builtinCommandOutputEdit->append(QString("<span style='color: #74b9ff;'>[%1] 正在连接SSH服务器...</span>").arg(timestamp));
    
    // 显示即将执行的命令（用于调试）
    builtinCommandOutputEdit->append(QString("<span style='color: #6c5ce7;'>[调试] 执行命令: %1</span>").arg(pendingSSHCommand));
    
    // 验证SSH公钥文件内容
    QString pubKeyPath = getSSHPublicKeyPath();
    QFile checkFile(pubKeyPath);
    if (checkFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = checkFile.readAll();
        checkFile.close();
        builtinCommandOutputEdit->append(QString("<span style='color: #6c5ce7;'>[调试] SSH公钥文件大小: %1 字节</span>").arg(content.length()));
        builtinCommandOutputEdit->append(QString("<span style='color: #6c5ce7;'>[调试] 文件路径: %1</span>").arg(QDir::toNativeSeparators(pubKeyPath)));
        if (content.isEmpty()) {
            builtinCommandOutputEdit->append("<span style='color: #ff6b6b;'>[警告] SSH公钥文件为空！</span>");
        } else {
            builtinCommandOutputEdit->append(QString("<span style='color: #6c5ce7;'>[调试] 公钥内容开头: %1...</span>").arg(content.left(50)));
        }
    } else {
        builtinCommandOutputEdit->append("<span style='color: #ff6b6b;'>[错误] 无法读取SSH公钥文件</span>");
    }
    
    // 显示即将执行的SSH命令详情
    builtinCommandOutputEdit->append(QString("<span style='color: #6c5ce7;'>[调试] SSH命令: %1</span>").arg(pendingSSHCommand));
    
    // 隐藏密码输入框
    hidePasswordInput();
    
    // 统一使用新的SSH密码处理方式
    executeSSHWithSshpass(password);
}

void MainWindow::executeSSHWithSshpass(const QString &password)
{
    // 直接执行待处理的SSH命令，不再依赖配置信息
    if (pendingSSHCommand.isEmpty()) {
        builtinCommandOutputEdit->append("<span style='color: #ff6b6b;'>[错误] 没有待执行的SSH命令</span>");
        return;
    }
    
    // 如果是Python脚本命令，需要添加密码参数
    QString finalCommand = pendingSSHCommand;
    if (finalCommand.contains("install_ssh_key.py")) {
        finalCommand += QString(" --password \"%1\"").arg(password);
        
        // 添加调试：验证密码参数（显示密码长度而不是内容）
        builtinCommandOutputEdit->append(QString("<span style='color: #6c5ce7;'>[调试] 密码参数长度: %1 字符</span>").arg(password.length()));
        builtinCommandOutputEdit->append(QString("<span style='color: #6c5ce7;'>[调试] 密码首字符: %1</span>").arg(password.isEmpty() ? "空" : password.left(1)));
    }
    
    builtinCommandProcess = new QProcess(this);
    
    // 连接信号处理实时输出
    connect(builtinCommandProcess, &QProcess::readyReadStandardOutput, 
            this, [this]() {
        QByteArray data = builtinCommandProcess->readAllStandardOutput();
        if (!data.isEmpty()) {
            QString output = QString::fromLocal8Bit(data); // Windows下使用本地编码
            if (!output.isEmpty()) {
                // 清理和格式化输出
                output = output.trimmed();
                if (!output.isEmpty()) {
                    builtinCommandOutputEdit->append(QString("<span style='color: #ffffff;'>%1</span>").arg(output));
                }
            }
        }
        // 自动滚动到底部
        QTextCursor cursor = builtinCommandOutputEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        builtinCommandOutputEdit->setTextCursor(cursor);
    });
    
    connect(builtinCommandProcess, &QProcess::readyReadStandardError, 
            this, [this]() {
        QByteArray data = builtinCommandProcess->readAllStandardError();
        if (!data.isEmpty()) {
            QString error = QString::fromLocal8Bit(data); // Windows下使用本地编码
            
            if (!error.isEmpty()) {
                error = error.trimmed();
                if (!error.isEmpty()) {
                    builtinCommandOutputEdit->append(QString("<span style='color: #ff7675;'>%1</span>").arg(error));
                }
            }
        }
        QTextCursor cursor = builtinCommandOutputEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        builtinCommandOutputEdit->setTextCursor(cursor);
    });
    
    // 连接完成信号
    connect(builtinCommandProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
                
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
        
        if (exitStatus == QProcess::NormalExit) {
            if (exitCode == 0) {
                builtinCommandOutputEdit->append(QString("<span style='color: #00b894;'>[%1] SSH公钥安装完成 (退出码: %2)</span>")
                                               .arg(timestamp).arg(exitCode));
                builtinCommandOutputEdit->append("<span style='color: #00b894;'>[提示] SSH公钥安装可能已完成，建议测试连接验证</span>");
                
                QTimer::singleShot(2000, this, [this]() {
                    int ret = QMessageBox::question(this, "SSH公钥安装", 
                        "SSH公钥安装命令已执行完成。\n\n"
                        "是否立即测试SSH连接验证安装结果？",
                        QMessageBox::Yes | QMessageBox::No,
                        QMessageBox::Yes);
                    
                    if (ret == QMessageBox::Yes) {
                        logMessage("[SSH安装后] 用户选择测试连接");
                        onTestConnection();
                    }
                });
            } else {
                builtinCommandOutputEdit->append(QString("<span style='color: #e17055;'>[%1] SSH命令执行有错误 (退出码: %2)</span>")
                                               .arg(timestamp).arg(exitCode));
                
                // 分析常见错误
                if (exitCode == 255) {
                    builtinCommandOutputEdit->append("<span style='color: #ffa500;'>[分析] 可能的原因：密码错误、网络连接问题或SSH服务未启动</span>");
                } else if (exitCode == 5) {
                    builtinCommandOutputEdit->append("<span style='color: #ffa500;'>[分析] 密码认证失败，请检查密码是否正确</span>");
                } else if (exitCode == 1) {
                    builtinCommandOutputEdit->append("<span style='color: #ffa500;'>[分析] 可能的原因：权限不足或目标路径不存在</span>");
                }
            }
        } else {
            builtinCommandOutputEdit->append(QString("<span style='color: #ff6b6b;'>[%1] SSH命令执行异常终止</span>").arg(timestamp));
        }
        
        builtinCommandOutputEdit->append("<span style='color: #74b9ff;'>---</span>");
        
        // 自动滚动到底部
        QTextCursor cursor = builtinCommandOutputEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        builtinCommandOutputEdit->setTextCursor(cursor);
        
        // 延迟清理批处理文件，确保批处理文件执行完成
        // 注意：批处理文件会在执行完成后自动删除自己
        QTimer::singleShot(60000, this, [this]() {
            QString execDir = QCoreApplication::applicationDirPath();
            QString batFile = QDir(execDir).absoluteFilePath("ssh_install_auto.bat");
            // 如果批处理文件没有自动删除，手动清理
            if (QFile::exists(batFile)) {
                QFile::remove(batFile);
            }
        });
        
        if (builtinCommandProcess) {
            builtinCommandProcess->deleteLater();
            builtinCommandProcess = nullptr;
        }
    });
    
    // 设置进程环境
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG", "zh_CN.UTF-8");
    
    // 添加调试：显示关键环境变量
    builtinCommandOutputEdit->append(QString("<span style='color: #6c5ce7;'>[调试] PATH环境变量: %1</span>").arg(env.value("PATH")));
    builtinCommandOutputEdit->append(QString("<span style='color: #6c5ce7;'>[调试] PYTHON路径检查: %1</span>").arg(env.value("PYTHONPATH", "未设置")));
    
    builtinCommandProcess->setProcessEnvironment(env);
    
    QString program;
    QStringList arguments;
    
    if (QSysInfo::productType() == "windows") {
        // Windows下启动新的cmd窗口并自动执行命令
        program = "cmd";
        
        // 创建批处理文件到程序执行目录
        QString execDir = QCoreApplication::applicationDirPath();
        QString batFile = QDir(execDir).absoluteFilePath("ssh_install_auto.bat");
        
        QFile file(batFile);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out.setCodec("GBK"); // Windows批处理文件使用GBK编码
            
            // 写入批处理命令
            out << "@echo off\n";
            out << "title SSH公钥自动安装\n";
            out << "color 0a\n"; // 设置绿色字体
            out << "echo ========================================\n";
            out << "echo        SSH公钥自动安装工具\n";
            out << "echo ========================================\n";
            out << "echo.\n";
            QString nativeExecDir = QDir::toNativeSeparators(execDir);
            out << "echo 程序目录: " << nativeExecDir << "\n";
            out << "echo 脚本位置: %~dp0\n";
            out << "echo.\n";
            out << "echo 正在执行SSH公钥安装命令...\n";
            out << "echo 命令: " << finalCommand << "\n";
            out << "echo.\n";
            out << "echo [执行中] 请等待...\n";
            out << "echo.\n";
            out << finalCommand << "\n";
            out << "echo.\n";
            out << "echo ========================================\n";
            out << "if %ERRORLEVEL% EQU 0 (\n";
            out << "    echo [✓成功] SSH公钥安装完成！\n";
            out << "    echo.\n";
            out << "    echo 现在您可以使用SSH密钥连接服务器：\n";
            out << "    echo ssh -p 22 ubuntu@1.13.80.192\n";
            out << ") else (\n";
            out << "    echo [✗失败] SSH公钥安装失败，错误码: %ERRORLEVEL%\n";
            out << "    echo.\n";
            out << "    echo 请检查：\n";
            out << "    echo 1. 网络连接是否正常\n";
            out << "    echo 2. 服务器地址和端口是否正确\n";
            out << "    echo 3. 用户名和密码是否正确\n";
            out << ")\n";
            out << "echo ========================================\n";
            out << "echo.\n";
            out << "echo 按任意键关闭窗口...\n";
            out << "pause >nul\n";
            out << "del \"%~f0\" >nul 2>&1\n"; // 执行完成后自动删除批处理文件
            
            file.close();
            
            // 启动新的cmd窗口执行批处理文件
            QString nativeBatFile = QDir::toNativeSeparators(batFile);
            arguments << "/c" << nativeBatFile;
            
            builtinCommandOutputEdit->append("<span style='color: #00b894;'>[系统] 正在启动独立的CMD窗口执行SSH安装命令...</span>");
            builtinCommandOutputEdit->append("<span style='color: #74b9ff;'>[提示] 请在弹出的CMD窗口中查看执行结果</span>");
            builtinCommandOutputEdit->append(QString("<span style='color: #6c5ce7;'>[调试] 批处理文件: %1</span>").arg(nativeBatFile));
            builtinCommandOutputEdit->append(QString("<span style='color: #6c5ce7;'>[调试] 执行命令: %1</span>").arg(finalCommand));
        } else {
            builtinCommandOutputEdit->append("<span style='color: #ff6b6b;'>[错误] 无法创建批处理文件</span>");
            return;
        }
    } else {
        // Linux/macOS下使用sh执行
        program = "sh";
        arguments << "-c" << finalCommand;
    }
    
    // 设置工作目录为程序目录（确保能找到install_ssh_key.py和批处理文件）
    QString execDir = QCoreApplication::applicationDirPath();
    builtinCommandProcess->setWorkingDirectory(execDir);
    
    // 添加调试：显示工作目录和文件信息
    builtinCommandOutputEdit->append(QString("<span style='color: #6c5ce7;'>[调试] 工作目录: %1</span>").arg(execDir));
    
    // 验证Python脚本文件是否存在
    QString scriptPath = QDir(execDir).absoluteFilePath("install_ssh_key.py");
    builtinCommandOutputEdit->append(QString("<span style='color: #6c5ce7;'>[调试] Python脚本路径: %1</span>").arg(scriptPath));
    builtinCommandOutputEdit->append(QString("<span style='color: #6c5ce7;'>[调试] Python脚本存在: %1</span>").arg(QFile::exists(scriptPath) ? "是" : "否"));
    
    // 启动进程
    builtinCommandProcess->start(program, arguments);
    
    // 启动进程
    if (builtinCommandProcess->waitForStarted(3000)) {
        if (QSysInfo::productType() == "windows") {
            builtinCommandOutputEdit->append("<span style='color: #00b894;'>[系统] CMD窗口已启动，SSH公钥安装正在独立窗口中执行...</span>");
            builtinCommandOutputEdit->append("<span style='color: #74b9ff;'>[说明] 安装过程将在弹出的CMD窗口中显示，完成后窗口会自动关闭</span>");
            
            // 立即清理进程，因为cmd会立即返回
            QTimer::singleShot(2000, this, [this]() {
                if (builtinCommandProcess) {
                    builtinCommandProcess->deleteLater();
                    builtinCommandProcess = nullptr;
                }
            });
        } else {
            builtinCommandOutputEdit->append("<span style='color: #00b894;'>[系统] Python脚本已启动，正在处理SSH连接...</span>");
            
            // 设置超时保护 - 60秒后强制终止进程
            QTimer::singleShot(60000, this, [this]() {
                if (builtinCommandProcess && builtinCommandProcess->state() == QProcess::Running) {
                    builtinCommandOutputEdit->append("<span style='color: #ff6b6b;'>[警告] Python脚本执行超时，正在终止进程...</span>");
                    builtinCommandProcess->kill();
                }
            });
        }
    } else {
        if (QSysInfo::productType() == "windows") {
            builtinCommandOutputEdit->append("<span style='color: #ff6b6b;'>[错误] 无法启动CMD窗口</span>");
        } else {
            builtinCommandOutputEdit->append("<span style='color: #ff6b6b;'>[错误] 无法启动Python脚本</span>");
        }
        
        if (builtinCommandProcess) {
            builtinCommandProcess->deleteLater();
            builtinCommandProcess = nullptr;
        }
    }
}

void MainWindow::executeDirectSSHCommand(const QString &password)
{
    builtinCommandProcess = new QProcess(this);
    
    // 连接信号处理实时输出
    connect(builtinCommandProcess, &QProcess::readyReadStandardOutput, 
            this, [this]() {
        QByteArray data = builtinCommandProcess->readAllStandardOutput();
        if (!data.isEmpty()) {
            QString output = QString::fromLocal8Bit(data);
            if (!output.isEmpty()) {
                output = output.trimmed();
                if (!output.isEmpty()) {
                    builtinCommandOutputEdit->append(QString("<span style='color: #ffffff;'>%1</span>").arg(output));
                }
            }
        }
        QTextCursor cursor = builtinCommandOutputEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        builtinCommandOutputEdit->setTextCursor(cursor);
    });
    
    connect(builtinCommandProcess, &QProcess::readyReadStandardError, 
            this, [this]() {
        QByteArray data = builtinCommandProcess->readAllStandardError();
        if (!data.isEmpty()) {
            QString error = QString::fromLocal8Bit(data);
            if (!error.isEmpty()) {
                error = error.trimmed();
                if (!error.isEmpty()) {
                    builtinCommandOutputEdit->append(QString("<span style='color: #ff7675;'>%1</span>").arg(error));
                }
            }
        }
        QTextCursor cursor = builtinCommandOutputEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        builtinCommandOutputEdit->setTextCursor(cursor);
    });
    
    // 连接完成信号
    connect(builtinCommandProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
                
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
        
        if (exitStatus == QProcess::NormalExit) {
            if (exitCode == 0) {
                builtinCommandOutputEdit->append(QString("<span style='color: #00b894;'>[%1] SSH命令执行完成 (退出码: %2)</span>")
                                               .arg(timestamp).arg(exitCode));
            } else {
                builtinCommandOutputEdit->append(QString("<span style='color: #e17055;'>[%1] SSH命令执行有错误 (退出码: %2)</span>")
                                               .arg(timestamp).arg(exitCode));
            }
        } else {
            builtinCommandOutputEdit->append(QString("<span style='color: #ff6b6b;'>[%1] SSH命令执行异常终止</span>").arg(timestamp));
        }
        
        builtinCommandOutputEdit->append("<span style='color: #74b9ff;'>---</span>");
        
        QTextCursor cursor = builtinCommandOutputEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        builtinCommandOutputEdit->setTextCursor(cursor);
        
        if (builtinCommandProcess) {
            builtinCommandProcess->deleteLater();
            builtinCommandProcess = nullptr;
        }
    });
    
    QString program;
    QStringList arguments;
    
    if (QSysInfo::productType() == "windows") {
        program = "cmd";
        arguments << "/c" << pendingSSHCommand;
    } else {
        program = "sh";
        arguments << "-c" << pendingSSHCommand;
    }
    
    builtinCommandProcess->start(program, arguments);
    
    if (builtinCommandProcess->waitForStarted(3000)) {
        QTimer::singleShot(2000, this, [this, password]() {
            if (builtinCommandProcess && builtinCommandProcess->state() == QProcess::Running) {
                builtinCommandProcess->write((password + "\r\n").toLocal8Bit());
            }
        });
    } else {
        builtinCommandOutputEdit->append("<span style='color: #ff6b6b;'>[错误] 无法启动SSH进程</span>");
        
        if (builtinCommandProcess) {
            builtinCommandProcess->deleteLater();
            builtinCommandProcess = nullptr;
        }
    }
}

void MainWindow::onPasswordInputEnterPressed()
{
    onPasswordInputFinished();
}

void MainWindow::onPasswordInputFinished()
{
    QString password = sshPasswordLineEdit->text();
    if (password.isEmpty()) {
        builtinCommandOutputEdit->append("<span style='color: #ff6b6b;'>[错误] 请输入密码</span>");
        sshPasswordLineEdit->setFocus();
        return;
    }
    
    processPasswordInput(password);
}

void MainWindow::onPasswordInputCanceled()
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    builtinCommandOutputEdit->append(QString("<span style='color: #ffa500;'>[%1] 用户取消了密码输入，SSH命令执行已中止</span>").arg(timestamp));
    
    hidePasswordInput();
    pendingSSHCommand.clear();
}

// ==========================================
// 一体化SSH密钥生成和部署功能
// ==========================================

void MainWindow::onGenerateAndDeploySSHKey()
{
    if (!validateSSHSettings()) {
        QMessageBox::warning(this, "连接信息不完整", 
            "请先完整配置服务器连接信息：\n"
            "- 服务器IP地址\n"
            "- 用户名\n"
            "- 端口（如果不是默认22端口）");
        return;
    }
    
    QString keyPath = getSSHKeyPath();
    bool keyExists = checkSSHKeyExists();
    
    if (keyExists) {
        int ret = QMessageBox::question(this, "SSH密钥已存在", 
            QString("检测到SSH密钥已存在：\n%1\n\n"
                   "选择操作：\n"
                   "• 是：重新生成密钥并部署到服务器\n"
                   "• 否：使用现有密钥直接部署到服务器\n"
                   "• 取消：取消操作")
            .arg(keyPath),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
            QMessageBox::No);
        
        if (ret == QMessageBox::Cancel) {
            logMessage("用户取消了SSH密钥生成和部署操作");
            return;
        } else if (ret == QMessageBox::Yes) {
            // 重新生成密钥
            executeSSHKeyGenerationAndDeployment();
        } else {
            // 使用现有密钥直接部署
            logMessage("[一体化部署] 使用现有SSH密钥进行部署...");
            installPublicKeyToServer();
        }
    } else {
        // 显示确认对话框
        QString ip = ipLineEdit->text().trimmed();
        int port = portSpinBox->value();
        QString username = usernameLineEdit->text().trimmed();
        
        int ret = QMessageBox::information(this, "一体化SSH密钥配置", 
            QString("程序将自动完成以下操作：\n\n"
                   "1️⃣ 生成新的SSH密钥对（RSA 4096位）\n"
                   "2️⃣ 自动部署公钥到服务器\n"
                   "3️⃣ 配置服务器免密码登录\n\n"
                   "目标服务器：%1@%2:%3\n\n"
                   "整个过程只需输入一次服务器密码。\n"
                   "配置完成后即可使用SSH密钥免密码登录。\n\n"
                   "是否开始自动配置？")
            .arg(username).arg(ip).arg(port),
            QMessageBox::Ok | QMessageBox::Cancel,
            QMessageBox::Ok);
        
        if (ret == QMessageBox::Ok) {
            executeSSHKeyGenerationAndDeployment();
        } else {
            logMessage("用户取消了一体化SSH密钥配置");
        }
    }
}

void MainWindow::executeSSHKeyGenerationAndDeployment()
{
    logMessage("[一体化配置] 开始SSH密钥生成和部署流程...");
    
    // 设置一体化流程标志
    isGeneratingAndDeploying = true;
    
    // 开始生成SSH密钥
    generateSSHKey();
}

bool MainWindow::validateSSHSettings()
{
    if (ipLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, "设置错误", "请输入服务器IP地址");
        ipLineEdit->setFocus();
        return false;
    }
    
    if (usernameLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, "设置错误", "请输入用户名");
        usernameLineEdit->setFocus();
        return false;
    }
    
    // SSH密钥功能不需要检查文件选择，只需要基本的连接信息
    // 认证方式（SSH密钥或密码）会在具体功能中按需检查
    
    return true;
}

void MainWindow::executeSSHWithPassword(const QString &command, const QString &password)
{
    // 直接使用已有密码执行SSH命令，不需要密码输入界面
    logMessage("[智能部署] 使用服务器连接密码自动执行SSH命令");
    
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    builtinCommandOutputEdit->append(QString("<span style='color: #74b9ff;'>[%1] $ %2</span>").arg(timestamp).arg(command));
    builtinCommandOutputEdit->append("<span style='color: #00b894;'>[SSH] 使用服务器连接设置中的密码进行认证</span>");
    
    // 验证SSH公钥文件内容
    QString pubKeyPath = getSSHPublicKeyPath();
    QFile checkFile(pubKeyPath);
    if (checkFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = checkFile.readAll();
        checkFile.close();
        builtinCommandOutputEdit->append(QString("<span style='color: #6c5ce7;'>[调试] SSH公钥文件大小: %1 字节</span>").arg(content.length()));
        builtinCommandOutputEdit->append(QString("<span style='color: #6c5ce7;'>[调试] 文件路径: %1</span>").arg(QDir::toNativeSeparators(pubKeyPath)));
        if (content.isEmpty()) {
            builtinCommandOutputEdit->append("<span style='color: #ff6b6b;'>[警告] SSH公钥文件为空！</span>");
        } else {
            builtinCommandOutputEdit->append(QString("<span style='color: #6c5ce7;'>[调试] 公钥内容开头: %1...</span>").arg(content.left(50)));
        }
    } else {
        builtinCommandOutputEdit->append("<span style='color: #ff6b6b;'>[错误] 无法读取SSH公钥文件</span>");
    }
    
    // 显示即将执行的SSH命令详情
    builtinCommandOutputEdit->append(QString("<span style='color: #6c5ce7;'>[调试] SSH命令: %1</span>").arg(command));
    
    // 直接执行SSH命令
    executeSSHWithDirectPassword(command, password);
}

void MainWindow::executeSSHWithDirectPassword(const QString &command, const QString &password)
{
    if (builtinCommandProcess) {
        builtinCommandProcess->deleteLater();
        builtinCommandProcess = nullptr;
    }
    
    // 如果是Python脚本命令，需要添加密码参数
    QString finalCommand = command;
    if (finalCommand.contains("install_ssh_key.py")) {
        finalCommand += QString(" --password \"%1\"").arg(password);
        
        // 添加调试：验证密码参数（显示密码长度而不是内容）
        builtinCommandOutputEdit->append(QString("<span style='color: #6c5ce7;'>[调试] 密码参数长度: %1 字符</span>").arg(password.length()));
        builtinCommandOutputEdit->append(QString("<span style='color: #6c5ce7;'>[调试] 密码首字符: %1</span>").arg(password.isEmpty() ? "空" : password.left(1)));
    }
    
    builtinCommandProcess = new QProcess(this);
    
    // 连接信号处理实时输出
    connect(builtinCommandProcess, &QProcess::readyReadStandardOutput, 
            this, [this]() {
        QByteArray data = builtinCommandProcess->readAllStandardOutput();
        if (!data.isEmpty()) {
            QString output = QString::fromLocal8Bit(data); // Windows下使用本地编码
            if (!output.isEmpty()) {
                // 清理和格式化输出
                output = output.trimmed();
                if (!output.isEmpty()) {
                    builtinCommandOutputEdit->append(QString("<span style='color: #ffffff;'>%1</span>").arg(output));
                }
            }
        }
        // 自动滚动到底部
        QTextCursor cursor = builtinCommandOutputEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        builtinCommandOutputEdit->setTextCursor(cursor);
    });
    
    connect(builtinCommandProcess, &QProcess::readyReadStandardError, 
            this, [this]() {
        QByteArray data = builtinCommandProcess->readAllStandardError();
        if (!data.isEmpty()) {
            QString error = QString::fromLocal8Bit(data); // Windows下使用本地编码
            
            if (!error.isEmpty()) {
                error = error.trimmed();
                if (!error.isEmpty()) {
                    builtinCommandOutputEdit->append(QString("<span style='color: #ff7675;'>%1</span>").arg(error));
                }
            }
        }
        QTextCursor cursor = builtinCommandOutputEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        builtinCommandOutputEdit->setTextCursor(cursor);
    });
    
    // 连接完成信号
    connect(builtinCommandProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
                
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
        
        if (exitStatus == QProcess::NormalExit) {
            if (exitCode == 0) {
                builtinCommandOutputEdit->append(QString("<span style='color: #00b894;'>[%1] SSH公钥安装完成 (退出码: %2)</span>")
                                               .arg(timestamp).arg(exitCode));
                builtinCommandOutputEdit->append("<span style='color: #00b894;'>[提示] SSH公钥安装可能已完成，建议测试连接验证</span>");
                
                QTimer::singleShot(2000, this, [this]() {
                    int ret = QMessageBox::question(this, "SSH公钥安装", 
                        "SSH公钥安装命令已执行完成。\n\n"
                        "是否立即测试SSH连接验证安装结果？",
                        QMessageBox::Yes | QMessageBox::No,
                        QMessageBox::Yes);
                    
                    if (ret == QMessageBox::Yes) {
                        logMessage("[SSH安装后] 用户选择测试连接");
                        onTestConnection();
                    }
                });
            } else {
                builtinCommandOutputEdit->append(QString("<span style='color: #e17055;'>[%1] SSH命令执行有错误 (退出码: %2)</span>")
                                               .arg(timestamp).arg(exitCode));
                
                // 分析常见错误
                if (exitCode == 255) {
                    builtinCommandOutputEdit->append("<span style='color: #ffa500;'>[分析] 可能的原因：密码错误、网络连接问题或SSH服务未启动</span>");
                } else if (exitCode == 5) {
                    builtinCommandOutputEdit->append("<span style='color: #ffa500;'>[分析] 密码认证失败，请检查密码是否正确</span>");
                } else if (exitCode == 1) {
                    builtinCommandOutputEdit->append("<span style='color: #ffa500;'>[分析] 可能的原因：权限不足或目标路径不存在</span>");
                }
            }
        } else {
            builtinCommandOutputEdit->append(QString("<span style='color: #ff6b6b;'>[%1] SSH命令执行异常终止</span>").arg(timestamp));
        }
        
        builtinCommandOutputEdit->append("<span style='color: #74b9ff;'>---</span>");
        
        // 自动滚动到底部
        QTextCursor cursor = builtinCommandOutputEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        builtinCommandOutputEdit->setTextCursor(cursor);
        
        // 重置一体化流程标志
        if (isGeneratingAndDeploying) {
            isGeneratingAndDeploying = false;
            logMessage("[一体化部署] SSH密钥生成和部署流程完成");
        }
        
        if (builtinCommandProcess) {
            builtinCommandProcess->deleteLater();
            builtinCommandProcess = nullptr;
        }
    });
    
    // 设置进程环境
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG", "zh_CN.UTF-8");
    
    builtinCommandProcess->setProcessEnvironment(env);
    
    QString program;
    QStringList arguments;
    
    if (QSysInfo::productType() == "windows") {
        // Windows下启动新的cmd窗口并自动执行命令
        program = "cmd";
        
        // 创建批处理文件到程序执行目录
        QString execDir = QCoreApplication::applicationDirPath();
        QString batFile = QDir(execDir).absoluteFilePath("ssh_install_auto.bat");
        
        QFile file(batFile);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out.setCodec("GBK"); // Windows批处理文件使用GBK编码
            
            // 写入批处理命令
            out << "@echo off\n";
            out << "title SSH公钥自动安装\n";
            out << "color 0a\n"; // 设置绿色字体
            out << "echo ========================================\n";
            out << "echo        SSH公钥自动安装工具\n";
            out << "echo ========================================\n";
            out << "echo.\n";
            QString nativeExecDir = QDir::toNativeSeparators(execDir);
            out << "echo 程序目录: " << nativeExecDir << "\n";
            out << "echo 脚本位置: %~dp0\n";
            out << "echo.\n";
            out << "echo 正在执行SSH公钥安装命令...\n";
            out << "echo 命令: " << finalCommand << "\n";
            out << "echo.\n";
            out << "echo [执行中] 请等待...\n";
            out << "echo.\n";
            out << finalCommand << "\n";
            out << "echo.\n";
            out << "echo ========================================\n";
            out << "if %ERRORLEVEL% EQU 0 (\n";
            out << "    echo [✓成功] SSH公钥安装完成！\n";
            out << "    echo.\n";
            out << "    echo 现在您可以使用SSH密钥连接服务器：\n";
            out << "    echo ssh -p 22 ubuntu@1.13.80.192\n";
            out << ") else (\n";
            out << "    echo [✗失败] SSH公钥安装失败，错误码: %ERRORLEVEL%\n";
            out << "    echo.\n";
            out << "    echo 请检查：\n";
            out << "    echo 1. 网络连接是否正常\n";
            out << "    echo 2. 服务器地址和端口是否正确\n";
            out << "    echo 3. 用户名和密码是否正确\n";
            out << ")\n";
            out << "echo ========================================\n";
            out << "echo.\n";
            out << "echo 按任意键关闭窗口...\n";
            out << "pause >nul\n";
            out << "del \"%~f0\" >nul 2>&1\n"; // 执行完成后自动删除批处理文件
            
            file.close();
            
            // 启动新的cmd窗口执行批处理文件
            QString nativeBatFile = QDir::toNativeSeparators(batFile);
            arguments << "/c" << nativeBatFile;
            
            builtinCommandOutputEdit->append("<span style='color: #00b894;'>[系统] 正在启动独立的CMD窗口执行SSH安装命令...</span>");
            builtinCommandOutputEdit->append("<span style='color: #74b9ff;'>[提示] 请在弹出的CMD窗口中查看执行结果</span>");
            builtinCommandOutputEdit->append(QString("<span style='color: #6c5ce7;'>[调试] 批处理文件: %1</span>").arg(nativeBatFile));
            builtinCommandOutputEdit->append(QString("<span style='color: #6c5ce7;'>[调试] 执行命令: %1</span>").arg(finalCommand));
        } else {
            builtinCommandOutputEdit->append("<span style='color: #ff6b6b;'>[错误] 无法创建批处理文件</span>");
            return;
        }
    } else {
        // Linux/macOS下使用sh执行
        program = "sh";
        arguments << "-c" << finalCommand;
    }
    
    // 设置工作目录为程序目录（确保能找到install_ssh_key.py和批处理文件）
    QString execDir = QCoreApplication::applicationDirPath();
    builtinCommandProcess->setWorkingDirectory(execDir);
    
    // 启动进程
    builtinCommandProcess->start(program, arguments);
    
    // 启动进程
    if (builtinCommandProcess->waitForStarted(3000)) {
        if (QSysInfo::productType() == "windows") {
            builtinCommandOutputEdit->append("<span style='color: #00b894;'>[系统] CMD窗口已启动，SSH公钥安装正在独立窗口中执行...</span>");
            builtinCommandOutputEdit->append("<span style='color: #74b9ff;'>[说明] 安装过程将在弹出的CMD窗口中显示，完成后窗口会自动关闭</span>");
            
            // 立即清理进程，因为cmd会立即返回
            QTimer::singleShot(2000, this, [this]() {
                if (builtinCommandProcess) {
                    builtinCommandProcess->deleteLater();
                    builtinCommandProcess = nullptr;
                }
            });
        } else {
            builtinCommandOutputEdit->append("<span style='color: #00b894;'>[系统] Python脚本已启动，正在处理SSH连接...</span>");
            
            // 设置超时保护 - 60秒后强制终止进程
            QTimer::singleShot(60000, this, [this]() {
                if (builtinCommandProcess && builtinCommandProcess->state() == QProcess::Running) {
                    builtinCommandOutputEdit->append("<span style='color: #ff6b6b;'>[警告] Python脚本执行超时，正在终止进程...</span>");
                    builtinCommandProcess->kill();
                }
            });
        }
    } else {
        if (QSysInfo::productType() == "windows") {
            builtinCommandOutputEdit->append("<span style='color: #ff6b6b;'>[错误] 无法启动CMD窗口</span>");
        } else {
            builtinCommandOutputEdit->append("<span style='color: #ff6b6b;'>[错误] 无法启动Python脚本</span>");
        }
        
        if (builtinCommandProcess) {
            builtinCommandProcess->deleteLater();
            builtinCommandProcess = nullptr;
        }
    }
}
