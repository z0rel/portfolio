import openpyxl

from .populate_cols_idx import populate_cols_idx
from .SheetValue import SheetValue


def get_sheet_value(wb_fname, wb_sheet, header_row=1):
    wb = openpyxl.load_workbook(wb_fname)
    sheet = wb.get_sheet_by_name(wb_sheet)
    cols = populate_cols_idx(sheet, header_row)
    print(cols)
    sheet_value = SheetValue(sheet, cols)
    return wb, cols, sheet, sheet_value
