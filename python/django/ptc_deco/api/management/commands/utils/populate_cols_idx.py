from collections import OrderedDict


def populate_cols_idx(sheet, header_row=1):
    dst = OrderedDict()
    for col_idx in range(1, sheet.max_column + 1):
        k = sheet.cell(header_row, col_idx).value
        dst[k.strip() if isinstance(k, str) else k] = col_idx
    return dst
