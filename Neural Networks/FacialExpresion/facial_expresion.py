# -*- coding: utf-8 -*-
import keras

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from sklearn.model_selection import train_test_split

from keras.models import Sequential
from keras.layers import Dense, Activation, Dropout, Flatten, MaxPool2D
from keras.layers.convolutional import Conv2D
from keras.layers.convolutional import MaxPooling2D

from keras.callbacks import ModelCheckpoint
from keras.optimizers import *
from keras.layers.normalization import BatchNormalization

# get the data
file_name = 'input/fer2013.csv'
label_map = ['Anger', 'Disgust', 'Fear', 'Happy', 'Sad', 'Surprise', 'Neutral']
names = ['emotion', 'pixels', 'usage']
df = pd.read_csv('input/fer2013.csv', names=names, na_filter=False)
im = df['pixels']
df.head(10)


def getData(file_name):
    # images are 48x48
    # N = 35887
    Y = []
    X = []
    first = True
    for line in open(file_name):
        if first:
            first = False
        else:
            row = line.split(',')
            Y.append(int(row[0]))
            X.append([int(p) for p in row[1].split()])

    X, Y = np.array(X) / 255.0, np.array(Y)
    return X, Y


X, Y = getData(file_name)
num_class = len(set(Y))
print(num_class)

# keras with tensorflow backend
N, D = X.shape
X = X.reshape(N, 48, 48, 1)

X_train, X_test, y_train, y_test = train_test_split(X, Y, test_size=0.1, random_state=0)
y_train = (np.arange(num_class) == y_train[:, None]).astype(np.float32)
y_test = (np.arange(num_class) == y_test[:, None]).astype(np.float32)


def my_model():

    # modelul 1

    # Initializarea CNN-ului secvential
    model = Sequential()

    # marimea imaginii folosite
    input_shape = (48, 48, 1)

    # 1 - prima convolutie
    model.add(Conv2D(64, (5, 5), input_shape=input_shape, activation='relu', padding='same'))
    model.add(Conv2D(64, (5, 5), activation='relu', padding='same'))
    model.add(BatchNormalization())
    model.add(MaxPooling2D(pool_size=(2, 2)))

    # 2 - al 2-lea nivel de convolutie
    model.add(Conv2D(128, (5, 5), activation='relu', padding='same'))
    model.add(Conv2D(128, (5, 5), activation='relu', padding='same'))
    model.add(BatchNormalization())
    model.add(MaxPooling2D(pool_size=(2, 2)))

    # 3 - al 3-lea nivel de convolutie
    model.add(Conv2D(256, (3, 3), activation='relu', padding='same'))
    model.add(Conv2D(256, (3, 3), activation='relu', padding='same'))
    model.add(BatchNormalization())
    model.add(MaxPooling2D(pool_size=(2, 2)))

    # Aplatizarea
    model.add(Flatten())

    # Primul nivel conectat
    model.add(Dense(128))
    model.add(BatchNormalization())
    model.add(Activation('relu'))
    model.add(Dropout(0.2))

    # al 2-lea nivel conectat
    model.add(Dense(7))
    model.add(Activation('softmax'))

    # setarea optimizatorului cu rata invatarii 0.0001
    opt = Adam(lr=0.0001)

    # compilarea modelului
    model.compile(optimizer=opt, loss='categorical_crossentropy', metrics=['accuracy'])

    # rezumatul modelului
    model.summary()

    # model 2

    # numar de clase posbile
    nb_classes = 7

    # Initializarea CNN-ului secvential
    model = Sequential()

    # marimea imaginii folosite
    input_shape = (48, 48, 1)

    # 1 - prima convolutie
    model.add(Conv2D(64, (3, 3), padding='same', input_shape=input_shape))
    model.add(BatchNormalization())
    model.add(Activation('relu'))
    model.add(MaxPooling2D(pool_size=(2, 2)))
    model.add(Dropout(0.25))

    # 2 - al 2-lea nivel de convolutie
    model.add(Conv2D(128, (5, 5), padding='same'))
    model.add(BatchNormalization())
    model.add(Activation('relu'))
    model.add(MaxPooling2D(pool_size=(2, 2)))
    model.add(Dropout(0.25))

    # 3 - al 3-lea nivel de convolutie
    model.add(Conv2D(512, (3, 3), padding='same'))
    model.add(BatchNormalization())
    model.add(Activation('relu'))
    model.add(MaxPooling2D(pool_size=(2, 2)))
    model.add(Dropout(0.25))

    # 4 - al 4-lea nivel de convolutie
    model.add(Conv2D(512, (3, 3), padding='same'))
    model.add(BatchNormalization())
    model.add(Activation('relu'))
    model.add(MaxPooling2D(pool_size=(2, 2)))
    model.add(Dropout(0.25))

    # Aplatizarea
    model.add(Flatten())

    # Primul nivel conectat
    model.add(Dense(256))
    model.add(BatchNormalization())
    model.add(Activation('relu'))
    model.add(Dropout(0.25))

    # al 2-lea nivel conectat
    model.add(Dense(512))
    model.add(BatchNormalization())
    model.add(Activation('relu'))
    model.add(Dropout(0.25))

    model.add(Dense(nb_classes, activation='softmax'))

    # setarea optimizatorului
    # opt = Adam(lr=0.001)
    opt2 = sgd(lr=0.01)

    # compilarea modelului
    model.compile(optimizer=opt2, loss='categorical_crossentropy', metrics=['accuracy'])

    # rezumatul modelului
    model.summary()

    # model 3

    # Initializarea CNN-ului secvential
    model = Sequential()

    # marimea imaginii folosite
    input_shape = (48, 48, 1)

    # 1 - prima convolutie
    model.add(Conv2D(input_shape=input_shape, filters=64, kernel_size=(3, 3), padding="same", activation="relu"))
    model.add(Conv2D(filters=64, kernel_size=(3, 3), padding="same", activation="relu"))
    model.add(MaxPool2D(pool_size=(2, 2), strides=(2, 2)))

    # 2 - a 2-a convolutie
    model.add(Conv2D(filters=128, kernel_size=(3, 3), padding="same", activation="relu"))
    model.add(Conv2D(filters=128, kernel_size=(3, 3), padding="same", activation="relu"))
    model.add(MaxPool2D(pool_size=(2, 2), strides=(2, 2)))

    # 3 - a 3-a convolutie
    model.add(Conv2D(filters=256, kernel_size=(3, 3), padding="same", activation="relu"))
    model.add(Conv2D(filters=256, kernel_size=(3, 3), padding="same", activation="relu"))
    model.add(Conv2D(filters=256, kernel_size=(3, 3), padding="same", activation="relu"))
    model.add(MaxPool2D(pool_size=(2, 2), strides=(2, 2)))

    # 4 - a 4-a convolutie
    model.add(Conv2D(filters=512, kernel_size=(3, 3), padding="same", activation="relu"))
    model.add(Conv2D(filters=512, kernel_size=(3, 3), padding="same", activation="relu"))
    model.add(Conv2D(filters=512, kernel_size=(3, 3), padding="same", activation="relu"))
    model.add(MaxPool2D(pool_size=(2, 2), strides=(2, 2)))

    # 5 - a 5-a convolutie
    model.add(Conv2D(filters=512, kernel_size=(3, 3), padding="same", activation="relu"))
    model.add(Conv2D(filters=512, kernel_size=(3, 3), padding="same", activation="relu"))
    model.add(Conv2D(filters=512, kernel_size=(3, 3), padding="same", activation="relu"))
    model.add(MaxPool2D(pool_size=(2, 2), strides=(2, 2)))

    # Aplatizarea
    model.add(Flatten())
    model.add(Dense(units=4096, activation="relu"))
    model.add(Dense(units=4096, activation="relu"))
    model.add(Dense(units=7, activation="softmax"))

    # setarea optimizatorului
    opt = Adam(lr=0.001)
    model.compile(optimizer=opt, loss=keras.losses.categorical_crossentropy, metrics=['accuracy'])

    return model

path_model = 'model_filter.h5'  # save model at this location after each epoch
K.tensorflow_backend.clear_session()  # destroys the current graph and builds a new one
model = my_model()  # create the model
model.summary()

# fit the model
history = model.fit(x=X_train,
              y=y_train,
              batch_size=64,
              epochs=100,
              verbose=1,
              validation_data=(X_test, y_test),
              shuffle=True,
              callbacks=[
                  ModelCheckpoint(filepath=path_model),
              ]
              )

# plot the evolution of Loss and Acuracy on the train and validation sets
plt.figure(figsize=(20, 10))
plt.subplot(1, 2, 1)
plt.suptitle('Optimizer : SGD', fontsize=10)
plt.ylabel('Loss', fontsize=16)
plt.plot(history.history['loss'], label='Training Loss')
plt.plot(history.history['val_loss'], label='Validation Loss')
plt.legend(loc='upper right')

plt.subplot(1, 2, 2)
plt.ylabel('Accuracy', fontsize=16)
plt.plot(history.history['accuracy'], label='Training Accuracy')
plt.plot(history.history['val_accuracy'], label='Validation Accuracy')
plt.legend(loc='lower right')
plt.show()





