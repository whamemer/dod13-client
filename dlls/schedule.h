//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#pragma once
#if !defined( SCHEDULE_H )
#define SCHEDULE_H

struct Task_t
{
    int iTask;
    float flData;
};

struct Schedule_t
{
    struct Task_t *pTasklist;
    int cTasks;
    int iInterruptMask;
    int iSoundMask;
    const char *pName;
};

struct WayPoint_t
{
    Vector vecLocation;
    int iType;
};
#endif // !SCHEDULE_H