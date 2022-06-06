#!/usr/bin/env python3

import datetime
import json
import multiprocessing
import re
import sys
import time
import traceback
from sys import stdout

import psycopg2
import psycopg2.extras

from .vk import user_fields, Session, API, AuthSession, exceptions
from .dumper_utils import printChange, ConnectionFailed
from .reconnect_context import ReconnectContext

usleep = lambda x: time.sleep(x / 1000000.0)


def dumpPagesInfo(threadAllUids, city, dumper):
    dumper.downloadChuncksFemaleInfo(threadAllUids, city)


def multiprocessExecute(chuncks, target):
    wp = []
    for args in chuncks:
        p = multiprocessing.Process(target = target, args = args)
        wp.append(p)
        p.start()

    for p in wp:
        p.join()


# dumperDb = shelve.open('dumperdb')

def postgresConn():
    return psycopg2.connect(database = 'vk', user = 'postgres', password = 'at89c51', port = '5436')


def isPublicVrnFemale(v):
    return v['sex'] == 1 and 'city' in v and v['city'] == 42


def exceptionInfo(text = ''):
    def traceprint(text):
        print(text)
        exc_type, exc_value, exc_traceback = sys.exc_info()
        t = traceback.format_exception(exc_type, exc_value, exc_traceback)
        for i in t:
            print(repr(i))

    try:
        traceprint('----- ERROR IN {0} -----'.format(text))
    except:
        traceprint('----- ERROR IN exceptionInfo: CRITICAL -----')


def inverseDict(dictionary):
    return {val: uid for (uid, val) in dictionary.items()}


def mergeDicts(x, y):
    '''Given two dicts, merge them into a new dict as a shallow copy.'''
    z = x.copy()
    z.update(y)
    return z


globalStatuses = {
    1: "не женат", 2: "встречается", 3: "помолвлен", 4: "женат", 5: "все сложно", 6: "в активном поиске", 7: "влюблен"
    }


class VkDumper:
    FRIENDS_TIMEOUT = 1.0 / 2.0
    FOLLOWERS_TIMEOUT = 1.0 / 2.0

    def setVkApi(self, self_access_token):
        session = Session(access_token = self_access_token)
        self.vk_api = API(session, lang = 'ru', timeout = 40)

    def __init__(self, accessToken, **kwargs):
        self.accessToken = accessToken
        self.metadataArgs = kwargs
        self.pgConn = postgresConn()
        self.pgCursor = self.pgConn.cursor()

    def dumerIndex(self):
        return self.metadataArgs.get('dumperIndex', 0)

    def connect(self):
        if 'vk_api' in self.__dict__:
            return

        if not self.accessToken.token:
            print('get connection from login')
            auth_session = AuthSession(
                    app_id = self.accessToken.sqliteDb.single(
                        "select LOGIN from ACCOUNTS where SELF_PAGE_ID = 'VK_APP_ID'"),
                    # (standalone) aka API/Client ID
                    user_login = self.accessToken.login,
                    user_password = self.accessToken.password,
                    scope = 'offline,wall,friends,photos,video,messages'
            )
            self.accessToken.token = auth_session.get_access_token()

        self.setVkApi(self.accessToken.token)
        print('connected')

    def forceReconnectByAccessToken(self, sleepTime, message = ""):
        print("sleep and try reconnect", message)
        time.sleep(sleepTime)
        self.setVkApi(self.accessToken.token)

    def __getattr__(self, name):
        if name == 'vk_api':
            self.connect()
            if 'vk_api' not in self.__dict__:
                raise ConnectionFailed()
            return self.vk_api
        else:
            raise ConnectionFailed()

    def __getstate__(self):
        return (self.accessToken, self.metadataArgs)

    def __setstate__(self, state):
        accessToken, args = state
        VkDumper.__init__(self, accessToken, **args)

    """ Выкачать UID подписчиков заданного паблика
    """

    def dumpPublicUsers(self, groupId):
        dumpedUsersId = set()
        items_key = 'items'
        val = self.vk_api.groups.getMembers(group_id = groupId, count = 1)
        if val == []:
            print("!!!!! self.vk_api.groups.getMembers(group_id={0}, count=1) == [] !!!!!!!".format(groupId))
            return

        userscnt = val['count']
        offset = 0
        cnt = 0
        try:
            while offset < userscnt:
                val = self.vk_api.groups.getMembers(group_id = groupId, offset = offset)
                dumpedUsersId |= set(val[items_key])
                oldOffset = offset
                offset += len(val[items_key])
                if oldOffset == offset:
                    offset += 1

                printChange(
                    "offset = {0} / total = {1} / unique dumped = {2}".format(offset, userscnt, len(dumpedUsersId)))
                time.sleep(2)

            stdout.write("\n")  # move the cursor to the next line
        except KeyboardInterrupt:
            print("KeyboardInterrupt")
        return dumpedUsersId


    def dumpFriends(self, userId):
        """ Выкачать список друзей заданного пользователя """

        dumpedUsersId = set()
        oldLen = -1
        offset = 0
        while True:
            with ReconnectContext(self):
                val = self.vk_api.friends.get(user_id = userId, order = 'hints', offset = offset)
                dumpedUsersId |= set(val)

                printChange("friends offset = {0} / unique dumped = {1}".format(offset, len(dumpedUsersId)))
                time.sleep(VkDumper.FRIENDS_TIMEOUT)
                if len(dumpedUsersId) == oldLen:
                    break
                else:
                    offset += len(val)
                    oldLen = len(dumpedUsersId)

        stdout.write("\n")  # move the cursor to the next line

        return dumpedUsersId


    def dumpFollowers(self, userId):
        """ Выкачать список подписчиков заданного пользователя """

        dumpedUsersId = set()
        oldLen = -1
        offset = 0
        while True:
            with ReconnectContext(self):
                val = self.vk_api.users.getFollowers(user_id = userId, offset = offset, count = 1000)
                cnt = val['count']
                items = val['items']
                dumpedUsersId |= set(items)

                printChange("followers offset = {0} / unique dumped = {1}".format(offset, len(dumpedUsersId)))
                time.sleep(VkDumper.FOLLOWERS_TIMEOUT)
                if len(dumpedUsersId) == oldLen or cnt == len(dumpedUsersId):
                    break
                else:
                    offset += len(items)
                    oldLen = len(dumpedUsersId)

        stdout.write("\n")  # move the cursor to the next line

        return dumpedUsersId


    def dumpPublicsContacts(self, publics):
        """ Выкачать список контактов для заданных пабликов """

        contacts = self.vk_api.groups.getById(group_ids = publics, fields = 'contacts')
        l = []
        for groupContacts in contacts:
            contacts = groupContacts.get("contacts", None)
            if contacts is None:
                continue
            l.extend(contacts)

        uidSet = set()
        phonesSet = set()
        emailsSet = set()
        for contact in l:
            try:
                uidSet.add(contact["user_id"])
            except KeyError:
                try:
                    phonesSet.add(contact.get("phone", ""))
                except:
                    emailsSet.add(contact.get("email", ""))
        return {"uids": uidSet, "emails": emailsSet, "phones": phonesSet}


    def search(self, criterias, getSearchMethod = lambda vk_api: vk_api.users.search, idKey = 'uid'):
        """ Выполнить поиск по заданным критериям и для заданного метода """

        users = set()

        class CountError(Exception):
            def __init__(message):
                self.message = message

        try:
            for criteria in criterias:
                offset = 0
                size = None
                while True:
                    val = getSearchMethod(self.vk_api)(offset = offset, count = 1000, **criteria)

                    time.sleep(1 / 2)

                    if not size:
                        size = val[0]
                    if size > 1000:
                        print(
                            'search: warning: size overflow: size = {0} for criterias: {1}'.format(size, str(criteria)))
                    elif not size:
                        break

                    print("Total progress: {0}/{1}       ".format(offset, size))
                    for (line, key) in [(line + 1, key) for (line, key) in enumerate(sorted(criteria))]:
                        print("  {0}: {1}     ".format(key, criteria[key]))

                    peoples = val[1:]

                    for v in peoples:
                        users.add(v[idKey])

                    if len(peoples) and offset + len(peoples) < size:
                        offset += len(peoples)
                    else:
                        break
        finally:
            None

        print("")
        return users

    def getDumpingRange(self, dumpingIds, step = 1000):
        return [(dumpingIds[i:i + step], i) for i in range(0, len(dumpingIds), step)]


    def dumpUserData(self, dumpedUsersId, force = False):
        """ Выкачать информацию со страничек заданных пользователей (старый метод для использования в dumpPhotoVideo)
        """
        def checkSettedKey(key, dflt):
            if key not in dumperDb.keys():
                dumperDb[key] = dflt
            return key

        key = checkSettedKey('userPages', list())
        handledPages = dumperDb[key]

        if not force:
            keyHandled = checkSettedKey('handledIds', set())
            dumpedUsersId -= handledIds
        else:
            handledIds = set()

        dumpingIds = sorted(dumpedUsersId)
        print(len(dumpingIds))

        fields = user_fields.getFields()
        # обход всего множества по тысяче кусков за раз
        try:
            for (chunk, i) in self.getDumpingRange(dumpingIds):
                val = self.vk_api.users.get(user_ids = chunk, fields = fields)
                for userData in val:
                    handledIds.add(userData['uid'])

                printChange("offset = {0} / total = {1} / filtered dumped = {2} / unique dumped = {3}".format(
                        i, len(dumpingIds), len(handledIds)))
                time.sleep(1 / 2)
            stdout.write("\n")  # move the cursor to the next line
        except KeyboardInterrupt:
            print("KeyboardInterrupt")
        except Exception:
            exceptionInfo("dumpUserData")


    def printOnlineCount(self, dumpingIds):
        """ Посчитать число онлайн-пользователей среди заданного множества пользователей.
            Может быть использовано для анализа активности среди подписчиков различных сообществ и
            подбора часов с максимальной активностью (для рекламы в эти часы.
        """
        fields = ["online"]
        handledIds = set()
        step = 1000

        for (chunk, i) in [(dumpingIds[i:i + step], i) for i in range(0, len(dumpingIds), step)]:
            val = self.vk_api.users.get(user_ids = chunk, fields = fields)
            for userData in val:
                if userData['online'] == 1:
                    handledIds.add(userData['uid'])

            printChange("offset = {0} / total = {1} / dumped = {2}".format(i, len(dumpingIds), len(handledIds)))
            time.sleep(1 / 2)
        stdout.write("\n")  # move the cursor to the next line
        print("online: {0}/{1}".format(len(handledIds), len(dumpingIds)))

    """ Выкачать информацию о всех воронежских девушках через кластеризованный поиск
    """

    def dumpAllVrnFemalesIds(self):
        class CountError(Exception):
            pass

        (age, day) = (None, None)
        try:
            startAge = 19
            endAge = 49
            startYear = datetime.datetime.now().year - startAge
            endYear = datetime.datetime.now().year - endAge
            # firstStartDay = 1
            # startYear     = 1978
            print("start year=", startYear, "end year=", endYear)

            ranges = []
            for year in range(startYear, endYear, -1):
                for day in range(1, 32):
                    if year == startYear and day < firstStartDay:
                        continue
                    ranges.append((year, day))

            def searchUsers(filterArgs, year, day, month):
                if month:
                    s = "\ryear={0}, month={1}, day={2} ".format(year, month, day)
                else:
                    s = "\ryear={0}, day={1} ".format(year, day)

                stdout.write(s)

                offset = 0
                size = None
                while True:
                    val = self.vk_api.users.search(
                            offset = offset,
                            count = 1000,
                            **filterArgs
                    )
                    time.sleep(1 / 2)

                    if not size:
                        size = val[0]
                    vk_users = val[1:]
                    if size > 1000:
                        print(
                            "error count={0}, current={1} limit=1000 for age={2} birthday={3}".format(size, len(vk_users),
                                                                                                      age, day))
                        raise CountError()
                    elif not size:
                        break  # пользователей с такими параметрами не найдено

                    for v in vk_users:
                        self.pgCursor.execute(
                            'INSERT INTO vk.dump_people (uid, birthday, birthyear, birthmonth) VALUES (%s, %s, %s, %s) ON CONFLICT DO NOTHING;',
                            (int(v['uid']), day, year, month))
                    self.pgConn.commit()

                    if len(vk_users) and offset + len(vk_users) < size:
                        offset += len(vk_users)
                        z = s + " {0}/{1}".format(offset, size)
                        stdout.write(z)
                        stdout.flush()
                    else:
                        break

            for year, day in ranges:
                filterArgs = {
                    "city":       42,
                    "country":    1,
                    "sex":        1,
                    "birth_year": year,
                    "birth_day":  day
                }
                try:
                    searchUsers(filterArgs, year, day, None)
                except CountError:
                    for month in range(1, 13):
                        filterArgs["birth_month"] = month
                        searchUsers(filterArgs, year, day, month)

                stdout.flush()

                stdout.write("\n")  # move the cursor to the next line

        except CountError:
            exceptionInfo("CountError: dumpAllVrnFemalesIds")
        except Exception:
            exceptionInfo("dumpAllVrnFemalesIds")
        except KeyboardInterrupt:
            pass


    def updateChunckUsersInfo(self, chunk, cityId = None):
        """ Выкачать информацию со страничек указанных в chunk людей.
            Длина chunck должна быть ограничена размером максимального
            однократного запроса в API.
        """

        def toJson(val):
            return json.dumps(val) if val else None

        val = None

        bdateRe = re.compile("([0-9]+)[.]([0-9]+)[.]([0-9]+)")

        fields = user_fields.getFields()
        try:
            with ReconnectContext(self):
                val = self.vk_api.users.get(user_ids = chunk, fields = fields)

        except exceptions.VkAPIError as err:
            print()
            print(str(err))
            print()
            return

        if not val:
            print('warning: empty val')
            return

        for user in val:
            career = user.get("career", None)
            relatives = toJson(user.get("relatives", None))
            schools = toJson(user.get("schools", None))
            universities = toJson(user.get("universities", None))
            langs = toJson(user.get("personal", {}).get("langs", None))
            occupation = user.get("occupation", None)
            military = toJson(user.get("military", None))
            exports = toJson(user.get("exports", None))
            education = toJson(user.get("education", None))
            connections = toJson(user.get("connections", None))
            bdate = user.get("bdate", None)
            if bdate:
                m = bdateRe.match(bdate)
                if m:
                    try:
                        bdate = psycopg2.Date(int(m.group(3)), int(m.group(2)), int(m.group(1)))
                    except ValueError:
                        bdate = None
                else:
                    bdate = None
            lastSeen = user.get("last_seen", {}).get('time', None)
            if lastSeen:
                lastSeen = int(lastSeen)
            uid = user['id']
            self.setCareerData(uid, career, occupation)
            usercity = user.get("city", cityId)
            if isinstance(usercity, dict):
                usercity = usercity.get('id', None)
            usercountry = user.get("country", None)
            if isinstance(usercountry, dict):
                usercountry = usercountry.get('id', None)
            user_photo_max_orig = user.get("photo_max_orig", None)
            if isinstance(user_photo_max_orig, bool):
                user_photo_max_orig = None

            dbFields = [
                ("uid", uid),
                ("first_name", user['first_name']),
                ("last_name", user['last_name']),
                ("birthdate", bdate),
                ("deactivated", user.get("deactivated", None)),
                ("about", user.get("about", None)),
                ("activities", user.get("activities", None)),
                ("books", user.get("books", None)),
                ("can_post", user.get("can_post", None)),
                ("can_see_all_posts", user.get("can_see_all_posts", None)),
                ("can_send_friend_request", user.get("can_send_friend_request", None)),
                ("can_write_private_message", user.get("can_write_private_message", None)),
                ("city", usercity),
                ("connections", connections),
                ("contacts_mobile_phone", user.get("contacts", {}).get("mobile_phone", None)),
                ("contacts_home_phone", user.get("contacts", {}).get("home_phone", None)),
                ("counters_albums", user.get("counters", {}).get("albums", None)),
                ("counters_videos", user.get("counters", {}).get("videos", None)),
                ("counters_audios", user.get("counters", {}).get("audios", None)),
                ("counters_photos", user.get("counters", {}).get("photos", None)),
                ("counters_notes", user.get("counters", {}).get("notes", None)),
                ("counters_friends", user.get("counters", {}).get("friends", None)),
                ("counters_group", user.get("counters", {}).get("group", None)),
                ("counters_pages", user.get("counters", {}).get("pages", None)),
                ("country", usercountry), # 1 = Россия
                ("domain", user.get("domain", None)),
                ("education", education),
                ("exports", exports),
                ("games", user.get("games", None)),
                ("has_mobile", user.get("has_mobile", None)),
                ("has_photo", user.get("has_photo", None)),
                ("home_town", user.get("home_town", None)),
                ("interests", user.get("interests", None)),
                ("last_seen", lastSeen),
                ("maiden_name", user.get("maiden_name", None)),
                ("military", military),
                ("movies", user.get("movies", None)),
                ("music", user.get("music", None)),
                ("nickname", user.get("nickname", None)),
                ("political", user.get("personal", {}).get("political", None)),
                ("langs", langs),
                ("religion", user.get("personal", {}).get("religion", None)),
                ("inspired_by", user.get("personal", {}).get("inspired_by", None)),
                ("people_main", user.get("personal", {}).get("people_main", None)),
                ("life_main", user.get("personal", {}).get("life_main", None)),
                ("smoking", user.get("personal", {}).get("smoking", None)),
                ("alcohol", user.get("personal", {}).get("alcohol", None)),
                ("photo_max_orig", user_photo_max_orig),
                ("quotes", user.get("quotes", None)),
                ("relatives", relatives),
                ("relation", user.get("relation", None)),
                ("relation_partner", user.get("relation_partner", {}).get('id', None)),
                ("schools", schools),
                ("screen_name", user.get("screen_name", None)),
                ("site", user.get("site", None)),
                ("status", user.get("status", None)),
                ("tv", user.get("tv", None)),
                ("universities", universities),
                ("verified", user.get("verified", None)),
                ("wall_comments", user.get("wall_comments", None)),
                ("sex", user.get("sex", 0)),
            ]
            # pprint.pprint(dbFields)

            keys = [fieldName for (fieldName, fieldValue) in dbFields]
            vals = [fieldValue for (fieldName, fieldValue) in dbFields]
            fmt = (
                    'INSERT INTO vk.dump_people ({0}) VALUES ({1}) ON CONFLICT ON CONSTRAINT "DUMP_PEOPLE_pkey" DO UPDATE SET {2};'.format(
                            ",".join(keys),
                            ",".join(["%s"] * len(dbFields)),
                            ",\n ".join(["{0} = coalesce(%s,dump_people.{0})".format(key) for key in keys])
                    )
            )
            self.pgCursor.execute(fmt, vals + vals)

        self.pgConn.commit()
        time.sleep(1 / 2)


    def downloadChuncksFemaleInfo(self, allUids, city):
        """ Выполнить закачку заданного списка пользователей (1 поток) """
        chuncks = self.getDumpingRange(allUids, step = 1000)
        offset = 0
        for (chunk, i) in chuncks:
            if i < offset:
                continue
            printChange("{2}: offset = {0} / total = {1}".format(i, len(allUids), self.accessToken.pageId))
            self.updateChunckUsersInfo(chunk, city)


    def dumpAllVrnFemalesInfo(self, dumpNewOnly = True, city = 42):
        """ Выкачать информацию страничек людей (по умолчанию dumpNewOnly = True - только для новых).
            Если город (city) указан, то для людей не указавших город - будет задаваться указанный город"""

        if dumpNewOnly:
            self.pgCursor.execute("SELECT uid FROM vk.dump_people WHERE first_name IS NULL ORDER BY uid;")
        else:
            self.pgCursor.execute("SELECT uid FROM vk.dump_people ORDER BY uid;")

        allUids = [uid for uid, *other in self.pgCursor.fetchall()]
        dumpers = self.wrkConfig.getSearchers()

        i = 0
        dumperIndex = 0
        allUidsChuncks = []
        while i < len(allUids):
            prevI = i
            i += int(len(allUids) / len(dumpers)) + 10
            if i > len(allUids):
                i = len(allUids)
            dumper = dumpers[dumperIndex]
            allUidsChuncks.append((allUids[prevI:i], city, dumper))
            dumperIndex += 1

        try:
            if len(allUids) < 100000:
                self.downloadChuncksFemaleInfo(allUids, city)
            else:
                multiprocessExecute(allUidsChuncks, dumpPagesInfo)
            stdout.write("\n")  # move the cursor to the next line
        except KeyboardInterrupt:
            print("KeyboardInterrupt")
        except Exception:
            exceptionInfo("dumpAllVrnFemalesInfo")


    def dumpConcurentPublics(self, concurrentsOnly = False):
        """ Выкачать информацию о пабликах конкурентов через поиск сообществ. Обновить для пабликов признак конкуренции
        """
        for keyword, isConcurrent in [
            ("стрижки", False),
            ("реснички", False),
            ("ресницы", False),
            ("ногти", False),
            ("биозавивка", False),
            ("маникюр", False),
            ("педикюр", False),
            ("красоты", False),
            ("фешн", False),
            ("fachion", False),
            ("косметология", False),
            ("косметолог", False),
            ("автомобили", False),
            ("шугаринг", True),
            ("депиляция", True),
            ("эпиляция", True),
            ("шугари", True),
            ("депиляц", True),
            ("эпиляц", True),
            ("бикини", True),
            ("воск", True),
        ]:
            if concurrentsOnly and not isConcurrent:
                continue

            criterias = [{'city_id': 42, 'country_id': 1, 'q': keyword}]
            searchedPublics = self.search(criterias, getSearchMethod = lambda vk_api: vk_api.groups.search,
                                          idKey = 'gid')
            for gid in searchedPublics:
                self.pgCursor.execute("""INSERT INTO vk.loyal_publics (public_id, is_concurent, description)
                                      VALUES (%s, %s, %s) 
                                      ON CONFLICT (public_id) WHERE public_id <> 0 DO UPDATE SET is_concurent = %s;""",
                                      (int(gid), isConcurrent, None, isConcurrent))
            self.pgConn.commit()
            time.sleep(1 / 2)
            print(searchedPublics)

        self.pgCursor.execute('SELECT public_id, public_shortname, description FROM vk.loyal_publics;')
        for (gid, shortname, descr) in self.pgCursor.fetchall():
            print((gid, shortname))
            needDumpContacts = False
            if gid == 0:
                gids = self.vk_api.groups.getById(group_ids = shortname)
                time.sleep(1 / 2)
                gid = gids[0]['gid']
                descr = gids[0].get('name', '')
                self.pgCursor.execute(
                    'UPDATE vk.loyal_publics SET public_id = %s, description = %s WHERE public_shortname = %s;',
                    (gid, descr, shortname))
                time.sleep(1 / 2)
                needDumpContacts = True
            elif shortname == '':
                gids = self.vk_api.groups.getById(group_ids = gid)
                shortname = gids[0].get('screen_name', '')
                descr = gids[0].get('name', '')
                self.pgCursor.execute(
                    'UPDATE vk.loyal_publics SET public_shortname = %s, description = %s WHERE public_id = %s;',
                    (shortname, descr, gid))
                time.sleep(1 / 2)
                needDumpContacts = True
            elif descr is None:
                gids = self.vk_api.groups.getById(group_ids = gid)
                descr = gids[0].get('name', '')
                self.pgCursor.execute('UPDATE vk.loyal_publics SET description = %s WHERE public_id = %s;',
                                      (descr, gid))
                time.sleep(1 / 2)
                needDumpContacts = True

            if needDumpContacts:
                c = self.dumpPublicsContacts(gid)
                time.sleep(1 / 2)
                for uid in c.get('uids', {}):
                    self.pgCursor.execute(
                        'INSERT INTO vk.public_contacts (public_id, uid) VALUES (%s, %s) ON CONFLICT DO NOTHING;',
                        (int(gid), uid))

            self.pgConn.commit()


    def dumpUserFriendsAndFollowers(self, uid, startOperationTimestamp):
        """ Выкачать друзей и подписчиков заданного пользователя
        """
        followers = set()
        friends = set()

        def dumpUids(container, method):
            try:
                container.update(method(uid))
                return True
            except exceptions.VkAPIError as err:
                print(str(err))
                if err.code == exceptions.USER_DELETED_OR_BANNED:
                    return False;
            return True

        for result in [dumpUids(container, method) for container, method in
                       [(followers, self.dumpFollowers), (friends, self.dumpFriends)]]:
            if not result:
                return

        for field, table, constraint, container in [('friend_uid', 'vk.friends', 'friends_pkey', friends),
                                                    ('follower_uid', 'vk.followers', 'followers_pkey', followers)]:
            self.pgCursor.execute(
                "SELECT {0} FROM {1} WHERE src_uid = %s AND (append_date > deleted_date OR deleted_date IS NULL);".format(
                    field, table), (uid,))
            dbActiveUids = set([uid[0] for uid in self.pgCursor.fetchall()])

            realDeletedUids = dbActiveUids - container
            realNewUids = container - dbActiveUids

            for friend in realDeletedUids:
                self.pgCursor.execute(
                    "UPDATE {0} SET deleted_date = %s WHERE src_uid = %s AND {1} = %s;".format(table, field),
                    (startOperationTimestamp, uid, friend))

            for friend in realNewUids:
                self.pgCursor.execute("""INSERT INTO {0} (src_uid, {1}, append_date) VALUES (%s, %s, %s)
                                 ON CONFLICT ON CONSTRAINT {2} DO UPDATE SET append_date = %s;""".format(table, field,
                                                                                                         constraint),
                                      (uid, friend, startOperationTimestamp, startOperationTimestamp))

        self.pgConn.commit()
        time.sleep(1 / 2)

    """ Выкачать информацию со страничек новых пользователей, которые были добавлены в ходе других операций. """
    def updateUsersInfo(self):
        # self.dumpAllVrnFemalesInfo(dumpNewOnly = False, city = 42)
        print("Start updating user info... Execute long time insert...")
        self.pgCursor.execute("SET work_mem='1GB';")
        self.pgCursor.execute(
                """INSERT INTO vk.dump_people (uid) ( (
                         (SELECT DISTINCT follower_uid FROM vk.followers ORDER BY follower_uid)
                         UNION
                         (SELECT DISTINCT friend_uid FROM vk.friends ORDER BY friend_uid)
                         UNION
                         (SELECT DISTINCT concurent_uid FROM vk.concurent_pages ORDER BY concurent_uid)
                         UNION
                         (SELECT DISTINCT uid FROM vk.public_peoples ORDER BY uid)
                         UNION
                         (SELECT DISTINCT uid FROM vk.public_contacts ORDER BY uid)
                       )
                       EXCEPT
                         (SELECT DISTINCT uid FROM vk.dump_people ORDER BY uid)
                     ) ON CONFLICT DO NOTHING;""")
        self.pgConn.commit()
        print("Inserting - ok")
        self.dumpAllVrnFemalesInfo(dumpNewOnly = True, city = None)


    def dumpPublicPeoples(self, gid, startOperationTimestamp=datetime.datetime.now()):
        """ Закачать списки пользователей для заданного паблика и обновить информацию о них в базе.
        """
        try:
            publicUids = self.dumpPublicUsers(gid)
        except exceptions.VkAPIError as err:
            print(str(err))
            return

        if publicUids:
            self.pgCursor.execute(
                "SELECT uid FROM vk.public_peoples WHERE public_id = %s AND (append_date > deleted_date OR deleted_date IS NULL);",
                (gid,))
            dbActiveUids = set([uid[0] for uid in self.pgCursor.fetchall()])

            realDeletedUids = dbActiveUids - publicUids
            realNewUids = publicUids - dbActiveUids

            for uid in realDeletedUids:
                self.pgCursor.execute(
                    "UPDATE vk.public_peoples SET deleted_date = %s WHERE public_id = %s AND uid = %s;",
                    (startOperationTimestamp, gid, uid))

            for uid in realNewUids:
                self.pgCursor.execute("""INSERT INTO vk.public_peoples (public_id, uid, append_date) VALUES (%s, %s, %s)
                                 ON CONFLICT ON CONSTRAINT public_peoples_pkey DO UPDATE SET append_date = %s;""",
                                      (gid, uid, startOperationTimestamp, startOperationTimestamp))

            self.pgConn.commit()


    def dumpPublicsPeoples(self, concurrentsOnly = False):
        """ Закачать списки пользователей для заданных (в базе) пабликов и обновить информацию о них в базе.
        """
        if concurrentsOnly:
            self.pgCursor.execute(
                "SELECT public_id FROM vk.loyal_publics where is_concurent = True ORDER BY public_id;")
        else:
            self.pgCursor.execute("SELECT public_id FROM vk.loyal_publics ORDER BY public_id;")
        groups = self.pgCursor.fetchall()
        cnt = 0;
        for gid in groups:
            print("dumping group:", gid, " {0}/{1}".format(cnt, len(groups)))
            cnt += 1
            self.dumpPublicPeoples(gid)


    def dumpUsersFriends(self):
        """ Выкачать друзей каждой живой воронежской девушки
        """
        unixYesterday = int(time.mktime((datetime.datetime.now() - datetime.timedelta(days = 1)).timetuple()))
        unixYesterday = 1481036806  # datetime.datetime(2016, month=12, day=6, hour=18, minutes=5, seconds=58))
        self.pgCursor.execute(
                """SELECT uid FROM vk.dump_people
                    WHERE city = 42
                      AND last_seen > %s
                      AND sex = 1
                      AND deactivated IS NULL
                    ORDER BY uid;""", (unixYesterday,)
        )
        uids = self.pgCursor.fetchall()
        cnt = 0;
        for uid in uids:
            print("dumping friends of:", uid, " {0}/{1}".format(cnt, len(uids)))
            cnt += 1
            self.dumpUserFriendsAndFollowers(uid)

    def setCareerData(self, uid, career, occupation):
        if occupation:
            occupation_type = occupation.get('type', '')
            occupation_id = int(occupation.get('id', 0))
            company = occupation.get('name', '')

            if occupation_type == 'work':
                public_id = occupation_id
                if public_id > 0:
                    self.pgCursor.execute("""INSERT INTO vk.public_contacts (public_id, uid, description) VALUES (%s, %s, %s)
                                     ON CONFLICT ON CONSTRAINT public_contacnts_pkey DO UPDATE SET description = coalesce(%s, public_contacts.description);""",
                                          (public_id, uid, company, company));
                else:
                    self.pgCursor.execute(
                        """INSERT INTO vk.peoples_career (uid, company, "from", "until", "position") VALUES (%s, %s, 0, 0, '') ON CONFLICT DO NOTHING;""",
                        (uid, company))

            elif occupation_type == 'university':
                self.pgCursor.execute(
                        """INSERT INTO vk.education (uid, is_university, place_id) VALUES (%s, True, %s)
                               ON CONFLICT DO NOTHING;""", (uid, occupation_id))

            elif occupation_type == 'school':
                self.pgCursor.execute(
                        """INSERT INTO vk.education (uid, is_university, place_id) VALUES (%s, False, %s)
                               ON CONFLICT DO NOTHING;""", (uid, occupation_id))
            else:
                print('unhandled occupation:', uid, occupation)

        if career:
            for careerr_item in career:
                public_id = careerr_item.get('group_id', None)
                city_id = careerr_item.get('city_id', None)
                country_id = careerr_item.get('country_id', None)
                position = careerr_item.get('position', '')
                from_year = careerr_item.get('from', 0)
                until_year = careerr_item.get('until', 0)
                company = careerr_item.get('company', '')
                city_name = careerr_item.get('city_name', None)

                if public_id:
                    self.pgCursor.execute(
                            """INSERT INTO vk.public_contacts (public_id, uid, city_id, country_id, position)
                                    VALUES (%s, %s, %s, %s, %s)
                               ON CONFLICT (public_id, uid)
                               DO UPDATE SET city_id    = coalesce(%s, public_contacts.city_id),
                                             country_id = coalesce(%s, public_contacts.country_id),
                                             position   = coalesce(%s, public_contacts.position);""",
                            (public_id, uid, city_id, country_id, position, city_id, country_id, position));
                else:
                    self.pgCursor.execute(
                            """INSERT INTO vk.peoples_career (uid, company, country_id, city_id, city_name, "from", "until", "position")
                                    VALUES (%s, %s, %s, %s, %s, %s, %s, %s)
                               ON CONFLICT DO NOTHING;""",
                            (uid, company, country_id, city_id, city_name, from_year, until_year, position));

        self.pgConn.commit()

    """ Задампить все о конкурентах
    """

    def dumpAllConcurrents(self):
        self.dumpConcurentPublics(concurrentsOnly = True)

        dumpers = self.wrkConfig.getSearchers()

        # Вью выполняет точную фильтрацию конкурентов с помощью полнотекстового поиска
        self.pgCursor.execute("SELECT public_id FROM vk.sugar_publics;")
        sugarPublics = self.pgCursor.fetchall()

        self.pgCursor.execute("SELECT CURRENT_TIMESTAMP;")
        startOperationTimestamp = self.pgCursor.fetchall()[0][0]

        def splitBalance(dumpers, srcContainer):
            nonlocal startOperationTimestamp
            i = 0
            dumperIndex = 0
            chunckSize = int(len(srcContainer) / len(dumpers)) + 10
            chuncks = []
            while i < len(srcContainer):
                prevI = i
                i += chunckSize
                if i > len(srcContainer):
                    i = len(srcContainer)
                dumper = dumpers[dumperIndex]
                chuncks.append((srcContainer[prevI:i], dumper, startOperationTimestamp))
                dumperIndex += 1
            return chuncks

        def dumpPublicPeoplesChunck(chunck, dumper, startOperationTimestamp):
            i = 0
            for public_id in chunck:
                print("dump public peoples:", dumper.accessToken.pageId, i, "/", len(chunck))
                i += 1
                dumper.dumpPublicPeoples(public_id, startOperationTimestamp)

        multiprocessExecute(splitBalance(dumpers, sugarPublics), dumpPublicPeoplesChunck)

        # Вью, соединяющее админов пабликов шугаринга и индивидуальных мастеров, выявленных с помощью полнотекстового поиска
        self.pgCursor.execute("SELECT uid FROM vk.all_sugar_masters;")
        sugarMasters = [uid[0] for uid in self.pgCursor.fetchall()]

        def dumpSugarMastersChunck(chunck, dumper, startOperationTimestamp):
            i = 0
            for uid in chunck:
                print("dump friends:", dumper.accessToken.pageId, i, "/", len(chunck))
                i += 1
                dumper.dumpUserFriendsAndFollowers(uid,
                                                   startOperationTimestamp)  # Выкачать и обновить списки друзей и подписчиков каждого воронежского мастера шугаринга

        multiprocessExecute(splitBalance(dumpers, sugarMasters), dumpSugarMastersChunck)

        # Обновить информацию обо всем народе, идентификаторы которого появились теперь в базе
        self.updateUsersInfo()
