bash循环读入文件的每一行并处理

cat afile | while read oneline
do
echo $oneline
done
这只是个简单例子，把文件afile的每一行读入，然后显示出来。

也可以这样(如果行中有空格或者tab则无法使用此方法,不好意思啊,呵呵):
for i in `cat afile`
do
echo $i
done
