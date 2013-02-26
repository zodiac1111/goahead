#!/bin/bash 
#使用的文件模板
VER_FILE_TEMPLATE=version-template.h
#替换模板文件中的文字

#生成的版本文件名
VER_FILE=version.h

git rev-list HEAD | sort > config.git-hash
LOCALVER=`wc -l config.git-hash | awk '{print $1}'`
echo "LOCALVER:" $LOCALVER

if [ $LOCALVER \> 1 ]  ; then
    VER=`git rev-list origin/master | sort | join config.git-hash - | wc -l | awk '{print $1}'`
    PATHLEVEL=$VER
    if [ $VER != $LOCALVER ]  ; then
        VER="$VER+$(($LOCALVER-$VER))"
    fi
    if git status | grep -q "modified:"  ; then
        VER="${VER}M"
    fi
    #VER="$VER $(git rev-list HEAD -n 1 | cut -c 1-7)"
    VER="$VER-$(git rev-list HEAD -n 1 | cut -c 1-7)"
    GIT_VERSION=r$VER
else
    PATHLEVEL=
    GIT_VERSION=
    VER="x"
fi
rm -f config.git-hash

echo -e "PATHLEVEL:\e[31m" $PATHLEVEL "\e[0m"
echo -e "VER:\e[31m"  $VER "\e[0m"
echo -e "GIT_VERSION:\e[31m"  $GIT_VERSION "\e[0m"
echo -e "Generated\e[31m" $VER_FILE  "\e[0m"

#git log --pretty=format:'[%h] %ad %s' --date=short  > dev.log
#echo `git log --pretty=format:'%ad(%h)%s.\n' --date=short` |
#sed -e 's/\\n/\n/g'
#生成并替换相应的字段
cat $VER_FILE_TEMPLATE | \
sed "s/\$PATHLEAVE/$PATHLEVEL/g" |
sed "s/\$GIT_VERSION/$GIT_VERSION/g" > $VER_FILE 

