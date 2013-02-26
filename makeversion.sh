#!/bin/bash 
#使用的文件模板
VER_FILE_TEMPLATE=version.h.template
#替换模板文件中的文字

#生成的版本文件名
VER_FILE=version.h

git rev-list HEAD | sort > config.git-hash
LOCALVER=`wc -l config.git-hash | awk '{print $1}'`
echo "LOCALVER:" $LOCALVER

if [ $LOCALVER \> 1 ]  ; then
    VER=`git rev-list origin/master | sort | join config.git-hash - | wc -l | awk '{print $1}'`
    if [ $VER != $LOCALVER ]  ; then
        VER="$VER+$(($LOCALVER-$VER))"
    fi
    if git status | grep -q "modified:"  ; then
        VER="${VER}M"
    fi
    #VER="$VER $(git rev-list HEAD -n 1 | cut -c 1-7)"
    PATHLEAVE=r$VER
    VER="$VER-$(git rev-list HEAD -n 1 | cut -c 1-7)"
    GIT_VERSION=$VER
else
    PATHLEAVE=
    GIT_VERSION=
    VER="x"
fi
rm -f config.git-hash

echo "VER: " $VER
echo "GIT_VERSION: " $GIT_VERSION
echo "Generated" $VER_FILE 

#git log --pretty=format:'[%h] %ad %s' --date=short  > dev.log
#echo `git log --pretty=format:'%ad(%h)%s.\n' --date=short` |
#sed -e 's/\\n/\n/g'
#生成并替换相应的字段
cat $VER_FILE_TEMPLATE | \
sed "s/\$PATHLEAVE/$PATHLEAVE/g" > $VER_FILE 

