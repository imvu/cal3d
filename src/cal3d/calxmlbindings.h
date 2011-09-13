#include "cal3d/tinybind.h"

struct CalHeader {
    int version;
    char const* magic;
};

const TiXmlBinding<CalHeader>* GetTiXmlBinding(CalHeader const&,  IdentityBase);
const TiXmlBinding<CalCoreMorphAnimation>* GetTiXmlBinding(CalCoreMorphAnimation const&,  IdentityBase);
const TiXmlBinding<CalCoreMorphTrack>* GetTiXmlBinding(CalCoreMorphTrack const&,  IdentityBase);
const TiXmlBinding<CalCoreMorphKeyframe>* GetTiXmlBinding(CalCoreMorphKeyframe const&,  IdentityBase);
