import pymorphy2

morph = pymorphy2.MorphAnalyzer()


def _inflect(w, kwargs):
    result = morph.parse(w)[0].inflect(kwargs)
    if result:
        if result.word == 'нон':
            return 'НОН'
        return result.word
    return w


def to_plural_form(words):
    new_words = [_inflect(w, {'plur'}) for w in words.split(' ')]
    if new_words:
        new_words[0] = new_words[0].capitalize()
    return ' '.join(new_words)


def to_plural_form_gent(words):
    new_words = [_inflect(w, {'plur', 'gent'}) for w in words.split(' ')]
    if new_words:
        new_words[0] = new_words[0].capitalize()
    return ' '.join(new_words)


def to_gent(words):
    lw = words.lower()
    if lw == 'устав' or lw == 'устава':
        return 'Устава'

    new_words = [_inflect(w, {'gent'}) for w in words.split(' ')]
    if new_words:
        new_words[0] = new_words[0].capitalize()
    return ' '.join(new_words)


def to_accs_first(words):
    new_words = [w for w in words.split(' ')]
    if new_words:
        new_words[0] = _inflect(new_words[0], {'accs'}).capitalize()
    result = ' '.join(new_words)
    if result == 'Дополнительного расход НОН РТС':
        return 'Дополнительный расход НОН РТС'
    if result == 'Рекламную сторона':
        return 'Рекламную сторону'
    return result


def to_accs_all(words):
    new_words = [w for w in words.split(' ')]
    if new_words:
        new_words[0] = _inflect(new_words[0], {'accs'}).capitalize()

    l = len(new_words)
    for i in range(1, l):
        new_words[i] = _inflect(new_words[i], {'accs'})

    result = ' '.join(new_words)
    if result == 'Дополнительного расход НОН РТС':
        return 'Дополнительный расход НОН РТС'
    if result == 'Рекламную сторона':
        return 'Рекламную сторону'
    return result
