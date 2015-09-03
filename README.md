# RememberKeyWP

A tools to remember passwords in Windows Phone.

## Motivation

一来是为了练手。

二来是当前的密码管理工具缺少跨平台，不仅是桌面跨平台，而且也跨移动平台，能够备份数据到云端。
当前满足要求的有 [LastPass](https://lastpass.com/), 桌面端免费，可惜移动端需要收费，
我的密码还没有需要付费的价值。另一个是 [KeePass](http://www.keepass.info/),也就是我现在
在用的，可是没有云端一键上传下载，导致我桌面两台机器两个版本，手机一个版本，又懒得去合并。

懒得去整 keepass，却有时间练手开发，真汗！

不过还是练手吧，因为 KeePass 很赞。

## Progress

桌面端客户端使用 Qt + openssl + sqlite 开发，已经初步完成，使用的是跨平台的套件，
目前在 Win10，FreeBSD 下测试通过，可能可以编译支持 Android。

Windows Phone 端的开发刚刚开始。

## Dependency

使用 QtCreator 编译，需要 

* Qt5
* openssl
* sqlite

在 windows 下，使用 openssl，需要安装 [Win32OpenSSL](http://slproweb.com/products/Win32OpenSSL.html)，
并编辑 RememberKey.pro 文件，修改路径。

这里提供一个 windows 下的打包下载。

Qt 下的 OneDrive Rest 客户端，使用并修改了 [AndreyMacritskiy/QtOneDrive](https://github.com/AndreyMacritskiy/QtOneDrive)。

## Oh, My GOD!

暴露自己 Windows 应用的 ClientID 和 SecretKey 了。

OneDrive 需求权限 wl.signin（登录），wl.skydrive（读取），wl.skydrive_update（写入），
wl.offline_access（可 Refresh Token 再次自动登陆）