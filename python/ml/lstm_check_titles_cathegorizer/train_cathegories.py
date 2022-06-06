import os
from collections import OrderedDict
import string

from django.core.management.base import BaseCommand, CommandError

# os.environ["CUDA_VISIBLE_DEVICES"] = "-1"  # Force CPU
from keras.callbacks import EarlyStopping

from .consts import MODEL_NAME, EPOCHS, BATCHES, PATIENCE_EPOCHS, VALIDATION_SPLIT

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3' # 0 = all messages are logged, 3 - INFO, WARNING, and ERROR messages are not printed

import numpy as np
import pandas as pd
import openpyxl

from keras.models import Sequential
from keras.layers import Dense, LSTM, Embedding, RepeatVector, Dropout
from keras.preprocessing.text import Tokenizer
from keras.preprocessing.sequence import pad_sequences
from keras.models import load_model
from keras import optimizers
from sklearn.model_selection import train_test_split

from .tokenizers import Tokenizers, prepare_and_lemmatize_string


def train_categories():
    tk = Tokenizers()
    # Split data into train and test set
    train, test = train_test_split(tk.dataset, test_size=0.01, random_state=12)
    # Prepare training data
    trainX = tk.encode_sequences(tk.src_tokenizer, tk.src_length, [x[0] for x in train])
    trainY = np.array([x[1] for x in train])

    # Prepare validation data
    testX = tk.encode_sequences(tk.src_tokenizer, tk.src_length, [x[0] for x in test])
    testY = np.array([x[1] for x in train])

    def make_model(in_vocab, out_vocab, in_timesteps, out_timesteps, n):
        model = Sequential()
        model.add(Embedding(in_vocab, n, input_length=in_timesteps, mask_zero=True))
        model.add(LSTM(n))
        model.add(Dropout(0.3))
        model.add(RepeatVector(out_timesteps))
        model.add(LSTM(n, return_sequences=True))
        model.add(Dropout(0.3))
        model.add(Dense(out_vocab, activation='softmax'))
        model.compile(optimizer=optimizers.RMSprop(learning_rate=0.001), loss='sparse_categorical_crossentropy')
        return model

    print("src_vocab_size:", tk.src_vocab_size, tk.src_length)
    print("dst_vocab_size:", tk.dst_vocab_size, tk.src_length)

    # early_stopping = [EarlyStopping(monitor='val_loss', mode='min', verbose=1, patience=PATIENCE_EPOCHS,
    #                               restore_best_weights=True)]
    early_stopping = []
    model = make_model(tk.src_vocab_size, tk.dst_vocab_size, tk.src_length, tk.dst_length, 512)
    num_epochs = EPOCHS

    history = model.fit(trainX, trainY, epochs=num_epochs, batch_size=BATCHES,
                        validation_split=VALIDATION_SPLIT, callbacks=early_stopping, verbose=1)

    print(tk.src_vocab_size)

    model.save(MODEL_NAME)


def test_cathegories():
    # Load model
    model = load_model(MODEL_NAME)
    tk = Tokenizers()

    # UserWarning: `model.predict_classes()` is deprecated and will be removed after 2021 - 01 - 01.
    # Please use instead: *`np.argmax(model.predict(x), axis=-1)`, if your model does multi- class classification
    #   (e.g.if it uses a `softmax` last-layer activation).*
    # `(model.predict(x) > 0.5).astype("int32")`, if your model does binary classification
    #    (e.g.if it uses a `sigmoid` last-layer activation).

    test_values = [
        prepare_and_lemmatize_string("высокотемпературный силикон"),
        prepare_and_lemmatize_string("счетчик энергии"),
        prepare_and_lemmatize_string("корпус металлический"),
        prepare_and_lemmatize_string("респиратор")
    ]

    phrs_enc = tk.encode_sequences(tk.src_tokenizer, tk.src_length,
                                   test_values)

    print("phrs_enc:", phrs_enc.shape)
    preds = np.argmax(model.predict(phrs_enc), axis=-1)
    # preds = model.predict_classes(phrs_enc)
    for src, dst in zip(test_values, preds):
        print(src, tk.reversed_indexed_categories[dst[0]])
