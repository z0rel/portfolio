
def get_construction_side_code(postcode: str, num_in_district: int, format_code: str, side_code: str,
                               advertising_side_code: str):
    return f'''{postcode or '_'}.{str(num_in_district).zfill(5)}.{format_code or '_'}.{side_code or '_'}.{
                  advertising_side_code or '_'}'''

def get_construction_code(postcode: str, num_in_district: int, format_code: str):
    return f'''{postcode or '_'}.{str(num_in_district).zfill(5)}.{format_code or '_'}'''
