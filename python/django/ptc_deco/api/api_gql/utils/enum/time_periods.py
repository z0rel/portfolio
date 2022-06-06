import graphene as g


class Period(g.Enum):
    CUSTOM = 'custom'
    CURRENT_WEEK = 'current_week'
    CURRENT_MONTH = 'current_month'
    CURRENT_QUARTER = 'current_quarter'
    CURRENT_YEAR = 'current_year'
    LAST_WEEK = 'last_week'
    LAST_MONTH = 'last_month'
    LAST_QUARTER = 'last_quarter'
    LAST_YEAR = 'last_year'


class UnitOfTime(g.Enum):
    DAY = 'day'
    WEEK = 'week'
    MONTH = 'month'
