import os
from .create_custom_mutation_class import create_custom_mutation_class, CudListNode

if os.getenv('REMOTE_DEBUG') and os.getenv('REMOTE_DEBUG') != '0':
    import pydevd_pycharm
    pydevd_pycharm.settrace(os.getenv('REMOTE_DEBUG_LOCAL_HOSTNAME'),
                            port=int(os.getenv('REMOTE_DEBUG_LOCAL_PORT')), stdoutToServer=True, stderrToServer=True)


