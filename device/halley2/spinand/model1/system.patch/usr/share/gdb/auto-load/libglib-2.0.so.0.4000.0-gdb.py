import sys
import gdb

# Update module path.
dir_ = '/home2/yjma/Bluez/network/apps/bluetooth/../output/app/usr/share/glib-2.0/gdb'
if not dir_ in sys.path:
    sys.path.insert(0, dir_)

from glib import register
register (gdb.current_objfile ())
