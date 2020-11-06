Import('RTT_ROOT')
Import('rtconfig')
from building import *

cwd     = GetCurrentDir()
src     = Glob('dap_vfs.c') 
src     = Glob('ummfp.c') 


CPPPATH = [cwd]

group = DefineGroup('ummfp', src, depend = ['PKG_USING_UMMFP'], CPPPATH = CPPPATH)

Return('group')
