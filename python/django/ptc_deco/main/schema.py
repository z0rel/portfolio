import graphene
from graphql_auth.schema import UserQuery, MeQuery
from ..api.api_gql import CustomMutation, AuthMutation, NodeQuery


class Query(NodeQuery, UserQuery, MeQuery, graphene.ObjectType):
    pass


if CustomMutation:
    class Mutation(CustomMutation, AuthMutation, graphene.ObjectType):
        pass

    schema = graphene.Schema(query=Query, mutation=Mutation)
else:
    schema = graphene.Schema(query=Query)
