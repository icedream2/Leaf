两个版本的jar
LeafDownText 只下载叶子对应的id和名字 输出格式为ID：xxx 中文名：xxx 英文名：xxx
使用方法，命令行>java -jar LeafDownPic.jar [beginId] [endId] [downloadPath]; 
举例： java -jar LeafDownPic.jar 1 100 D:/123;会在D：/123下保存一个text文件test.txt
使用此版本，图片下载可用迅雷完成。

LeafDownPic 叶子对应的id和名字连同叶子图片一块保存在路径下 输出格式为ID：xxx 中文名：xxx 英文名：xxx
使用方法，命令行>java -jar LeafDownPic.jar [beginId] [endId] [downloadPath]; 
举例： java -jar LeafDownPic.jar 1 100 D:/123;会在D：/123下保存一个text文件test.txt


LeafDL 下载对应id的叶子图片
用法：命令行>java -jar LeafDL.jar  [downloadPath] [beginId] [endId] ; 批量下载
或：命令行>java -jar LeafDL.jar  [downloadPath] [Id]; 单独下载

另外搭建了数据库mysql下的leafdata，其中表leaf为原生表，id为网上的对应id。leafc为分好类之后的表。对应id，名字，图片数量生成在一个txt文件中
可用来查表获得想下载的图片