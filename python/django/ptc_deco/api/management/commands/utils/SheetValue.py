from .TrSideTypes import TrSideTypes


def strip(s):
    return s.strip() if (s is not None and isinstance(s, str)) else s


class SheetValue:
    def __init__(self, sheet, cols):
        self.sheet = sheet
        self.cols = cols
        self.row_idx = 0

    def set_row_idx(self, row_idx):
        self.row_idx = row_idx

    def get(self, key):
        col_idx = self.cols[key]
        return strip(self.sheet.cell(self.row_idx, col_idx).value)

    def get_city(self, key):
        result = self.get(key)
        if result is None or not isinstance(result, str):
            return result
        result = '-'.join(x.lower().capitalize() for x in result.split('-'))
        return result

    def getstr(self, key):
        result = self.get(key)
        return str(result) if result is not None else None

    RESULT = {
        'families': set(),
        'underfamilies': set(),
        'sides': set(),
        'adv_sides': set(),
        'formats': set(),
    }

    def get_capitalize_family(self, key):
        return self._tr_family(self.get(key))

    def get_capitalize_underfamily(self, key):
        return self._tr_underfamily(self.get(key))

    def get_capitalize_format(self, key):
        return self._tr_format(self.get(key))

    def get_capitalize_side(self, key):
        return self._tr_side(self.get(key))

    def get_capitalize_adv_side(self, key):
        return self._tr_adv_side(self.get(key))

    def _tr_adv_side(self, s):
        result = TrSideTypes.tr_adv_side_type(s)
        # result = s.capitalize() if s else s
        # print(result)
        self.RESULT['adv_sides'].add(result)
        return result

    def _tr_side(self, s):
        result = TrSideTypes.tr_side_type(s, None)
        # result = s.capitalize() if s else s
        # print(result)
        self.RESULT['sides'].add(result)
        return result

    def _tr_format(self, s):
        result = TrSideTypes.tr_format(s)
        # result = s.capitalize() if s else s
        # print(result)
        self.RESULT['formats'].add(result)
        return result

    def _tr_family(self, s):
        result = s.capitalize() if s else s
        self.RESULT['families'].add(result)
        return result

    def _tr_underfamily(self, s):
        result = s.capitalize() if s else s
        self.RESULT['underfamilies'].add(result)
        return result

    def get_capitalize_adv_side_by_idx(self, col_idx):
        result = strip(self.sheet.cell(self.row_idx, col_idx).value)
        return self._tr_adv_side(result)
