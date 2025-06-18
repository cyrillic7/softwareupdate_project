/**
 * @File Name: auth_tool.cpp
 * @brief  680图像机软件综合授权工具，包含机器码管理和授权文件生成验证功能
 * @Author : chency email:121888719@qq.com
 * @Version : 1.0
 * @Creat Date : 2025
 *
 */

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QSysInfo>
#include <QNetworkInterface>
#include <QCryptographicHash>
#include <QClipboard>

class AuthTool : public QWidget
{
    Q_OBJECT

public:
    AuthTool(QWidget *parent = nullptr) : QWidget(parent)
    {
        setWindowTitle("680图像机软件授权工具");
        setFixedSize(500, 400);
        
        setupUI();
        connectSignals();
        
        // 显示当前机器码
        displayCurrentMachineCode();
    }

private slots:
    void onGenerateAuthFile()
    {
        QString machineCode = machineCodeEdit->text().trimmed();
        if (machineCode.isEmpty()) {
            QMessageBox::warning(this, "警告", "请输入机器码！");
            return;
        }
        
        QString fileName = QFileDialog::getSaveFileName(this,
            "保存授权文件", "machine_auth.key", "授权文件 (*.key)");
        
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly)) {
                QTextStream stream(&file);
                stream.setCodec("UTF-8");
                stream << machineCode;
                file.close();
                
                QMessageBox::information(this, "成功", 
                    QString("授权文件已生成：%1").arg(fileName));
                logEdit->append(QString("授权文件已生成：%1，机器码：%2")
                              .arg(fileName).arg(machineCode));
            } else {
                QMessageBox::critical(this, "错误", "无法创建授权文件！");
            }
        }
    }
    
    void onLoadAuthFile()
    {
        QString fileName = QFileDialog::getOpenFileName(this,
            "选择授权文件", "", "授权文件 (*.key)");
        
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::ReadOnly)) {
                QTextStream stream(&file);
                stream.setCodec("UTF-8");
                QString authCode = stream.readAll().trimmed();
                file.close();
                
                authCodeEdit->setText(authCode);
                logEdit->append(QString("已加载授权文件：%1，机器码：%2")
                              .arg(fileName).arg(authCode));
                
                // 检查是否与当前机器码匹配
                QString currentCode = getCurrentMachineCode();
                if (authCode == currentCode) {
                    QMessageBox::information(this, "验证结果", 
                        "授权验证成功！该授权文件与当前机器匹配。");
                    logEdit->append("✓ 授权验证成功！");
                } else {
                    QMessageBox::warning(this, "验证结果", 
                        "授权验证失败！该授权文件与当前机器不匹配。");
                    logEdit->append("✗ 授权验证失败！");
                }
            } else {
                QMessageBox::critical(this, "错误", "无法读取授权文件！");
            }
        }
    }
    
    void onCopyMachineCode()
    {
        QString machineCode = getCurrentMachineCode();
        QApplication::clipboard()->setText(machineCode);
        QMessageBox::information(this, "复制成功", "机器码已复制到剪贴板！");
        logEdit->append("机器码已复制到剪贴板");
    }

private:
    void setupUI()
    {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        
        // 当前机器码显示
        QLabel *currentLabel = new QLabel("当前机器码：");
        currentMachineCodeEdit = new QLineEdit();
        currentMachineCodeEdit->setReadOnly(true);
        
        QPushButton *copyButton = new QPushButton("复制机器码");
        
        QHBoxLayout *currentLayout = new QHBoxLayout();
        currentLayout->addWidget(currentLabel);
        currentLayout->addWidget(currentMachineCodeEdit);
        currentLayout->addWidget(copyButton);
        
        // 生成授权文件区域
        QLabel *generateLabel = new QLabel("生成授权文件：");
        QLabel *machineLabel = new QLabel("目标机器码：");
        machineCodeEdit = new QLineEdit();
        machineCodeEdit->setPlaceholderText("输入要授权的机器码");
        
        QPushButton *generateButton = new QPushButton("生成授权文件");
        
        QHBoxLayout *generateLayout = new QHBoxLayout();
        generateLayout->addWidget(machineLabel);
        generateLayout->addWidget(machineCodeEdit);
        generateLayout->addWidget(generateButton);
        
        // 验证授权文件区域
        QLabel *verifyLabel = new QLabel("验证授权文件：");
        QLabel *authLabel = new QLabel("授权机器码：");
        authCodeEdit = new QLineEdit();
        authCodeEdit->setReadOnly(true);
        
        QPushButton *loadButton = new QPushButton("加载授权文件");
        
        QHBoxLayout *verifyLayout = new QHBoxLayout();
        verifyLayout->addWidget(authLabel);
        verifyLayout->addWidget(authCodeEdit);
        verifyLayout->addWidget(loadButton);
        
        // 日志区域
        QLabel *logLabel = new QLabel("操作日志：");
        logEdit = new QTextEdit();
        logEdit->setMaximumHeight(150);
        
        // 添加到主布局
        mainLayout->addWidget(currentLabel);
        mainLayout->addLayout(currentLayout);
        mainLayout->addWidget(new QLabel(""));  // 空行
        
        mainLayout->addWidget(generateLabel);
        mainLayout->addLayout(generateLayout);
        mainLayout->addWidget(new QLabel(""));  // 空行
        
        mainLayout->addWidget(verifyLabel);
        mainLayout->addLayout(verifyLayout);
        mainLayout->addWidget(new QLabel(""));  // 空行
        
        mainLayout->addWidget(logLabel);
        mainLayout->addWidget(logEdit);
        
        // 连接按钮信号
        connect(copyButton, &QPushButton::clicked, this, &AuthTool::onCopyMachineCode);
        connect(generateButton, &QPushButton::clicked, this, &AuthTool::onGenerateAuthFile);
        connect(loadButton, &QPushButton::clicked, this, &AuthTool::onLoadAuthFile);
    }
    
    void connectSignals()
    {
        // 已在setupUI中完成
    }
    
    void displayCurrentMachineCode()
    {
        QString machineCode = getCurrentMachineCode();
        currentMachineCodeEdit->setText(machineCode);
        machineCodeEdit->setText(machineCode);  // 默认授权当前机器
        
        logEdit->append("授权工具启动完成");
        logEdit->append(QString("当前机器码：%1").arg(machineCode));
    }
    
    QString getCurrentMachineCode()
    {
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
        return hash.result().toHex().toUpper();
    }

private:
    QLineEdit *currentMachineCodeEdit;
    QLineEdit *machineCodeEdit;
    QLineEdit *authCodeEdit;
    QTextEdit *logEdit;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    AuthTool tool;
    tool.show();
    
    return app.exec();
}

#include "auth_tool.moc" 