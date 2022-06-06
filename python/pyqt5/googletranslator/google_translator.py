#! /usr/bin/env python3
# -*- coding: utf-8 -*-

from xvfbwrapper import Xvfb
from selenium import webdriver
import time


class Driwer:
  def __init__(self):
    #self.xvfb = Xvfb(width=1280, height=720)
    #self.xvfb.start()
    self.driver = webdriver.Firefox()

  def __del__(self):
    self.driver.quit()
    #self.xvfb.stop()

  def initTrDirection(self, fromLang, toLang):
    # Отключить моментальный перевод
    self.driver.find_element_by_id("gt-otf-switch").click()

    def setupTr(dropdownId, langItemId, selectedLang):
      langmap = {
        'ru': 'русский',
        'en': 'английский'
      }
      item_text = {
        'ru': ['Russan' , 'Русский', 'русский'], 
        'en': ['English', 'Английский', 'английский'] 
      }

      l_dropdown = self.driver.find_element_by_id(dropdownId)
      l_dropdown.click()
      
      for i in item_text[selectedLang]:
          langs = self.driver.find_elements_by_xpath('//div[contains(text(), "{0}") and @class="goog-menuitem-content"]'.format(i))
          if len(langs):
            break
      
      langToClick = [lang for lang in langs if lang.text == langmap[selectedLang]]
      langs[0].click()

    setupTr("gt-sl-gms", 'Russian', fromLang)
    self.lastFromLanguage = fromLang

    setupTr("gt-tl-gms", "#gt-tl-gms-menu div.goog-menuitem-content", toLang)
    self.lastToLanguage = toLang

    if fromLang != 'en':
      self.driver.find_element_by_id("gt-swap").click()

  def getGoogleTranslatePage(self):
    self.driver.get("https://translate.google.com/")


class GTranslator:
  def __init__(self):
    self.lastFromLanguage  = None
    self.lastToLanguage    = None
    self.openBrowserWindow = False
    self.trBrowsers = {}

  def gTranslate(self, baseTerm, fromLang, toLang):
    if not baseTerm:
      return ''

    if (fromLang, toLang) not in self.trBrowsers:
      drv = Driwer()
      self.trBrowsers[(fromLang, toLang)] = drv
      drv.getGoogleTranslatePage()
      drv.initTrDirection(fromLang, toLang)
    else:
      drv = self.trBrowsers[(fromLang, toLang)]
      drv.getGoogleTranslatePage()

    sourceEdit = drv.driver.find_element_by_id("source")
    sourceEdit.send_keys(baseTerm)

    trButton = drv.driver.find_element_by_id("gt-submit")
    trButton.click()

    retText = ''
    for i in range(0, 10):
      resBox = drv.driver.find_element_by_id("result_box")
      retText = resBox.text
      if len(retText) > 0:
        return retText
      else:
        time.sleep(0.5)

    return retText

  def __del__(self):
    del self.trBrowsers


gtr = GTranslator()


def test():
  print(gtr.gTranslate("оптимизация LP-вывода", 'ru', 'en'))
  print(gtr.gTranslate('LP-output optimization', 'en', 'ru'))
  print(gtr.gTranslate("оптимизация LP-вывода", 'ru', 'en'))
  print(gtr.gTranslate('LP-output optimization', 'en', 'ru'))


if __name__ == '__main__':
  test()
