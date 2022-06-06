#!/usr/bin/env python3
# coding=utf8

""" Pipeline module for upload items to mysql dmbs """

# this module requires mysql-connector

import os
import mysql
from mysql.connector import connection
from mysql.connector import errorcode
import pandas as pd  # require openpyxl


def get_handled_sources(user, password, host, port, database):
    """ Get url pages that are already scrapped """

    cnx = connection.MySQLConnection(user=user, password=password, host=host,
                                     port=port, database=database)
    cursor = cnx.cursor()

    def sel(query_str):
        """ Execute query and skip table not found error """
        try:
            cursor.execute(query_str)
            return [x[0] for x in cursor.fetchall()]
        except mysql.connector.Error as err:
            if err.errno == 1146:
                return []
            raise

    comments = sel('SELECT phone_url FROM scrap_comments_status where vendor_source = "gsmarena";')
    vendors = sel('SELECT vendor_name FROM scrap_vendor_status where vendor_source = "gsmarena";')
    phones = sel('SELECT a.phone_url FROM scrap_comments_status a, gsmarena_phones b '
                 + 'WHERE a.vendor_source = "gsmarena" and b.vendor_source = "gsmarena" '
                 + '  and a.phone_url = b.phone_url;')

    cursor.close()
    cnx.close()
    return {"comments": comments, "vendors": vendors, "phones": phones}


def upload_data_to_excel(filename, user, password, host, port, database):  # pylint: disable=too-many-arguments
    """ Upload data from mysql database to xlsx workbook """

    cnx = connection.MySQLConnection(user=user, password=password, host=host,
                                     port=port, database=database)
    cursor = cnx.cursor()
    excel_writer = pd.ExcelWriter(filename)

    def cur_to_dataframe(tablename):
        cursor.execute("SELECT * FROM " + tablename + ";")
        cols = [x[0] for x in cursor.description]
        return pd.DataFrame(columns=cols, data=cursor.fetchall())
    cur_to_dataframe('gsmarena_phones').to_excel(excel_writer, "phones_gsmarena")
    cur_to_dataframe('gsmarena_comments').to_excel(excel_writer, "comments_gsmarena")
    cur_to_dataframe('vendors').to_excel(excel_writer, "vendors_gsmarena")
    excel_writer.save()


def insert_into_values(cursor, tablename, values):
    """ Insert values into table """
    fields = [f for (f, v) in values]
    values = [(None if isinstance(v, str) and not v else v) for (f, v) in values]

    cmd = ("INSERT IGNORE INTO " + tablename + "(" + ", ".join(fields) + ") VALUES ("
           + (r"%s," * len(fields))[:-1] + ");")
    cursor.execute(cmd, values)


class WritePhonesToDB:
    """ Pipeline for upload item to database """

    def __init__(self):
        """ Pipeline constructor """
        self.autocommit = 1
        self.cnx = None

    def open_spider(self, spider):
        """ Initialize pipeline by spider opened from spider settings and
            create model tables in database"""

        self.cnx = connection.MySQLConnection(
            user=spider.settings.get('MYSQL_USERNAME', None),
            password=spider.settings.get('MYSQL_PASSWORD', None),
            host=spider.settings.get('MYSQL_HOST', None),
            port=int(spider.settings.get('MYSQL_PORT', None)),
            database=spider.settings.get('MYSQL_DATABASE', None)
        )
        self.autocommit = spider.settings.get('MYSQL_AUTOCOMMIT', 1)
        if self.cnx.is_connected():
            print('Connected to MySQL database')
        else:
            raise Exception("Error: Fail connect to database")

        self.cnx.autocommit = bool(self.autocommit)

        path = os.path.dirname(os.path.abspath(__file__))
        with open(os.path.join(path, 'model.sql'), 'r') as model_file:
            lines = "\n".join(model_file.readlines()).split("-- =</>=")
            create_table_commands = [s.strip() for s in lines]

        cur = self.cnx.cursor()
        for create_table_command in create_table_commands:
            try:
                cur.execute(create_table_command)
            except mysql.connector.Error as err:
                # 1068 - multiple primary key defined
                if err.errno == errorcode.ER_TABLE_EXISTS_ERROR or err.errno == 1068:
                    pass
                else:
                    print(err.msg)

        cur.close()

    def close_spider(self, spider):  # pylint: disable=unused-argument
        """ Close connection to database by spider closing and commit if autocommit disabled """

        if not self.autocommit:
            self.cnx.commit()

        self.cnx.close()

    vendor_source_item = [("vendor_source", "gsmarena")]

    def process_item(self, item, spider):  # pylint: disable=unused-argument
        """ Pipeline method: route item data to database tables """

        cur = self.cnx.cursor()

        v_src = WritePhonesToDB.vendor_source_item
        if "phone_info" in item:
            insert_into_values(cur, "gsmarena_phones", item["phone_info"] + v_src)
        elif "user_post" in item:
            insert_into_values(cur, "gsmarena_comments", item["user_post"] + v_src)
        elif "scrap_vendor_status" in item:
            insert_into_values(cur, "scrap_vendor_status", item["scrap_vendor_status"] + v_src)
        elif "scrap_comments_status" in item:
            insert_into_values(cur, "scrap_comments_status", item["scrap_comments_status"] + v_src)
        elif "vendors_info" in item:
            for vendor in item["vendors_info"]:
                insert_into_values(cur, "vendors", vendor + v_src)

        cur.close()

        return item
