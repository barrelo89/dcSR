# dcSR: Practical Video Quality Enhancement using Data-Centric Super-Resolution
This is the repository that contains all the python and ffmpeg source code for the publication accepted in ACM CONEXT'21. You can access the paper through this link.

## Prerequisities
- Environment: Ubuntu 16.04 or Ubuntu 18.04
- Language: Python (Super Resolution), C (H264 Video Decoding)
- Required Python packages: tensorflow, opencv
- Need to install ['FFMPEG'](https://www.ffmpeg.org/download.html) from "source".

## How to install FFMPEG from source code
1. Download FFMPEG source code (We used ffmpeg-4.2.1, which can be found in this [link](https://ffmpeg.org/releases/))
2. Unzip "ffmpeg-X.X.X.tar" file (henceforth, we use "ffmpeg-4.2.1.tar").
3. Navigate to 'ffmpeg-4.2.1' and check the file in the path.
```
cd ffmpeg-4.2.1
ls
```
You can see the directory contains the following files.

![image](https://user-images.githubusercontent.com/25336939/135013433-94da5fbd-ff44-4c8a-9db6-c3197d85c4ed.png)

4. Now, let's compile FFMPEG. But there is one thing to note before you run './configure' in command window. Since the H.264 is not enabled by default FFMPEG configuration, we need to configure FFMPEG with '--enable-gpl --enable-libx264' [[reference]](https://trac.ffmpeg.org/wiki/CompilationGuide/Ubuntu#:~:text=libx264,118%20then%20you%20can%20install%20that%20instead%20of%20compiling%3A)  
```
./configure --enable-gpl --enable-libx264
```
5. Once the configuration is completed, compile and install FFMPEG (it may require sudo). 
```
sudo make
sudo make install
```

## How to integrate Super Resolution (SR) into H.264 decoding pipeline
The key point of itegrating SR into H.264 decoding pipeline is to locate the decoded picture buffer (DPB). After wandering around the internet for a while, we could track down where the DPB is in the H.264 decoding pipeline source code.

### Where does H.264 decoding happen?
Though the concept of video encoing/decoding is quite straight-forward, its whole engineering is very complicated. Especially, considering the fact that the actual decoding happens at "macro-block" level, it is way more complicated than it looks.

After countless trial and errors, we finally figured out the actual decoding occurs in 'decode_simple_internal' method of 'ffmpeg-4.2.1/avcodec/decode.c'. As H.264 keeps the decoded picture buffer (DPB) for P and B frames, we access it via the following data structure.
```
H264Context *h = avctx->priv_data; 
```

### How to compile FFMPEG with SR enabled
1. Download 'decode.c' file in this github repository and replace the existing 'decode.c' in 'ffmpeg-4.2.1/avcodec/decode.c' with the downloaded one.
2. On 'ffempg-4.2.1', re-compile and re-install FFMPEG

### How to compile SR-FFMPEG
The integration is based on the example code within 'ffmpeg-4.2.1/doc/decode_video.c'. Once updating the source code is completed, you need to change one line in 'decode_video.c' to specify the video codec to use. Around line #113, you can find the following line that set the video codec to MPEG1.  
```
codec = avcodec_find_decoder(AV_CODEC_ID_MPEG1VIDEO);
```
To use H.264 codec, change the line above to the following.
```
codec = avcodec_find_decoder(AV_CODEC_ID_H264);
```
Then, you can compile 'decode_video.c' with the following command.
#### Note:
1. it may vary depending on your system set-up.
2. my_app can be replaced with your_application_name, e.g, SR-decoding
```
gcc decode_video.c -o my_app -L../../ -L/usr/bin -L/usr/local/lib ../../libswscale/libswscale.a ../../libavdevice/libavdevice.a ../../libavformat/libavformat.a ../../libavcodec/libavcodec.a ../../libavutil/libavutil.a -lpthread -lbz2 -lm -lz -lfaac -lmp3lame -lx264 -lfaad -lswresample -lm -lz -llzma  -lavutil -lX11
```
As a result of this compilation, we now have an application, 'my_app' that applies SR process to I-frames.

### How to use compiled SR-FFMPEG
Once the compilation is successful, you can run the compiled program. To use it, you need to specify two arguments:
1. INPUT_H264_FILE_PATH
2. OUTPUT_IMG_FILE_PREFIX
If you run the following command, you will get output-xxx.ppm files decoded by SR-FFMPEG.  
```
./my_app output_video018.h264 output
```
Based on this workflow, we use the parallel processing available in Python to enable both SR process and H.264 decoding process. Note that deploying SR models in FFMPEG itself can be feasible using tensorflow C API. However, due to lots of engineering involved and lack of documentation, we take the parallel processing approach. 











