#ifndef PLATFORMHELPER_H
#define PLATFORMHELPER_H

#include <QString>
#include <QStringList>
#include <QSysInfo>
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QTextCodec>

/**
 * @brief 平台兼容性辅助类
 * 
 * 提供跨平台的系统调用、路径处理、程序执行等功能
 * 特别优化了对Linux和ARM平台的支持
 */
class PlatformHelper {
public:
    // ===================平台检测=================== 
    
    /// 判断是否为Windows平台
    static bool isWindows() {
        return QSysInfo::productType() == "windows";
    }
    
    /// 判断是否为Linux平台
    static bool isLinux() {
        return QSysInfo::productType() == "linux";
    }
    
    /// 判断是否为macOS平台
    static bool isMacOS() {
        return QSysInfo::productType() == "osx" || QSysInfo::productType() == "macos";
    }
    
    /// 判断是否为Unix-like系统（Linux/macOS/BSD）
    static bool isUnixLike() {
        return isLinux() || isMacOS() || QSysInfo::productType().contains("bsd");
    }
    
    /// 判断是否为ARM架构
    static bool isArm() {
        QString arch = QSysInfo::currentCpuArchitecture().toLower();
        return arch.contains("arm") || arch.contains("aarch64") || arch.contains("arm64");
    }
    
    /// 判断是否为64位架构
    static bool is64Bit() {
        QString arch = QSysInfo::currentCpuArchitecture().toLower();
        return arch.contains("64") || arch.contains("x86_64") || arch.contains("amd64");
    }
    
    // ===================程序和工具检测=================== 
    
    /// 获取Python解释器名称（自动检测可用版本）
    static QString getPythonInterpreter() {
        if (isWindows()) {
            // Windows上优先检测python3，再检测python
            QStringList pythons = {"python3", "python", "py"};
            for (const QString &python : pythons) {
                if (!QStandardPaths::findExecutable(python).isEmpty()) {
                    return python;
                }
            }
            return "python"; // 默认回退
        } else {
            // Unix-like系统优先使用python3
            QStringList pythons = {"python3", "python3.9", "python3.8", "python3.7", "python"};
            for (const QString &python : pythons) {
                if (!QStandardPaths::findExecutable(python).isEmpty()) {
                    return python;
                }
            }
            return "python3";
        }
    }
    
    /// 获取Shell程序名称
    static QString getShellProgram() {
        if (isWindows()) {
            return "cmd";
        } else {
            // Unix-like系统优先检测可用的shell
            QStringList shells = {"bash", "sh", "zsh", "dash"};
            for (const QString &shell : shells) {
                if (!QStandardPaths::findExecutable(shell).isEmpty()) {
                    return shell;
                }
            }
            return "sh"; // 最基本的shell
        }
    }
    
    /// 获取终端程序名称（智能检测）
    static QString getTerminalProgram() {
        if (isWindows()) {
            return "cmd";
        } else if (isLinux()) {
            // Linux上按优先级检测多种终端程序
            QStringList terminals = {
                "gnome-terminal",  // GNOME桌面
                "konsole",         // KDE桌面  
                "xterm",           // 通用X11终端
                "mate-terminal",   // MATE桌面
                "xfce4-terminal",  // XFCE桌面
                "lxterminal",      // LXDE桌面
                "qterminal",       // LXQt桌面
                "alacritty",       // 现代终端
                "terminator",      // 高级终端
                "x-terminal-emulator" // Debian/Ubuntu通用链接
            };
            
            for (const QString &terminal : terminals) {
                if (!QStandardPaths::findExecutable(terminal).isEmpty()) {
                    return terminal;
                }
            }
            return "xterm"; // 最基本的终端
        } else {
            return "Terminal"; // macOS
        }
    }
    
    /// 检测SSH客户端程序
    static QString getSSHProgram() {
        QStringList sshPrograms = {"ssh", "openssh"};
        for (const QString &ssh : sshPrograms) {
            if (!QStandardPaths::findExecutable(ssh).isEmpty()) {
                return ssh;
            }
        }
        return "ssh"; // 默认
    }
    
    /// 检测SCP程序
    static QString getSCPProgram() {
        QStringList scpPrograms = {"scp", "openssh-scp"};
        for (const QString &scp : scpPrograms) {
            if (!QStandardPaths::findExecutable(scp).isEmpty()) {
                return scp;
            }
        }
        return "scp"; // 默认
    }
    
    // ===================命令行参数处理=================== 
    
    /// 获取平台特定的Shell命令行参数
    static QStringList getShellArgs(const QString &command) {
        if (isWindows()) {
            return {"/c", command};
        } else {
            return {"-c", command};
        }
    }
    
    /// 获取终端程序的命令行参数
    static QStringList getTerminalArgs(const QString &command) {
        QString terminal = getTerminalProgram();
        
        if (isWindows()) {
            return {"/c", "start", "cmd", "/k", command};
        } else if (terminal == "gnome-terminal") {
            return {"--", "bash", "-c", command + "; read -p '按Enter键继续...'"};
        } else if (terminal == "konsole") {
            return {"-e", "bash", "-c", command + "; read -p '按Enter键继续...'"};
        } else if (terminal == "xterm" || terminal == "mate-terminal" || terminal == "xfce4-terminal") {
            return {"-e", "bash", "-c", command + "; read -p '按Enter键继续...'"};
        } else {
            // 通用参数
            return {"-e", "bash", "-c", command + "; read -p '按Enter键继续...'"};
        }
    }
    
    // ===================路径处理=================== 
    
    /// 处理路径格式（平台自适应）
    static QString formatPath(const QString &path) {
        if (isWindows()) {
            return QDir::toNativeSeparators(path);
        } else {
            return QDir::fromNativeSeparators(path); // 确保Unix风格
        }
    }
    
    /// 转义路径中的特殊字符
    static QString escapePath(const QString &path) {
        if (isWindows()) {
            // Windows路径转义
            QString escaped = path;
            if (escaped.contains(' ') && !escaped.startsWith('"')) {
                escaped = '"' + escaped + '"';
            }
            return escaped;
        } else {
            // Unix路径转义
            QString escaped = path;
            escaped.replace(' ', "\\ ");
            escaped.replace('(', "\\(");
            escaped.replace(')', "\\)");
            escaped.replace('&', "\\&");
            escaped.replace(';', "\\;");
            escaped.replace('|', "\\|");
            return escaped;
        }
    }
    
    /// 获取用户SSH目录
    static QString getSSHDirectory() {
        if (isWindows()) {
            return QDir::homePath() + "/.ssh";
        } else {
            return QDir::homePath() + "/.ssh";
        }
    }
    
    // ===================编码处理=================== 
    
    /// 获取平台默认文本编码
    static QTextCodec* getDefaultTextCodec() {
        if (isWindows()) {
            // Windows通常使用GBK或UTF-8
            QTextCodec* codec = QTextCodec::codecForName("GBK");
            if (!codec) codec = QTextCodec::codecForName("UTF-8");
            return codec;
        } else {
            // Unix系统通常使用UTF-8
            return QTextCodec::codecForName("UTF-8");
        }
    }
    
    /// 智能解码字节数据
    static QString decodeBytes(const QByteArray &data) {
        if (data.isEmpty()) return QString();
        
        // 先尝试UTF-8
        QString utf8Text = QString::fromUtf8(data);
        if (!utf8Text.contains(QChar::ReplacementCharacter)) {
            return utf8Text;
        }
        
        // UTF-8失败，使用系统默认编码
        if (isWindows()) {
            return QString::fromLocal8Bit(data);
        } else {
            return QString::fromLatin1(data);
        }
    }
    
    // ===================系统信息=================== 
    
    /// 获取详细的平台信息字符串
    static QString getPlatformInfo() {
        QString info = QString("OS: %1 (%2), Arch: %3")
                      .arg(QSysInfo::productType())
                      .arg(QSysInfo::productVersion())
                      .arg(QSysInfo::currentCpuArchitecture());
        
        QStringList features;
        if (isArm()) features << "ARM";
        if (is64Bit()) features << "64-bit";
        if (!features.isEmpty()) {
            info += " [" + features.join(", ") + "]";
        }
        
        return info;
    }
    
    /// 获取系统环境变量
    static QString getEnvironmentVariable(const QString &name, const QString &defaultValue = QString()) {
        QString value = qgetenv(name.toLocal8Bit().constData());
        return value.isEmpty() ? defaultValue : value;
    }
    
    // ===================进程执行优化=================== 
    
    /// 设置进程的平台特定环境变量
    static void setupProcessEnvironment(QProcess *process) {
        if (!process) return;
        
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        
        if (isWindows()) {
            // Windows特定环境变量
            env.insert("PYTHONIOENCODING", "utf-8");
            env.insert("PYTHONDONTWRITEBYTECODE", "1");
        } else {
            // Unix特定环境变量
            env.insert("LC_ALL", "C.UTF-8");
            env.insert("LANG", "C.UTF-8");
            env.insert("PYTHONIOENCODING", "utf-8");
            env.insert("TERM", "xterm-256color");
        }
        
        process->setProcessEnvironment(env);
    }
    
    /// 获取推荐的进程超时时间（毫秒）
    static int getProcessTimeout() {
        if (isArm()) {
            // ARM设备可能较慢，给更多时间
            return 30000; // 30秒
        } else {
            return 15000; // 15秒
        }
    }
};

#endif // PLATFORMHELPER_H 