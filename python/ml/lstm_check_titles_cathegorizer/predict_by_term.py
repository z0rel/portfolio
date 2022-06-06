from typing import List

import numpy as np

from .tokenizers import Tokenizers


def predict_by_term(model, tk: Tokenizers, terms: List[str]):
    # Load model

    phrs_enc = tk.encode_sequences(tk.src_tokenizer, tk.src_length,
                                   terms)

    print("phrs_enc:", phrs_enc.shape)
    preds = np.argmax(model.predict(phrs_enc), axis=-1)
    # preds = model.predict_classes(phrs_enc)
    result = []
    for src, dst in zip(terms, preds):
        result.append(tk.reversed_indexed_categories[dst[0]])
    return result
