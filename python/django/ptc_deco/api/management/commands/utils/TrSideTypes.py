class TrSideTypes:
    SIDE_MAP = {
        'Статичная A': 'Статичная A',
        'Статичная B': 'Статичная B',
        'Cкроллерная A': 'Скроллерная A',
        'Cкроллерная B': 'Скроллерная B',
    }

    ADV_SIDE_MAP = {
        'Статичная A/z': 'Статичная A/Z',
        'Статичная A/v': 'Статичная A/V',
        'Статичная A/n': 'Статичная A/N',
    }

    CODE_MAP = {
        ('A1', 'A1'): 'A',
        ('A2', 'A2'): 'A',
        ('A3', 'A3'): 'A',
        ('B1', 'B1'): 'B',
        ('B2', 'B2'): 'B',
        ('B3', 'B3'): 'B',
    }

    SET_TO_SWAP = {
        ('A/Z', 'A'),
        ('A/V', 'B'),
        ('A1', 'A'),
        ('A2', 'A'),
        ('A3', 'A'),
        ('B1', 'B'),
        ('B2', 'B'),
        ('B3', 'B'),
    }

    # Предобработка несоответствий типов сторон и рекламных сторон заданным в базе типам
    @staticmethod
    def tr_capitalize(s):
        if s is None:
            return s
        return ' '.join([v.capitalize() for v in s.split(' ')])

    @staticmethod
    def tr_format(s):
        s = TrSideTypes.tr_capitalize(s)
        if s == 'Ситилайт Decaux Mupi':
            return 'Ситилайт Decaux MUPI'
        return s

    @staticmethod
    def tr_side_type(s, adv_code):
        s = TrSideTypes.tr_capitalize(s)
        if s:
            arr = s.split(' ')
            if len(arr) > 1:
                arr[1] = TrSideTypes.CODE_MAP.get((arr[1], adv_code), arr[1])
            s = ' '.join(arr)
        sm = TrSideTypes.SIDE_MAP
        return sm.get(s, s)

    @staticmethod
    def tr_adv_side_type(s):
        s = TrSideTypes.tr_capitalize(s)
        return TrSideTypes.ADV_SIDE_MAP.get(s, s)
