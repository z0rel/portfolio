#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import itertools
import math

testdata = [
    ('1', " Для того чтобы инициировать вход источника журнала должен передать все данные,"
          " связанные с записью журнала, к сердечнику лесозаготовительной. Эти данные или,"
          " точнее, логика сбора данных представлена с набором названных атрибутов."
          " Каждый атрибут, в основном, функцией, чей результат называется &quot;значение"
          " атрибута&quot; и фактически обрабатывается на последующих этапах"),
    ('2', " Чтобы инициировать журналирование источника журнала, должен передать все данные,"
          " связанные с записью журнала, к ядру журналирования. Эти данные или, более точно,"
          " логика сбора данных представлена с рядом именованных атрибутов. Каждый атрибут -"
          ' в основном, функция, результат которой вызывают "значением атрибута" и на самом деле'
          " обрабатывают на дальнейших этапах"),
    ('3', " Для того чтобы инициировать вход источника журнала должен передать все данные, связанные"
          " с записью журнала, к сердечнику лесозаготовительной. Эти данные или, точнее, логика сбора"
          " данных представлена с набором названных атрибутов. Каждый атрибут, в основном, функцией,"
          ' чей результат называется "значение атрибута" и фактически обрабатывается на последующих этапах'),
    ('4', " Для того, чтобы инициировать ведение журнала журнала источника необходимо передать все данные,"
          " связанные с записью журнала для ведения журнала ядра. Эти данные или, точнее, логика сбора"
          " данных представлены с набором именованных атрибутов. Каждый атрибут имеет, по сути, функция,"
          " результат которого называется «значение атрибута» и фактически обрабатывается на последующих этапах"),
    ('5', " Для того, чтобы начать ведение журнала источник журнал должен передать все данные, связанные"
          " с записью в журнале, на Лесоповал ядра. Эти данные или, точнее, логики сбора данных представляет"
          " собой набор именованных атрибутов. Каждый атрибут является, по сути, функция, чей результат"
          ' называется "значение атрибута" и фактически обрабатывается на последующих этапах')
]


# returns a set of blocks containing all of the cuts in the inputs
# Вернуть множество блоков, содержащих все сокращения на входах
def mergeBlocks(leftblocks, rightblocks):
    leftblocks, rightblocks, b = leftblocks[:], rightblocks[:], []
    while len(leftblocks) > 0:
        nleft, nright = leftblocks[0], rightblocks[0]
        n = min(nleft, nright)
        if n < nleft:
            leftblocks[0] -= n
        else:
            del leftblocks[0]
        if n < nright:
            rightblocks[0] -= n
        else:
            del rightblocks[0]
        b.append(n)
    return b


# Наибольшая общая подпоследовательность уникальных элементов общих для 'a' и 'b'
def patienceSubsequence(a, b):
    # Найти уникальные значения в 'arr' и записать их в
    def findUniqueValues(arr):
        resUniqValues = {}
        for i, s in enumerate(arr):
            if s in resUniqValues:
                resUniqValues[s] = -1
            else:
                resUniqValues[s] = i

        return resUniqValues

    # Значения уникальных строк по их порядку в каждом списке
    # Найти уникальные значения в 'a' и 'b'
    value_a, value_b = findUniqueValues(a), findUniqueValues(b)

    # Положить элементы в 'b' как если играть в patience если элемент является уникальным в 'a' и 'b'
    pile, pointers, a_to_b = [], {}, {}  # pile - множество, груда, куча
    get, append = value_a.get, pile.append
    for s in b:
        v = get(s, -1)
        if v != -1:
            vb = value_b[s]
            if vb != -1:
                a_to_b[v] = vb
                # найти соответствующее множество для v
                start, end = 0, len(pile)
                # оптимизация т.к. значения обычно возрастают
                if end and v > pile[-1]:
                    start = end
                else:
                    while start < end:
                        mid = (start + end) // 2
                        if v < pile[mid]:
                            end = mid
                        else:
                            start = mid + 1
                if start < len(pile):
                    pile[start] = v
                else:
                    append(v)
                if start:
                    pointers[v] = pile[start - 1]
    # Проверить наше множество для определения наибольшей общей подпоследовательности
    result = []
    if pile:
        v, append = pile[-1], result.append
        append((v, a_to_b[v]))
        while v in pointers:
            v = pointers[v]
            append((v, a_to_b[v]))
        result.reverse()
    return result


# Аппроксимация наибольшей общей подпоследовательности в стиле difflib
def lcsApprox(a, b):
    count1, lookup = {}, {}
    # Подсчитать вхождения каждого элемента в 'a'
    for s in a:
        count1[s] = count1.get(s, 0) + 1
    # Сконструировать отображение c элементом, где он можеть быть найден в 'b'
    for i, s in enumerate(b):
        if s in lookup:
            lookup[s].append(i)
        else:
            lookup[s] = [i]
    if set(lookup).intersection(count1):
        # У нас есть некоторые общие элементы
        # Определить популярные записи
        popular = {}
        n = len(a)
        if n > 200:
            for k, v in count1.items():
                if 100 * v > n:
                    popular[k] = 1
        n = len(b)
        if n > 200:
            for k, v in lookup.items():
                if 100 * len(v) > n:
                    popular[k] = 1
        # Во время обхода записей в 'a', с приращением обновлять список сопоставленных подпоследовательностей в 'b' и
        # отслеживать найденность наибольшего сопоставления
        prev_matches, matches, max_length, max_indices = {}, {}, 0, []
        for ai, s in enumerate(a):
            if s in lookup:
                if s in popular:
                    # Мы только расширяем существующие ранее найденные сопоставления, чтобы избежать проблем производительности
                    for bi in prev_matches:
                        if bi + 1 < n and b[bi + 1] == s:
                            matches[bi] = v = prev_matches[bi] + 1
                            # Проверить если это теперь новое длиннейшее сопоставление
                            if v >= max_length:
                                if v == max_length:
                                    max_indices.append((ai, bi))
                                else:
                                    max_length = v
                                    max_indices = [(ai, bi)]
                else:
                    prev_get = prev_matches.get
                    for bi in lookup[s]:
                        matches[bi] = v = prev_get(bi - 1, 0) + 1
                        # Проверить если это теперь новое длиннейшее сопоставление
                        if v >= max_length:
                            if v == max_length:
                                max_indices.append((ai, bi))
                            else:
                                max_length = v
                                max_indices = [(ai, bi)]
            prev_matches, matches = matches, {}

        if max_indices:
            # Вначале включить любые популярные записи
            aidx, bidx, nidx = 0, 0, 0
            for ai, bi in max_indices:
                n = max_length
                ai += 1 - n
                bi += 1 - n
                while ai and bi and a[ai - 1] == b[bi - 1]:
                    ai -= 1
                    bi -= 1
                    n += 1
                if n > nidx:
                    aidx, bidx, nidx = ai, bi, n
            return aidx, bidx, nidx


# Терпеливый diff с запасным вариантом в стиле difflib
def patience_diff(a, b):
    matches, len_a, len_b = [], len(a), len(b)
    if len_a and len_b:
        blocks = [(0, len_a, 0, len_b, 0)]
        while blocks:
            start_a, end_a, start_b, end_b, match_idx = blocks.pop()
            aa, bb = a[start_a:end_a], b[start_b:end_b]
            # Попытаться выполнить терпеливый diff
            pivots = patienceSubsequence(aa, bb)
            if pivots:
                offset_a, offset_b = start_a, start_b
                for pivot_a, pivot_b in pivots:
                    pivot_a += offset_a
                    pivot_b += offset_b
                    if start_a <= pivot_a:
                        # Расширить до
                        idx_a, idx_b = pivot_a, pivot_b
                        while start_a < idx_a and start_b < idx_b and a[idx_a - 1] == b[idx_b - 1]:
                            idx_a -= 1
                            idx_b -= 1
                        # Если что-то есть перед рекурсией на секции
                        if start_a < idx_a and start_b < idx_b:
                            blocks.append((start_a, idx_a, start_b, idx_b, match_idx))
                        # Расширить после
                        start_a, start_b = pivot_a + 1, pivot_b + 1
                        while start_a < end_a and start_b < end_b and a[start_a] == b[start_b]:
                            start_a += 1
                            start_b += 1
                        # Записать сопоставление
                        matches.insert(match_idx, (idx_a, idx_b, start_a - idx_a))
                        match_idx += 1
                # Если что-то есть после рекурсии на секции
                if start_a < end_a and start_b < end_b:
                    blocks.append((start_a, end_a, start_b, end_b, match_idx))
            else:
                # Резервный вариант, если терпеливый diff не удался
                pivots = lcsApprox(aa, bb)
                if pivots:
                    idx_a, idx_b, n = pivots
                    idx_a += start_a
                    idx_b += start_b
                    # Если что-то есть перед рекурсией на секции
                    if start_a < idx_a and start_b < idx_b:
                        blocks.append((start_a, idx_a, start_b, idx_b, match_idx))
                    # Записать сопоставление
                    matches.insert(match_idx, (idx_a, idx_b, n))
                    match_idx += 1
                    idx_a += n
                    idx_b += n
                    # Если что-то есть после рекурсии на секции
                    if idx_a < end_a and idx_b < end_b:
                        blocks.append((idx_a, end_a, idx_b, end_b, match_idx))
    # Попытаться сопоставить от начала до первого блока сопоставления
    if matches:
        end_a, end_b = matches[0][:2]
    else:
        end_a, end_b = len_a, len_b
    i = 0
    while i < end_a and i < end_b and a[i] == b[i]:
        i += 1
    if i:
        matches.insert(0, (0, 0, i))
    # Попытаться сопоставить от последнего блока до конца
    if matches:
        start_a, start_b, n = matches[-1]
        start_a += n
        start_b += n
    else:
        start_a, start_b = 0, 0
    end_a, end_b = len_a, len_b
    while start_a < end_a and start_b < end_b and a[end_a - 1] == b[end_b - 1]:
        end_a -= 1
        end_b -= 1
    if end_a < len_a:
        matches.append((end_a, end_b, len_a - end_a))
    # Добавление блока нулевой длины к концу
    matches.append((len_a, len_b, 0))
    return matches


# Удалить из строки пробелы и \t\n\r\x0b\x0c, затем преобразовать ее в верхний регистр
def alignmentHash(line):
    text = line
    if text is None:
        return None
    # выравнивание игнорирует пробелы
    # удалить все пробелы из строки
    for c in ' \t\n\r\x0b\x0c':
        text = text.replace(c, '')

    # выравнивание игнорирует регистр
    # преобразовать все в верхний регистр
    return text.upper()


# Выровнять наборы строк, вставив NULL-распорки и обновив размер блоков, которым они принадлежат
#
# leftlines и rightlines - являются списком списков строк.
# Выравнивается только внутренний список строк (leftlines[-1] и rightlines[0]).
# Любые распорки, необходимые для выравнивания, вставляются во всех списки
# строк для конкретной стороны, чтобы сохранить их все синхронизированными.
def alignBlocks(leftblocks, leftlines, rightblocks, rightlines):
    blocks = (leftblocks, rightblocks)
    lines = (leftlines, rightlines)
    # получить внутренние строки, которые мы должны сопоставить
    middle = (leftlines[-1], rightlines[0])
    # ликвидировать любые существующие строки-распорки
    mlines = ([line for line in middle[0] if line is not None],
              [line for line in middle[1] if line is not None])
    s1, s2 = mlines
    n1, n2 = 0, 0
    # хэшировать строки согласно предпочтениям выравнивания
    t1 = [alignmentHash(s) for s in s1]
    t2 = [alignmentHash(s) for s in s2]
    # выровнять s1 и s2, вставив строки-распорки
    # это будет использоваться для определения, какие строки из внутреннего
    # списка строк должны быть соседями
    for block in patience_diff(t1, t2):
        delta = (n1 + block[0]) - (n2 + block[1])
        if delta < 0:
            # вставить строки-распорки в s1
            i = n1 + block[0]
            s1[i:i] = -delta * [None]
            n1 -= delta
        elif delta > 0:
            # вставить строки-распорки в s2
            i = n2 + block[1]
            s2[i:i] = delta * [None]
            n2 += delta
    nmatch = len(s1)

    # вставить распорные строки leftlines и rightlines и увеличить размер
    # блоков в leftblocks и rightblocks, вставленных строк-распорок
    #
    # увеличить одну строку в момент вставки линии-разделители, как мы идем
    # 'i' указывает, какую строку мы обрабатываем
    # 'k' указывает, какую пару соседей мы обрабатываем
    i, k = 0, 0
    bi = [0, 0]
    bn = [0, 0]
    while True:
        # Если мы достигли конца списка для любой стороны, она нуждается в распорочной строке для выравнивания с другой стороной
        insert = [i >= len(m) for m in middle]
        if insert == [True, True]:
            # мы достигли конца обоих внутренних списков строк, которые мы сделали
            break
        if insert == [False, False] and k < nmatch:
            # определить, если одна из сторон нуждается в распорочных строках, чтобы сделать сопоставление внутреннего списка строк
            accept = True
            for j in range(2):
                m = mlines[j][k]
                if middle[j][i] is not m:
                    # эта строка не соответствует паре соседей которую мы ожидали
                    if m is None:
                        # Мы ожидали найти здесь null, так что вставить
                        insert[j] = True
                    else:
                        # У нас есть нуль, но мы не ожидали что мы не получим соединение пар, которое мы ожидали, вставляя значения NULL
                        accept = False
            if accept:
                # наши линии будут корректно состыкованы в пару, перейти к следующей паре
                k += 1
            else:
                # вставить распорные строки при необходимости
                insert = [mIt[i] is not None for mIt in middle]
        for j in range(2):
            if insert[j]:
                # вставить распорные строки для стороны 'j'
                for temp in lines[j]:
                    temp.insert(i, None)
                blocksj = blocks[j]
                bij = bi[j]
                bnj = bn[j]
                # добавить новый блок, если это необходимо
                if len(blocksj) == 0:
                    blocksj.append(0)
                # перейти к текущему блоку
                while bnj + blocksj[bij] < i:
                    bnj += blocksj[bij]
                    bij += 1
                # увеличить текущий размер блока
                blocksj[bij] += 1
        # перейти к следующей строке
        i += 1


# Действие "перевыровнять всё"
def realignAll(srcTexts):
    lines = []
    blocks = []

    for srcText in srcTexts:
        # создать новый список строк без пробелов
        newlines = [[line for line in srcText if line is not None]]
        len0 = len(newlines[0])
        newblocks = [len0] if len0 > 0 else []
        if len(lines) > 0:
            # сопоставить с соседом налево
            alignBlocks(blocks, lines, newblocks, newlines)
            blocks = mergeBlocks(blocks, newblocks)
        else:
            blocks = newblocks
        lines.extend(newlines)

    return lines


def stripLine(l):
    l = l.replace('«', "'")
    l = l.replace('»', "'")
    l = l.replace('«', "'")
    l = l.replace('"', "'")
    l = l.replace('&quot;', "'")
    return l


def merge_main_base(lines):
    lines = realignAll([stripLine(l).split(' ') for l in lines])
    merged = []
    lengths = []
    for i in itertools.zip_longest(*lines):
        itemSet = {(j if j else None) for j in i}
        if None in itemSet:
            if len(itemSet) == 1:
                continue
            else:
                itemSet.remove(None)
                itemSet.add('_')
        itemSet = sorted(itemSet)
        for x in [x for x in itemSet if x.find(',') != -1]:
            withoutComma = x.replace(',', '')
            if withoutComma in itemSet:
                itemSet.remove(withoutComma)

        item = list(itemSet)
        if len(item) != 1:
            lengths.append(len(item))

        merged.append(item[0] if len(item) == 1 else "({0})".format("|".join(item)))

    return " ".join(merged), len(lengths)


def merge_main(lines):
    min_case = None
    min_text = None

    lines_indices = list(range(len(lines)))
    lines_indices_set = set(lines_indices)
    cnt = 0
    max_cnt = 1000

    def return_min_text(min_text):
        return min_text if min_text else ["", ""]

    for perm_case_ind in itertools.combinations(lines_indices, 2):
        perm_case1 = [lines[i] for i in perm_case_ind]
        ind2 = sorted(list(lines_indices_set - set(perm_case_ind)))
        perm_case2 = [lines[i] for i in ind2]
        for case1 in itertools.permutations(perm_case1):
            for case2 in itertools.permutations(perm_case2):
                cnt += 1
                text1, length1 = merge_main_base(case1)
                text2, length2 = merge_main_base(case2)

                curr_case = (math.hypot(length1, length2), math.hypot(len(text1), len(text2)))
                if min_case is None or curr_case < min_case:
                    min_case = curr_case
                    min_text = [text1, text2]

                if cnt > max_cnt:
                    return return_min_text(min_text)

    return return_min_text(min_text)


def main():
    x = merge_main([l[1].replace(r"\n", " ") for l in testdata])
    print(x[0])
    print("")
    print(x[1])


if __name__ == '__main__':
    main()
