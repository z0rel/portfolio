from django.db.models.signals import post_delete, post_save
from ....logger.signals import journal_save_handler, journal_delete_handler

def journal_save_delete(cls):
    post_save.connect(journal_save_handler, sender=cls)
    post_delete.connect(journal_delete_handler, sender=cls)
    return cls


def journal_save_delete_named_meta(cls):
    post_save.connect(journal_save_handler, sender=cls)
    post_delete.connect(journal_delete_handler, sender=cls)
    return cls
