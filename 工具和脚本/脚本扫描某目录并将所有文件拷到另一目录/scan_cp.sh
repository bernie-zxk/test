#!/bin/bash
ORI_PATH=/home/d5000/huazhong/tmp/test/adir
OBJ_PATH=/home/d5000/huazhong/tmp/test/bdir

echo `ls $ORI_PATH`
#列出源目录所有文件，循环存入file变量
for file in `ls $ORI_PATH`
do
 echo $file
#判断源目录ls结果，哪个是regular文件（不是目录或设备文件），是则拷贝到目标目录并在源目录删除。
 if [ -f $ORI_PATH/$file ]
 then
   cp $ORI_PATH/$file $OBJ_PATH
   echo "cp $ORI_PATH/$file "
   rm $ORI_PATH/$file
   echo "rm $ORI_PATH/$file"
 fi
done