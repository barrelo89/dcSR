import sys
import time
import tensorflow as tf
import matplotlib.pyplot as plt

from resnet_new import *

video_nas_model_path = sys.argv[1]
video_nas_model = tf.keras.models.load_model(video_nas_model_path)

#this is defined in line 583-584 of 'decode.c'. You can change them as you want
decoded_frames = ['decoded-0-0.ppm', 'decoded-0-1.ppm']

while(1):

    for idx, frame in enumerate(decoded_frames):

        while not os.path.exists(frame):
            11

        time.sleep(1) #this buffer is to make sure we have a decoded frame. 
        lr = load_image(frame)
        start_time = time.process_time()
        sr = resolve_single(video_nas_model, lr)
        end_time = time.process_time()

        #running time: 0.06..
        print('Running Time: {}'.format(end_time - start_time))

        sr_img_path = 'sr-0-' + str(idx) + '.ppm'

        plt.imsave(sr_img_path, sr.numpy())
        os.remove(frame)

os.remove("decoded-0-0.ppm")
os.remove("decoded-0-1.ppm");




































#end
