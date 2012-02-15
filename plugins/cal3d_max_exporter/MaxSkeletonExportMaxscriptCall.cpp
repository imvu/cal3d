//
// Copyright (C) 2004 Mekensleep
//
// Mekensleep
// 24 rue vieille du temple
// 75004 Paris
//       licensing@mekensleep.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//----------------------------------------------------------------------------//
// SkeletonCandidate.cpp                                                      //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//----------------------------------------------------------------------------//
// This program is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU General Public License as published by the Free //
// Software Foundation; either version 2 of the License, or (at your option)  //
// any later version.                                                         //
//----------------------------------------------------------------------------//

#include <cal3d/coreskeleton.h>
#include <cal3d/error.h>
#include <cal3d/saver.h>
#include "StdAfx.h"
#include "Exporter.h"
#include "SkeletonCandidate.h"
#include "BoneCandidate.h"
#include "BaseInterface.h"
#include "BaseNode.h"
#include "MaxInterface.h"
#include "SkeletonExportsheet.h"

bool CSkeletonCandidate::CreateFromInterfaceFromMaxScriptCall()
{
	int nodeId;

	// clear current content
	Clear();

	CMaxInterface* imax	= static_cast<CMaxInterface*>(theExporter.GetInterface());

	// loop through all nodes in memorized array (that has been passed by Maxscript)
	const int NumNodes = imax->GetNumNodesInTabNodeFromMaxscript();
	for(nodeId = 0; nodeId < NumNodes; nodeId++)
	{
		CBaseNode* basenode = imax->GetNodeFromMaxscriptArray(nodeId);
		if (!basenode) return false;

		// recursively add the node to the skeleton candidate
		if(!AddNode(basenode, -1)) return false;
	}

	return true;
}

//------------------------------------------------------------------------------//
// Export the skeleton from a Maxscript call                                  --//
//------------------------------------------------------------------------------//
bool CExporter::ExportSkeletonFromMaxscriptCall(const std::string& strFilename, bool bShowUI)
{
	// check if a valid interface is set
	if(m_pInterface == 0)
	{
		SetLastError("Invalid handle.", __FILE__, __LINE__);
		return false;
	}

	// build a skeleton candidate
	CSkeletonCandidate skeletonCandidate;
	if(!skeletonCandidate.CreateFromInterfaceFromMaxScriptCall()) return false;

	//Does the user wants to see the UI and select the bones himself ?
	if (bShowUI)
	{
		// show export wizard sheet
		CSkeletonExportSheet sheet(m_pInterface, "Cal3D Skeleton Export", m_pInterface->GetMainWnd());
		sheet.SetSkeletonCandidate(&skeletonCandidate);
		sheet.SetWizardMode();
		if(sheet.DoModal() != ID_WIZFINISH) return true;
	}

	// build the selected ids of the bone candidates
	int selectedCount;
	selectedCount = skeletonCandidate.BuildSelectedId();
	if(selectedCount == 0)
	{
		SetLastError("No bones selected to export.", __FILE__, __LINE__);
		return false;
	}

	// create the core skeleton instance
	CalCoreSkeleton coreSkeleton;

	// get bone candidate vector
	const std::vector<CBoneCandidate *>& vectorBoneCandidate = skeletonCandidate.GetVectorBoneCandidate();

	// start the progress info
  CStackProgress progress(m_pInterface, "Exporting to skeleton file...");

	size_t boneCandidateId;
	int selectedId;
	for(boneCandidateId = 0, selectedId = 0; boneCandidateId < vectorBoneCandidate.size(); boneCandidateId++)
	{
		// get the bone candidate
		CBoneCandidate *pBoneCandidate;
		pBoneCandidate = vectorBoneCandidate[boneCandidateId];

		// only export selected bone candidates
		if(pBoneCandidate->IsSelected())
		{
			m_pInterface->SetProgressInfo(int(100.0f * (selectedId + 1) / selectedCount));
			selectedId++;

			// get the parent id of the bone candidate
			int parentId = skeletonCandidate.GetParentSelectedId(boneCandidateId);

                        CalCoreBonePtr pCoreBone(new CalCoreBone(pBoneCandidate->GetNode()->GetName(), parentId));

			// get the translation and the rotation of the bone candidate
			CalVector translation;
			CalQuaternion rotation;
			skeletonCandidate.GetTranslationAndRotation(boneCandidateId, -1.0f, translation, rotation);

			// set the translation and rotation
			pCoreBone->relativeTransform.translation = (translation);
			pCoreBone->relativeTransform.rotation = (rotation);

			// get the bone space translation and the rotation of the bone candidate
			CalVector translationBoneSpace;
			CalQuaternion rotationBoneSpace;
			skeletonCandidate.GetTranslationAndRotationBoneSpace(boneCandidateId, -1.0f, translationBoneSpace, rotationBoneSpace);

			// set the bone space translation and rotation
			pCoreBone->inverseBindPoseTransform.translation = translationBoneSpace;
			pCoreBone->inverseBindPoseTransform.rotation = rotationBoneSpace;

			coreSkeleton.addCoreBone(pCoreBone);
		}
	}

	// save core skeleton to the file
	if(!CalSaver::saveCoreSkeleton(strFilename, &coreSkeleton))
	{
		SetLastError(CalError::getLastErrorText(), __FILE__, __LINE__);
		return false;
	}

	return true;
}
