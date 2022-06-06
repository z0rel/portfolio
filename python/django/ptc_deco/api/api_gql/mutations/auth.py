import graphene
from graphql_auth import mutations

AuthMutation = type(
    'AuthMutation',
    (graphene.ObjectType,),
    {
        k: v.Field()
        for (k, v) in [
            ['register', mutations.Register],
            ['verify_account', mutations.VerifyAccount],
            ['token_auth', mutations.ObtainJSONWebToken],
            ['resend_activation_email', mutations.ResendActivationEmail],
            ['send_password_reset_email', mutations.SendPasswordResetEmail],
            ['password_reset', mutations.PasswordReset],
            ['password_change', mutations.PasswordChange],
            ['archive_account', mutations.ArchiveAccount],
            ['delete_account', mutations.DeleteAccount],
            ['update_account', mutations.UpdateAccount],
            ['send_secondary_email_activation', mutations.SendSecondaryEmailActivation],
            ['verify_secondary_email', mutations.VerifySecondaryEmail],
            ['swap_emails', mutations.SwapEmails],
            ['verify_token', mutations.VerifyToken],
            ['refresh_token', mutations.RefreshToken],
            ['revoke_token', mutations.RevokeToken],
        ]
    },
)


# TODO Example of authorization
# class CreateCityMutation(DjangoCreateMutation):
#    class Meta:
#        model = m.City
#        # login_required = False
#        # permissions = ('api.add_city',)
#        # many_to_one_extras = {
#        #     'district': {
#        #         'exact': {
#        #             'type': 'auto',
#        #             'operation': 'add',
#        #             'many_to_one_extras': {
#        #                 'postcode': {
#        #                     'exact': {
#        #                         'operation': 'add',
#        #                         'type': 'auto'
#        #                     }
#        #                 }
#        #             }
#        #         }
#        #     }
#        # }


# class CreatePostcodeMutation(DjangoCreateMutation):
#     class Meta:
#         model = m.Postcode
#         # login_required = True
#         # permissions = ('api.add_postcode',)
