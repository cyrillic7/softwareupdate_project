/**
 * @File Name: generate_auth.cpp
 * @brief  680图像机软件授权文件生成工具，供软件提供商使用，支持机器码加密
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
#include <QDateTime>
#include "crypto_utils.h"

class AuthGenerator : public QWidget
{
    Q_OBJECT

public:
    AuthGenerator(QWidget *parent = nullptr) : QWidget(parent)
    {
        setWindowTitle("680图像机软件 - 授权文件生成工具");
        setFixedSize(500, 400);
        
        setupUI();
        connectSignals();
        
        logEdit->append("授权文件生成工具启动完成");
        logEdit->append("请输入要授权的机器码，然后生成授权文件");
    }

private slots:
    void onLoadMachineCodeFile()
    {
        QString fileName = QFileDialog::getOpenFileName(this,
            "选择机器码文件", "", "文本文件 (*.txt);;所有文件 (*)");
        
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::ReadOnly)) {
                QTextStream stream(&file);
                stream.setCodec("UTF-8");
                QString content = stream.readAll();
                file.close();
                
                // 尝试从文件中提取机器码
                QString machineCode = extractMachineCodeFromFile(content);
                if (!machineCode.isEmpty()) {
                    machineCodeEdit->setText(machineCode);
                    logEdit->append(QString("已从文件加载机器码：%1").arg(fileName));
                    logEdit->append(QString("提取的机器码：%1").arg(machineCode));
                } else {
                    // 如果无法提取，显示文件内容让用户手动复制
                    QMessageBox::information(this, "文件内容", 
                        QString("文件内容：\n\n%1\n\n请手动复制机器码到输入框").arg(content));
                }
            } else {
                QMessageBox::critical(this, "错误", "无法读取文件！");
            }
        }
    }
    
    void onGenerateAuthFile()
    {
        QString machineCode = machineCodeEdit->text().trimmed().toUpper();
        if (machineCode.isEmpty()) {
            QMessageBox::warning(this, "警告", "请输入机器码！");
            return;
        }
        
        // 验证机器码格式（应该是32位十六进制字符）
        if (machineCode.length() != 32 || !isValidHex(machineCode)) {
            QMessageBox::warning(this, "警告", 
                "机器码格式错误！\n正确格式应为32位十六进制字符串。");
            return;
        }
        
        QString fileName = QFileDialog::getSaveFileName(this,
            "保存授权文件", "machine_auth.key", "授权文件 (*.key);;所有文件 (*)");
        
        if (!fileName.isEmpty()) {
            // 使用加密算法加密机器码
            QString encryptedData = CryptoUtils::encryptMachineCode(machineCode);
            
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly)) {
                QTextStream stream(&file);
                stream.setCodec("UTF-8");
                stream << encryptedData;  // 写入加密后的数据
                file.close();
                
                QMessageBox::information(this, "Success", 
                    QString("Authorization file generated: %1\n\nPlease send this file to end user,\nUser needs to place it in software directory.").arg(fileName));
                logEdit->append(QString("Authorization file generated: %1").arg(fileName));
                logEdit->append(QString("Original machine code: %1").arg(machineCode));
                logEdit->append(QString("Encrypted data: %1").arg(encryptedData));
                logEdit->append(QString("Generation time: %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
                
                // 清空输入框
                machineCodeEdit->clear();
            } else {
                QMessageBox::critical(this, "Error", "Unable to create authorization file!");
            }
        }
    }
    
    void onClearLog()
    {
        logEdit->clear();
        logEdit->append("日志已清空");
    }

private:
    void setupUI()
    {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        
        // 标题
        QLabel *titleLabel = new QLabel("680图像机软件 - 授权文件生成工具");
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2c3e50; margin: 10px;");
        
        // 机器码输入区域
        QLabel *machineLabel = new QLabel("目标机器码：");
        machineCodeEdit = new QLineEdit();
        machineCodeEdit->setPlaceholderText("输入32位十六进制机器码（例如：A1B2C3D4E5F6789012345678901234AB）");
        machineCodeEdit->setStyleSheet("font-family: monospace; padding: 8px;");
        
        QPushButton *loadFileButton = new QPushButton("从文件加载");
        loadFileButton->setStyleSheet("QPushButton { background-color: #3498db; color: white; padding: 8px; border-radius: 4px; }");
        
        QHBoxLayout *machineCodeLayout = new QHBoxLayout();
        machineCodeLayout->addWidget(machineLabel);
        machineCodeLayout->addWidget(machineCodeEdit, 1);
        machineCodeLayout->addWidget(loadFileButton);
        
        // 按钮区域
        QPushButton *generateButton = new QPushButton("生成授权文件");
        QPushButton *clearLogButton = new QPushButton("清空日志");
        QPushButton *exitButton = new QPushButton("退出");
        
        generateButton->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; padding: 10px; border-radius: 4px; font-weight: bold; }");
        clearLogButton->setStyleSheet("QPushButton { background-color: #f39c12; color: white; padding: 8px; border-radius: 4px; }");
        exitButton->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; padding: 8px; border-radius: 4px; }");
        
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->addWidget(generateButton);
        buttonLayout->addWidget(clearLogButton);
        buttonLayout->addStretch();
        buttonLayout->addWidget(exitButton);
        
        // 警告信息
        QLabel *warningLabel = new QLabel(
            "⚠️ 重要提醒：\n"
            "• 此工具仅供软件提供商使用\n"
            "• 请妥善保管生成的授权文件\n"
            "• 每个机器码对应唯一的授权文件\n"
            "• 授权文件应通过安全渠道发送给用户"
        );
        warningLabel->setStyleSheet("color: #e74c3c; background-color: #fdf2f2; padding: 10px; border: 1px solid #f5c6cb; border-radius: 4px;");
        
        // 日志区域
        QLabel *logLabel = new QLabel("操作日志：");
        logEdit = new QTextEdit();
        logEdit->setMaximumHeight(120);
        logEdit->setStyleSheet("font-size: 11px; background-color: #f8f9fa;");
        
        // 添加到主布局
        mainLayout->addWidget(titleLabel);
        mainLayout->addLayout(machineCodeLayout);
        mainLayout->addLayout(buttonLayout);
        mainLayout->addWidget(warningLabel);
        mainLayout->addWidget(logLabel);
        mainLayout->addWidget(logEdit);
        
        // 连接按钮信号
        connect(loadFileButton, &QPushButton::clicked, this, &AuthGenerator::onLoadMachineCodeFile);
        connect(generateButton, &QPushButton::clicked, this, &AuthGenerator::onGenerateAuthFile);
        connect(clearLogButton, &QPushButton::clicked, this, &AuthGenerator::onClearLog);
        connect(exitButton, &QPushButton::clicked, this, &QWidget::close);
    }
    
    void connectSignals()
    {
        // 已在setupUI中完成
    }
    
    QString extractMachineCodeFromFile(const QString &content)
    {
        // 尝试从文件内容中提取机器码
        QStringList lines = content.split('\n');
        for (const QString &line : lines) {
            if (line.contains("Machine Code:") || line.contains("机器码:") || line.contains("机器码：")) {
                // 提取冒号后的内容
                int colonIndex = line.indexOf(':');
                if (colonIndex == -1) {
                    colonIndex = line.indexOf(QString("："));
                }
                if (colonIndex != -1) {
                    QString code = line.mid(colonIndex + 1).trimmed();
                    if (code.length() == 32 && isValidHex(code)) {
                        return code.toUpper();
                    }
                }
            }
        }
        
        // 如果没有找到标准格式，尝试查找32位十六进制字符串
        QRegExp hexPattern("[A-Fa-f0-9]{32}");
        int pos = hexPattern.indexIn(content);
        if (pos != -1) {
            return hexPattern.cap(0).toUpper();
        }
        
        return QString();
    }
    
    bool isValidHex(const QString &str)
    {
        QRegExp hexPattern("^[A-Fa-f0-9]+$");
        return hexPattern.exactMatch(str);
    }

private:
    QLineEdit *machineCodeEdit;
    QTextEdit *logEdit;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    AuthGenerator generator;
    generator.show();
    
    return app.exec();
}

#include "generate_auth.moc" 