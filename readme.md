	解压缩图片，直接显示到FrameBuffer正中央位置


	使用方法:

		compile:
			cmake . && make
		    or
			arm-linux-gnueabihf-g++ -std=c++11 fb-images.cpp -o splash

		running example: ./splash /dev/fb1 /home/root/images/


	支持的图片格式：

		Tga: 24、32位 压缩,非压缩

		Bmp: 24位 非压缩

	注：FrameBuffer位深度应与Tga图片位数相对应
		即FrameBuffer 24位　显示　24位Tga图片
