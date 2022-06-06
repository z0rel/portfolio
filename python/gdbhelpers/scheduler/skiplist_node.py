#!/usr/bin/env python3


class stack_head:
    def __init__(self, funfile, line, funname, items):
        self.funfile = funfile
        self.line    = line
        self.funname = funname
        self.items   = items

    def __str__(self):
        return "{0}:{1} {3}".format(self.funfile, self.line, self.funname)

    def __repr__(self):
        return str(self)


class py_value:
    def __init__(self, name, strvaltype, straddr, value, attributes):
        self.name = name
        self.strvaltype = strvaltype
        self.straddr = straddr
        self.value = value
        self.attributes = attributes


class skiplist_node:
    def __init__(self, keyval, address, level, value, prev_addrs, next_addrs, container, is_head):
        self.key        = keyval
        self.address    = str(address)
        self.level      = level
        self.value      = value
        self.prev_addrs = prev_addrs
        self.next_addrs = next_addrs
        self.container  = container
        self.is_head    = is_head

    def __lt__(self, oth):
        if self.address == oth.address or oth.address not in self.container:
            return False

        def recurse_find(node):
            for next_key in node.next_addrs:
                next_node = self.container[next_key]
                if next_node.is_head:
                    return False
                if next_node.address == oth.address:
                    return True
                if recurse_find(next_node):
                    return True
            return False

        return recurse_find(self)

    def __hash__(self):
        return hash(self.address)

    def __eq__(self, oth):
        return self.address == oth.address

    def __str__(self):
        return '{{"level":{level},"key":{key},"val":{value},"addr":{addr}}}'.format(
            level=self.level,
            key="'MAX'" if self.key == "18446744073709551615" else self.key,
            value=self.value,
            addr=self.address
        )




