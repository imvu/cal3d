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
// MaxMeshExport.cpp                                                          //
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
#include <cal3d/coremesh.h>
#include <cal3d/error.h>
#include <cal3d/saver.h>
#include "Exporter.h"
#include "BaseInterface.h"
#include "SkeletonCandidate.h"
#include "BoneCandidate.h"
#include "BaseNode.h"
#include "SkeletonExportSheet.h"
#include "AnimationExportSheet.h"
#include "MaxMeshExport.h"
#include "MaxInterface.h"
#include "MeshExportSheet.h"
#include "MaterialExportSheet.h"
#include "MeshCandidate.h"
#include "SubmeshCandidate.h"
#include "VertexCandidate.h"
#include "MaterialLibraryCandidate.h"
#include "MaterialCandidate.h"
#include "BaseNode.h"
#include "BaseMesh.h"
#include "MaxNode.h"


bool CExporter::ExportMeshFromMaxscriptCall(const std::string& strFilename, void * MeshExportParams)
{
	if (! MeshExportParams)return false;

	const MeshMaxscriptExportParams* _param = reinterpret_cast<const MeshMaxscriptExportParams*>(MeshExportParams);

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

	// build a mesh candidate
	CMeshCandidate meshCandidate;

	// build a skeleton candidate
	CSkeletonCandidate skeletonCandidate;

	//Remove user interface
	/*// show export wizard sheet
	CMeshExportSheet sheet("Cal3D Mesh Export", m_pInterface->GetMainWnd());
	sheet.SetSkeletonCandidate(&skeletonCandidate);
	sheet.SetMeshCandidate(&meshCandidate);
	sheet.SetWizardMode();
	if(sheet.DoModal() != ID_WIZFINISH) return true;
	*/

	//This block replaces the user interface interactions with the parameters passed to the Maxscript function
	{
		// create the skeleton candidate from the skeleton file
		if(! skeletonCandidate.CreateFromSkeletonFile(_param->m_SkeletonFilename))
		{
			AfxMessageBox(theExporter.GetLastError().c_str(), MB_OK | MB_ICONEXCLAMATION);
			return false;
		}

		// Create the mesh candidate with maxnumbonespervertex and weightthreshold from maxscript params
		//First, create a CMaxNode for the Mesh node
		CMaxNode* maxnode;
		maxnode = new CMaxNode;
		if (! maxnode->Create(_param->m_MeshNode)) return false;

		if(! meshCandidate.Create( maxnode, &skeletonCandidate, _param->m_MaxNumBonesPerVertex, _param->m_WeightThreshold))
		{
			AfxMessageBox(theExporter.GetLastError().c_str(), MB_OK | MB_ICONEXCLAMATION);
			return false;
		}

		// check if LOD needs to be applied
		if(_param->m_LODCreation)
		{
			// create the LOD data
			if(! meshCandidate.CalculateLOD())
			{
				AfxMessageBox(theExporter.GetLastError().c_str(), MB_OK | MB_ICONEXCLAMATION);
				return false;
			}
		}
		else
		{
			// disable the LOD data
			if(! meshCandidate.DisableLOD())
			{
				AfxMessageBox(theExporter.GetLastError().c_str(), MB_OK | MB_ICONEXCLAMATION);
				return false;
			}
		}

		//Do we need to use the springs system ?
		if(_param->m_springsystem)
		{
			// create the spring system data
			if(! meshCandidate.CalculateSpringSystem())
			{
				AfxMessageBox(theExporter.GetLastError().c_str(), MB_OK | MB_ICONEXCLAMATION);
				return false;
			}
		}

	}

	// create the core mesh instance
	CalCoreMesh coreMesh;

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
			pCoreSubmesh->setCoreMaterialThreadId(pSubmeshCandidate->GetMaterialThreadId());

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
                                vertex.position.setAsPoint(position);

				// set the vertex normal
				CalVector normal;
				pVertexCandidate->GetNormal(normal);
                                vertex.normal.setAsVector(normal);

                                // set the vert color
                                CalVector vc;
                                pVertexCandidate->GetVertColor(vc);
                                vertexColor = CalMakeColor(vc);

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
                                assert(false); // Not sure exactly how this call should work.
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
				pCoreSubmesh->setFace(vectorFace[faceId].lodId, face);
			}

			coreMesh.addCoreSubmesh(pCoreSubmesh);
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

