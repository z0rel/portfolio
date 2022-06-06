from graphql import GraphQLError
from graphql import ResolveInfo
from django.conf import settings


def login_or_permissions_required(login_required=False, permissions=()):
    def decorator(func):
        def wrapped(*args, **kwargs):
            if not settings.DISABLE_GQL_AUTH_CONTROL:
                info = None
                for param in args:
                    if isinstance(param, ResolveInfo):
                        info = param
                        break
                if not info:
                    raise Exception("No 'ResolveInfo' provided.")
                if login_required and info.context.user.is_anonymous:
                    raise GraphQLError("Must be logged in to access this query.")
                if permissions and not info.context.user.has_perms(permissions):
                    raise GraphQLError("Must have corresponding permissions to access this query.")
            return func(*args, **kwargs)
        return wrapped
    return decorator
