from typing import Dict


def g_resolve(self, cache: Dict):
    return None if self is None else self.resolve(cache=cache)


def g_resolve_or_update(self, cache: Dict):
    return None if self is None else self.resolve_or_update(cache=cache)


def g_resolve_or_create(self, cache: Dict):
    return None if self is None else self.resolve_or_create(cache=cache)


def g_create(self, cache: Dict):
    return None if self is None else self.create(cache=cache)


def get_ak(src, cache):
    if src.agency_commission is None:
        return None
    return cache[src.cache_key()]
