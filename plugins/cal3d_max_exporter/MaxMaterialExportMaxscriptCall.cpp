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
#include <cal3d/error.h>
#include <cal3d/saver.h>
#include <cal3d/corematerial.h>
#include "Exporter.h"
#include "BaseInterface.h"
#include "SkeletonCandidate.h"
#include "BoneCandidate.h"
#include "BaseNode.h"
#include "SkeletonExportSheet.h"
#include "AnimationExportSheet.h"
#include "MeshExportSheet.h"
#include "MaterialExportSheet.h"
#include "MeshCandidate.h"
#include "SubmeshCandidate.h"
#include "VertexCandidate.h"
#include "MaterialLibraryCandidate.h"
#include "MaterialCandidate.h"
#include "Maxinterface.h"

bool CExporter::ExportMaterialFromMaxscriptCall	(const std::string& strFilename)
{
	// check if a valid interface is set
	if(m_pInterface == 0)
	{
		SetLastError("Invalid interface pointer.", __FILE__, __LINE__);
		return false;
	}

	// build a material library candidate
	CMaterialLibraryCandidate materialLibraryCandidate;
	if(! materialLibraryCandidate.CreateFromInterfaceFromMaxscriptCall()) return false;

	//Remove user interface
	/*
	// show export wizard sheet
	CMaterialExportSheet sheet("Cal3D Material Export", m_pInterface->GetMainWnd());
	sheet.SetMaterialLibraryCandidate(&materialLibraryCandidate);
	sheet.SetWizardMode();
	if(sheet.DoModal() != ID_WIZFINISH) return true;
	*/
	//The material has already been selected in the function materialLibraryCandidate.CreateFromInterfaceFromMaxscriptCall()

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
	CalCoreMaterial::Color coreColor;
	float color[4];
	pMaterialCandidate->GetAmbientColor(&color[0]);
	coreColor.red = (unsigned char)(255.0f * color[0]);
	coreColor.green = (unsigned char)(255.0f * color[1]);
	coreColor.blue = (unsigned char)(255.0f * color[2]);
	coreColor.alpha = (unsigned char)(255.0f * color[3]);
	coreMaterial.ambientColor = coreColor;

	// set the diffuse color
	pMaterialCandidate->GetDiffuseColor(&color[0]);
	coreColor.red = (unsigned char)(255.0f * color[0]);
	coreColor.green = (unsigned char)(255.0f * color[1]);
	coreColor.blue = (unsigned char)(255.0f * color[2]);
	coreColor.alpha = (unsigned char)(255.0f * color[3]);
	coreMaterial.diffuseColor = coreColor;

	// set the specular color
	pMaterialCandidate->GetSpecularColor(&color[0]);
	coreColor.red = (unsigned char)(255.0f * color[0]);
	coreColor.green = (unsigned char)(255.0f * color[1]);
	coreColor.blue = (unsigned char)(255.0f * color[2]);
	coreColor.alpha = (unsigned char)(255.0f * color[3]);
	coreMaterial.specularColor = coreColor;

	// set the shininess factor
	coreMaterial.shininess = pMaterialCandidate->GetShininess();

	// get the map vector of the material candidate
	std::vector<CMaterialCandidate::Map>& vectorMap = pMaterialCandidate->GetVectorMap();

	for(size_t mapId = 0; mapId < vectorMap.size(); mapId++)
	{
		CalCoreMaterial::Map map;
		map.filename = vectorMap[mapId].strFilename;
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
bool CMaterialLibraryCandidate::CreateFromInterfaceFromMaxscriptCall()
{
	// clear current content
	Clear();

	// get the number of materials of the mesh
	CMaxInterface* imax = static_cast<CMaxInterface*>(theExporter.GetInterface());
	
	// allocate a new material candidate
	CMaterialCandidate *pMaterialCandidate;
	pMaterialCandidate = new CMaterialCandidate();
	if(pMaterialCandidate == 0)
	{
		theExporter.SetLastError("Memory allocation failed!", __FILE__, __LINE__);
		return false;
	}

	// create the new material candidate
	if(!pMaterialCandidate->Create(imax->GetBaseMatFromMaxscript())) return false;

	// add material candidate to the material candidate vector
	m_vectorMaterialCandidate.push_back(pMaterialCandidate);

	//Select it
	SetSelectedMaterialCandidate(pMaterialCandidate);

	return true;
}
