from dateutil.relativedelta import relativedelta


def gen_months_ranges(date_from, date_to):
    # Возвращает список списков в формате
    # [[начало_месяца, конец_месяца], ]
    # за указанный период
    date_from = date_from.replace(day=1)
    date_to = date_to.replace(day=1)
    months = []
    num_months = (date_to.year - date_from.year) * 12 + (date_to.month - date_from.month)
    for i in range(0, num_months + 1):
        next_month = date_from + relativedelta(months=i)
        months.append([next_month, next_month + relativedelta(months=1) - relativedelta(days=1)])
    return months


def gen_weeks_ranges(date_from, date_to):
    # Возвращает список списков в формате
    # [[начало_недели, конец_недели], ]
    # за указанный период
    date = date_from
    weeks = []
    while date_from <= date <= date_to:
        if date.isoweekday() == 1:
            weeks.append([date, date + relativedelta(weeks=1) - relativedelta(days=1)])
            date = date + relativedelta(weeks=1)
            continue
        elif date.isoweekday() == 7 and len(weeks) == 0:
            weeks.append([date - relativedelta(weeks=1) + relativedelta(days=1), date])
        date = date + relativedelta(days=1)
    return weeks


def gen_days_ranges(date_from, date_to):
    # Возвращает список списков в формате
    # [[Начало_дня, конец_дня], ]
    # за указанный период
    day = date_from
    days = []
    while date_from <= day <= date_to:
        days.append([day, day])
        day = day + relativedelta(days=1)
    return days
