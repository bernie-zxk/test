#!/bin/bash
ORI_PATH=/home/d5000/huazhong/tmp/test/adir
OBJ_PATH=/home/d5000/huazhong/tmp/test/bdir

echo `ls $ORI_PATH`
#�г�ԴĿ¼�����ļ���ѭ������file����
for file in `ls $ORI_PATH`
do
 echo $file
#�ж�ԴĿ¼ls������ĸ���regular�ļ�������Ŀ¼���豸�ļ��������򿽱���Ŀ��Ŀ¼����ԴĿ¼ɾ����
 if [ -f $ORI_PATH/$file ]
 then
   cp $ORI_PATH/$file $OBJ_PATH
   echo "cp $ORI_PATH/$file "
   rm $ORI_PATH/$file
   echo "rm $ORI_PATH/$file"
 fi
done