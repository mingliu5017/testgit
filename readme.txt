Github : https://github.com/mingliu5017?tab=repositories
账号：mingliu5017
邮箱：673100664@qq.com
密码：Lm3625285017

*************  Git 使用教程 ****************

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
add前：git diff readme.txt
add后：git diff --staged readme.txt
6、git log 查看提交日志
git log --pretty=oneline
7、查看所有提交记录
git reflog

二、版本回退
1、git 回退上一版本
git reset --hard HEAD^
2、git 回退到前100个版本
git reset --hard HEAD~100
3、获取git log版本号
git relog
4、回退到固定版本
git reset --hard 7c0f72c

三、git 撤销修改与删除文件
1、撤销未放到缓存区修改
git checkout -- readme.txt
2、git删除文件
rm test.txt
git commit -m "删除 test.txt"
git rm readme.txt
3、未提交时恢复
git checkout -- test.txt
git checkout .

四、远程仓库
1、./ssh 下查看 SSH key
cat ./id_rsa.pub
2、登录github,打开” settings”中的SSH Keys页面，然后点击“Add SSH Key”,填上任意title，在Key文本框里黏贴id_rsa.pub文件的内容
3、登录github上，然后在右上角找到“create a new repo”创建一个新的仓库。
4、添加远程仓库，在本地的testgit仓库下运行命令
git remote add origin https://github.com/mingliu5017/testgit.git
5、把本地库推送到远程仓库
git push -u origin master
6、远程库克隆到本地
git clone https://github.com/mingliu5017/testgit.git

五、创建与合并分支
1、创建分支
git branch dev
git checkout dev
或者
git checkout -b dev
2、分支合并，master 分支上合并dev分支,合并某分支到当前分支
Fast forward ：git merge dev
git merge --no-ff -m "merge fixed bug 404" issue-404
3、删除dev分支
git branch -d dev
4、查看分支
git branch -a
5、切换到远程分支
git checkout --track remotes/origin/master

六、bug 临时分支管理
1、将当前工作现场隐藏
git stash
2、隐藏内容恢复
git stash pop

七、远程同步,多人协作
1、显示所有远程仓库
git remote -v
2、下载远程仓库所有变动
git fetch origin
3、抓取分支
git pull --set-upstream master origin/master
git pull
git pull origin master
4、推送分支
git push origin master

八、git path 生成与使用
1、提交的patch文件生成
git format-patch -1 5392bd4c
2、未提交的patch文件生成
git diff readme.txt > readme.txt.patch
3、检查patch 文件
git apply --stat 0001-pc-git.patch
4、检查能否应用成功
git apply --check 0001-pc-git.patch
5、打补丁
git am < 0001-pc-git.patch
6、手动打patch
patch -p1 < 0001-pc-git.patch

