
How to maintain the Max exporters

3ds Max exposes a C++ API that is tied to the ABI of the specific version of MSVC that it was built against.
This means that we have to use separate headers for each version of 3ds Max that we build against.
To support a new version of Max, do the following:

1) copy the "maxsdk" folder from the 3ds Max SDK install into Ext/3dsmaxNNNN/maxsdk 

2) add folders "DebugNNNN" and "ReleaseNNNN" into source/cal3d/shared.scons

3) inside Ext/3dsmaxNNNN/maxsdk, do the following:
  copy samples\modifiers\morpher\wm3.h include\
  copy samples\modifiers\morpher\include\Morph* include\

4) Make some light edits to includes\wm3.h:
- delete the include of resourceOverride.h and relative ".\include\" includes.
- add the following block somewhere after include resource.h:

#include "MorpherClassID.h"
#include "MorpherExport.h"

/* this is stuff you have to copy forward to each new version of wm3.h
  - jwatte
 */
#define MorphExport __declspec( dllimport )
#define IDS_CLASS_NAME                  3
#define IDS_MORPHMTL                    39
#define IDS_MTL_BASENAME                46
#define IDS_MTL_MAPNAME                 45


5) It might now build. Try "s\scons -j 1" from the Stable directory. If not, you need to fix whatever API breaking changes were made.

6) Try the built exporter. Export some morphs and morph animations. This may crash, or generate bad data. 
If the morph export crashes or generates bad data, you have an ABI compatibility problem. The reason is 
that 3ds Max uses std::vector<> in the header for wm3.h. The implementation of std::vector<> may change 
between versions of MSVC. Thus, if we upgrade MSVC, or if we use a different version than Autodesk used to build 3ds Max version NNNN, it may mis-match.
The solution is to define a CompatibleVector template class, inside wm3.h, and replace all uses of vector<> with CompatibleVector<> in wm3.h.
Check the implementation in 3dsmax2010/maxsdk/include/wm3.h for an example.
Note: This is highly technical! You will have to debug the crashing export to figure out what the structure of the vector class should be in memory 
(there's likely some start and end pointer, perhaps a size count, etc) and then mirror this structure/layout exactly in your CompatibleVector class.

7) Update the WiX scripts to install the release exporter if the appropriate version of 3ds Max is found on the target computer.
The WiX source is in: Source\WiX\ImvuCal3DMaxExport.wxs
The build script is in: s\build_max_exporter_installer.bat
