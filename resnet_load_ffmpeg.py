import os
import sys
import cv2
import time
import subprocess

#how to load a pre-trained model
#https://towardsdatascience.com/model-sub-classing-and-custom-training-loop-from-scratch-in-tensorflow-2-cc1d4f10fb4e

###loading the pre-trained model###
'''Directory to contain pretrained SR models'''
nas_model_base_path = 'NAS_39'

'''Directory to contain h264 files for quality improvement'''
h264_base_path = 'h264'

'''Directory to save decoded frames'''
decoded_save_base_path = 'decoded'

decoding_base_command = ['./my_app']
sr_base_command = ['python', 'resnet_run.py']

for video_idx in sorted(os.listdir(nas_model_base_path)):

    video_nas_model_path = os.path.join(nas_model_base_path, video_idx)
    video_h264_path = os.path.join(h264_base_path, video_idx)
    video_decoded_save_path = os.path.join(decoded_save_base_path, video_idx)

    video_sr_command = sr_base_command + [video_nas_model_path]

    if not os.path.exists(video_nas_model_path):
        sys.stdouit.write('No NAS Model for Video {}!'.format(video_idx))
    else:
        video_sr_process = subprocess.Popen(video_sr_command)

    for segment_h264 in sorted(os.listdir(video_h264_path)):

        segment_h264_path = os.path.join(video_h264_path, segment_h264)
        segment_name, _ = segment_h264.split('.')
        print(segment_h264_path)
        segment_decoded_save_path = os.path.join(video_decoded_save_path, segment_name)

        if not os.path.exists(segment_decoded_save_path):
            os.makedirs(segment_decoded_save_path)

        segment_decoding_command = decoding_base_command + [segment_h264_path, os.path.join(segment_decoded_save_path, segment_name)]
        subprocess.run(segment_decoding_command)

        for img in sorted(os.listdir(segment_decoded_save_path)): #convert decoded ppm files into png files

            img_name, extension = img.split('.')

            img_path = os.path.join(segment_decoded_save_path, img)
            new_img_path = os.path.join(segment_decoded_save_path, img_name + '.png')

            frame = cv2.imread(img_path)
            cv2.imwrite(new_img_path, frame)

            os.remove(img_path)

    video_sr_process.terminate()






























#end
