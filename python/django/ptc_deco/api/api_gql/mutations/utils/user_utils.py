def get_user_password_err_message(limit, user_id):
    message = f"Пароль пользователя {user_id} не может совпадать "
    if limit:
        message += f"ни c одним из {limit} последних использованных паролей"
    else:
        message += "ни с одним из ранее использованных паролей"
    return message
