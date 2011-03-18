#include "cal3d/saver.h"
#include "cal3d/error.h"
#include "cal3d/vector.h"
#include "cal3d/quaternion.h"
#include "cal3d/coreskeleton.h"
#include "cal3d/corebone.h"
#include "cal3d/coreanimation.h"
#include "cal3d/coreanimatedmorph.h"
#include "cal3d/coretrack.h"
#include "cal3d/coremorphtrack.h"
#include "cal3d/corekeyframe.h"
#include "cal3d/coremorphkeyframe.h"
#include "cal3d/coremesh.h"
#include "cal3d/coresubmesh.h"
#include "cal3d/coresubmorphtarget.h"
#include "cal3d/corematerial.h"
#include "cal3d/tinyxml.h"
#include "cal3d/tinybind.h"
#include "calxmlbindings.h"

TiXmlBinding<CalHeader> const *
GetTiXmlBinding( CalHeader const &,  IdentityBase  )
{
  static MemberTiXmlBinding<CalHeader> binding;
  if( binding.empty() ) {
    binding.AddMember( "VERSION", MemberAttribute(&CalHeader::version) );
    binding.AddMember( "MAGIC", MemberAttribute(&CalHeader::magic) );
  }
  return &binding;
}


TiXmlBinding<CalCoreAnimatedMorph> const *
GetTiXmlBinding( CalCoreAnimatedMorph const &,  IdentityBase  )
{
  static MemberTiXmlBinding<CalCoreAnimatedMorph> binding;
  if( binding.empty() ) {
    binding.AddMember( "DURATION", MemberAttribute(&CalCoreAnimatedMorph::getDuration,
        &CalCoreAnimatedMorph::setDuration) );
    binding.AddMember( "TRACK", MemberPeer(&CalCoreAnimatedMorph::getListCoreTrack) );
  }
  return &binding;
}

TiXmlBinding<CalCoreMorphTrack> const *
GetTiXmlBinding( CalCoreMorphTrack const &,  IdentityBase  )
{
  static MemberTiXmlBinding<CalCoreMorphTrack> binding;
  if( binding.empty() ) {
    binding.AddMember("MORPHNAME", MemberAttribute(&CalCoreMorphTrack::morphName));
    binding.AddMember("NUMKEYFRAMES", MemberAttribute(&CalCoreMorphTrack::getCoreMorphKeyframeCount));
    binding.AddMember("KEYFRAME", MemberPeer(&CalCoreMorphTrack::keyframes));
  }
  return &binding;
}

TiXmlBinding<CalCoreMorphKeyframe> const *
GetTiXmlBinding( CalCoreMorphKeyframe const &,  IdentityBase  )
{
  static MemberTiXmlBinding<CalCoreMorphKeyframe> binding;
  if( binding.empty() ) {
    binding.AddMember("TIME", MemberAttribute(&CalCoreMorphKeyframe::time));
    binding.AddMember("WEIGHT", MemberAttribute(&CalCoreMorphKeyframe::weight));
  }
  return &binding;
}


