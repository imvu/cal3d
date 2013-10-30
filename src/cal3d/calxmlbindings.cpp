#include "cal3d/saver.h"
#include "cal3d/error.h"
#include "cal3d/vector.h"
#include "cal3d/quaternion.h"
#include "cal3d/coreskeleton.h"
#include "cal3d/corebone.h"
#include "cal3d/coreanimation.h"
#include "cal3d/coremorphanimation.h"
#include "cal3d/coretrack.h"
#include "cal3d/coremorphtrack.h"
#include "cal3d/corekeyframe.h"
#include "cal3d/coremorphkeyframe.h"
#include "cal3d/coremesh.h"
#include "cal3d/coresubmesh.h"
#include "cal3d/coremorphtarget.h"
#include "cal3d/corematerial.h"
#include "cal3d/tinyxml.h"
#include "cal3d/tinybind.h"
#include "calxmlbindings.h"

TiXmlBinding<CalCoreMorphAnimation> const*
GetTiXmlBinding(CalCoreMorphAnimation const&,  IdentityBase) {
    static MemberTiXmlBinding<CalCoreMorphAnimation> binding;
    if (binding.empty()) {
        binding.AddMember("DURATION", MemberAttribute(&CalCoreMorphAnimation::duration));
        binding.AddMember("TRACK", MemberPeer(&CalCoreMorphAnimation::tracks));
    }
    return &binding;
}

TiXmlBinding<CalCoreMorphTrack> const*
GetTiXmlBinding(CalCoreMorphTrack const&,  IdentityBase) {
    static MemberTiXmlBinding<CalCoreMorphTrack> binding;
    if (binding.empty()) {
        binding.AddMember("MORPHNAME", MemberAttribute(&CalCoreMorphTrack::morphName));
        binding.AddMember("KEYFRAME", MemberPeer(&CalCoreMorphTrack::keyframes));
    }
    return &binding;
}

TiXmlBinding<CalCoreMorphKeyframe> const*
GetTiXmlBinding(CalCoreMorphKeyframe const&,  IdentityBase) {
    static MemberTiXmlBinding<CalCoreMorphKeyframe> binding;
    if (binding.empty()) {
        binding.AddMember("TIME", MemberAttribute(&CalCoreMorphKeyframe::time));
        binding.AddMember("WEIGHT", Member(&CalCoreMorphKeyframe::weight));
    }
    return &binding;
}


