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
      upgradeKu5pProcess(nullptr), progressTimer(nullptr), timeoutTimer(nullptr), keyFile(nullptr),
      settingsDialog(nullptr), remoteDirectory("/media/sata/ue_data/")
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
    autoCleanLog = false;
    logRetentionDays = 30;
    qtExtractPath = "/mnt/qtfs";    // Qt软件升级默认解压路径
    sevEvExtractPath = "/mnt/mmcblk0p1";  // 7ev固件升级默认解压路径
    
    // 加载应用设置
    loadApplicationSettings();
    
    // 根据设置初始化界面
    if (showLogByDefault) {
        logTextEdit->show();
    } else {
        logTextEdit->hide();
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
    
    // 连接菜单动作
    connect(openSettingsAction, &QAction::triggered, this, &MainWindow::onOpenSettings);
    connect(saveSettingsAction, &QAction::triggered, this, &MainWindow::onMenuAction);
    connect(loadSettingsAction, &QAction::triggered, this, &MainWindow::onMenuAction);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onMenuAction);
    connect(toggleLogAction, &QAction::triggered, this, &MainWindow::onToggleLogView);
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
    env.insert("SSH_ASKPASS_REQUIRE", "never");
    testProcess->setProcessEnvironment(env);
    
    // 使用ssh命令测试连接
    QString program = "ssh";
    QStringList arguments;
    arguments << "-o" << "ConnectTimeout=10"
              << "-o" << "StrictHostKeyChecking=no"
              << "-o" << "UserKnownHostsFile=/dev/null"
              << "-o" << "PasswordAuthentication=yes"
              << "-p" << QString::number(port)
              << QString("%1@%2").arg(username).arg(ip)
              << "echo 'SSH连接测试成功'";
    
    logMessage(QString("[测试] 执行SSH测试命令"));
    
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
        logMessage("[提示] 请检查：1)服务器IP和端口 2)用户名和密码 3)SSH服务状态 4)防火墙设置");
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
    
    if (passwordLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, "设置错误", "请输入密码");
        passwordLineEdit->setFocus();
        return false;
    }
    
    if (selectedFilePath.isEmpty()) {
        QMessageBox::warning(this, "文件错误", "请先选择要上传的文件");
        return false;
    }
    
    if (!QFile::exists(selectedFilePath)) {
        QMessageBox::warning(this, "文件错误", "选择的文件不存在");
        return false;
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
    env.insert("SSH_ASKPASS_REQUIRE", "never");
    uploadProcess->setProcessEnvironment(env);
    
    // 构建SCP命令
    QString program = "scp";
    QStringList arguments;
    arguments << "-o" << "ConnectTimeout=30"
              << "-o" << "StrictHostKeyChecking=no"
              << "-o" << "UserKnownHostsFile=/dev/null"
              << "-o" << "PasswordAuthentication=yes"
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
        logMessage("日志面板已显示");
    } else {
        toggleLogAction->setText("显示日志(&L)");
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
    env.insert("SSH_ASKPASS_REQUIRE", "never");
    verifyProcess->setProcessEnvironment(env);
    
    // 构建SSH命令来计算远程文件MD5
    QString program = "ssh";
    QStringList arguments;
    arguments << "-o" << "ConnectTimeout=30"
              << "-o" << "StrictHostKeyChecking=no"
              << "-o" << "UserKnownHostsFile=/dev/null"
              << "-o" << "PasswordAuthentication=yes"
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
    env.insert("SSHPASS", passwordLineEdit->text());
    preCheck7evProcess->setProcessEnvironment(env);
    
    // 构建SSH命令
    QString program = "ssh";
    QStringList arguments;
    arguments << "-o" << "ConnectTimeout=30"
              << "-o" << "StrictHostKeyChecking=no"
              << "-o" << "UserKnownHostsFile=/dev/null"
              << "-o" << "PasswordAuthentication=yes"
              << "-p" << QString::number(portSpinBox->value())
              << QString("%1@%2").arg(usernameLineEdit->text()).arg(ipLineEdit->text())
              << command;
    
    logMessage("开始执行7ev固件升级预检查...");
    
    // 启动进程
    preCheck7evProcess->start(program, arguments);
    
    if (!preCheck7evProcess->waitForStarted(5000)) {
        logMessage("[错误] 无法启动SSH进程进行预检查");
        transferProgressBar->setVisible(false);
        statusLabel->setText("预检查失败");
        
        // 恢复所有操作按钮
        enableAllOperationButtons();
        
        QMessageBox::critical(this, "预检查失败", 
            "无法启动SSH进程进行预检查，请检查系统是否安装了SSH客户端工具。");
        
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
    env.insert("SSHPASS", passwordLineEdit->text());
    upgrade7evProcess->setProcessEnvironment(env);
    
    // 构建SSH命令
    QString program = "ssh";
    QStringList arguments;
    arguments << "-o" << "ConnectTimeout=30"
              << "-o" << "StrictHostKeyChecking=no"
              << "-o" << "UserKnownHostsFile=/dev/null"
              << "-o" << "PasswordAuthentication=yes"
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
        transferProgressBar->setVisible(false);
        statusLabel->setText("命令执行失败");
        
        // 恢复所有操作按钮
        enableAllOperationButtons();
        
        QMessageBox::critical(this, "执行失败", 
            "无法启动SSH进程，请检查系统是否安装了SSH客户端工具。");
        
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
    env.insert("SSHPASS", passwordLineEdit->text());
    remoteCommandProcess->setProcessEnvironment(env);
    
    // 构建SSH命令
    QString program = "ssh";
    QStringList arguments;
    arguments << "-o" << "ConnectTimeout=30"
              << "-o" << "StrictHostKeyChecking=no"
              << "-o" << "UserKnownHostsFile=/dev/null"
              << "-o" << "PasswordAuthentication=yes"
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
        transferProgressBar->setVisible(false);
        statusLabel->setText("命令执行失败");
        
        // 恢复所有操作按钮
        enableAllOperationButtons();
        
        QMessageBox::critical(this, "执行失败", 
            "无法启动SSH进程，请检查系统是否安装了SSH客户端工具。");
        
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
    env.insert("SSHPASS", passwordLineEdit->text());
    customCommandProcess->setProcessEnvironment(env);
    
    // 构建SSH命令
    QString program = "ssh";
    QStringList arguments;
    arguments << "-o" << "ConnectTimeout=30"
              << "-o" << "StrictHostKeyChecking=no"
              << "-o" << "UserKnownHostsFile=/dev/null"
              << "-o" << "PasswordAuthentication=yes"
              << "-p" << QString::number(portSpinBox->value())
              << QString("%1@%2").arg(usernameLineEdit->text()).arg(ipLineEdit->text())
              << command;
    
    // 禁用执行按钮防止重复执行
    executeCommandButton->setEnabled(false);
    
    // 启动进程
    customCommandProcess->start(program, arguments);
    
    if (!customCommandProcess->waitForStarted(5000)) {
        commandOutputEdit->append("<span style='color: #ff6b6b;'>[错误] 无法启动SSH进程，请检查系统是否安装了SSH客户端工具</span>");
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
    env.insert("SSHPASS", passwordLineEdit->text());
    upgradeKu5pProcess->setProcessEnvironment(env);
    
    // 构建SSH命令
    QString program = "ssh";
    QStringList arguments;
    arguments << "-o" << "ConnectTimeout=30"
              << "-o" << "StrictHostKeyChecking=no"
              << "-o" << "UserKnownHostsFile=/dev/null"
              << "-o" << "PasswordAuthentication=yes"
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
        transferProgressBar->setVisible(false);
        statusLabel->setText("命令执行失败");
        
        // 恢复所有操作按钮
        enableAllOperationButtons();
        
        QMessageBox::critical(this, "执行失败", 
            "无法启动SSH进程，请检查系统是否安装了SSH客户端工具。");
        
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
    
    // 恢复输入控件
    ipLineEdit->setEnabled(true);
    portSpinBox->setEnabled(true);
    usernameLineEdit->setEnabled(true);
    passwordLineEdit->setEnabled(true);
    commandLineEdit->setEnabled(true);
    filePathLineEdit->setEnabled(true);
    
    logMessage("[系统] 升级操作完成，已恢复所有操作按钮");
}