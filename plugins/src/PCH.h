#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <stack>
#include <map>
#include <set>
#include <sstream>
#include <algorithm>

#include <afxwin.h>
#include <afxext.h>
#include <afxcmn.h>

#include "max.h"
#if MAX_VERSION_MAJOR >= 14
#include "maxscript/maxscript.h"
#else
#include "maxscrpt/maxscrpt.h"
#endif
#include "bipexp.h"
#include "phyexp.h"
#include "decomp.h"
#include "utillib.h"
#include "SimpObj.h"

#if MAX_RELEASE >= 4000
#include "iskin.h"
#endif

#include "resource.h"
