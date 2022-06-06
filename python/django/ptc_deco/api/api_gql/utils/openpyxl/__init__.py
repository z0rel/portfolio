from openpyxl import Workbook


def copy_sheet_to_workbook(workbook: Workbook, sheet, sheet_name):
    mr = sheet.max_row
    mc = sheet.max_column
    new_sheet = workbook.create_sheet(sheet_name)
    for i in range(1, mr + 1):
        for j in range(1, mc + 1):
            c = sheet.cell(row=i, column=j)
            new_sheet.cell(row=i, column=j).value = c.value


def create_empty_workbook():
    workbook = Workbook()
    for name in workbook.sheetnames:
        sheet = workbook.get_sheet_by_name(name)
        workbook.remove_sheet(sheet)
    return workbook
