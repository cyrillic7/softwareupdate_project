/**
 * @File Name: get_machine_code.cpp
 * @brief  680图像机软件机器码查看工具，供最终用户获取机器唯一标识码
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
#include <QClipboard>
#include <QSysInfo>
#include <QNetworkInterface>
#include <QCryptographicHash>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>

class MachineCodeViewer : public QWidget
{
    Q_OBJECT

public:
    MachineCodeViewer(QWidget *parent = nullptr) : QWidget(parent)
    {
        setWindowTitle("680图像机软件 - 机器码查看工具");
        setFixedSize(500, 300);
        
        setupUI();
        connectSignals();
        
        // 显示当前机器码
        displayMachineCode();
    }

private slots:
    void onCopyMachineCode()
    {
        QString machineCode = getCurrentMachineCode();
        QApplication::clipboard()->setText(machineCode);
        QMessageBox::information(this, "复制成功", 
            "机器码已复制到剪贴板！\n\n请将此机器码发送给软件提供商以获取授权文件。");
        logEdit->append("机器码已复制到剪贴板");
    }
    
    void onSaveMachineCode()
    {
        QString machineCode = getCurrentMachineCode();
        QString fileName = QString("machine_code_%1.txt").arg(QSysInfo::machineHostName());
        
        // 获取可执行文件所在目录
        QString appDir = QApplication::applicationDirPath();
        QString fullPath = QDir(appDir).absoluteFilePath(fileName);
        
        QFile file(fullPath);
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream stream(&file);
            stream.setCodec("UTF-8");
            stream << "680 Image Machine Software - Machine Code Information\n";
            stream << "====================================================\n";
            stream << "Machine Name: " << QSysInfo::machineHostName() << "\n";
            stream << "CPU Architecture: " << QSysInfo::currentCpuArchitecture() << "\n";
            stream << "Machine Code: " << machineCode << "\n";
            stream << "Generated Time: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
            stream << "\nPlease send this machine code to software vendor to get authorization file.\n";
            file.close();
            
            QMessageBox::information(this, "Save Successful", 
                QString("Machine code information saved to file:\n%1").arg(fullPath));
            logEdit->append(QString("Machine code information saved to: %1").arg(fullPath));
        } else {
            QMessageBox::critical(this, "Save Failed", "Unable to save machine code file!");
        }
    }

private:
    void setupUI()
    {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        
        // 标题
        QLabel *titleLabel = new QLabel("680图像机软件 - 机器码查看工具");
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2c3e50; margin: 10px;");
        
        // 当前机器码显示
        QLabel *currentLabel = new QLabel("当前机器码：");
        currentMachineCodeEdit = new QLineEdit();
        currentMachineCodeEdit->setReadOnly(true);
        currentMachineCodeEdit->setStyleSheet("font-family: monospace; background-color: #f8f9fa;");
        
        QHBoxLayout *machineCodeLayout = new QHBoxLayout();
        machineCodeLayout->addWidget(currentLabel);
        machineCodeLayout->addWidget(currentMachineCodeEdit);
        
        // 按钮区域
        QPushButton *copyButton = new QPushButton("复制机器码");
        QPushButton *saveButton = new QPushButton("保存到文件");
        QPushButton *exitButton = new QPushButton("退出");
        
        copyButton->setStyleSheet("QPushButton { background-color: #3498db; color: white; padding: 8px; border-radius: 4px; }");
        saveButton->setStyleSheet("QPushButton { background-color: #2ecc71; color: white; padding: 8px; border-radius: 4px; }");
        exitButton->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; padding: 8px; border-radius: 4px; }");
        
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->addWidget(copyButton);
        buttonLayout->addWidget(saveButton);
        buttonLayout->addStretch();
        buttonLayout->addWidget(exitButton);
        
        // 说明文字
        QLabel *instructionLabel = new QLabel(
            "使用说明：\n"
            "1. 点击「复制机器码」将机器码复制到剪贴板\n"
            "2. 将机器码发送给软件提供商\n"
            "3. 获得授权文件后，将其放置在软件安装目录\n"
            "4. 重新启动主程序即可正常使用"
        );
        instructionLabel->setStyleSheet("color: #7f8c8d; background-color: #ecf0f1; padding: 10px; border-radius: 4px;");
        
        // 日志区域
        QLabel *logLabel = new QLabel("操作日志：");
        logEdit = new QTextEdit();
        logEdit->setMaximumHeight(80);
        logEdit->setStyleSheet("font-size: 10px; background-color: #f8f9fa;");
        
        // 添加到主布局
        mainLayout->addWidget(titleLabel);
        mainLayout->addLayout(machineCodeLayout);
        mainLayout->addLayout(buttonLayout);
        mainLayout->addWidget(instructionLabel);
        mainLayout->addWidget(logLabel);
        mainLayout->addWidget(logEdit);
        
        // 连接按钮信号
        connect(copyButton, &QPushButton::clicked, this, &MachineCodeViewer::onCopyMachineCode);
        connect(saveButton, &QPushButton::clicked, this, &MachineCodeViewer::onSaveMachineCode);
        connect(exitButton, &QPushButton::clicked, this, &QWidget::close);
    }
    
    void connectSignals()
    {
        // 已在setupUI中完成
    }
    
    void displayMachineCode()
    {
        QString machineCode = getCurrentMachineCode();
        currentMachineCodeEdit->setText(machineCode);
        
        logEdit->append("机器码查看工具启动完成");
        logEdit->append(QString("机器名：%1").arg(QSysInfo::machineHostName()));
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
    QTextEdit *logEdit;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    MachineCodeViewer viewer;
    viewer.show();
    
    return app.exec();
}

#include "get_machine_code.moc" 