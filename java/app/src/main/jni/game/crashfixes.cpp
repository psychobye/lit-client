#include "../main.h"

#include "RW/RenderWare.h"
#include "game.h"
#include "util.h"

#include "../util/armhook.h"
#include "../util/patch.h"

int (*SetCompAlphaCB)(int a1, char a2);
int SetCompAlphaCB_hook(int a1, char a2)
{
	if (!a1) return 0;
	return SetCompAlphaCB(a1, a2);
}

uintptr_t(*CPlayerPedDataSaveStructure__Construct)(int a1, int a2);
uintptr_t CPlayerPedDataSaveStructure__Construct_hook(int a1, int a2)
{
	if (!a1 || !a2) return 0;
	if (!*(int*)a2) return 0;
	return CPlayerPedDataSaveStructure__Construct(a1, a2);
}

int (*CTextureDatabaseRuntime__GetEntry)(uintptr_t thiz, const char* a2, bool* a3);
int CTextureDatabaseRuntime__GetEntry_hook(uintptr_t thiz, const char* a2, bool* a3)
{
	if (!thiz)
	{
		return -1;
	}
	return CTextureDatabaseRuntime__GetEntry(thiz, a2, a3);
}

int (*emu_ArraysGetID)(unsigned int a1);
int emu_ArraysGetID_hook(unsigned int a1)
{
	if(!a1 || !*(uint32_t*)(a1+36)) return 0;
	return emu_ArraysGetID(a1);
}

int (*_RwResourcesFreeResEntry)(RwResEntry*);
int HOOK__RwResourcesFreeResEntry(RwResEntry* entry)
{
	if(!entry) return 0;
	return _RwResourcesFreeResEntry(entry);
}

int (*_RpMaterialDestroy)(uintptr_t);
int HOOK__RpMaterialDestroy(uintptr_t mat)
{
	if(mat) return _RpMaterialDestroy(mat);
	return 1;
}

#pragma pack(1)
typedef struct _RES_ENTRY_OBJ1
{
	PADDING(_pad0, 48); 	// 0-48
	uintptr_t validate; 	//48-52
	PADDING(_pad1, 4); 		//52-56
} RES_ENTRY_OBJ1;
static_assert(sizeof(_RES_ENTRY_OBJ1) == 56);
int (*CustomPipeRenderCB)(uintptr_t resEntry, uintptr_t object, uint8_t type, uint32_t flags);
int CustomPipeRenderCB_hook(uintptr_t resEntry, uintptr_t object, uint8_t type, uint32_t flags)
{
	if(!resEntry) return 0;
    uint16_t size = *(uint16_t *)(resEntry + 26);
    if(size)
    {
        RES_ENTRY_OBJ1* arr = (RES_ENTRY_OBJ1*)(resEntry + 28);
		if(!arr) return 0;
        uint32_t validFlag = flags & 0x84;
        for(int i = 0; i < size; i++)
        {
            if(!arr[i].validate) break;
            if(validFlag)
            {
                uintptr_t* v4 = *(uintptr_t **)(arr[i].validate);
				if(!v4);
				else
				{
					if((uintptr_t)v4 > (uintptr_t)0xFFFFFF00) return 0;
                	else
					{	
                    	if(!*(uintptr_t **)v4) return 0;
					}
				}
                
            }
        }
    }
    return CustomPipeRenderCB(resEntry, object, type, flags);
}

int (*rxOpenGLDefaultAllInOneRenderCB)(uintptr_t resEntry, uintptr_t object, uint8_t type, uint32_t flags);
int rxOpenGLDefaultAllInOneRenderCB_hook(uintptr_t resEntry, uintptr_t object, uint8_t type, uint32_t flags)
{
    if(!resEntry) return 0;
    uint16_t size = *(uint16_t *)(resEntry + 26);
    if(size)
    {
        RES_ENTRY_OBJ1* arr = (RES_ENTRY_OBJ1*)(resEntry + 28);
		if(!arr) return 0;
        uint32_t validFlag = flags & 0x84;
        for(int i = 0; i < size; i++)
        {
            if(!arr[i].validate) break;
            if(validFlag)
            {
                uintptr_t* v4 = *(uintptr_t **)(arr[i].validate);
				if(!v4);
				else
				{
					if((uintptr_t)v4 > (uintptr_t)0xFFFFFF00) return 0;
                	else
					{	
                    	if(!*(uintptr_t **)v4) return 0;
					}
				}
                
            }
        }
    }
    return rxOpenGLDefaultAllInOneRenderCB(resEntry, object, type, flags);
}

int (*CAudioEngine__Service)(uintptr_t a1);
int CAudioEngine__Service_hook(uintptr_t a1)
{
	if(!a1) return 0;
	return CAudioEngine__Service(a1);
}

int (*CAEVehicleAudioEntity__GetAccelAndBrake)(int a1, int a2);
int CAEVehicleAudioEntity__GetAccelAndBrake_hook(int a1, int a2)
{
	if(!a1 || !a2) return 0;
	if(!*(uintptr_t *)(a1 + 12)) return 0;
	return CAEVehicleAudioEntity__GetAccelAndBrake(a1, a2);
}

void InstallHookFixes()
{
    CHook::InlineHook(g_libGTASA, 0x1BA580, &HOOK__RwResourcesFreeResEntry, &_RwResourcesFreeResEntry);
    //CHook::InlineHook(g_libGTASA, 0x1E3C54, &HOOK__RpMaterialDestroy, &_RpMaterialDestroy);
    //CHook::InlineHook(g_libGTASA, 0x28AAAC, &CustomPipeRenderCB_hook, &CustomPipeRenderCB);
    CHook::InlineHook(g_libGTASA, 0x1EEC90, &rxOpenGLDefaultAllInOneRenderCB_hook, &rxOpenGLDefaultAllInOneRenderCB);
    //CHook::InlineHook(g_libGTASA, 0x194B20, &emu_ArraysGetID_hook, &emu_ArraysGetID);
    //CHook::InlineHook(g_libGTASA, 0x50C5F8, &SetCompAlphaCB_hook, &SetCompAlphaCB);
    //CHook::InlineHook(g_libGTASA, 0x41EAB4, &CPlayerPedDataSaveStructure__Construct_hook, &CPlayerPedDataSaveStructure__Construct);
    CHook::InlineHook(g_libGTASA, 0x1BDC3C, &CTextureDatabaseRuntime__GetEntry_hook, &CTextureDatabaseRuntime__GetEntry);
    //CHook::InlineHook(g_libGTASA, 0x368850, &CAudioEngine__Service_hook, &CAudioEngine__Service);
    //installHook(SA_ADDR(0x35AC44), (uintptr_t)CAEVehicleAudioEntity__GetAccelAndBrake_hook, (uintptr_t*)&CAEVehicleAudioEntity__GetAccelAndBrake);
}