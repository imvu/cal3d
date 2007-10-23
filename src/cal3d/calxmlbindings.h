#include "cal3d/tinybind.h"


TiXmlBinding<CalHeader> const *
GetTiXmlBinding( CalHeader const &,  IdentityBase  );

TiXmlBinding<CalCoreAnimatedMorph> const *
GetTiXmlBinding( CalCoreAnimatedMorph const &,  IdentityBase  );

TiXmlBinding<CalCoreMorphTrack> const *
GetTiXmlBinding( CalCoreMorphTrack const &,  IdentityBase  );

TiXmlBinding<CalCoreMorphKeyframe> const *
GetTiXmlBinding( CalCoreMorphKeyframe const &,  IdentityBase  );
