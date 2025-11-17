// ----------------------------------------------------------------------- //
//
// MODULE  : CursorMgr.cpp
//
// PURPOSE : Manage all mouse cursor related functionality
//
// CREATED : 12/3/01
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "vartrack.h"
#include "interfacemgr.h"
#include "cursormgr.h"
#include "clientresshared.h"

VarTrack	g_vtCursorHack;

CCursorMgr * g_pCursorMgr = LTNULL;
constexpr float CURSOR_W = 32.0f;
constexpr float CURSOR_H = 32.0f;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr constructor and destructor
//
//	PURPOSE:	Set initial values on ctor, force a Term() on dtor
//
// ----------------------------------------------------------------------- //

CCursorMgr::CCursorMgr()
{
	g_pCursorMgr = this;

    m_bUseCursor			= LTFALSE;
    m_bUseHardwareCursor	= LTFALSE;
	m_bInitialized			= LTFALSE;
}

CCursorMgr::~CCursorMgr()
{
	Term();

	g_pCursorMgr = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr::Init
//
//	PURPOSE:	Init the cursor
//
// ----------------------------------------------------------------------- //

LTBOOL CCursorMgr::Init()
{
	if (m_bInitialized)
		return LTTRUE;

	m_hTexCursor = g_pInterfaceResMgr->GetTexture("SA_INTERFACE/CURSOR/CURSOR.dtx");
	g_pDrawPrim->SetRGBA(&m_PolyCursor, 0xFFFFFFFF);
	g_pDrawPrim->SetUVWH(&m_PolyCursor, 0.0f, 0.0f, 1.0f, 1.0f);

	m_bInitialized = LTTRUE;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr::Term
//
//	PURPOSE:	Free cursor resources
//
// ----------------------------------------------------------------------- //

void CCursorMgr::Term()
{
	if (!m_bInitialized)
		return;

	m_bInitialized = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr::ScheduleReinit(float fHack)
//
//	PURPOSE:	Set up a delayed initialization
//
// ----------------------------------------------------------------------- //

void CCursorMgr::ScheduleReinit(float fDelay)
{
	g_vtCursorHack.SetFloat(fDelay);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr::CheckForReinit
//
//	PURPOSE:	Update any hack variables (reducing frame delay counter)
//
// ----------------------------------------------------------------------- //

void CCursorMgr::CheckForReinit()
{
	// because of driver bugs, we need to wait a frame after reinitializing the renderer and
	// reinitialize the cursor
	int nCursorHackFrameDelay = (int)g_vtCursorHack.GetFloat();
	if (nCursorHackFrameDelay)
	{
		nCursorHackFrameDelay--;
		g_vtCursorHack.SetFloat((LTFLOAT)nCursorHackFrameDelay);
		if (nCursorHackFrameDelay == 1)
			Init();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr::UseCursor
//
//	PURPOSE:	Handle activation and deactivation of visible cursor
//
// ----------------------------------------------------------------------- //

void CCursorMgr::UseCursor(LTBOOL bUseCursor, LTBOOL bLockCursorToCenter)
{
	m_bUseCursor = bUseCursor;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr::UseHardwareCursor
//
//	PURPOSE:	(De)activate the Windows cursor drawing routines
//
// ----------------------------------------------------------------------- //

void CCursorMgr::UseHardwareCursor(LTBOOL bUseHardwareCursor,bool bForce)
{
	m_bUseHardwareCursor = bUseHardwareCursor;

	if (m_bUseHardwareCursor && m_bUseCursor)
	{
		g_pLTClient->Cursor()->SetCursorMode(CM_Hardware,bForce);
	}
	else
	{
		g_pLTClient->Cursor()->SetCursorMode(CM_None,bForce);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr::Update
//
//	PURPOSE:	Display a cursor bitmap, if required, or update the sprite coords
//
// ----------------------------------------------------------------------- //

void CCursorMgr::Update()
{
	if (!m_bUseCursor)
		return;

	LTIntPt CursorPos = g_pInterfaceMgr->GetCursorPos();

	// If a software cursor is needed but none has been specified, use the default
	g_pLTClient->Start3D();
	g_pLTClient->StartOptimized2D();

	g_pDrawPrim->SetTexture(m_hTexCursor);
	g_pDrawPrim->SetXYWH(&m_PolyCursor, (LTFLOAT)CursorPos.x, (LTFLOAT)CursorPos.y, CURSOR_W, CURSOR_H);
	g_pDrawPrim->DrawPrim(&m_PolyCursor);

	g_pLTClient->EndOptimized2D();
	g_pLTClient->End3D(END3D_CANDRAWCONSOLE);
}