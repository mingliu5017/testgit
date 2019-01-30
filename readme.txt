一、git 创建与文件提交
1、git init把这个目录变成git可以管理的仓库
git init
2、使用命令 git add readme.txt添加到暂存区里面去
git add readme.txt
3、用命令 git commit告诉Git，把文件提交到仓库
git commit -m "readme.txt 提交"
4、git status来查看是否还有文件未提交
git status
5、git diff 查看修改
git diff readme.txt
6、git log 查看提交日志
git log --pretty=oneline

二、版本回退
1、git 回退上一版本
git reset --hard HEAD^
2、git 回退到前100个版本
git reset --hard HEAD~100
3、获取git log版本号
git relog
4、回退到固定版本
git reset --hard 7c0f72c
