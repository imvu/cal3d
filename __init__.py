import imvu
import imvu.opsys

if imvu.opsys.isMacOSX():
    # HACK: Work around lame link dependencies.
    # _avatarwindow provides some standard C++ stuff that
    # _cal3d doesn't.
    import imvu.avatarwindow

if __debug__:
    from _cal3d_debug import *
else:
    from _cal3d import *
