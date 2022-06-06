def construct_related_and_only_lists(kwargs, only_list, related_list, args_mapped_to_related_list):
    result_only_list = set(only_list)
    result_related_list = set(related_list) if related_list is not None else set()
    for arg in kwargs:
        if arg in args_mapped_to_related_list:
            related = args_mapped_to_related_list[arg]
            for v in related[0]:
                result_related_list.add(v)
            for v in related[1]:
                result_only_list.add(v)

    return list(sorted(result_only_list)), list(sorted(result_related_list))
