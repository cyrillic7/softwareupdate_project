# SSH设置指南 - Windows下的密码认证解决方案

## 问题说明

在Windows下，标准的SSH客户端不支持自动密码认证，导致连接时会卡在密码输入界面。

## 解决方案

### 方案1：安装PuTTY（推荐）

1. **下载PuTTY套件**：
   - 访问：https://www.putty.org/
   - 下载完整的PuTTY套件（包含plink和pscp）

2. **安装到系统PATH**：
   - 将PuTTY安装目录添加到系统环境变量PATH中
   - 或者将putty.exe、plink.exe、pscp.exe复制到程序目录下

3. **验证安装**：
   ```cmd
   plink -V
   pscp
   ```

### 方案2：使用SSH密钥认证

1. **生成SSH密钥对**：
   ```cmd
   ssh-keygen -t rsa -b 2048 -f ~/.ssh/id_rsa
   ```

2. **将公钥上传到服务器**：
   ```cmd
   ssh-copy-id -i ~/.ssh/id_rsa.pub ubuntu@1.13.80.192
   ```

3. **配置程序使用密钥认证**：
   - 程序会自动使用默认的SSH密钥

### 方案3：使用WSL（Windows Subsystem for Linux）

1. **启用WSL**：
   ```cmd
   wsl --install
   ```

2. **在WSL中安装sshpass**：
   ```bash
   sudo apt update
   sudo apt install sshpass
   ```

3. **配置程序使用WSL中的SSH工具**：
   - 修改程序使用WSL路径：`wsl sshpass` 和 `wsl ssh`

## 当前程序支持的工具

程序会按以下顺序自动检测和使用：

1. **sshpass** - Linux/WSL环境下的密码认证工具
2. **plink/pscp** - PuTTY套件的命令行工具
3. **ssh/scp** - 标准SSH工具（仅支持密钥认证）

## 测试连接

完成上述任一方案后：

1. 重新启动程序
2. 输入服务器信息
3. 点击"测试连接"
4. 查看日志中显示使用的认证方式

## 常见问题

### Q: 为什么Windows下SSH连接会超时？
A: Windows下的SSH客户端无法自动处理密码输入，需要额外的工具支持。

### Q: 推荐使用哪种方案？
A: 推荐使用PuTTY套件，因为它在Windows下兼容性最好，且支持密码认证。

### Q: SSH密钥认证是否更安全？
A: 是的，SSH密钥认证比密码认证更安全，且无需额外工具支持。

## 联系支持

如果仍有问题，请联系：
- 邮箱：121888719@qq.com
- 提供详细的错误日志和系统信息 