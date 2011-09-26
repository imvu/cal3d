//----------------------------------------------------------------------------//
// Exporter.cpp                                                               //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//----------------------------------------------------------------------------//
// This program is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU General Public License as published by the Free //
// Software Foundation; either version 2 of the License, or (at your option)  //
// any later version.                                                         //
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// Includes                                                                   //
//----------------------------------------------------------------------------//

#include "StdAfx.h"
#include <cal3d/coreanimation.h>
#include <cal3d/coremorphanimation.h>
#include <cal3d/corekeyframe.h>
#include <cal3d/coretrack.h>
#include <cal3d/corematerial.h>
#include <cal3d/coremesh.h>
#include <cal3d/coreskeleton.h>
#include <cal3d/error.h>
#include <cal3d/saver.h>
#include "Exporter.h"
#include "BaseInterface.h"
#include "SkeletonCandidate.h"
#include "BoneCandidate.h"
#include "BaseNode.h"
#include "BaseMesh.h"
#include "SkeletonExportSheet.h"
#include "AnimationExportSheet.h"
#include "MorphAnimationExportSheet.h"
#include "MeshExportSheet.h"
#include "MaterialExportSheet.h"
#include "MeshCandidate.h"
#include "MorphAnimationCandidate.h"
#include "SubmeshCandidate.h"
#include "VertexCandidate.h"
#include "MaterialLibraryCandidate.h"
#include "MaterialCandidate.h"

//----------------------------------------------------------------------------//
// The one and only exporter instance                                         //
//----------------------------------------------------------------------------//

CExporter theExporter;

//----------------------------------------------------------------------------//
// Constructors                                                               //
//----------------------------------------------------------------------------//

CExporter::CExporter()
{
	m_pInterface = 0;
}

//----------------------------------------------------------------------------//
// Destructor                                                                 //
//----------------------------------------------------------------------------//

CExporter::~CExporter()
{
}

//----------------------------------------------------------------------------//
// Create an exporter instance for a given interface                          //
//----------------------------------------------------------------------------//

bool CExporter::Create(CBaseInterface *pInterface)
{
  ::OutputDebugString("CExporter::Create()\n");
	// check if a valid interface is set
	if(pInterface == 0)
	{
		SetLastError("Invalid handle.", __FILE__, __LINE__);
		return false;
	}

	m_pInterface = pInterface;

	return true;
}

//----------------------------------------------------------------------------//
// Export the animation to a given file                                       //
//----------------------------------------------------------------------------//

bool CExporter::ExportAnimation(const std::string& strFilename)
{
	// check if a valid interface is set
	if(m_pInterface == 0)
	{
		SetLastError("Invalid handle.", __FILE__, __LINE__);
		return false;
	}

	// build a skeleton candidate
	CSkeletonCandidate skeletonCandidate;

	// show export wizard sheet
	CAnimationExportSheet sheet(m_pInterface, _T("Cal3D Animation Export"), m_pInterface->GetMainWnd());
	sheet.SetSkeletonCandidate(&skeletonCandidate);
	sheet.SetAnimationTime(m_pInterface->GetStartFrame(), m_pInterface->GetEndFrame(), m_pInterface->GetCurrentFrame(), m_pInterface->GetFps());
	sheet.SetWizardMode();
	if(sheet.DoModal() != ID_WIZFINISH) return true;

	// get the number of selected bone candidates
	int selectedCount;
	selectedCount = skeletonCandidate.GetSelectedCount();
	if(selectedCount == 0)
	{
		SetLastError("No bones selected to export.", __FILE__, __LINE__);
		return false;
	}

	// create the core animation instance
	CalCoreAnimation coreAnimation;

	// set the duration of the animation
	float duration;
	duration = (float)(sheet.GetEndFrame() - sheet.GetStartFrame()) / (float)m_pInterface->GetFps();
	coreAnimation.duration = duration;

	// get bone candidate vector
	std::vector<CBoneCandidate *>& vectorBoneCandidate = skeletonCandidate.GetVectorBoneCandidate();

	size_t boneCandidateId;
	for(boneCandidateId = 0; boneCandidateId < vectorBoneCandidate.size(); boneCandidateId++)
	{
		// get the bone candidate
		CBoneCandidate *pBoneCandidate;
		pBoneCandidate = vectorBoneCandidate[boneCandidateId];

		// only create tracks for the selected bone candidates
		if(pBoneCandidate->IsSelected())
		{
			coreAnimation.tracks.push_back(CalCoreTrack(boneCandidateId, CalCoreTrack::KeyframeList()));
		}
	}

	// start the progress info
  CStackProgress progress(m_pInterface, "Exporting to animation file...");

	// calculate the end frame
	int endFrame;
	endFrame = (int)(duration * (float)sheet.GetFps() + 0.5f);

	// calculate the displaced frame
  int displacedFrame;
  displacedFrame = (int)(((float)sheet.GetDisplacement() / (float)m_pInterface->GetFps()) * (float)sheet.GetFps() + 0.5f) % endFrame;

	// calculate the possible wrap frame
  int wrapFrame;
  wrapFrame = (displacedFrame > 0) ? 1 : 0;
  float wrapTime;
  wrapTime = 0.0f;

  int frame;
  int outputFrame;
  for(frame = 0,  outputFrame = 0; frame <= (endFrame + wrapFrame); frame++)
	{
		// update the progress info
		m_pInterface->SetProgressInfo(int(100.0f * (float)frame / (float)(endFrame + wrapFrame + 1)));

		// calculate the time in seconds
		float time;
		time = (float)sheet.GetStartFrame() / (float)m_pInterface->GetFps() + (float)displacedFrame / (float)sheet.GetFps();

/* DEBUG
CString str;
str.Format("frame=%d, endframe=%d, disframe=%d, ouputFrame=%d (%f), time=%f\n", frame, endFrame, displacedFrame, outputFrame, (float)outputFrame / (float)sheet.GetFps() + wrapTime, time);
OutputDebugString(str);
*/

		for(boneCandidateId = 0; boneCandidateId < vectorBoneCandidate.size(); boneCandidateId++)
		{
			// get the bone candidate
			CBoneCandidate *pBoneCandidate;
			pBoneCandidate = vectorBoneCandidate[boneCandidateId];

			// only export keyframes for the selected bone candidates
			if(pBoneCandidate->IsSelected())
			{
				// allocate new core keyframe instance
				CalCoreKeyframe pCoreKeyframe;
				pCoreKeyframe.time = (float)outputFrame / (float)sheet.GetFps() + wrapTime;

				CalVector translation;
				CalQuaternion rotation;
				skeletonCandidate.GetTranslationAndRotation(boneCandidateId, time, translation, rotation);

				pCoreKeyframe.transform.translation = translation;
				pCoreKeyframe.transform.rotation = rotation;

                                // oh god
                                CalCoreTrack::KeyframeList& ls = const_cast<CalCoreTrack::KeyframeList&>(coreAnimation.getCoreTrack(pBoneCandidate->GetId())->keyframes);
                                ls.push_back(pCoreKeyframe);
			}
		}

    // calculate the next displaced frame and its frame time
    if(wrapFrame > 0)
    {
      if(displacedFrame == endFrame)
      {
        wrapTime = 0.0001f;
        displacedFrame = 0;
      }
      else
      {
        wrapTime = 0.0f;
        outputFrame++;
        displacedFrame++;
      }
    }
    else
    {
      outputFrame++;
      displacedFrame++;
   }
	}

	for(boneCandidateId = 0; boneCandidateId < vectorBoneCandidate.size(); boneCandidateId++)
	{
		CBoneCandidate *pBoneCandidate;
		pBoneCandidate = vectorBoneCandidate[boneCandidateId];
		CalCoreTrack pCoreTrack = coreAnimation.tracks[pBoneCandidate->GetId()];
                static double translationTolerance = 0.05;
		static double rotationToleranceDegrees = 0.1;
		// there is no pCoreTrack for bones that are deselected
                CalCoreSkeleton * skelOrNull = skeletonCandidate.GetCoreSkeleton();
		coreAnimation.tracks[pBoneCandidate->GetId()] = *pCoreTrack.compress(translationTolerance, rotationToleranceDegrees, skelOrNull );
	}

	// save core animation to the file
	if(!CalSaver::saveCoreAnimation(strFilename, &coreAnimation))
	{
		SetLastError(CalError::getLastErrorText(), __FILE__, __LINE__);
		return false;
	}

	return true;
}

bool CExporter::ExportMorphAnimation(const std::string& strFilename)
{
	// check if a valid interface is set
	if(m_pInterface == 0)
	{
		SetLastError("Invalid handle.", __FILE__, __LINE__);
		return false;
	}

	// build a morph animation candidate
	CMorphAnimationCandidate morphAnimationCandidate;
        if( !morphAnimationCandidate.Create() ) {
          SetLastError("Creation of CMorphAnimationCandidate instance failed.", __FILE__, __LINE__);
          return false;
        }          

	// show export wizard sheet
	CMorphAnimationExportSheet sheet(_T("Cal3D Animation Export"), m_pInterface->GetMainWnd());
	sheet.SetMorphAnimationTime(m_pInterface->GetStartFrame(), m_pInterface->GetEndFrame(), m_pInterface->GetCurrentFrame(), m_pInterface->GetFps());
	sheet.SetWizardMode();
	if(sheet.DoModal() != ID_WIZFINISH) return true;

	// create the core animation instance
	CalCoreMorphAnimation coreAnimation;

	coreAnimation.duration = (float)(sheet.GetEndFrame() - sheet.GetStartFrame()) / (float)m_pInterface->GetFps();

        
        // find the selected mesh
        // find the morpher modifier
        // foreach channel
        //   create a morph anim track
        //   set the name
        // foreach frame
        //   foreach channel
        //     create keyframe
        //     set value from modifier

	// start the progress info
  CStackProgress progress(m_pInterface, "Exporting to animation file...");

    CBaseMesh * pMesh = morphAnimationCandidate.meshAtTime(-1);
	if( !pMesh ) {
            ::OutputDebugString("No mesh found.\n");
            return false;
	}
    int numMC = pMesh->numMorphChannels();
    for( int i = 0; i < numMC; i++ ) {
      CalCoreMorphTrack pTrack;
      CBaseMesh::MorphKeyFrame keyFrame = pMesh->frameForChannel(i, 0);
      pTrack.morphName = keyFrame.name;
      coreAnimation.tracks.push_back(pTrack);
    }

	// calculate the end frame
	int endFrame;
	endFrame = (int)(coreAnimation.duration * (float)sheet.GetFps() + 0.5f);
        
	// calculate the displaced frame
        int displacedFrame;
        displacedFrame = (int)(((float)sheet.GetDisplacement() / (float)m_pInterface->GetFps()) * (float)sheet.GetFps() + 0.5f) % endFrame;
        
	// calculate the possible wrap frame
        int wrapFrame;
        wrapFrame = (displacedFrame > 0) ? 1 : 0;
        float wrapTime;
        wrapTime = 0.0f;
        
        int frame;
        int outputFrame;
        {
          std::stringstream ss;
          ss << "Writing " << (endFrame + wrapFrame) << " frames of animation." << std::endl;
          ::OutputDebugString(ss.str().c_str());
        }
        for(frame = 0,  outputFrame = 0; frame <= (endFrame + wrapFrame); frame++)
	{
          // update the progress info
          m_pInterface->SetProgressInfo(int(100.0f * (float)frame / (float)(endFrame + wrapFrame + 1)));

          // calculate the time in seconds
          float time;
          time = (float)sheet.GetStartFrame() / (float)m_pInterface->GetFps() + (float)displacedFrame / (float)sheet.GetFps();
          
          /* DEBUG
             CString str;
             str.Format("frame=%d, endframe=%d, disframe=%d, ouputFrame=%d (%f), time=%f\n", frame, endFrame, displacedFrame, outputFrame, (float)outputFrame / (float)sheet.GetFps() + wrapTime, time);
             OutputDebugString(str);
          */
          

          for( int i = 0; i < numMC; i++ ) {
            CBaseMesh::MorphKeyFrame keyFrame = pMesh->frameForChannel(i, time);
            CalCoreMorphTrack * pTrack = coreAnimation.getCoreTrack(keyFrame.name);
            CalCoreMorphKeyframe pFrame;
            pFrame.time = keyFrame.time;
            pFrame.weight = keyFrame.weight / keyFrame.totalWeight;
            pTrack->keyframes.push_back(pFrame);
          }

          // calculate the next displaced frame and its frame time
          if(wrapFrame > 0)
          {
            if(displacedFrame == endFrame)
            {
              wrapTime = 0.0001f;
              displacedFrame = 0;
            }
            else
            {
              wrapTime = 0.0f;
              outputFrame++;
              displacedFrame++;
            }
          }
          else
          {
            outputFrame++;
            displacedFrame++;
          }
        }
        
	// save core animation to the file
	if(!CalSaver::saveCoreMorphAnimation(strFilename, &coreAnimation))
	{
          SetLastError(CalError::getLastErrorText(), __FILE__, __LINE__);
          return false;
	}
        
	// destroy the core animation

	return true;
}

//----------------------------------------------------------------------------//
// Export the material to a given file                                        //
//----------------------------------------------------------------------------//

bool CExporter::ExportMaterial(const std::string& strFilename)
{
	// check if a valid interface is set
	if(m_pInterface == 0)
	{
		SetLastError("Invalid handle.", __FILE__, __LINE__);
		return false;
	}

	// check if a valid interface is set
	if(m_pInterface == 0)
	{
		SetLastError("Invalid handle.", __FILE__, __LINE__);
		return false;
	}

	// build a material library candidate
	CMaterialLibraryCandidate materialLibraryCandidate;
	if(!materialLibraryCandidate.CreateFromInterface()) return false;

	// show export wizard sheet
	CMaterialExportSheet sheet("Cal3D Material Export", m_pInterface->GetMainWnd());
	sheet.SetMaterialLibraryCandidate(&materialLibraryCandidate);
	sheet.SetWizardMode();
	if(sheet.DoModal() != ID_WIZFINISH) return true;

	// get selected material candidate
	CMaterialCandidate *pMaterialCandidate;
	pMaterialCandidate = materialLibraryCandidate.GetSelectedMaterialCandidate();
	if(pMaterialCandidate == 0)
	{
		SetLastError("No material selected.", __FILE__, __LINE__);
		return false;
	}

	// create the core material instance
	CalCoreMaterial coreMaterial;
	// set the ambient color


	// get the map vector of the material candidate
	std::vector<CMaterialCandidate::Map>& vectorMap = pMaterialCandidate->GetVectorMap();

	// load all maps
	for(size_t mapId = 0; mapId < vectorMap.size(); mapId++)
	{
		CalCoreMaterial::Map map;

        // set map data
        map.filename = vectorMap[mapId].strFilename;
        map.type = vectorMap[mapId].mapType;

        // set map in the core material instance
        coreMaterial.maps.push_back(map);
    }

	// save core mesh to the file
	if(!CalSaver::saveCoreMaterial(strFilename, &coreMaterial))
	{
		SetLastError(CalError::getLastErrorText(), __FILE__, __LINE__);
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------//
// Export the mesh to a given file                                            //
//----------------------------------------------------------------------------//

bool CExporter::ExportMesh(const std::string& strFilename)
{
	// check if a valid interface is set
	if(m_pInterface == 0)
	{
		SetLastError("Invalid handle.", __FILE__, __LINE__);
		return false;
	}

	// build a mesh candidate
	CMeshCandidate meshCandidate;

	// build a skeleton candidate
	CSkeletonCandidate skeletonCandidate;

	// show export wizard sheet
	CMeshExportSheet sheet("Cal3D Mesh Export", m_pInterface->GetMainWnd());
	sheet.SetSkeletonCandidate(&skeletonCandidate);
	sheet.SetMeshCandidate(&meshCandidate);
	sheet.SetWizardMode();
	if(sheet.DoModal() != ID_WIZFINISH) return true;

	// create the core mesh instance
	CalCoreMesh coreMesh;

        CalVector zero;
        bool r = meshCandidateToCoreMesh(meshCandidate, coreMesh, zero);
        if( !r ) {
          return false;
        }

        int numMorphs = meshCandidate.numMorphs();
        for( int morphI = 0; morphI < numMorphs; morphI++ ) {
          CMeshCandidate morphCandidate;

          CBaseNode * morphNode = meshCandidate.nthMorphNode(morphI);
          if(!morphCandidate.Create(morphNode, &skeletonCandidate, sheet.GetMaxBoneCount(),
              sheet.GetWeightThreshold()))
          {
            SetLastError("Creation of core mesh instance failed.", __FILE__, __LINE__);
			return false;
          }

          if( !morphCandidate.DisableLOD() ) {
            SetLastError("CalculateLOD failed.", __FILE__, __LINE__);
			return false;
          }
          
          CalCoreMesh morphCoreMesh;

          CalVector trans;
          CalQuaternion rot;
          m_pInterface->GetTranslationAndRotation(morphNode, meshCandidate.getNode(),
            0, trans, rot);
          
          if( !meshCandidateToCoreMesh(morphCandidate, morphCoreMesh, trans) ) {
            SetLastError("Creation of core mesh instance failed.", __FILE__, __LINE__);
			return false;
          }
          if( coreMesh.addAsMorphTarget(&morphCoreMesh, morphNode->GetName()) == -1 ) {
            SetLastError(CalError::getLastErrorText(), __FILE__, __LINE__);
            return false;
          }
            
        }
        
	// save core mesh to the file
	if(!CalSaver::saveCoreMesh(strFilename, &coreMesh))
	{
		SetLastError(CalError::getLastErrorText(), __FILE__, __LINE__);
		return false;
	}

        return true;
}

bool CExporter::meshCandidateToCoreMesh(CMeshCandidate const & meshCandidate, CalCoreMesh & coreMesh,
  CalVector const & positionOffset)
{
	// get the submesh candidate vector
	std::vector<CSubmeshCandidate *> const & vectorSubmeshCandidate = meshCandidate.GetVectorSubmeshCandidate();

	// start the progress info
  CStackProgress progress(m_pInterface, "Exporting to mesh file...");

	size_t submeshCandidateId;
	for(submeshCandidateId = 0; submeshCandidateId < vectorSubmeshCandidate.size(); submeshCandidateId++)
	{
		// update the progress info
		m_pInterface->SetProgressInfo(int(100.0f * (float)submeshCandidateId / (float)vectorSubmeshCandidate.size()));

		// get the submesh candidate
		CSubmeshCandidate *pSubmeshCandidate;
		pSubmeshCandidate = vectorSubmeshCandidate[submeshCandidateId];

		// get the face vector
		std::vector<CSubmeshCandidate::Face>& vectorFace = pSubmeshCandidate->GetVectorFace();

		// check if the submesh actually contains faces
		if(vectorFace.size() > 0)
		{
			// get the vertex candidate vector
			std::vector<CVertexCandidate *>& vectorVertexCandidate = pSubmeshCandidate->GetVectorVertexCandidate();

            // allocate new core submesh instance
            boost::shared_ptr<CalCoreSubmesh> pCoreSubmesh(new CalCoreSubmesh(
                    vectorVertexCandidate.size(),
                    pSubmeshCandidate->GetMapCount(),
                    vectorFace.size()));

			// set the core material id
			pCoreSubmesh->coreMaterialThreadId = pSubmeshCandidate->GetMaterialThreadId();

			size_t vertexCandidateId;
			for(vertexCandidateId = 0; vertexCandidateId < vectorVertexCandidate.size(); vertexCandidateId++)
			{
				// get the vertex candidate
				CVertexCandidate *pVertexCandidate;
				pVertexCandidate = vectorVertexCandidate[vertexCandidateId];

				CalCoreSubmesh::Vertex vertex;
                                CalColor32 vertexColor;

				// set the vertex position
                                CalVector position;
				pVertexCandidate->GetPosition(position);
                                position -= positionOffset;
                                vertex.position.setAsPoint(position);

                                // set the vertex color
                                CalVector vc;
                                pVertexCandidate->GetVertColor(vc);
                                vertexColor = CalMakeColor(vc);

				// set the vertex normal
				CalVector normal;
				pVertexCandidate->GetNormal(normal);
                                vertex.normal.setAsVector(normal);

				// get the texture coordinate vector
				std::vector<CVertexCandidate::TextureCoordinate>& vectorTextureCoordinate = pVertexCandidate->GetVectorTextureCoordinate();

				// set all texture coordinates
				size_t textureCoordinateId;
				for(textureCoordinateId = 0; textureCoordinateId < vectorTextureCoordinate.size(); textureCoordinateId++)
				{
					CalCoreSubmesh::TextureCoordinate textureCoordinate;
					textureCoordinate.u = vectorTextureCoordinate[textureCoordinateId].u;
					textureCoordinate.v = vectorTextureCoordinate[textureCoordinateId].v;

					// set texture coordinate
					pCoreSubmesh->setTextureCoordinate(pVertexCandidate->GetLodId(), textureCoordinateId, textureCoordinate);
				}

				// get the influence vector
				std::vector<CalCoreSubmesh::Influence>& vectorInfluence = pVertexCandidate->GetVectorInfluence();

				// set vertex in the core submesh instance
                                // Chad says that this might work. -- andy 4 June 2009
                                //assert(false); // Not sure exactly how this call should work.
				//pCoreSubmesh->setVertex(pVertexCandidate->GetLodId(), vertex, vertexColor, lodData, vectorInfluence);
                                pCoreSubmesh->addVertex(vertex, vertexColor, vectorInfluence);
			}

			size_t faceId;
			for(faceId = 0; faceId < vectorFace.size(); faceId++)
			{
				CalCoreSubmesh::Face face;

				// set the vertex ids
				face.vertexId[0] = vectorFace[faceId].vertexLodId[0];
				face.vertexId[1] = vectorFace[faceId].vertexLodId[1];
				face.vertexId[2] = vectorFace[faceId].vertexLodId[2];

				// set face in the core submesh instance
				pCoreSubmesh->faces[vectorFace[faceId].lodId] = face;
			}

			coreMesh.submeshes.push_back(pCoreSubmesh);
		}
	}

	return true;
}

//----------------------------------------------------------------------------//
// Export the skeleton to a given file                                        //
//----------------------------------------------------------------------------//

bool CExporter::ExportSkeleton(const std::string& strFilename)
{
	// check if a valid interface is set
	if(m_pInterface == 0)
	{
		SetLastError("Invalid handle.", __FILE__, __LINE__);
		return false;
	}

	// build a skeleton candidate
	CSkeletonCandidate skeletonCandidate;
	if(!skeletonCandidate.CreateFromInterface()) return false;

	// show export wizard sheet
	CSkeletonExportSheet sheet(m_pInterface, "Cal3D Skeleton Export", m_pInterface->GetMainWnd());
	sheet.SetSkeletonCandidate(&skeletonCandidate);
	sheet.SetWizardMode();
	if(sheet.DoModal() != ID_WIZFINISH) return true;

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
	std::vector<CBoneCandidate *>& vectorBoneCandidate = skeletonCandidate.GetVectorBoneCandidate();

  CalVector sceneAmbientColor;
  m_pInterface->GetAmbientLight( sceneAmbientColor );
  coreSkeleton.sceneAmbientColor = ( sceneAmbientColor );
  
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
			// update the progress info
			m_pInterface->SetProgressInfo(int(100.0f * (selectedId + 1) / selectedCount));
			selectedId++;

                        int parentId = skeletonCandidate.GetParentSelectedId(boneCandidateId);
                        CalCoreBonePtr pCoreBone(new CalCoreBone(pBoneCandidate->GetNode()->GetName(), parentId));
                        
			// get the translation and the rotation of the bone candidate
			CalVector translation;
			CalQuaternion rotation;
			skeletonCandidate.GetTranslationAndRotation(boneCandidateId, -1.0f, translation, rotation);

			// set the translation and rotation
			pCoreBone->relativeTransform.translation = translation;
                        pCoreBone->relativeTransform.rotation = rotation;

			// get the bone space translation and the rotation of the bone candidate
			CalVector translationBoneSpace;
			CalQuaternion rotationBoneSpace;
			skeletonCandidate.GetTranslationAndRotationBoneSpace(boneCandidateId, -1.0f, translationBoneSpace, rotationBoneSpace);

			// set the bone space translation and rotation
			pCoreBone->inverseBindPoseTransform.translation = translationBoneSpace;
			pCoreBone->inverseBindPoseTransform.rotation = rotationBoneSpace;

      CBaseNode * pBoneNode = pBoneCandidate->GetNode();
      pCoreBone->lightType = ( pBoneNode->GetLightType() );
      CalVector color;
      pBoneNode->GetLightColor( color );
      pCoreBone->lightColor = ( color );
                        
			// add the core bone to the core skeleton instance
			coreSkeleton.addCoreBone(pCoreBone);
		}
	}

    // save core skeleton to the file
    if(!CalSaver::saveCoreSkeleton(strFilename, &coreSkeleton))
    {
        return false;
    }

	HKEY hk;
	LONG lret=RegCreateKey(HKEY_CURRENT_USER, "Software\\Cal3D\\Exporter", &hk);
	if(lret==ERROR_SUCCESS && NULL!=hk)
	{
		lret=RegSetValueEx(hk,"skeleton",NULL,REG_SZ,(unsigned char *)strFilename.c_str() ,strFilename.length());
		RegCloseKey(hk);
	}


	return true;
}

//----------------------------------------------------------------------------//
// Get the last error message                                                 //
//----------------------------------------------------------------------------//

CBaseInterface *CExporter::GetInterface()
{
	return m_pInterface;
}

//----------------------------------------------------------------------------//
// Get the last error message                                                 //
//----------------------------------------------------------------------------//

const std::string& CExporter::GetLastError()
{
	return m_strLastError;
}

//----------------------------------------------------------------------------//
// Set the last error message                                                 //
//----------------------------------------------------------------------------//

void CExporter::SetLastError(const std::string& strText, const std::string& strFilename, int line)
{
	std::stringstream strstrError;
	strstrError << strText << "\n(" << strFilename << " " << line << ")" << std::ends;

	m_strLastError = strstrError.str();
  std::string s(m_strLastError);
  ::OutputDebugString(s.c_str());
}

//----------------------------------------------------------------------------//
// Set the last error message from the cal3d library                          //
//----------------------------------------------------------------------------//

void CExporter::SetLastErrorFromCal(const std::string& strFilename, int line)
{
	std::stringstream strstrError;
	strstrError << "cal3d : " << CalError::getLastErrorDescription();

	if(!CalError::getLastErrorTextInternal().empty())
	{
		strstrError << " '" << CalError::getLastErrorText() << "'";
	}

	strstrError << " in " << CalError::getLastErrorFile()
				<< "(" << CalError::getLastErrorLine() << ")";

	strstrError << "\n(" << strFilename << " " << line << ")" << std::ends;

	m_strLastError = strstrError.str();
  std::string s(m_strLastError);
  ::OutputDebugString(s.c_str());
}

//----------------------------------------------------------------------------//
