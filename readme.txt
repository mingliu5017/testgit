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

三、git 撤销修改与删除文件
1、撤销未放到缓存区修改
git checkout -- readme.txt
2、git删除文件
rm test.txt
git commit -m "删除 test.txt"
3、未提交时恢复
git checkout -- test.txt

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
git merge dev
3、删除dev分支
git branch -d dev
4、查看分支
git branch -a

