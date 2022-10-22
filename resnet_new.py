#https://github.com/krasserm/super-resolution

import os
import cv2
import numpy as np
import tensorflow as tf
import matplotlib.pyplot as plt

from PIL import Image
from tensorflow.python.data.experimental import AUTOTUNE

#tf.config.experimental_run_functions_eagerly(True)
'''
In the overall pipeline
    - data preprocessing:
        1. resize the low quality image
        2. keep the original quality image size
    - RESNET:
        1. apply upsample at the end
'''

class ResBlock(tf.keras.layers.Layer):

    def __init__(self, filter_size, filter_number, residual_scale):
        super(ResBlock, self).__init__()

        self.filter_size = filter_size
        self.filter_number = filter_number
        self.residual_scale = residual_scale

    def build(self, input_shape):

        _, _, _, num_channel = input_shape
        self.conv_1 = tf.keras.layers.Conv2D(filters = self.filter_number, kernel_size = self.filter_size, padding = 'same', activation = 'relu')
        #self.relu = tf.keras.layers.LeakyReLU()
        self.conv_2 = tf.keras.layers.Conv2D(filters = self.filter_number, kernel_size = self.filter_size, padding = 'same')

    def call(self, input):

        x = input

        x = self.conv_1(x)
        #x = self.relu(x)
        x = self.conv_2(x)

        return self.residual_scale*x + input

class Upsample(tf.keras.layers.Layer):

    def __init__(self, size_up, num_filters):
        super(Upsample, self).__init__()
        self.size_up = size_up
        self.num_filters = num_filters

    def build(self, input_shape):
        self.conv = tf.keras.layers.Conv2D(self.num_filters*(2**2), kernel_size = 3, padding = 'same')

    def upsample_base(self, input, num_filters, size_up):

        x = self.conv(input)
        return tf.nn.depth_to_space(x, size_up)

    def call(self, input):

        if self.size_up == 2:
            x = self.upsample_base(input, self.num_filters, 2)

        elif self.size_up == 3:
            x = self.upsample_base(input, self.num_filters, 3)

        elif self.size_up == 4:
            x = self.upsample_base(input, self.num_filters, 2)
            x = self.upsample_base(x, self.num_filters, 2)

        return x

class ResNet(tf.keras.Model):

    #def __init__(self, num_block, filter_number, kernel_size):
    def __init__(self, num_block, filter_number, kernel_size, size_up, residual_scale):
        super(ResNet, self).__init__()
        self.num_res_block = num_block
        self.filter_number = filter_number
        self.kernel_size = kernel_size
        self.block_list = []
        self.size_up = size_up
        self.residual_scale = residual_scale

    def build(self, input_shape):

        self.dim = input_shape[1:]
        self.conv_in = tf.keras.layers.Conv2D(filters = self.filter_number, kernel_size = self.kernel_size, padding = 'same')

        for _ in range(self.num_res_block):
            self.block_list.append(ResBlock(self.kernel_size, self.filter_number, self.residual_scale))

        self.upsample = Upsample(self.size_up, self.filter_number)
        self.conv_out = tf.keras.layers.Conv2D(filters = 3, kernel_size = self.kernel_size, padding = 'same')

    def call(self, input):

        x = self.conv_in(input)
        conv_in = x

        for idx in range(self.num_res_block):
            x = self.block_list[idx](x)

        x += conv_in

        x = self.upsample(x)
        x = self.conv_out(x)

        #x = tf.clip_by_value(x, 0.0, 255.0)
        return x

    def build_graph(self):
        x = tf.keras.Input(shape = (None, None, 3))
        return tf.keras.Model(inputs = [x], outputs = self.call(x))

'''
def load_dataset(image_directory_path):

    ds = tf.data.Dataset.from_tensor_slices([os.path.join(image_directory_path, img_name) for img_name in sorted(os.listdir(image_directory_path))])
    ds = ds.map(tf.io.read_file)
    ds = ds.map(lambda x: tf.cast(tf.image.decode_png(x, channels = 3), tf.float32), num_parallel_calls=AUTOTUNE)

    return ds
'''

def load_image(img_path):
    #print(img_path)
    img = np.array(Image.open(img_path))
    width, height = int(1920 / 4), int(1080 / 4)

    return cv2.resize(img, (width,height), interpolation=cv2.INTER_LINEAR)# / 255.

def load(img_path):
    img = tf.io.read_file(img_path)
    img = tf.cast(tf.image.decode_png(img, channels = 3), tf.float32)
    return img

def load_low(img_path):
    img = tf.io.read_file(img_path)
    img = tf.image.decode_png(img, channels = 3)

    width, height = int(1920 / 4), int(1080 / 4)

    img = tf.image.resize(img, [height, width], method=tf.image.ResizeMethod.BILINEAR)
    img = tf.cast(img, tf.float32)

    return img

def resolve_single(model, lr):
    return resolve(model, tf.expand_dims(lr, axis=0))[0]

def resolve(model, lr_batch):
    lr_batch = tf.cast(lr_batch, tf.float32)
    sr_batch = model(lr_batch)
    #sr_batch = sr_batch*255
    sr_batch = tf.clip_by_value(sr_batch, 0, 255)
    sr_batch = tf.round(sr_batch)
    sr_batch = tf.cast(sr_batch, np.uint8)
    return sr_batch































#end
