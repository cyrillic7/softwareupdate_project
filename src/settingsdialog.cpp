#include "settingsdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("设置");
    setWindowIcon(QIcon(":/icons/settings.png"));
    setModal(true);
    setFixedSize(600, 500);
    
    setupUI();
    connectSignals();
    loadDefaults();
    
    // 设置CheckBox样式，确保可点击性，字体大小与其他控件保持一致
    setStyleSheet(
        "QCheckBox {"
        "    spacing: 8px;"
        "    font-size: 9pt;"  // 与其他控件保持一致的字体大小
        "}"
        "QCheckBox::indicator {"
        "    width: 18px;"      // 稍微增大指示器尺寸
        "    height: 18px;"
        "    border: 2px solid #999999;"
        "    border-radius: 3px;"
        "    background-color: white;"
        "}"
        "QCheckBox::indicator:hover {"
        "    border-color: #0078d4;"
        "    background-color: #f0f8ff;"
        "}"
        "QCheckBox::indicator:checked {"
        "    background-color: #0078d4;"
        "    border-color: #0078d4;"
        "    image: url(data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTQiIGhlaWdodD0iMTQiIHZpZXdCb3g9IjAgMCAxNCAxNCIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KPHBhdGggZD0iTTExIDRMNS41IDkuNUwzIDciIHN0cm9rZT0id2hpdGUiIHN0cm9rZS13aWR0aD0iMiIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIi8+Cjwvc3ZnPgo=);"
        "}"
        "QCheckBox::indicator:checked:hover {"
        "    background-color: #106ebe;"
        "    border-color: #106ebe;"
        "}"
        "QCheckBox::indicator:disabled {"
        "    background-color: #f0f0f0;"
        "    border-color: #cccccc;"
        "}"
        "QCheckBox:disabled {"
        "    color: #999999;"
        "}"
    );
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // 创建标签页
    tabWidget = new QTabWidget(this);
    tabWidget->setObjectName("settingsTabWidget");
    
    setupConnectionSettings();
    setupApplicationSettings();
    setupAdvancedSettings();
    
    mainLayout->addWidget(tabWidget);
    
    // 按钮区域
    buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    defaultsButton = new QPushButton("恢复默认", this);
    defaultsButton->setObjectName("defaultsButton");
    
    buttonLayout->addWidget(defaultsButton);
    buttonLayout->addStretch();
    
    cancelButton = new QPushButton("取消", this);
    cancelButton->setObjectName("cancelButton");
    
    okButton = new QPushButton("确定", this);
    okButton->setObjectName("okButton");
    okButton->setDefault(true);
    
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(okButton);
    
    mainLayout->addLayout(buttonLayout);
}

void SettingsDialog::setupConnectionSettings()
{
    connectionTab = new QWidget();
    connectionLayout = new QVBoxLayout(connectionTab);
    connectionLayout->setSpacing(15);
    connectionLayout->setContentsMargins(15, 15, 15, 15);
    
    // 远程服务器设置组
    remoteGroup = new QGroupBox("远程服务器设置", connectionTab);
    remoteLayout = new QGridLayout(remoteGroup);
    remoteLayout->setSpacing(10);
    remoteLayout->setContentsMargins(15, 20, 15, 15);
    
    remoteDirLabel = new QLabel("远程目录:", remoteGroup);
    remoteDirLineEdit = new QLineEdit(remoteGroup);
    remoteDirLineEdit->setPlaceholderText("例如: /media/sata/ue_data/");
    remoteDirLineEdit->setObjectName("remoteDirLineEdit");
    
    testRemoteDirButton = new QPushButton("测试目录", remoteGroup);
    testRemoteDirButton->setObjectName("testRemoteDirButton");
    testRemoteDirButton->setMinimumWidth(100);
    
    timeoutLabel = new QLabel("连接超时(秒):", remoteGroup);
    timeoutSpinBox = new QSpinBox(remoteGroup);
    timeoutSpinBox->setRange(5, 120);
    timeoutSpinBox->setValue(30);
    timeoutSpinBox->setSuffix(" 秒");
    
    remoteLayout->addWidget(remoteDirLabel, 0, 0);
    remoteLayout->addWidget(remoteDirLineEdit, 0, 1);
    remoteLayout->addWidget(testRemoteDirButton, 0, 2);
    remoteLayout->addWidget(timeoutLabel, 1, 0);
    remoteLayout->addWidget(timeoutSpinBox, 1, 1);
    
    remoteLayout->setColumnStretch(1, 1);
    
    connectionLayout->addWidget(remoteGroup);
    
    // 升级路径设置组
    upgradePathGroup = new QGroupBox("升级解压路径设置", connectionTab);
    upgradePathLayout = new QGridLayout(upgradePathGroup);
    upgradePathLayout->setSpacing(10);
    upgradePathLayout->setContentsMargins(15, 20, 15, 15);
    
    // Qt软件升级解压路径
    qtExtractPathLabel = new QLabel("Qt软件解压路径:", upgradePathGroup);
    qtExtractPathLineEdit = new QLineEdit(upgradePathGroup);
    qtExtractPathLineEdit->setPlaceholderText("Qt软件升级时的解压目标路径（默认: /mnt/qtfs）");
    qtExtractPathLineEdit->setObjectName("qtExtractPathLineEdit");
    
    selectQtExtractPathButton = new QPushButton("浏览", upgradePathGroup);
    selectQtExtractPathButton->setObjectName("selectQtExtractPathButton");
    selectQtExtractPathButton->setMinimumWidth(80);
    
    // 7ev固件升级解压路径
    sevEvExtractPathLabel = new QLabel("7ev固件解压路径:", upgradePathGroup);
    sevEvExtractPathLineEdit = new QLineEdit(upgradePathGroup);
    sevEvExtractPathLineEdit->setPlaceholderText("7ev固件升级时的解压目标路径（默认: /mnt/mmcblk0p1）");
    sevEvExtractPathLineEdit->setObjectName("sevEvExtractPathLineEdit");
    
    select7evExtractPathButton = new QPushButton("浏览", upgradePathGroup);
    select7evExtractPathButton->setObjectName("select7evExtractPathButton");
    select7evExtractPathButton->setMinimumWidth(80);
    
    // 布局升级路径设置
    upgradePathLayout->addWidget(qtExtractPathLabel, 0, 0);
    upgradePathLayout->addWidget(qtExtractPathLineEdit, 0, 1);
    upgradePathLayout->addWidget(selectQtExtractPathButton, 0, 2);
    upgradePathLayout->addWidget(sevEvExtractPathLabel, 1, 0);
    upgradePathLayout->addWidget(sevEvExtractPathLineEdit, 1, 1);
    upgradePathLayout->addWidget(select7evExtractPathButton, 1, 2);
    
    upgradePathLayout->setColumnStretch(1, 1);
    
    connectionLayout->addWidget(upgradePathGroup);
    connectionLayout->addStretch();
    
    tabWidget->addTab(connectionTab, "连接设置");
}

void SettingsDialog::setupApplicationSettings()
{
    applicationTab = new QWidget();
    applicationLayout = new QVBoxLayout(applicationTab);
    applicationLayout->setSpacing(15);
    applicationLayout->setContentsMargins(15, 15, 15, 15);
    
    // 常规设置组
    generalGroup = new QGroupBox("常规设置", applicationTab);
    generalLayout = new QGridLayout(generalGroup);
    generalLayout->setSpacing(12);
    generalLayout->setContentsMargins(15, 20, 15, 15);
    
    // 创建CheckBox控件并设置属性
    autoSaveCheckBox = new QCheckBox("自动保存设置", generalGroup);
    autoSaveCheckBox->setObjectName("autoSaveCheckBox");
    autoSaveCheckBox->setChecked(true);
    autoSaveCheckBox->setToolTip("程序关闭时自动保存当前设置");
    autoSaveCheckBox->setMinimumHeight(25);
    autoSaveCheckBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    
    showLogCheckBox = new QCheckBox("启动时显示日志", generalGroup);
    showLogCheckBox->setObjectName("showLogCheckBox");
    showLogCheckBox->setChecked(false);
    showLogCheckBox->setToolTip("程序启动时默认显示操作日志");
    showLogCheckBox->setMinimumHeight(25);
    showLogCheckBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    
    // 自动清理日志设置
    autoCleanLogCheckBox = new QCheckBox("自动清理过期日志", generalGroup);
    autoCleanLogCheckBox->setObjectName("autoCleanLogCheckBox");
    autoCleanLogCheckBox->setChecked(false);
    autoCleanLogCheckBox->setToolTip("自动删除超过指定天数的日志文件");
    autoCleanLogCheckBox->setMinimumHeight(25);
    autoCleanLogCheckBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    
    retentionDaysLabel = new QLabel("保留天数:", generalGroup);
    retentionDaysSpinBox = new QSpinBox(generalGroup);
    retentionDaysSpinBox->setRange(1, 365);
    retentionDaysSpinBox->setValue(30);
    retentionDaysSpinBox->setSuffix(" 天");
    retentionDaysSpinBox->setToolTip("保留多少天内的日志文件");
    retentionDaysSpinBox->setEnabled(false); // 默认禁用，当勾选自动清理时启用
    retentionDaysSpinBox->setMinimumWidth(100);
    
    // 文件路径设置
    defaultPathLabel = new QLabel("默认文件路径:", generalGroup);
    defaultPathLineEdit = new QLineEdit(generalGroup);
    defaultPathLineEdit->setPlaceholderText("选择文件时的默认目录（默认为程序目录）");
    defaultPathLineEdit->setObjectName("defaultPathLineEdit");
    
    selectPathButton = new QPushButton("浏览", generalGroup);
    selectPathButton->setObjectName("selectPathButton");
    selectPathButton->setMinimumWidth(80);
    
    // 日志存储路径设置
    logPathLabel = new QLabel("日志存储路径:", generalGroup);
    logPathLineEdit = new QLineEdit(generalGroup);
    logPathLineEdit->setPlaceholderText("日志文件存储的目录路径（默认为程序目录）");
    logPathLineEdit->setObjectName("logPathLineEdit");
    
    selectLogPathButton = new QPushButton("浏览", generalGroup);
    selectLogPathButton->setObjectName("selectLogPathButton");
    selectLogPathButton->setMinimumWidth(60);
    
    openLogDirButton = new QPushButton("打开", generalGroup);
    openLogDirButton->setObjectName("openLogDirButton");
    openLogDirButton->setMinimumWidth(60);
    openLogDirButton->setToolTip("在文件管理器中打开日志目录");
    
    // 布局设置 - 使用更清晰的行布局
    int row = 0;
    
    // 第一行：自动保存设置
    generalLayout->addWidget(autoSaveCheckBox, row, 0, 1, 4);
    row++;
    
    // 第二行：启动时显示日志
    generalLayout->addWidget(showLogCheckBox, row, 0, 1, 4);
    row++;
    
    // 第三行：自动清理过期日志和保留天数
    generalLayout->addWidget(autoCleanLogCheckBox, row, 0, 1, 2);
    generalLayout->addWidget(retentionDaysLabel, row, 2);
    generalLayout->addWidget(retentionDaysSpinBox, row, 3);
    row++;
    
    // 第四行：默认文件路径
    generalLayout->addWidget(defaultPathLabel, row, 0);
    generalLayout->addWidget(defaultPathLineEdit, row, 1, 1, 2);
    generalLayout->addWidget(selectPathButton, row, 3);
    row++;
    
    // 第五行：日志存储路径
    generalLayout->addWidget(logPathLabel, row, 0);
    generalLayout->addWidget(logPathLineEdit, row, 1);
    generalLayout->addWidget(selectLogPathButton, row, 2);
    generalLayout->addWidget(openLogDirButton, row, 3);
    
    // 设置列拉伸比例
    generalLayout->setColumnStretch(1, 1);
    
    applicationLayout->addWidget(generalGroup);
    applicationLayout->addStretch();
    
    tabWidget->addTab(applicationTab, "应用设置");
}

void SettingsDialog::setupAdvancedSettings()
{
    advancedTab = new QWidget();
    advancedLayout = new QVBoxLayout(advancedTab);
    advancedLayout->setSpacing(15);
    advancedLayout->setContentsMargins(15, 15, 15, 15);
    
    // 日志设置组
    logGroup = new QGroupBox("日志设置", advancedTab);
    logLayout = new QGridLayout(logGroup);
    logLayout->setSpacing(10);
    logLayout->setContentsMargins(15, 20, 15, 15);
    
    maxLogLinesLabel = new QLabel("最大日志行数:", logGroup);
    maxLogLinesSpinBox = new QSpinBox(logGroup);
    maxLogLinesSpinBox->setRange(100, 10000);
    maxLogLinesSpinBox->setValue(1000);
    maxLogLinesSpinBox->setSuffix(" 行");
    maxLogLinesSpinBox->setToolTip("超过此行数时自动清理旧日志");
    
    logLayout->addWidget(maxLogLinesLabel, 0, 0);
    logLayout->addWidget(maxLogLinesSpinBox, 0, 1);
    logLayout->setColumnStretch(2, 1);
    
    // 安全设置组
    securityGroup = new QGroupBox("安全设置", advancedTab);
    securityLayout = new QGridLayout(securityGroup);
    securityLayout->setSpacing(10);
    securityLayout->setContentsMargins(15, 20, 15, 15);
    
    verifyHostCheckBox = new QCheckBox("严格主机密钥检查", securityGroup);
    verifyHostCheckBox->setChecked(false);
    verifyHostCheckBox->setToolTip("启用后会验证SSH主机密钥，提高安全性但可能影响连接");
    
    savePasswordCheckBox = new QCheckBox("记住密码", securityGroup);
    savePasswordCheckBox->setChecked(false);
    savePasswordCheckBox->setToolTip("将密码保存到配置文件中（不推荐）");
    
    securityLayout->addWidget(verifyHostCheckBox, 0, 0);
    securityLayout->addWidget(savePasswordCheckBox, 1, 0);
    
    advancedLayout->addWidget(logGroup);
    advancedLayout->addWidget(securityGroup);
    advancedLayout->addStretch();
    
    tabWidget->addTab(advancedTab, "高级设置");
}

void SettingsDialog::connectSignals()
{
    // 按钮信号连接
    connect(okButton, &QPushButton::clicked, this, &SettingsDialog::onAccept);
    connect(cancelButton, &QPushButton::clicked, this, &SettingsDialog::onReject);
    connect(defaultsButton, &QPushButton::clicked, this, &SettingsDialog::onRestoreDefaults);
    connect(selectPathButton, &QPushButton::clicked, this, &SettingsDialog::onSelectDefaultPath);
    connect(selectLogPathButton, &QPushButton::clicked, this, &SettingsDialog::onSelectLogPath);
    connect(openLogDirButton, &QPushButton::clicked, this, &SettingsDialog::onOpenLogDirectory);
    connect(testRemoteDirButton, &QPushButton::clicked, this, &SettingsDialog::onTestRemoteDirectory);
    connect(selectQtExtractPathButton, &QPushButton::clicked, this, &SettingsDialog::onSelectQtExtractPath);
    connect(select7evExtractPathButton, &QPushButton::clicked, this, &SettingsDialog::onSelect7evExtractPath);
    
    // CheckBox信号连接
    connect(autoCleanLogCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        retentionDaysSpinBox->setEnabled(checked);
        retentionDaysLabel->setEnabled(checked);
    });
    
    // 确保CheckBox能够正常响应点击事件
    connect(autoSaveCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        // 可以在这里添加额外的处理逻辑
        Q_UNUSED(checked);
    });
    
    connect(showLogCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        // 可以在这里添加额外的处理逻辑
        Q_UNUSED(checked);
    });
}

void SettingsDialog::loadDefaults()
{
    // 连接设置默认值
    remoteDirLineEdit->setText("/media/sata/ue_data/");
    timeoutSpinBox->setValue(30);
    
    // 升级路径默认值
    qtExtractPathLineEdit->setText("/mnt/qtfs");
    sevEvExtractPathLineEdit->setText("/mnt/mmcblk0p1");
    
    // 应用设置默认值
    autoSaveCheckBox->setChecked(true);
    showLogCheckBox->setChecked(false);
    autoCleanLogCheckBox->setChecked(false);
    retentionDaysSpinBox->setValue(30);
    retentionDaysSpinBox->setEnabled(false); // 默认禁用
    retentionDaysLabel->setEnabled(false);   // 标签也禁用
    
    // 获取当前可执行程序的路径
    QString appDir = QApplication::applicationDirPath();
    
    // 设置默认文件路径为应用程序目录
    defaultPathLineEdit->setText(appDir);
    
    // 设置日志存储路径为应用程序目录
    logPathLineEdit->setText(appDir);
    
    // 高级设置默认值
    maxLogLinesSpinBox->setValue(1000);
    verifyHostCheckBox->setChecked(false);
    savePasswordCheckBox->setChecked(false);
    
    // 强制刷新CheckBox状态
    autoSaveCheckBox->repaint();
    showLogCheckBox->repaint();
    autoCleanLogCheckBox->repaint();
}

void SettingsDialog::onAccept()
{
    // 验证设置
    QString remoteDir = remoteDirLineEdit->text().trimmed();
    if (remoteDir.isEmpty()) {
        QMessageBox::warning(this, "设置错误", "远程目录不能为空！");
        tabWidget->setCurrentIndex(0); // 切换到连接设置标签页
        remoteDirLineEdit->setFocus();
        return;
    }
    
    if (!remoteDir.startsWith("/")) {
        QMessageBox::warning(this, "设置错误", "远程目录必须是绝对路径（以/开头）！");
        tabWidget->setCurrentIndex(0);
        remoteDirLineEdit->setFocus();
        return;
    }
    
    accept();
}

void SettingsDialog::onReject()
{
    reject();
}

void SettingsDialog::onRestoreDefaults()
{
    int ret = QMessageBox::question(this, "恢复默认设置", 
                                   "确定要恢复所有设置为默认值吗？\n这将清除您的自定义配置。",
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        loadDefaults();
        QMessageBox::information(this, "设置已恢复", "所有设置已恢复为默认值。");
    }
}

void SettingsDialog::onSelectDefaultPath()
{
    QString currentPath = defaultPathLineEdit->text();
    if (currentPath.isEmpty()) {
        currentPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
    
    QString selectedPath = QFileDialog::getExistingDirectory(this,
                                                           "选择默认文件路径",
                                                           currentPath);
    
    if (!selectedPath.isEmpty()) {
        defaultPathLineEdit->setText(selectedPath);
    }
}

void SettingsDialog::onSelectLogPath()
{
    QString currentPath = logPathLineEdit->text();
    if (currentPath.isEmpty()) {
        currentPath = QApplication::applicationDirPath();
    }
    
    QString selectedPath = QFileDialog::getExistingDirectory(this,
                                                           "选择日志存储路径",
                                                           currentPath);
    
    if (!selectedPath.isEmpty()) {
        logPathLineEdit->setText(selectedPath);
    }
}

void SettingsDialog::onOpenLogDirectory()
{
    QString logPath = logPathLineEdit->text().trimmed();
    if (logPath.isEmpty()) {
        logPath = QApplication::applicationDirPath();
    }
    
    // 检查目录是否存在
    QDir dir(logPath);
    if (!dir.exists()) {
        // 尝试创建目录
        if (!dir.mkpath(logPath)) {
            QMessageBox::warning(this, "打开失败", 
                               QString("无法创建日志目录：%1").arg(logPath));
            return;
        }
    }
    
    // 在文件管理器中打开目录
    QDesktopServices::openUrl(QUrl::fromLocalFile(logPath));
}

void SettingsDialog::onTestRemoteDirectory()
{
    QString remoteDir = remoteDirLineEdit->text().trimmed();
    if (remoteDir.isEmpty()) {
        QMessageBox::warning(this, "测试失败", "请先输入远程目录路径！");
        return;
    }
    
    QMessageBox::information(this, "目录测试", 
                           QString("测试远程目录: %1\n\n注意：实际测试需要在主界面建立连接后进行。").arg(remoteDir));
}

void SettingsDialog::onSelectQtExtractPath()
{
    QString currentPath = qtExtractPathLineEdit->text();
    if (currentPath.isEmpty()) {
        currentPath = "/mnt/qtfs";
    }
    
    QString selectedPath = QFileDialog::getExistingDirectory(this,
                                                           "选择Qt软件解压路径",
                                                           currentPath);
    
    if (!selectedPath.isEmpty()) {
        qtExtractPathLineEdit->setText(selectedPath);
    }
}

void SettingsDialog::onSelect7evExtractPath()
{
    QString currentPath = sevEvExtractPathLineEdit->text();
    if (currentPath.isEmpty()) {
        currentPath = "/mnt/mmcblk0p1";
    }
    
    QString selectedPath = QFileDialog::getExistingDirectory(this,
                                                           "选择7ev固件解压路径",
                                                           currentPath);
    
    if (!selectedPath.isEmpty()) {
        sevEvExtractPathLineEdit->setText(selectedPath);
    }
}

// Getter functions
QString SettingsDialog::getRemoteDirectory() const
{
    return remoteDirLineEdit->text().trimmed();
}

int SettingsDialog::getConnectionTimeout() const
{
    return timeoutSpinBox->value();
}

bool SettingsDialog::getAutoSaveSettings() const
{
    return autoSaveCheckBox->isChecked();
}

bool SettingsDialog::getShowLogByDefault() const
{
    return showLogCheckBox->isChecked();
}

QString SettingsDialog::getDefaultLocalPath() const
{
    return defaultPathLineEdit->text().trimmed();
}

QString SettingsDialog::getLogStoragePath() const
{
    return logPathLineEdit->text().trimmed();
}

bool SettingsDialog::getAutoCleanLog() const
{
    return autoCleanLogCheckBox->isChecked();
}

int SettingsDialog::getLogRetentionDays() const
{
    return retentionDaysSpinBox->value();
}

int SettingsDialog::getMaxLogLines() const
{
    return maxLogLinesSpinBox->value();
}

QString SettingsDialog::getQtExtractPath() const
{
    return qtExtractPathLineEdit->text().trimmed();
}

QString SettingsDialog::get7evExtractPath() const
{
    return sevEvExtractPathLineEdit->text().trimmed();
}

// Setter functions
void SettingsDialog::setRemoteDirectory(const QString &path)
{
    remoteDirLineEdit->setText(path);
}

void SettingsDialog::setConnectionTimeout(int timeout)
{
    timeoutSpinBox->setValue(timeout);
}

void SettingsDialog::setAutoSaveSettings(bool autoSave)
{
    autoSaveCheckBox->setChecked(autoSave);
}

void SettingsDialog::setShowLogByDefault(bool showLog)
{
    showLogCheckBox->setChecked(showLog);
}

void SettingsDialog::setDefaultLocalPath(const QString &path)
{
    defaultPathLineEdit->setText(path);
}

void SettingsDialog::setLogStoragePath(const QString &path)
{
    logPathLineEdit->setText(path);
}

void SettingsDialog::setAutoCleanLog(bool autoClean)
{
    autoCleanLogCheckBox->setChecked(autoClean);
}

void SettingsDialog::setLogRetentionDays(int days)
{
    retentionDaysSpinBox->setValue(days);
}

void SettingsDialog::setMaxLogLines(int maxLines)
{
    maxLogLinesSpinBox->setValue(maxLines);
}

void SettingsDialog::setQtExtractPath(const QString &path)
{
    qtExtractPathLineEdit->setText(path);
}

void SettingsDialog::set7evExtractPath(const QString &path)
{
    sevEvExtractPathLineEdit->setText(path);
} 