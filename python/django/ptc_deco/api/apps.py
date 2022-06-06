from django.apps import AppConfig


class ApiConfig(AppConfig):
    name = 'ptc_deco.api'

    def ready(self):
        import ptc_deco.api.signals
