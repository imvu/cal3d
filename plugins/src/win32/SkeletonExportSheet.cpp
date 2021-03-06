//----------------------------------------------------------------------------//
// SkeletonExportSheet.cpp                                                    //
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
#include "SkeletonExportSheet.h"

//----------------------------------------------------------------------------//
// Message mapping                                                            //
//----------------------------------------------------------------------------//

BEGIN_MESSAGE_MAP(CSkeletonExportSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CSkeletonExportSheet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//----------------------------------------------------------------------------//
// Constructors                                                               //
//----------------------------------------------------------------------------//

CSkeletonExportSheet::CSkeletonExportSheet(CBaseInterface* iface, UINT nIDCaption, CWnd *pParentWnd, UINT iSelectPage)
: CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
, m_skeletonHierarchyPage(iface)
{
	FillPages();
}

CSkeletonExportSheet::CSkeletonExportSheet(CBaseInterface* iface, LPCTSTR pszCaption, CWnd *pParentWnd, UINT iSelectPage)
: CPropertySheet(pszCaption, pParentWnd, iSelectPage)
, m_skeletonHierarchyPage(iface)
{
	FillPages();
}

//----------------------------------------------------------------------------//
// Destructor                                                                 //
//----------------------------------------------------------------------------//

CSkeletonExportSheet::~CSkeletonExportSheet()
{
}

//----------------------------------------------------------------------------//
// Fill in all propoerty pages                                                //
//----------------------------------------------------------------------------//

void CSkeletonExportSheet::FillPages()
{
	// add all property pages to this sheet
	m_skeletonHierarchyPage.SetStep(1, 1);
	m_skeletonHierarchyPage.SetDescription(IDS_SKELETON_EXPORT_DESCRIPTION);
	AddPage(&m_skeletonHierarchyPage);
}

//----------------------------------------------------------------------------//
// Set the node hierarchy                                                     //
//----------------------------------------------------------------------------//

void CSkeletonExportSheet::SetSkeletonCandidate(CSkeletonCandidate *pSkeletonCandidate)
{
	m_skeletonHierarchyPage.SetSkeletonCandidate(pSkeletonCandidate);
}

//----------------------------------------------------------------------------//
