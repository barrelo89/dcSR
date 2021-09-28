# dcSR: Practical Video Quality Enhancement using Data-Centric Super-Resolution
This is the repository that contains all the python and ffmpeg source code for the publication accepted in ACM CONEXT'21. You can access the paper through this link.

## Prerequisities
- Environment: Ubuntu 16.04 or Ubuntu 18.04
- Language: Python (Super Resolution), C (H264 Video Decoding)
- Need to install ['FFMPEG'](https://www.ffmpeg.org/download.html) from "source".

## How to install FFMPEG from source code
1. Download FFMPEG source code (We used ffmpeg-4.2.1, which can be found in this [link](https://ffmpeg.org/releases/))
2. Unzip "ffmpeg-X.X.X.tar" file (henceforth, we use "ffmpeg-4.2.1.tar").
3. Navigate to 'ffmpeg-4.2.1/ffmpeg-4.2.1' and check the file in the path.
```
cd ffmpeg-4.2.1
ls
```
You can see the directory contains the following files.

![image](https://user-images.githubusercontent.com/25336939/135013433-94da5fbd-ff44-4c8a-9db6-c3197d85c4ed.png)

4. Now, let's compile FFMPEG. But there is one thing to note before you run './configure' in command window. Since the H.264 is not enabled in default FFMPEG configuration, we need to configure FFMPEG with '--enable-gpl --enable-libx264' [reference](https://trac.ffmpeg.org/wiki/CompilationGuide/Ubuntu#:~:text=libx264,118%20then%20you%20can%20install%20that%20instead%20of%20compiling%3A)  
```
./configure --enable-gpl --enable-libx264
```
5. Once the configuration is completed, compile and install FFMPEG (it may require sudo). 
```
sudo make
sudo make install
```
