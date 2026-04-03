/**
 * @File Name: encrypt_package.cpp
 * @brief  680 Software Package Encryption Tool for tar.gz files
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
#include <QFileInfo>
#include <QProgressBar>
#include <QDateTime>
#include <QCryptographicHash>
#include <QGroupBox>

// File Encryptor Class
class FileEncryptor
{
public:
    // Private Key
    static const QString PRIVATE_KEY;
    
    // Encrypt file
    static bool encryptFile(const QString &inputPath, const QString &outputPath, 
                           std::function<void(int)> progressCallback = nullptr)
    {
        QFile inputFile(inputPath);
        if (!inputFile.open(QIODevice::ReadOnly)) {
            return false;
        }
        
        QFile outputFile(outputPath);
        if (!outputFile.open(QIODevice::WriteOnly)) {
            inputFile.close();
            return false;
        }
        
        // Read all data
        QByteArray data = inputFile.readAll();
        inputFile.close();
        
        qint64 totalSize = data.size();
        
        // Write file header (for identifying encrypted files)
        QByteArray header = "680ENC01";  // 8-byte header
        outputFile.write(header);
        
        // Write original file size (8 bytes)
        QByteArray sizeBytes;
        sizeBytes.append((char*)&totalSize, sizeof(qint64));
        outputFile.write(sizeBytes);
        
        // Calculate MD5 checksum of original file
        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData(data);
        QByteArray checksum = md5.result();
        outputFile.write(checksum);  // 16-byte MD5
        
        // Encrypt data in blocks
        const int BLOCK_SIZE = 65536;  // 64KB blocks
        int processedSize = 0;
        
        while (processedSize < totalSize) {
            int blockSize = qMin((qint64)BLOCK_SIZE, totalSize - processedSize);
            QByteArray block = data.mid(processedSize, blockSize);
            
            // Encrypt block
            QByteArray encryptedBlock = encryptBlock(block, processedSize);
            outputFile.write(encryptedBlock);
            
            processedSize += blockSize;
            
            // Progress callback
            if (progressCallback) {
                int progress = (int)((processedSize * 100) / totalSize);
                progressCallback(progress);
            }
        }
        
        outputFile.close();
        return true;
    }
    
    // Decrypt file
    static bool decryptFile(const QString &inputPath, const QString &outputPath,
                           std::function<void(int)> progressCallback = nullptr)
    {
        QFile inputFile(inputPath);
        if (!inputFile.open(QIODevice::ReadOnly)) {
            return false;
        }
        
        // Read and validate file header
        QByteArray header = inputFile.read(8);
        if (header != "680ENC01") {
            inputFile.close();
            return false;  // Not a valid encrypted file
        }
        
        // Read original file size
        QByteArray sizeBytes = inputFile.read(sizeof(qint64));
        qint64 originalSize = *((qint64*)sizeBytes.constData());
        
        // Read MD5 checksum
        QByteArray expectedChecksum = inputFile.read(16);
        
        // Read encrypted data
        QByteArray encryptedData = inputFile.readAll();
        inputFile.close();
        
        // Decrypt data
        QByteArray decryptedData;
        const int BLOCK_SIZE = 65536;
        int processedSize = 0;
        qint64 totalEncryptedSize = encryptedData.size();
        
        while (processedSize < totalEncryptedSize) {
            int blockSize = qMin((qint64)BLOCK_SIZE, totalEncryptedSize - processedSize);
            QByteArray block = encryptedData.mid(processedSize, blockSize);
            
            // Decrypt block
            QByteArray decryptedBlock = decryptBlock(block, decryptedData.size());
            decryptedData.append(decryptedBlock);
            
            processedSize += blockSize;
            
            // Progress callback
            if (progressCallback) {
                int progress = (int)((processedSize * 100) / totalEncryptedSize);
                progressCallback(progress);
            }
        }
        
        // Truncate to original size
        decryptedData = decryptedData.left(originalSize);
        
        // Verify MD5 checksum
        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData(decryptedData);
        QByteArray actualChecksum = md5.result();
        
        if (actualChecksum != expectedChecksum) {
            return false;  // Checksum verification failed
        }
        
        // Write decrypted file
        QFile outputFile(outputPath);
        if (!outputFile.open(QIODevice::WriteOnly)) {
            return false;
        }
        
        outputFile.write(decryptedData);
        outputFile.close();
        
        return true;
    }
    
private:
    // Encrypt single data block
    static QByteArray encryptBlock(const QByteArray &data, int offset)
    {
        QByteArray result = data;
        QByteArray keyBytes = PRIVATE_KEY.toUtf8();
        
        // Calculate MD5 hash of private key as mix key
        QCryptographicHash hash(QCryptographicHash::Md5);
        hash.addData(keyBytes);
        QByteArray keyHash = hash.result();
        
        for (int i = 0; i < result.size(); i++) {
            unsigned char dataByte = result[i];
            unsigned char keyByte = keyHash[(i + offset) % keyHash.size()];
            
            // Step 1: Key mixing - XOR then add
            unsigned char mixed = ((dataByte ^ keyByte) + keyByte) % 256;
            
            // Step 2: Bit shift transformation
            int shift = (offset + i) % 7 + 1;
            unsigned char shifted = ((mixed << shift) | (mixed >> (8 - shift))) & 0xFF;
            
            // Step 3: Position-based XOR
            result[i] = shifted ^ ((offset + i) % 256);
        }
        
        return result;
    }
    
    // Decrypt single data block
    static QByteArray decryptBlock(const QByteArray &data, int offset)
    {
        QByteArray result = data;
        QByteArray keyBytes = PRIVATE_KEY.toUtf8();
        
        // Calculate MD5 hash of private key as mix key
        QCryptographicHash hash(QCryptographicHash::Md5);
        hash.addData(keyBytes);
        QByteArray keyHash = hash.result();
        
        for (int i = 0; i < result.size(); i++) {
            unsigned char encByte = result[i];
            unsigned char keyByte = keyHash[(i + offset) % keyHash.size()];
            
            // Reverse step 3: Position-based XOR
            unsigned char unxored = encByte ^ ((offset + i) % 256);
            
            // Reverse step 2: Bit shift transformation
            int shift = (offset + i) % 7 + 1;
            unsigned char unshifted = ((unxored >> shift) | (unxored << (8 - shift))) & 0xFF;
            
            // Reverse step 1: Key mixing
            int temp = (unshifted - keyByte + 256) % 256;
            result[i] = temp ^ keyByte;
        }
        
        return result;
    }
};

// Private key definition
const QString FileEncryptor::PRIVATE_KEY = "ChencyUe2025_680Package";

// Main window class
class EncryptPackageTool : public QWidget
{
    Q_OBJECT

public:
    EncryptPackageTool(QWidget *parent = nullptr) : QWidget(parent)
    {
        setWindowTitle(tr("680 Package Encryption Tool"));
        setMinimumSize(500, 400);
        resize(650, 550);
        
        setupUI();
        
        logEdit->append(tr("Package encryption tool started"));
        logEdit->append(tr("Supports encryption and decryption of tar.gz packages"));
        logEdit->append(QString("Start time: %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
    }

private slots:
    void onSelectSourceFile()
    {
        QString fileName = QFileDialog::getOpenFileName(this,
            tr("Select file to encrypt"), "", 
            tr("Archives (*.tar.gz *.tgz *.gz *.tar);;All files (*)"));
        
        if (!fileName.isEmpty()) {
            sourceFileEdit->setText(fileName);
            
            // Auto-generate output filename
            QFileInfo fileInfo(fileName);
            QString outputName = fileInfo.absolutePath() + "/" + 
                                fileInfo.completeBaseName() + ".enc";
            if (fileInfo.suffix() == "gz" && fileInfo.completeBaseName().endsWith(".tar")) {
                outputName = fileInfo.absolutePath() + "/" + 
                            fileInfo.completeBaseName().left(fileInfo.completeBaseName().length() - 4) + ".tar.gz.enc";
            } else {
                outputName = fileName + ".enc";
            }
            targetFileEdit->setText(outputName);
            
            // Show file info
            QFile file(fileName);
            qint64 fileSize = file.size();
            logEdit->append(QString("Selected file: %1").arg(fileName));
            logEdit->append(QString("File size: %1").arg(formatFileSize(fileSize)));
        }
    }
    
    void onSelectTargetFile()
    {
        QString defaultName = targetFileEdit->text();
        if (defaultName.isEmpty()) {
            defaultName = "encrypted_package.enc";
        }
        
        QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save encrypted file"), defaultName, 
            tr("Encrypted files (*.enc);;All files (*)"));
        
        if (!fileName.isEmpty()) {
            targetFileEdit->setText(fileName);
            logEdit->append(QString("Output file: %1").arg(fileName));
        }
    }
    
    void onEncrypt()
    {
        QString sourceFile = sourceFileEdit->text().trimmed();
        QString targetFile = targetFileEdit->text().trimmed();
        
        if (sourceFile.isEmpty()) {
            QMessageBox::warning(this, tr("Warning"), tr("Please select source file!"));
            return;
        }
        
        if (targetFile.isEmpty()) {
            QMessageBox::warning(this, tr("Warning"), tr("Please specify output file path!"));
            return;
        }
        
        if (!QFile::exists(sourceFile)) {
            QMessageBox::critical(this, tr("Error"), tr("Source file does not exist!"));
            return;
        }
        
        // Confirm overwrite
        if (QFile::exists(targetFile)) {
            int ret = QMessageBox::question(this, tr("Confirm"), 
                tr("Target file already exists. Overwrite?"),
                QMessageBox::Yes | QMessageBox::No);
            if (ret != QMessageBox::Yes) {
                return;
            }
        }
        
        // Disable buttons
        encryptButton->setEnabled(false);
        decryptButton->setEnabled(false);
        progressBar->setValue(0);
        progressBar->setVisible(true);
        
        logEdit->append(tr("Starting encryption..."));
        QDateTime startTime = QDateTime::currentDateTime();
        
        // Execute encryption
        bool success = FileEncryptor::encryptFile(sourceFile, targetFile, 
            [this](int progress) {
                progressBar->setValue(progress);
                QApplication::processEvents();
            });
        
        QDateTime endTime = QDateTime::currentDateTime();
        qint64 elapsed = startTime.msecsTo(endTime);
        
        if (success) {
            QFileInfo outputInfo(targetFile);
            QFileInfo inputInfo(sourceFile);
            
            logEdit->append(tr("Encryption completed!"));
            logEdit->append(QString("  Original file: %1").arg(formatFileSize(inputInfo.size())));
            logEdit->append(QString("  Encrypted file: %1").arg(formatFileSize(outputInfo.size())));
            logEdit->append(QString("  Time elapsed: %1 ms").arg(elapsed));
            logEdit->append(QString("  Output path: %1").arg(targetFile));
            
            QMessageBox::information(this, tr("Success"), 
                QString(tr("File encrypted successfully!\n\nOutput file: %1")).arg(targetFile));
        } else {
            logEdit->append(tr("Encryption failed! Please check file permissions and disk space."));
            QMessageBox::critical(this, tr("Error"), tr("File encryption failed! Please check file permissions and disk space."));
        }
        
        // Restore buttons
        encryptButton->setEnabled(true);
        decryptButton->setEnabled(true);
        progressBar->setVisible(false);
    }
    
    void onDecrypt()
    {
        QString sourceFile = sourceFileEdit->text().trimmed();
        
        if (sourceFile.isEmpty()) {
            // If no file selected, let user choose encrypted file
            sourceFile = QFileDialog::getOpenFileName(this,
                tr("Select file to decrypt"), "", 
                tr("Encrypted files (*.enc);;All files (*)"));
            
            if (sourceFile.isEmpty()) {
                return;
            }
            sourceFileEdit->setText(sourceFile);
        }
        
        if (!QFile::exists(sourceFile)) {
            QMessageBox::critical(this, tr("Error"), tr("Source file does not exist!"));
            return;
        }
        
        // Generate decrypted filename
        QString targetFile;
        if (sourceFile.endsWith(".enc")) {
            targetFile = sourceFile.left(sourceFile.length() - 4);
        } else {
            targetFile = sourceFile + ".dec";
        }
        
        targetFile = QFileDialog::getSaveFileName(this,
            tr("Save decrypted file"), targetFile, 
            tr("All files (*)"));
        
        if (targetFile.isEmpty()) {
            return;
        }
        
        // Confirm overwrite
        if (QFile::exists(targetFile)) {
            int ret = QMessageBox::question(this, tr("Confirm"), 
                tr("Target file already exists. Overwrite?"),
                QMessageBox::Yes | QMessageBox::No);
            if (ret != QMessageBox::Yes) {
                return;
            }
        }
        
        // Disable buttons
        encryptButton->setEnabled(false);
        decryptButton->setEnabled(false);
        progressBar->setValue(0);
        progressBar->setVisible(true);
        
        logEdit->append(tr("Starting decryption..."));
        QDateTime startTime = QDateTime::currentDateTime();
        
        // Execute decryption
        bool success = FileEncryptor::decryptFile(sourceFile, targetFile, 
            [this](int progress) {
                progressBar->setValue(progress);
                QApplication::processEvents();
            });
        
        QDateTime endTime = QDateTime::currentDateTime();
        qint64 elapsed = startTime.msecsTo(endTime);
        
        if (success) {
            QFileInfo outputInfo(targetFile);
            
            logEdit->append(tr("Decryption completed!"));
            logEdit->append(QString("  Decrypted file: %1").arg(formatFileSize(outputInfo.size())));
            logEdit->append(QString("  Time elapsed: %1 ms").arg(elapsed));
            logEdit->append(QString("  Output path: %1").arg(targetFile));
            
            QMessageBox::information(this, tr("Success"), 
                QString(tr("File decrypted successfully!\n\nOutput file: %1")).arg(targetFile));
        } else {
            logEdit->append(tr("Decryption failed! File may not be a valid encrypted file or is corrupted."));
            QMessageBox::critical(this, tr("Error"), 
                tr("File decryption failed!\n\nPossible reasons:\n1. File is not a valid encrypted file\n2. File is corrupted\n3. Encryption key mismatch"));
        }
        
        // Restore buttons
        encryptButton->setEnabled(true);
        decryptButton->setEnabled(true);
        progressBar->setVisible(false);
    }
    
    void onClearLog()
    {
        logEdit->clear();
        logEdit->append(tr("Log cleared"));
    }

private:
    void setupUI()
    {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setSpacing(12);
        mainLayout->setContentsMargins(16, 16, 16, 16);
        
        // Title
        QLabel *titleLabel = new QLabel(tr("680 Package Encryption Tool"));
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setStyleSheet(
            "font-size: 18px; font-weight: bold; color: #2c3e50; "
            "padding: 12px; background-color: #ecf0f1; border-radius: 6px;"
        );
        titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        
        // File selection area
        QGroupBox *fileGroup = new QGroupBox(tr("File Selection"));
        fileGroup->setStyleSheet(
            "QGroupBox { font-weight: bold; border: 2px solid #bdc3c7; "
            "border-radius: 6px; margin-top: 12px; padding: 12px; padding-top: 20px; } "
            "QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; }"
        );
        fileGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        
        QVBoxLayout *fileLayout = new QVBoxLayout(fileGroup);
        fileLayout->setSpacing(8);
        
        // Source file
        QLabel *sourceLabel = new QLabel(tr("Source file (tar.gz):"));
        sourceFileEdit = new QLineEdit();
        sourceFileEdit->setPlaceholderText(tr("Select tar.gz package to encrypt..."));
        sourceFileEdit->setStyleSheet("padding: 8px; border: 1px solid #bdc3c7; border-radius: 4px;");
        sourceFileEdit->setMinimumHeight(36);
        sourceFileEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        
        QPushButton *browseSourceButton = new QPushButton(tr("Browse..."));
        browseSourceButton->setStyleSheet(
            "QPushButton { background-color: #3498db; color: white; padding: 8px 16px; "
            "border-radius: 4px; font-weight: bold; min-width: 80px; } "
            "QPushButton:hover { background-color: #2980b9; }"
        );
        browseSourceButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        
        QHBoxLayout *sourceLayout = new QHBoxLayout();
        sourceLayout->setSpacing(8);
        sourceLayout->addWidget(sourceFileEdit, 1);
        sourceLayout->addWidget(browseSourceButton);
        
        // Target file
        QLabel *targetLabel = new QLabel(tr("Output file (encrypted):"));
        targetFileEdit = new QLineEdit();
        targetFileEdit->setPlaceholderText(tr("Encrypted file will be saved to this path..."));
        targetFileEdit->setStyleSheet("padding: 8px; border: 1px solid #bdc3c7; border-radius: 4px;");
        targetFileEdit->setMinimumHeight(36);
        targetFileEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        
        QPushButton *browseTargetButton = new QPushButton(tr("Browse..."));
        browseTargetButton->setStyleSheet(
            "QPushButton { background-color: #3498db; color: white; padding: 8px 16px; "
            "border-radius: 4px; font-weight: bold; min-width: 80px; } "
            "QPushButton:hover { background-color: #2980b9; }"
        );
        browseTargetButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        
        QHBoxLayout *targetLayout = new QHBoxLayout();
        targetLayout->setSpacing(8);
        targetLayout->addWidget(targetFileEdit, 1);
        targetLayout->addWidget(browseTargetButton);
        
        fileLayout->addWidget(sourceLabel);
        fileLayout->addLayout(sourceLayout);
        fileLayout->addSpacing(4);
        fileLayout->addWidget(targetLabel);
        fileLayout->addLayout(targetLayout);
        
        // Progress bar
        progressBar = new QProgressBar();
        progressBar->setVisible(false);
        progressBar->setMinimumHeight(24);
        progressBar->setStyleSheet(
            "QProgressBar { border: 1px solid #bdc3c7; border-radius: 4px; text-align: center; } "
            "QProgressBar::chunk { background-color: #27ae60; }"
        );
        progressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        
        // Action buttons
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->setSpacing(10);
        
        encryptButton = new QPushButton(tr("Encrypt File"));
        encryptButton->setStyleSheet(
            "QPushButton { background-color: #e74c3c; color: white; padding: 12px 28px; "
            "border-radius: 5px; font-size: 14px; font-weight: bold; min-width: 120px; } "
            "QPushButton:hover { background-color: #c0392b; } "
            "QPushButton:disabled { background-color: #bdc3c7; }"
        );
        encryptButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        
        decryptButton = new QPushButton(tr("Decrypt File"));
        decryptButton->setStyleSheet(
            "QPushButton { background-color: #27ae60; color: white; padding: 12px 28px; "
            "border-radius: 5px; font-size: 14px; font-weight: bold; min-width: 120px; } "
            "QPushButton:hover { background-color: #219a52; } "
            "QPushButton:disabled { background-color: #bdc3c7; }"
        );
        decryptButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        
        QPushButton *clearLogButton = new QPushButton(tr("Clear Log"));
        clearLogButton->setStyleSheet(
            "QPushButton { background-color: #f39c12; color: white; padding: 10px 16px; "
            "border-radius: 4px; } "
            "QPushButton:hover { background-color: #d68910; }"
        );
        clearLogButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        
        QPushButton *exitButton = new QPushButton(tr("Exit"));
        exitButton->setStyleSheet(
            "QPushButton { background-color: #95a5a6; color: white; padding: 10px 16px; "
            "border-radius: 4px; } "
            "QPushButton:hover { background-color: #7f8c8d; }"
        );
        exitButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        
        buttonLayout->addWidget(encryptButton);
        buttonLayout->addWidget(decryptButton);
        buttonLayout->addStretch(1);
        buttonLayout->addWidget(clearLogButton);
        buttonLayout->addWidget(exitButton);
        
        // Info label
        QLabel *infoLabel = new QLabel(
            tr("Usage Instructions:\n"
            "* Encrypt: Select tar.gz file, click [Encrypt File] button, generates .enc encrypted file\n"
            "* Decrypt: Select .enc encrypted file, click [Decrypt File] button, restores original file\n"
            "* Encrypted files contain integrity checksums to prevent tampering")
        );
        infoLabel->setStyleSheet(
            "color: #2980b9; background-color: #ebf5fb; padding: 12px; "
            "border: 1px solid #aed6f1; border-radius: 4px; font-size: 12px;"
        );
        infoLabel->setWordWrap(true);
        infoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        
        // Log area with header
        QHBoxLayout *logHeaderLayout = new QHBoxLayout();
        QLabel *logLabel = new QLabel(tr("Operation Log:"));
        logLabel->setStyleSheet("font-weight: bold; color: #2c3e50;");
        logHeaderLayout->addWidget(logLabel);
        logHeaderLayout->addStretch();
        
        logEdit = new QTextEdit();
        logEdit->setReadOnly(true);
        logEdit->setMinimumHeight(100);
        logEdit->setStyleSheet(
            "QTextEdit { background-color: #2c3e50; color: #ecf0f1; "
            "font-family: 'Consolas', 'Courier New', monospace; font-size: 11px; "
            "border: 1px solid #34495e; border-radius: 4px; padding: 8px; }"
        );
        logEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        
        // Add to main layout with proper stretch factors
        mainLayout->addWidget(titleLabel, 0);
        mainLayout->addWidget(fileGroup, 0);
        mainLayout->addWidget(progressBar, 0);
        mainLayout->addLayout(buttonLayout, 0);
        mainLayout->addWidget(infoLabel, 0);
        mainLayout->addLayout(logHeaderLayout, 0);
        mainLayout->addWidget(logEdit, 1);  // Log area expands
        
        // Connect signals
        connect(browseSourceButton, &QPushButton::clicked, this, &EncryptPackageTool::onSelectSourceFile);
        connect(browseTargetButton, &QPushButton::clicked, this, &EncryptPackageTool::onSelectTargetFile);
        connect(encryptButton, &QPushButton::clicked, this, &EncryptPackageTool::onEncrypt);
        connect(decryptButton, &QPushButton::clicked, this, &EncryptPackageTool::onDecrypt);
        connect(clearLogButton, &QPushButton::clicked, this, &EncryptPackageTool::onClearLog);
        connect(exitButton, &QPushButton::clicked, this, &QWidget::close);
    }
    
    QString formatFileSize(qint64 bytes)
    {
        if (bytes < 1024) {
            return QString("%1 B").arg(bytes);
        } else if (bytes < 1024 * 1024) {
            return QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 2);
        } else if (bytes < 1024 * 1024 * 1024) {
            return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 2);
        } else {
            return QString("%1 GB").arg(bytes / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
        }
    }

private:
    QLineEdit *sourceFileEdit;
    QLineEdit *targetFileEdit;
    QPushButton *encryptButton;
    QPushButton *decryptButton;
    QProgressBar *progressBar;
    QTextEdit *logEdit;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application info
    app.setApplicationName("680 Package Encryption Tool");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Chency");
    
    EncryptPackageTool tool;
    tool.show();
    
    return app.exec();
}

#include "encrypt_package.moc"
