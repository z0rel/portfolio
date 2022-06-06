#!/usr/bin/env python3

import sys
import logging
import traceback
import os
import stat
import shelve


sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from utils import ora2lin_utils
from utils.ora2lin_utils import MetaRepr, StatTree, needReload

logging.basicConfig(level=logging.INFO,
                    filename='/tmp/ora2lin.log', filemode='w',
                    format='%(asctime)s %(message)s',
                    datefmt='%m-%d %H:%M')

logger = logging.getLogger('ora2lin')

exceptionInfo = ora2lin_utils.ExceptionInfo(logger.info, lambda x : '----- {0} -----'.format(x))

def initPatches():
  try:
    sys.path.append('/usr/share/qtcreator/debugger')

  except:
    exceptionInfo('import dumper packages')
    pass







if __name__ == '__main__':
  ora2lin_utils.testStatTree()



