#include "../main.h"
#include "../util/armhook.h"
#include "RW/RenderWare.h"
#include "game.h"
#include "../net/netgame.h"
#include "../gui/gui.h"
#include "../util/CJavaWrapper.h"
#include "../CAudioStream.h"
#include "..///..//santrope-tea-gtasa/encryption/CTinyEncrypt.h"
#include "..///..//santrope-tea-gtasa/encryption/encrypt.h"

#include "../gtasa/Skybox.h"

#include "..//voice/CVoiceChatClient.h"
#include "..//chatwindow.h"
extern CGame* pGame;
#include "..//CSettings.h"
#include "../gtasa/ImageLoader.h"
extern CSettings* pSettings;
extern CChatWindow* pChatWindow;
extern CVoiceChatClient* pVoice;
extern CAudioStream* 	pAudioStream;

extern "C"
{
#include "..//..//santrope-tea-gtasa/encryption/aes.h"
}

char(*CStreaming__ConvertBufferToObject)(int, int, int);
int __attribute__((noinline)) g_unobfuscate(int a)
{
    return UNOBFUSCATE_DATA(a);
}
#include "..//str_obfuscator_no_template.hpp"
#define MAX_ENCRYPTED_TXD 2
const cryptor::string_encryptor encrArch[MAX_ENCRYPTED_TXD] = { cryptor::create("texdb/samp/samp.txt", 21), cryptor::create("texdb/gui/gui.txt", 19) };
static int lastOpenedFile = 0;

extern CNetGame *pNetGame;
extern CGUI *pGUI;
// Neiae/SAMP
bool g_bPlaySAMP = false;

void InitInMenu();
void MainLoop();
void HookCPad();

/* ================ ie?oee aey ani. anoaaie =================== */

extern "C" uintptr_t get_lib()
{
    return g_libGTASA;
}

/* ====================================================== */

struct stFile
{
    int isFileExist;
    FILE *f;
};

stFile* (*NvFOpen)(const char*, const char*, int, int);
stFile* NvFOpen_hook(const char* r0, const char* r1, int r2, int r3)
{
    char path[0xFF] = { 0 };
    if(!strncmp(r1+12, "mainV1.scm", 10))
    {
        sprintf(path, "%smultiplayer/main.scm", g_pszStorage);
        goto open;
    }
    if(!strncmp(r1+12, "SCRIPTV1.IMG", 12))
    {
        sprintf(path, "%smultiplayer/script.img", g_pszStorage);
        goto open;
    }
    if(!strncmp(r1, "DATA/PEDS.IDE", 13))
    {
        sprintf(path, "%s/multiplayer/peds.ide", g_pszStorage);
        goto open;
    }
    if(!strncmp(r1, "DATA/VEHICLES.IDE", 17))
    {
        sprintf(path, "%s/multiplayer/vehicles.ide", g_pszStorage);
        goto open;
    }
    if (!strncmp(r1, "DATA/GTA.DAT", 12))
    {
        sprintf(path, "%s/multiplayer/gta.dat", g_pszStorage);
        goto open;
    }
    if (!strncmp(r1, "MODELS/COLL/WEAPONS.COL", 23))
    {
        sprintf(path, "%s/utils/coll/weapons.col", g_pszStorage);
        goto open;
    }
    Log("%s", r1); // логгируем загрузку файлов

    orig:
    return NvFOpen(r0, r1, r2, r3);

    open:
    stFile *st = (stFile*)malloc(8);
    st->isFileExist = false;

    FILE *f  = fopen(path, "rb");
    if(f)
    {
        st->isFileExist = true;
        st->f = f;
        return st;
    }
    else
    {
        free(st);
        st = nullptr;
        return 0;
    }
}

void ShowHud()
{
    if(pGame)
    {
        if(pNetGame)
        {
            if(pGame->FindPlayerPed() || GamePool_FindPlayerPed())
            {
                CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
                if(pPlayerPool)
                {
                    g_pJavaWrapper->UpdateHudInfo(
                            pGame->FindPlayerPed()->GetHealth(),
                            pGame->FindPlayerPed()->GetArmour(),
                            pGUI->GetEat(),
                            GamePool_FindPlayerPed()->WeaponSlots[GamePool_FindPlayerPed()->byteCurWeaponSlot].dwType,
                            GamePool_FindPlayerPed()->WeaponSlots[GamePool_FindPlayerPed()->byteCurWeaponSlot].dwAmmo,
                            (int)pPlayerPool->GetLocalPlayerID(),
                            pGame->GetLocalMoney(), pGame->GetWantedLevel()
                    );
                }
                if(pSettings && pSettings->GetReadOnly().iHud)
                {
                    *(uint8_t*)(g_libGTASA+0x7165E8) = 0;
                }
                else if(pSettings && !pSettings->GetReadOnly().iHud)
                {
                    *(uint8_t*)(g_libGTASA+0x7165E8) = 1;
                }
            }
        }
    }
}

bool bGameStarted = false;
uint32_t bProcessedRender2dstuff = 0;
void (*Render2dStuff)();
void Render2dStuff_hook()
{
    bGameStarted = true;
    MAKE_PROFILE(test, test_time);
    MainLoop();
    LOG_PROFILE(test, test_time);
    bProcessedRender2dstuff = GetTickCount();
    Render2dStuff();
    ShowHud();

    // ------------- graphics -------------

    /*CGraphic* graphic;
    std::string filePath = "/storage/emulated/0/DagStyle/multiplayer/graphics.ini";
    std::string table = "graphics";

    // resolution
    std::string valueNameResolution = "resolution";
    int resolutionValue = ReadGraphicValue(filePath.c_str(), table.c_str(), valueNameResolution.c_str());
    graphic->ApplyResolution(resolutionValue);
    // end resolution

    // effects
    std::string valueNameEffects = "effects";
    int effectsValue = ReadGraphicValue(filePath.c_str(), table.c_str(), valueNameEffects.c_str());
    graphic->ApplyEffects(effectsValue);
    // end effects

    // reflections
    std::string valueNameReflections = "reflections";
    int reflectionValue = ReadGraphicValue(filePath.c_str(), table.c_str(), valueNameReflections.c_str());
    graphic->ApplyReflections(reflectionValue);
    // end reflections*/

    // ------------- end graphics -------------

    if (pNetGame)
    {
        CLocalPlayer* pPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
        if (pPlayer)
        {
            if (pPlayer->GetPlayerPed())
            {
                pNetGame->GetTextDrawPool()->Draw();
            }
        }
    }
    //delete graphic;
    return;
}

void __fillArray()
{
}

void InitCTX(AES_ctx&, unsigned char const*)
{

}

void (*Render2dStuffAfterFade)();
void Render2dStuffAfterFade_hook()
{
    Render2dStuffAfterFade();
    if (pGUI && bGameStarted) pGUI->Render();
    return;
}

uint16_t gxt_string[0x7F];
uint16_t* (*CText_Get)(uintptr_t thiz, const char* text);
uint16_t* CText_Get_hook(uintptr_t thiz, const char* text)
{
    if(text[0] == 'S' && text[1] == 'A' && text[2] == 'M' && text[3] == 'P')
    {
        const char* code = &text[5];
        if(!strcmp(code, "MP")) CFont::AsciiToGxtChar("MultiPlayer", gxt_string);

        return gxt_string;
    }

    return CText_Get(thiz, text);
}

void MainMenu_OnStartSAMP()
{
    if(g_bPlaySAMP) return;

    InitInMenu();

    // StartGameScreen::OnNewGameCheck()
    (( void (*)())(g_libGTASA+0x261C8C+1))();

    g_bPlaySAMP = true;
    return;
}

void (*MenuItem_add)(int r0, uintptr_t r1);
void MenuItem_add_hook(int r0, uintptr_t r1)
{
    static bool bMenuInited = false;
    char* name = *(char**)(r1+4);

    if(!strcmp(name, "FEP_STG") && !bMenuInited)
    {
        // "New Game"
        MenuItem_add(r0, r1);

        // "MultiPlayer"
        *(char**)(r1+4) = "SAMP_MP";
        *(uintptr_t*)r1 = LoadTextureFromDB("samp", "menu_mainmp");
        *(uintptr_t*)(r1+8) = (uintptr_t)MainMenu_OnStartSAMP;

        bMenuInited = true;
        goto ret;
    }

    if(g_bPlaySAMP && (
            !strcmp(name, "FEP_STG") ||
            !strcmp(name, "FEH_STA") ||
            !strcmp(name, "FEH_BRI") ))
        return;

    ret:
    return MenuItem_add(r0, r1);
}

void RenderSplashScreen();
void (*CLoadingScreen_DisplayPCScreen)();
void CLoadingScreen_DisplayPCScreen_hook()
{
    RwCamera* camera = *(RwCamera**)(g_libGTASA+0x95B064);

    if(RwCameraBeginUpdate(camera))
    {
        DefinedState2d();
        (( void (*)())(g_libGTASA+0x5519C8+1))(); // CSprite2d::InitPerFrame()
        RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);
        (( void (*)(bool))(g_libGTASA+0x198010+1))(false); // emu_GammaSet()
        RenderSplashScreen();
        //RwCameraEndUpdate(camera);
        RwCameraShowRaster(camera, 0, 0);
    }

    return;
}
int bBlockCWidgetRegionLookUpdate = 0;

/* ====================================================== */
#include "..//keyboard.h"
#include "../obfuscate/str_obfuscate.hpp"
#include "../gtasa/TextureLoader.h"
#include "../gtasa/ImageLoader.h"

extern CKeyBoard* pKeyBoard;
void (*TouchEvent)(int, int, int posX, int posY);
void TouchEvent_hook(int type, int num, int posX, int posY)
{
    if (*(uint8_t*)(g_libGTASA + 0x008C9BA3) == 1)
    {
        return TouchEvent(type, num, posX, posY);
    }

    if (g_pWidgetManager)
    {
        g_pWidgetManager->OnTouchEventSingle(WIDGET_CHATHISTORY_UP, type, num, posX, posY);
        g_pWidgetManager->OnTouchEventSingle(WIDGET_CHATHISTORY_DOWN, type, num, posX, posY);
    }
    bool bRet = pGUI->OnTouchEvent(type, num, posX, posY);
    if (!bRet)
    {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();

    if (pKeyBoard && pKeyBoard->IsOpen())
    {
        if (posX >= io.DisplaySize.x - ImGui::GetFontSize() * 4 && posY >= io.DisplaySize.y / 2 - (ImGui::GetFontSize() * 2.5) * 3 && posY <= io.DisplaySize.y / 2) // keys
        {
            return;
        }
    }

    if (g_pWidgetManager)
    {
        g_pWidgetManager->OnTouchEvent(type, num, posX, posY);
        if (g_pWidgetManager->GetWidget(WIDGET_MICROPHONE))
        {
            if (g_pWidgetManager->GetWidget(WIDGET_MICROPHONE)->GetState() == 1)
            {
                bBlockCWidgetRegionLookUpdate = 1;
            }
            if (g_pWidgetManager->GetWidget(WIDGET_MICROPHONE)->GetState() == 2)
            {
                bBlockCWidgetRegionLookUpdate = 0;
            }
        }
    }
    if(bRet)
        return TouchEvent(type, num, posX, posY);
}

void (*InitialiseRenderWare)();
void InitialiseRenderWare_hook()
{
    Log(OBFUSCATE("Loading texdb.."));

    InitialiseRenderWare();
    TextureLoader::Load("samp", false, TextureLoader::TextureDatabaseFormat::DF_DXT);

    //TextureDatabase* assets = TextureLoader::Load("assets", false, TextureLoader::TextureDatabaseFormat::DF_DXT);
	//TextureLoader::Register(assets);
    return;
}

void (*CStreaming_InitImageList)();
void CStreaming_InitImageList_hook()
{
    char* ms_files = (char*)(g_libGTASA+0x6702FC);
    ms_files[0] = 0;
    *(uint32_t*)&ms_files[44] = 0;
    ms_files[48] = 0;
    *(uint32_t*)&ms_files[92] = 0;
    ms_files[96] = 0;
    *(uint32_t*)&ms_files[140] = 0;
    ms_files[144] = 0;
    *(uint32_t*)&ms_files[188] = 0;
    ms_files[192] = 0;
    *(uint32_t*)&ms_files[236] = 0;
    ms_files[240] = 0;
    *(uint32_t*)&ms_files[284] = 0;
    ms_files[288] = 0;
    *(uint32_t*)&ms_files[332] = 0;
    ms_files[336] = 0;
    *(uint32_t*)&ms_files[380] = 0;

    Log(OBFUSCATE("Loading images..."));

    ImageLoader::AddImageToList(OBFUSCATE("TEXDB\\GTA3.MTK"), true);
    ImageLoader::AddImageToList(OBFUSCATE("TEXDB\\GTA_INT.IMG"), true);
    ImageLoader::AddImageToList(OBFUSCATE("TEXDB\\SAMP.IMG"), true);
    ImageLoader::AddImageToList(OBFUSCATE("TEXDB\\SAMPCOL.IMG"), true);

    // тут не идет загрузка, ОНА В ImageLoader.cpp
}

typedef struct _PED_MODEL
{
    uintptr_t 	vtable;
    uint8_t		data[88];
} PED_MODEL; // SIZE = 92

PED_MODEL PedsModels[370];
int PedsModelsCount = 0;

PED_MODEL* (*CModelInfo_AddPedModel)(int id);
PED_MODEL* CModelInfo_AddPedModel_hook(int id)
{
    PED_MODEL* model = &PedsModels[PedsModelsCount];
    memset(model, 0, sizeof(PED_MODEL));

    ((void(*)(void* thiz))(g_libGTASA + 0x0033559C + 1))((void*)model); // CBaseModelInfo::CBaseModelInfo();
    model->vtable = (uintptr_t)(g_libGTASA+0x5C6E90); // assign CPedModelInfo vmt

    (( uintptr_t (*)(PED_MODEL*))(*(void**)(model->vtable+0x1C)))(model);  // CClumpModelInfo::Init()
    *(PED_MODEL**)(g_libGTASA+0x87BF48+(id*4)) = model; // CModelInfo::ms_modelInfoPtrs

    PedsModelsCount++;
    return model;
}
/* ====================================================== */

uint32_t (*CRadar__GetRadarTraceColor)(uint32_t color, uint8_t bright, uint8_t friendly);
uint32_t CRadar__GetRadarTraceColor_hook(uint32_t color, uint8_t bright, uint8_t friendly)
{
    return TranslateColorCodeToRGBA(color);
}

int (*CRadar__SetCoordBlip)(int r0, float X, float Y, float Z, int r4, int r5, char* name);
int CRadar__SetCoordBlip_hook(int r0, float X, float Y, float Z, int r4, int r5, char* name)
{
    if(pNetGame && !strncmp(name, OBFUSCATE("CODEWAY"), 7))
    {
        float findZ = (( float (*)(float, float))(g_libGTASA+0x3C3DD8+1))(X, Y);
        findZ += 1.5f;

        RakNet::BitStream bsSend;
        bsSend.Write(X);
        bsSend.Write(Y);
        bsSend.Write(findZ);
        pNetGame->GetRakClient()->RPC(&RPC_MapMarker, &bsSend, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, nullptr);
    }
    return CRadar__SetCoordBlip(r0, X, Y, Z, r4, r5, name);
}

uint8_t bGZ = 0;
void (*CRadar__DrawRadarGangOverlay)(uint8_t v1);
void CRadar__DrawRadarGangOverlay_hook(uint8_t v1)
{
    bGZ = v1;
    if (pNetGame && pNetGame->GetGangZonePool())
        pNetGame->GetGangZonePool()->Draw();
}

uint32_t dwParam1, dwParam2;
extern "C" void pickup_ololo()
{
    if(pNetGame && pNetGame->GetPickupPool())
    {
        CPickupPool *pPickups = pNetGame->GetPickupPool();
        pPickups->PickedUp( ((dwParam1-(g_libGTASA+0x70E264))/0x20) );
    }
}

__attribute__((naked)) void PickupPickUp_hook()
{
    __asm__ volatile("push {lr}\n\t"
                     "push {r0}\n\t"
                     "blx get_lib\n\t"
                     "add r0, #0x2D0000\n\t"
                     "add r0, #0x009A00\n\t"
                     "add r0, #1\n\t"
                     "mov r1, r0\n\t"
                     "pop {r0}\n\t"
                     "pop {lr}\n\t"
                     "push {r1}\n\t");

    __asm__ volatile("push {r0-r11, lr}\n\t"
                     "mov %0, r4" : "=r" (dwParam1));

    __asm__ volatile("blx pickup_ololo\n\t");


    __asm__ volatile("pop {r0-r11, lr}\n\t");

    // restore
    __asm__ volatile("ldrb r1, [r4, #0x1C]\n\t"
                     "sub.w r2, r1, #0xD\n\t"
                     "sub.w r2, r1, #8\n\t"
                     "cmp r1, #6\n\t"
                     "pop {pc}\n\t");
}

extern "C" bool NotifyEnterVehicle(VEHICLE_TYPE *_pVehicle)
{
    if(!pNetGame) return false;

    CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
    CVehicle *pVehicle;
    VEHICLEID VehicleID = pVehiclePool->FindIDFromGtaPtr(_pVehicle);

    if(VehicleID == INVALID_VEHICLE_ID) return false;
    if(!pVehiclePool->GetSlotState(VehicleID)) return false;
    pVehicle = pVehiclePool->GetAt(VehicleID);
    if(pVehicle->m_bDoorsLocked) return false;
    if(pVehicle->m_pVehicle->entity.nModelIndex == TRAIN_PASSENGER) return false;

    if(pVehicle->m_pVehicle->pDriver &&
       pVehicle->m_pVehicle->pDriver->dwPedType != 0)
        return false;

    CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();

    pLocalPlayer->SendEnterVehicleNotification(VehicleID, false);

    return true;
}

void (*CTaskComplexEnterCarAsDriver)(uint32_t thiz, uint32_t pVehicle);
extern "C" void call_taskEnterCarAsDriver(uintptr_t a, uint32_t b)
{
    CTaskComplexEnterCarAsDriver(a, b);
}
void __attribute__((naked)) CTaskComplexEnterCarAsDriver_hook(uint32_t thiz, uint32_t pVehicle)
{
    __asm__ volatile("push {r0-r11, lr}\n\t"
                     "mov r2, lr\n\t"
                     "blx get_lib\n\t"
                     "add r0, #0x3A0000\n\t"
                     "add r0, #0xEE00\n\t"
                     "add r0, #0xF7\n\t"
                     "cmp r2, r0\n\t"
                     "bne 1f\n\t"
                     "mov r0, r1\n\t"
                     "blx NotifyEnterVehicle\n\t" // call NotifyEnterVehicle
                     "1:\n\t"  // call orig
                     "pop {r0-r11, lr}\n\t"
                     "push {r0-r11, lr}\n\t"
                     "blx call_taskEnterCarAsDriver\n\t"
                     "pop {r0-r11, pc}");
}

void ProcessPedDamage(PED_TYPE* pIssuer, PED_TYPE* pDamaged)
{
    if (!pNetGame) return;

    PED_TYPE* pPedPlayer = GamePool_FindPlayerPed();
    if (pDamaged && (pPedPlayer == pIssuer))
    {
        if (pNetGame->GetPlayerPool()->FindRemotePlayerIDFromGtaPtr((PED_TYPE*)pDamaged) != INVALID_PLAYER_ID)
        {
            CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
            CAMERA_AIM* caAim = pPlayerPool->GetLocalPlayer()->GetPlayerPed()->GetCurrentAim();

            VECTOR aim;
            aim.X = caAim->f1x;
            aim.Y = caAim->f1y;
            aim.Z = caAim->f1z;

            pPlayerPool->GetLocalPlayer()->SendBulletSyncData(pPlayerPool->FindRemotePlayerIDFromGtaPtr((PED_TYPE*)pDamaged), 1, aim);
        }
    }
}

void (*CTaskComplexLeaveCar)(uintptr_t** thiz, VEHICLE_TYPE *pVehicle, int iTargetDoor, int iDelayTime, bool bSensibleLeaveCar, bool bForceGetOut);
void CTaskComplexLeaveCar_hook(uintptr_t** thiz, VEHICLE_TYPE *pVehicle, int iTargetDoor, int iDelayTime, bool bSensibleLeaveCar, bool bForceGetOut)
{
    uintptr_t dwRetAddr = 0;
    __asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));
    dwRetAddr -= g_libGTASA;

    if (dwRetAddr == 0x3AE905 || dwRetAddr == 0x3AE9CF)
    {
        if (pNetGame)
        {
            if (GamePool_FindPlayerPed()->pVehicle == (uint32_t)pVehicle)
            {
                CVehiclePool *pVehiclePool=pNetGame->GetVehiclePool();
                VEHICLEID VehicleID=pVehiclePool->FindIDFromGtaPtr((VEHICLE_TYPE *)GamePool_FindPlayerPed()->pVehicle);
                CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
                pLocalPlayer->SendExitVehicleNotification(VehicleID);
            }
        }
    }
    (*CTaskComplexLeaveCar)(thiz, pVehicle, iTargetDoor, iDelayTime, bSensibleLeaveCar, bForceGetOut);
}

unsigned int (*MainMenuScreen__Update)(uintptr_t thiz, float a2);
unsigned int MainMenuScreen__Update_hook(uintptr_t thiz, float a2)
{
    unsigned int ret = MainMenuScreen__Update(thiz, a2);
    MainMenu_OnStartSAMP();
    return ret;
}

extern signed int (*OS_FileOpen)(unsigned int a1, int* a2, const char* a3, int a4);
signed int OS_FileOpen_hook(unsigned int a1, int* a2, const char* a3, int a4);

extern int(*OS_FileRead)(void* a1, void* a2, int a3);
int OS_FileRead_hook(void* a1, void* a2, int a33);
extern char(*CStreaming__ConvertBufferToObject)(int, int, int);
char CStreaming__ConvertBufferToObject_hook(int a1, int a2, int a3);
extern char(*CFileMgr__ReadLine)(int, int, int);
char CFileMgr__ReadLine_hook(int a1, int a2, int a3);

void RedirectCall(uintptr_t addr, uintptr_t func);

static char szLastBufferedName[40];
int (*cHandlingDataMgr__FindExactWord)(uintptr_t thiz, char* line, char* nameTable, int entrySize, int entryCount);
int cHandlingDataMgr__FindExactWord_hook(uintptr_t thiz, char* line, char* nameTable, int entrySize, int entryCount)
{
	strncpy(&szLastBufferedName[0], line, entrySize);
	return cHandlingDataMgr__FindExactWord(thiz, line, nameTable, entrySize, entryCount);
}

void (*cHandlingDataMgr__ConvertDataToGameUnits)(uintptr_t thiz, tHandlingData* handling);
void cHandlingDataMgr__ConvertDataToGameUnits_hook(uintptr_t thiz, tHandlingData* handling)
{
	int iHandling = ((int(*)(uintptr_t, char*))(g_libGTASA + 0x004FBC4C + 1))(thiz, &szLastBufferedName[0]);

	CHandlingDefault::FillDefaultHandling((uint16_t)iHandling, handling);

	return cHandlingDataMgr__ConvertDataToGameUnits(thiz, handling);
}


#include "..//nv_event.h"
int32_t(*NVEventGetNextEvent_hooked)(NVEvent* ev, int waitMSecs);
int32_t NVEventGetNextEvent_hook(NVEvent* ev, int waitMSecs)
{
	int32_t ret = NVEventGetNextEvent_hooked(ev, waitMSecs);

	if (ret)
	{
		if (ev->m_type == NV_EVENT_MULTITOUCH)
		{
			// process manually
			ev->m_type = (NVEventType)228;
		}

	}

	NVEvent event;
	NVEventGetNextEvent(&event);

	if (event.m_type == NV_EVENT_MULTITOUCH)
	{
		int type = event.m_data.m_multi.m_action & NV_MULTITOUCH_ACTION_MASK;
		int num = (event.m_data.m_multi.m_action & NV_MULTITOUCH_POINTER_MASK) >> NV_MULTITOUCH_POINTER_SHIFT;

		int x1 = event.m_data.m_multi.m_x1;
		int y1 = event.m_data.m_multi.m_y1;

		int x2 = event.m_data.m_multi.m_x2;
		int y2 = event.m_data.m_multi.m_y2;

		int x3 = event.m_data.m_multi.m_x3;
		int y3 = event.m_data.m_multi.m_y3;

		if (type == NV_MULTITOUCH_CANCEL)
		{
			type = NV_MULTITOUCH_UP;
		}

		if ((x1 || y1) || num == 0)
		{
			if (num == 0 && type != NV_MULTITOUCH_MOVE)
			{
				((void(*)(int, int, int posX, int posY))(g_libGTASA + 0x00239D5C + 1))(type, 0, x1, y1); // AND_TouchEvent
			}
			else
			{
				((void(*)(int, int, int posX, int posY))(g_libGTASA + 0x00239D5C + 1))(NV_MULTITOUCH_MOVE, 0, x1, y1); // AND_TouchEvent
			}
		}

		if ((x2 || y2) || num == 1)
		{
			if (num == 1 && type != NV_MULTITOUCH_MOVE)
			{
				((void(*)(int, int, int posX, int posY))(g_libGTASA + 0x00239D5C + 1))(type, 1, x2, y2); // AND_TouchEvent
			}
			else
			{
				((void(*)(int, int, int posX, int posY))(g_libGTASA + 0x00239D5C + 1))(NV_MULTITOUCH_MOVE, 1, x2, y2); // AND_TouchEvent
			}
		}
		if ((x3 || y3) || num == 2)
		{
			if (num == 2 && type != NV_MULTITOUCH_MOVE)
			{
				((void(*)(int, int, int posX, int posY))(g_libGTASA + 0x00239D5C + 1))(type, 2, x3, y3); // AND_TouchEvent
			}
			else
			{
				((void(*)(int, int, int posX, int posY))(g_libGTASA + 0x00239D5C + 1))(NV_MULTITOUCH_MOVE, 2, x3, y3); // AND_TouchEvent
			}
		}
	}

	return ret;
}

void(*CStreaming__Init2)();
void CStreaming__Init2_hook()
{
	CStreaming__Init2();
	*(uint32_t*)(g_libGTASA + 0x005DE734) = 536870912;
}

void (*NvUtilInit)(void);
void NvUtilInit_hook(void)
{
	NvUtilInit();
	*(char**)(g_libGTASA + 0x5D1608) = "/storage/emulated/0/LEVEL/";
}

#include "Pipelines/CustomCar/CustomCarEnvMapPipeline.h"
#include "Pipelines/CustomBuilding/CustomBuildingDNPipeline.h"
void InjectHooks() {
    ImageLoader::InjectHooks();
    CCustomCarEnvMapPipeline::InjectHooks();
    CCustomBuildingDNPipeline::InjectHooks();
}

void InstallHookFixes();
void InstallSpecialHooks()
{
    InjectHooks();

    WriteMemory(g_libGTASA + 0x23BEDC, (uintptr_t) "\xF8\xB5", 2);
    WriteMemory(g_libGTASA + 0x23BEDE, (uintptr_t) "\x00\x46\x00\x46", 4);

    CHook::InlineHook(g_libGTASA, 0x23BEDC, &OS_FileRead_hook, &OS_FileRead);
    CHook::InlineHook(g_libGTASA, 0x23B3DC, &NvFOpen_hook, &NvFOpen);
    CHook::InlineHook(g_libGTASA, 0x241D94, &NvUtilInit_hook, &NvUtilInit);
    CHook::InlineHook(g_libGTASA, 0x269974, &MenuItem_add_hook, &MenuItem_add);
    CHook::InlineHook(g_libGTASA, 0x4D3864, &CText_Get_hook, &CText_Get);
    CHook::InlineHook(g_libGTASA, 0x40C530, &InitialiseRenderWare_hook, &InitialiseRenderWare);
    CHook::InlineHook(g_libGTASA, 0x0025E660, &MainMenuScreen__Update_hook, &MainMenuScreen__Update);
    CHook::InlineHook(g_libGTASA, 0x0023BB84, &OS_FileOpen_hook, &OS_FileOpen);

    CHook::InlineHook(g_libGTASA, 0x004FBBB0, &cHandlingDataMgr__FindExactWord_hook, &cHandlingDataMgr__FindExactWord);
    CHook::InlineHook(g_libGTASA, 0x004FBCF4, &cHandlingDataMgr__ConvertDataToGameUnits_hook, &cHandlingDataMgr__ConvertDataToGameUnits);
    CHook::InlineHook(g_libGTASA, 0x0023ACC4, &NVEventGetNextEvent_hook, &NVEventGetNextEvent_hooked);
    CHook::InlineHook(g_libGTASA, 0x004042A8, &CStreaming__Init2_hook, &CStreaming__Init2);

    InstallHookFixes();
}

void ProcessPedDamage(PED_TYPE* pIssuer, PED_TYPE* pPlayer);

uintptr_t (*ComputeDamageResponse)(uintptr_t, uintptr_t, int, int);
uintptr_t ComputeDamageResponse_hooked(uintptr_t issuer, uintptr_t ped, int a3, int a4)
{
	ProcessPedDamage((PED_TYPE*)*(uintptr_t*)issuer, (PED_TYPE*)ped );
	return ComputeDamageResponse(issuer, ped, a3, a4);
}

int(*RwFrameAddChild)(int, int);
int RwFrameAddChild_hook(int a1, int a2)
{
	if (a2 && a1)
	{
		return RwFrameAddChild(a1, a2);
	}
	else return 0;
}

void (*CAnimManager__UncompressAnimation)(int, int);
void CAnimManager__UncompressAnimation_hook(int a1, int a2)
{
	if (a1)
	{
		CAnimManager__UncompressAnimation(a1, a2);
	}
}

void(*CCustomRoadsignMgr__RenderRoadsignAtomic)(int, int);
void CCustomRoadsignMgr__RenderRoadsignAtomic_hook(int a1, int a2)
{
	if (a1)
	{
		CCustomRoadsignMgr__RenderRoadsignAtomic(a1, a2);
	}
}

int(*CUpsideDownCarCheck__IsCarUpsideDown)(int, int);
int CUpsideDownCarCheck__IsCarUpsideDown_hook(int a1, int a2)
{
	if (*(uintptr_t*)(a2 + 20))
	{
		return CUpsideDownCarCheck__IsCarUpsideDown(a1, a2);
	}
	return 0;
}

int(*CAnimBlendNode__FindKeyFrame)(int, float, int, int);
int CAnimBlendNode__FindKeyFrame_hook(int a1, float a2, int a3, int a4)
{
	if (*(uintptr_t*)(a1 + 16))
	{
		return CAnimBlendNode__FindKeyFrame(a1, a2, a3, a4);
	}
	else return 0;
}


/* ====================================================== */

typedef struct _ATOMIC_MODEL
{
	uintptr_t 	vtable;
	uint8_t		data[52];
} ATOMIC_MODEL; // SIZE = 56

ATOMIC_MODEL AtomicModels[16000];
int AtomicModelsCount = 0;

ATOMIC_MODEL* (*CModelInfo_AddAtomicModel)(int id);
ATOMIC_MODEL* CModelInfo_AddAtomicModel_hook(int id)
{
	ATOMIC_MODEL* model = &AtomicModels[AtomicModelsCount];
	memset(model, 0, sizeof(ATOMIC_MODEL));

	((void(*)(void* thiz))(g_libGTASA + 0x0033559C + 1))((void*)model); // CBaseModelInfo::CBaseModelInfo();

	model->vtable = (uintptr_t)(g_libGTASA + 0x005C6C68); // assign CAtomicModelInfo vmt

	((uintptr_t(*)(ATOMIC_MODEL*))(*(void**)(model->vtable + 0x1C)))(model); // CClumpModelInfo::Init()

	*(ATOMIC_MODEL * *)(g_libGTASA + 0x87BF48 + (id * 4)) = model; // CModelInfo::ms_modelInfoPtrs

	AtomicModelsCount++;
	return model;
}

/* ====================================================== */
typedef struct _VEHICLE_MODEL
{
	uintptr_t 	vtable;
	uint8_t		data[932];
} VEHICLE_MODEL; // SIZE = 936

VEHICLE_MODEL VehicleModels[370];
int VehicleModelsCount = 0;

VEHICLE_MODEL* (*CModelInfo_AddVehicleModel)(int id);
VEHICLE_MODEL* CModelInfo_AddVehicleModel_hook(int id)
{
	VEHICLE_MODEL* model = &VehicleModels[VehicleModelsCount];
	memset(model, 0, sizeof(VEHICLE_MODEL));

	((void(*)(void* thiz))(g_libGTASA + 0x00337AA0 + 1))((void*)model); // CVehicleModelInfo::CVehicleModelInfo();

	model->vtable = (uintptr_t)(g_libGTASA + 0x005C6EE0);			// assign CVehicleModelInfo vmt

	((uintptr_t(*)(VEHICLE_MODEL*))(*(void**)(model->vtable + 0x1C)))(model); // CVehicleModelInfo::Init()

	*(VEHICLE_MODEL * *)(g_libGTASA + 0x87BF48 + (id * 4)) = model; // CModelInfo::ms_modelInfoPtrs

	VehicleModelsCount++;
	return model;
}
/* ====================================================== */
void(*CHud__DrawScriptText)(uintptr_t, uint8_t);

float g_fMicrophoneButtonPosX;
float g_fMicrophoneButtonPosY;
bool g_IsVoiceServer();
void CHud__DrawScriptText_hook(uintptr_t thiz, uint8_t unk)
{
	/*CHud__DrawScriptText(thiz, unk);
	if (pGame && pNetGame)
	{
	

		if (g_pWidgetManager)
		{
			ImGuiIO& io = ImGui::GetIO();

			if (!g_pWidgetManager->GetSlotState(WIDGET_CAMERA_CYCLE))
			{
                g_pWidgetManager->New(WIDGET_CAMERA_CYCLE,
                                      pSettings->GetReadOnly().fButtonCameraCycleX,
                                      pSettings->GetReadOnly().fButtonCameraCycleY,
                                      pSettings->GetReadOnly().fButtonCameraCycleSize,
                                      pSettings->GetReadOnly().fButtonCameraCycleSize,
                                      "cam-toggle");
			}
		}
	}*/
}

#include "..//keyboard.h"
#include "../gtasa/HUD.h"

extern CKeyBoard* pKeyBoard;
int(*CWidgetButtonEnterCar_Draw)(uintptr_t);
uint32_t g_uiLastTickVoice = 0;
int CWidgetButtonEnterCar_Draw_hook(uintptr_t thiz)
{
	/*if (g_pWidgetManager)
	{
		CWidget* pWidget = g_pWidgetManager->GetWidget(WIDGET_CHATHISTORY_UP);
		pWidget = g_pWidgetManager->GetWidget(WIDGET_CAMERA_CYCLE);
		if (pWidget)
		{
                pWidget->SetDrawState(true);
		}*/
		/*pWidget = g_pWidgetManager->GetWidget(WIDGET_MICROPHONE);
		if (pWidget)
		{
			    CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
                CVehicle *pVehicle;
				if (pVehicle->GetDistanceFromLocalPlayerPed() < 5.0f)
				{
				pWidget->SetDrawState(true);
				static uint32_t lastTick = GetTickCount();
				if (pWidget->GetState() == 2 && GetTickCount() - lastTick >= 250)
				{
					//pNetGame->SendChatCommand("/l");

					CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
					if (pVehiclePool)
					{
						VEHICLEID ClosetVehicleID = pVehiclePool->FindNearestToLocalPlayerPed();
						if (ClosetVehicleID < MAX_VEHICLES && pVehiclePool->GetSlotState(ClosetVehicleID))
						{
							CVehicle* pVehicle = pVehiclePool->GetAt(ClosetVehicleID);
							if (pVehicle)
							{
								if (pVehicle->GetDistanceFromLocalPlayerPed() < 5.0f)
								{
									CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
									if (pPlayerPool)
									{
										CLocalPlayer* pLocalPlayer;
										if (!pPlayerPool->GetLocalPlayer()->GetPlayerPed()->IsInVehicle() && !pPlayerPool->GetLocalPlayer()->GetPlayerPed()->IsAPassenger())
										{
											//if (ImGui::Button("G", vecButSize))
											{
												if (pNetGame)
												{
													if (pPlayerPool)
													{
														pLocalPlayer = pPlayerPool->GetLocalPlayer();
														if (pLocalPlayer)
														{
															pLocalPlayer->HandlePassengerEntryEx();
														}
													}
												}
											}
										}
										
									}
								}
							}
						}
						
				}
					lastTick = GetTickCount();
				}
		}*/

		/*if (!pGame->IsToggledHUDElement(HUD_ELEMENT_BUTTONS))
		{
			for (int i = 0; i < MAX_WIDGETS; i++)
			{
				CWidget* pWidget = g_pWidgetManager->GetWidget(i);
				if (pWidget)
				{
					pWidget->SetDrawState(false);
				}
			}
		}

		g_pWidgetManager->Draw();
	}*/
	return CWidgetButtonEnterCar_Draw(thiz);
}

uint64_t(*CWorld_ProcessPedsAfterPreRender)();
uint64_t CWorld_ProcessPedsAfterPreRender_hook()
{
	uint64_t res = CWorld_ProcessPedsAfterPreRender();
	if (pNetGame && pNetGame->GetPlayerPool())
	{
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			CPlayerPed* pPed = nullptr;
			if (pNetGame->GetPlayerPool()->GetLocalPlayerID() == i)
			{
				pPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
			}
			else
			{
				if (pNetGame->GetPlayerPool()->GetSlotState(i))
				{
					pPed = pNetGame->GetPlayerPool()->GetAt(i)->GetPlayerPed();
				}
			}
			if (!pPed) continue;
			else
			{
				pPed->ProcessAttach();
			}
		}
	}

	return res;
}

int RemoveModelIDs[MAX_REMOVE_MODELS];
VECTOR RemovePos[MAX_REMOVE_MODELS];
float RemoveRad[MAX_REMOVE_MODELS];
int iTotalRemovedObjects = 0;


#include "util.h"
int(*CFileLoader__LoadObjectInstance)(uintptr_t, uintptr_t);
int CFileLoader__LoadObjectInstance_hook(uintptr_t thiz, uintptr_t name)
{
	for (int i = 0; i < iTotalRemovedObjects; i++)
	{
		if (RemoveModelIDs[i] == *(uint32_t*)(thiz + 28))
		{
			VECTOR pos;
			pos.X = *(float*)(thiz);
			pos.Y = *(float*)(thiz + 4);
			pos.Z = *(float*)(thiz + 8);
			if (GetDistanceBetween3DPoints(&pos, &RemovePos[i]) <= RemoveRad[i])
			{
				*(int*)(thiz + 28) = 19300;
			}
		}
	}
	return CFileLoader__LoadObjectInstance(thiz, name);
}

#include <list>

std::list<std::pair<unsigned int*, unsigned int>> resetEntries;

static uint32_t Color32Reverse(uint32_t x)
{
	return
		((x & 0xFF000000) >> 24) |
		((x & 0x00FF0000) >> 8) |
		((x & 0x0000FF00) << 8) |
		((x & 0x000000FF) << 24);
}

static RwRGBA DWORD2RGBAinternal(uint32_t dwColor)
{
	RwRGBA tmp;

	tmp.blue = dwColor & 0xFF; dwColor >>= 8;
	tmp.green = dwColor & 0xFF; dwColor >>= 8;
	tmp.red = dwColor & 0xFF; dwColor >>= 8;
	tmp.alpha = dwColor & 0xFF; /* dwColor >>= 8; */

	return tmp;
}

uintptr_t RwFrameForAllObjectsCALLBACK1(uintptr_t object, CObject* pObject)
{
	if (*(uint8_t*)object != 1)
	{
		return object;
	}
	uintptr_t pAtomic = object;
	RpGeometry* pGeom = *(RpGeometry * *)(pAtomic + 24);
	if (!pGeom)
	{
		return object;
	}

	int numMats = pGeom->matList.numMaterials;
	if (numMats > 16)
	{
		numMats = 16;
	}
	for (int i = 0; i < numMats; i++)
	{
		RpMaterial* pMat = pGeom->matList.materials[i];
		if (!pMat)
		{
			continue;
		}
		if (pObject->m_pMaterials[i].m_bCreated)
		{
			RpMaterial* pMat = pGeom->matList.materials[i];
			if (!pMat)
			{
				continue;
			}
			if (pObject->m_pMaterials[i].pTex)
			{
				resetEntries.push_back(std::make_pair(reinterpret_cast<unsigned int*>(&pMat->texture), *reinterpret_cast<unsigned int*>(&pMat->texture)));
				pMat->texture = pObject->m_pMaterials[i].pTex;
			}
			/*if (pObject->m_pMaterials[i].dwColor)
			{
				//((int(*)())(g_libGTASA + 0x00559FC8 + 1))();
				resetEntries.push_back(std::make_pair(reinterpret_cast<unsigned int*>(&pGeom->flags), *reinterpret_cast<unsigned int*>(&pGeom->flags)));
				pGeom->flags |= 0x00000040;
				pGeom->flags &= 0xFFFFFFF7;
				RwRGBA r = DWORD2RGBAinternal(pObject->m_pMaterials[i].dwColor);

				resetEntries.push_back(std::make_pair(reinterpret_cast<unsigned int*>(&pMat->color), *reinterpret_cast<unsigned int*>(&pMat->color)));
				pMat->color = r;
				pMat->surfaceProps.ambient = 1.0f;
				pMat->surfaceProps.specular = 0.0f;
				pMat->surfaceProps.diffuse = 1.0f;
			}*/
		}
	}
	return object;
}

void (*CEntity__Render)(ENTITY_TYPE *);
void CEntity__Render_hook(ENTITY_TYPE *thiz) {
    if (Skybox::GetSkyObject()) {
        if (Skybox::GetSkyObject()->m_pEntity == thiz && !Skybox::IsNeedRender()) {
            return;
        }
    }

    CEntity__Render(thiz);
}

int g_iLastRenderedObject;
void(*CObject__Render)(ENTITY_TYPE*);
void CObject__Render_hook(ENTITY_TYPE* thiz)
{
	uintptr_t dwRetAddr = 0;
	__asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));
	dwRetAddr -= g_libGTASA;

	if (dwRetAddr == 0x0051ABB0 + 1 || dwRetAddr == 0x004353FE + 1 || dwRetAddr == 0x004352C4 + 1)
	{
		return CObject__Render(thiz);
	}

	uintptr_t pAtomic = thiz->m_RwObject;
	if (!pAtomic)
	{
		return CObject__Render(thiz); // CObject::Render(CObject*)
	}
	if (!*(uintptr_t*)(pAtomic + 4))
	{
		return CObject__Render(thiz); // CObject::Render(CObject*)
	}

	if (pNetGame)
	{
		CObjectPool* pObjectPool = pNetGame->GetObjectPool();
		if (pObjectPool)
		{
			CObject* pObject = pObjectPool->GetObjectFromGtaPtr(thiz);
			if (pObject)
			{
				if (pObject->m_pEntity)
				{
					g_iLastRenderedObject = pObject->m_pEntity->nModelIndex;
				}

				((void(*)())(g_libGTASA + 0x00559EF8 + 1))(); // DeActivateDirectional
				if (pObject->m_bMaterials)
				{
					((uintptr_t(*)(uintptr_t, uintptr_t, uintptr_t))(g_libGTASA + 0x001AEE2C + 1))(*(uintptr_t*)(pAtomic + 4), (uintptr_t)RwFrameForAllObjectsCALLBACK1, (uintptr_t)pObject); // RwFrameForAllObjects
				}
			}

		}
	}

	CObject__Render(thiz); // CObject::Render(CObject*)
	for (auto& p : resetEntries)
		* p.first = p.second;
	resetEntries.clear();
}

#include "../gtasa/GLES.h"
#include "../CGameResourcesDecryptor.h"

void (*CRenderer__RenderEverythingBarRoads)();
void CRenderer__RenderEverythingBarRoads_hook() {
    Skybox::Process();

    CRenderer__RenderEverythingBarRoads();
}

#pragma optimize( "", off )

char CStreaming__ConvertBufferToObject_hook(int a1, int a2, int a3) {
    uintptr_t dwRetAddr = 0;
    __asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));

    uint32_t tickStart = GetTickCount();
    CGameResourcesDecryptor::CStreaming__ConvertBufferToObject_hook((char*)a1, a2, a3);
    if (a2 >= 15000 && a2 <= 15100) {
        pChatWindow->AddDebugMessage("loading time %d", GetTickCount() - tickStart);
    }
    char a12 = CStreaming__ConvertBufferToObject(a1, a2, a3);
    return a12;
}

bool isEncrypted(const char *szArch) {
    return false;

    for (const auto &i: encrArch)
        if (!strcmp(i.decrypt(), szArch)) return true;

    return false;
}

signed int (*OS_FileOpen)(unsigned int a1, int* a2, const char* a3, int a4);
signed int OS_FileOpen_hook(unsigned int a1, int *a2, const char *a3, int a4) {
    uintptr_t calledFrom = 0;
    __asm__ volatile ("mov %0, lr" : "=r" (calledFrom));
    calledFrom -= g_libGTASA;
    signed int retn = OS_FileOpen(a1, a2, a3, a4);

    if (calledFrom == 0x001BCE9A + 1) {
        if (isEncrypted(a3))
            lastOpenedFile = *a2;
    }

    return retn;
}

static uint32_t dwRLEDecompressSourceSize = 0;

int(*OS_FileRead)(void* a1, void* a2, int a3);
void InitCTX(AES_ctx& ctx, const uint8_t* pKey);
int OS_FileRead_hook(void *a1, void *a2, int a3) {
    uintptr_t calledFrom = 0;
    __asm__ volatile ("mov %0, lr" : "=r" (calledFrom));
    calledFrom -= g_libGTASA;

    if (!a3)
        return 0;

    if (calledFrom == 0x001BCEE0 + 1 && a1 == (void *) lastOpenedFile) {
        lastOpenedFile = 0;

        AES_ctx ctx;
        InitCTX(ctx, &g_iEncryptionKeyTXD[0]);

        int retv = OS_FileRead(a1, a2, a3);
        int fileSize = a3;
        int iters = fileSize / PART_SIZE_TXD;

        auto pointer = (uintptr_t) a2;

        for (int i = 0; i < iters; i++) {
            AES_CBC_decrypt_buffer(&ctx, (uint8_t *) pointer, PART_SIZE_TXD);
            pointer += PART_SIZE_TXD;
        }

        return retv;
    }

    if (calledFrom == 0x001BDD34 + 1) {
        int retn = OS_FileRead(a1, a2, a3);

        dwRLEDecompressSourceSize = *(uint32_t *) a2;

        return retn;
    } else {
        int retn = OS_FileRead(a1, a2, a3);

        return retn;
    }
}

/*int(*OS_FileRead)(void* a1, void* a2, int a3);
int OS_FileRead_hook(void* a1, void* a2, int a3)
{
	uintptr_t calledFrom = 0;
	__asm__ volatile ("mov %0, lr" : "=r" (calledFrom));
	calledFrom -= g_libGTASA;

	if (!a3)
	{
		return 0;
	}

	if (calledFrom == 0x001BCEE0 + 1 && a1 == (void*)lastOpenedFile)
	{
		lastOpenedFile = 0;
		CTinyEncrypt tinyEnc;
		tinyEnc.SetKey(&g_iEncryptionKeyVersion2TXD[0]);
		int retv = OS_FileRead(a1, a2, a3);
		int fileSize = a3;
		int iters = fileSize / PART_SIZE_TXD;
		uintptr_t pointer = (uintptr_t)a2;
		for (int i = 0; i < iters; i++)
		{
			tinyEnc.DecryptData((void*)pointer, PART_SIZE_TXD, 16);
			pointer += PART_SIZE_TXD;
		}
		return retv;
	}
	else
	{
		int retn = OS_FileRead(a1, a2, a3);

		return retn;
	}
}*/

#include <sstream>
char(*CFileMgr__ReadLine)(int, int, int);
char CFileMgr__ReadLine_hook(int a1, int a2, int a3)
{
	char retv = CFileMgr__ReadLine(a1, a2, a3);
	char* pStr = (char*)a2;
	int value = *(int*)pStr;
	if (value == g_unobfuscate(g_iIdentifierVersion2))
	{
		pStr += 4;
		int length = *(int*)pStr;
		pStr -= 4;
		memcpy((void*)pStr, (const void*)& pStr[8], length);

		pStr[length] = 0;
		std::stringstream ss;

		uint32_t keyi = g_unobfuscate(g_i64Encrypt);

		ss << keyi;

		std::string key(ss.str());
		std::string val(pStr);

		std::string decr = decrypt(val, key);

		strcpy((char*)a2, decr.c_str());
	}
	return retv;
}

#pragma optimize( "", on )

uintptr_t(*GetTexture_orig)(const char*);
uintptr_t GetTexture_hook(const char* a1)
{
	//001BE990
	uintptr_t tex = ((uintptr_t(*)(const char*))(g_libGTASA + 0x001BE990 + 1))(a1);
	if (!tex)
	{
		return 0;
	}
	else
	{
		++* (uintptr_t*)(tex + 84);
		return tex;
	}
}

uintptr_t(*CPlayerInfo__FindObjectToSteal)(uintptr_t, uintptr_t);
uintptr_t CPlayerInfo__FindObjectToSteal_hook(uintptr_t a1, uintptr_t a2)
{
	return 0;
}

RwFrame* CClumpModelInfo_GetFrameFromId_Post(RwFrame* pFrameResult, RpClump* pClump, int id)
{
	if (pFrameResult)
		return pFrameResult;

	uintptr_t calledFrom = 0;
	__asm__ volatile ("mov %0, lr" : "=r" (calledFrom));
	calledFrom -= g_libGTASA;

	// �� ��������� ���� ���� ��� �����
	if (calledFrom == 0x00515708                // CVehicle::SetWindowOpenFlag
		|| calledFrom == 0x00515730             // CVehicle::ClearWindowOpenFlag
		|| calledFrom == 0x00338698             // CVehicleModelInfo::GetOriginalCompPosition
		|| calledFrom == 0x00338B2C)            // CVehicleModelInfo::CreateInstance
		return NULL;

	for (uint i = 2; i < 40; i++)
	{
		RwFrame* pNewFrameResult = NULL;
		uint     uiNewId = id + (i / 2) * ((i & 1) ? -1 : 1);
		
		pNewFrameResult = ((RwFrame * (*)(RpClump * pClump, int id))(g_libGTASA + 0x00335CC0 + 1))(pClump, i);

		if (pNewFrameResult)
		{
			return pNewFrameResult;
		}
	}

	return NULL;
}

RwFrame* (*CClumpModelInfo_GetFrameFromId)(RpClump*, int);
RwFrame* CClumpModelInfo_GetFrameFromId_hook(RpClump* a1, int a2)
{
	return CClumpModelInfo_GetFrameFromId_Post(CClumpModelInfo_GetFrameFromId(a1, a2), a1, a2);
}

void (*CWidgetRegionLook__Update)(uintptr_t thiz);
void CWidgetRegionLook__Update_hook(uintptr_t thiz)
{
	if (bBlockCWidgetRegionLookUpdate)
	{
		return;
	}
	else
	{
		return CWidgetRegionLook__Update(thiz);
	}
}

extern char g_iLastBlock[512];
unsigned int(*LoadFullTexture)(uintptr_t *thiz, unsigned int entryIndex);
unsigned int LoadFullTexture_hook(uintptr_t *thiz, unsigned int entryIndex)
{
    strcpy(g_iLastBlock, *(const char **)(*((DWORD *)thiz + 7) + 0x17 * entryIndex));

    return LoadFullTexture(thiz, entryIndex);
}

uint8_t* (*RLEDecompress)(uint8_t* pDest, size_t uiDestSize, uint8_t const* pSrc, size_t uiSegSize, uint32_t uiEscape);
uint8_t* RLEDecompress_hook(uint8_t* pDest, size_t uiDestSize, uint8_t const* pSrc, size_t uiSegSize, uint32_t uiEscape) {
	if (!pDest) 
	{
		return pDest;
	}

	if (!pSrc) 
	{
		return pDest;
	}

	uint8_t* pTempDest = pDest;
	const uint8_t* pTempSrc = pSrc;
	uint8_t* pEndOfDest = &pDest[uiDestSize];

	uint8_t* pEndOfSrc = (uint8_t*)&pSrc[dwRLEDecompressSourceSize];

	if (pDest < pEndOfDest) 
	{
		do 
		{
			if (*pTempSrc == uiEscape) 
			{
				uint8_t ucCurSeg = pTempSrc[1];
				if (ucCurSeg) 
				{
					uint8_t* ucCurDest = pTempDest;
					uint8_t ucCount = 0;
					do 
					{
						++ucCount;
						//Log("WRITE111 TO 0x%x FROM 0x%x SIZE %d", ucCurDest, pTempSrc + 2, uiSegSize);
						pDest = (uint8_t*)memcpy(ucCurDest, pTempSrc + 2, uiSegSize);
						
						ucCurDest += uiSegSize;
					} while (ucCurSeg != ucCount);


					pTempDest += uiSegSize * ucCurSeg;
				}
				pTempSrc += 2 + uiSegSize;
			}

			else 
			{
				if (pTempSrc + uiSegSize >= pEndOfSrc)
				{
					//Log("AVOID CRASH TO 0x%x FROM 0x%x SIZE %d END OF SRC 0x%x", pTempDest, pTempSrc, pEndOfSrc - pTempSrc - 1, pEndOfSrc);
					return pDest;
				}
				else
				{
					//Log("WRITE222 TO 0x%x FROM 0x%x SIZE %d END OF SRC 0x%x", pTempDest, pTempSrc, uiSegSize, pEndOfSrc);
					pDest = (uint8_t*)memcpy(pTempDest, pTempSrc, uiSegSize);
					pTempDest += uiSegSize;
					pTempSrc += uiSegSize;
				}
			}
		} while (pEndOfDest > pTempDest);
	}

	dwRLEDecompressSourceSize = 0;

	return pDest;
}

#include "..//crashlytics.h"

char g_bufRenderQueueCommand[200];
uintptr_t g_dwRenderQueueOffset;

char* (*RenderQueue__ProcessCommand)(uintptr_t thiz, char* a2);
char* RenderQueue__ProcessCommand_hook(uintptr_t thiz, char* a2)
{
	if (thiz && a2)
	{
		uintptr_t* dwRenderQueue = (uintptr_t*)thiz;

		memset(g_bufRenderQueueCommand, 0, sizeof(g_bufRenderQueueCommand));

		g_dwRenderQueueOffset = *(uintptr_t*)a2;
		snprintf(g_bufRenderQueueCommand, 190, "offset: %d | name: %s", g_dwRenderQueueOffset, (const char*)(*(dwRenderQueue + 100 + g_dwRenderQueueOffset)));

		return RenderQueue__ProcessCommand(thiz, a2);
	}
	else
	{
		return nullptr;
	}
}

int (*_rwFreeListFreeReal1)(int a1, unsigned int a2);
int _rwFreeListFreeReal_hook(int a1, unsigned int a2)
{
	if (a1)
	{
		return _rwFreeListFreeReal1(a1, a2);
	}
	else
	{
		return 0;
	}
}

/*uintptr_t* CTouchInterface__m_bWidgets;

static bool ShouldBeProcessedButton(int result)
{
	CTouchInterface__m_bWidgets = (uintptr_t*)(g_libGTASA + 0x00657E48);

	if (result == CTouchInterface__m_bWidgets[18])
	{
		return false;
	}

	if (result == CTouchInterface__m_bWidgets[26] || result == CTouchInterface__m_bWidgets[27])
	{
		if (pNetGame)
		{
			CLocalPlayer* pLocal = pNetGame->GetPlayerPool()->GetLocalPlayer();
			if (pLocal)
			{
				if (pLocal->GetPlayerPed())
				{
					VEHICLE_TYPE* pVehicle = pLocal->GetPlayerPed()->GetGtaVehicle();
					if (pVehicle)
					{
						uintptr_t this_vtable = pVehicle->entity.vtable;
						this_vtable -= g_libGTASA;

						if (this_vtable == 0x5CCE60)
						{
							return 1;
						}
					}
				}
			}
		}
		return 0;
	}
	return 1;
}

void (*CWidgetButton__Update)(int result, int a2, int a3, int a4);
void CWidgetButton__Update_hook(int result, int a2, int a3, int a4)
{
	if (!result)
	{
		return;
	}
	if (ShouldBeProcessedButton(result))
	{
		return CWidgetButton__Update(result, a2, a3, a4);
	}
	else
	{
		return;
	}
}

int (*CWidget__IsTouched)(uintptr_t a1);
int CWidget__IsTouched_hook(uintptr_t a1)
{
	if (ShouldBeProcessedButton(a1))
	{
		return CWidget__IsTouched(a1);
	}
	else
	{
		return 0;
	}
}*/
void readVehiclesAudioSettings();
void (*CVehicleModelInfo__SetupCommonData)();
void CVehicleModelInfo__SetupCommonData_hook()
{
	CVehicleModelInfo__SetupCommonData();
	readVehiclesAudioSettings();
}

extern VehicleAudioPropertiesStruct VehicleAudioProperties[20000];
static uintptr_t addr_veh_audio = (uintptr_t)& VehicleAudioProperties[0];

void (*CAEVehicleAudioEntity__GetVehicleAudioSettings)(uintptr_t thiz, int16_t a2, int a3);
void CAEVehicleAudioEntity__GetVehicleAudioSettings_hook(uintptr_t dest, int16_t a2, int ID)
{
	memcpy((void*)dest, &VehicleAudioProperties[(ID - 400)], sizeof(VehicleAudioPropertiesStruct));
}

void (*CDarkel__RegisterCarBlownUpByPlayer)(void* pVehicle, int arg2);
void CDarkel__RegisterCarBlownUpByPlayer_hook(void* pVehicle, int arg2)
{
	return;
}
void (*CDarkel__ResetModelsKilledByPlayer)(int playerid);
void CDarkel__ResetModelsKilledByPlayer_hook(int playerid)
{
	return;
}
int(*CDarkel__QueryModelsKilledByPlayer)(int, int);
int CDarkel__QueryModelsKilledByPlayer_hook(int player, int modelid)
{
	return 0;
}

int (*CDarkel__FindTotalPedsKilledByPlayer)(int player);
int CDarkel__FindTotalPedsKilledByPlayer_hook(int player)
{
	return 0;
}

void (*CDarkel__RegisterKillByPlayer)(void* pKilledPed, unsigned int damageWeaponID, bool bHeadShotted, int arg4);
void CDarkel__RegisterKillByPlayer_hook(void* pKilledPed, unsigned int damageWeaponID, bool bHeadShotted, int arg4)
{
	return;
}

std::list<std::pair<unsigned int*, unsigned int>> resetEntriesVehicle;


RpMaterial* CVehicle__SetupRenderMatCB(RpMaterial* material, void* data)
{
	if (material)
	{
		if (material->texture)
		{
			CVehicle* pVeh = (CVehicle*)data;
			for (size_t i = 0; i < MAX_REPLACED_TEXTURES; i++)
			{
				if (pVeh->m_bReplaceTextureStatus[i])
				{
					if (!strcmp(&(material->texture->name[0]), &(pVeh->m_szReplacedTextures[i].szOld[0])))
					{
						if (pVeh->m_szReplacedTextures[i].pTexture)
						{
							resetEntriesVehicle.push_back(std::make_pair(reinterpret_cast<unsigned int*>(&(material->texture)), *reinterpret_cast<unsigned int*>(&(material->texture))));
							material->texture = pVeh->m_szReplacedTextures[i].pTexture;
							if (strstr(pVeh->m_szReplacedTextures[i].szOld, "ret_t"))
							{
								material->color.alpha = 255;
							}
						}
					}
				}
			}
		}
	}
	return material;
}

uintptr_t CVehicle__SetupRenderCB(uintptr_t atomic, void* data)
{
	if (*(RpGeometry * *)(atomic + 24))
	{
		((RpGeometry * (*)(RpGeometry*, uintptr_t, void*))(g_libGTASA + 0x001E284C + 1))(*(RpGeometry * *)(atomic + 24), (uintptr_t)CVehicle__SetupRenderMatCB, (void*)data); // RpGeometryForAllMaterials
	}

	return atomic;
}
#include "..//cryptors/MODELINFO_EDITABLE_result.h"
void(*CVehicleModelInfo__SetEditableMaterials)(uintptr_t);
void CVehicleModelInfo__SetEditableMaterials_hook(uintptr_t clump)
{
	PROTECT_CODE_MODELINFO_EDITABLE;
	RpClump* pClump = (RpClump*)clump;

	

	if (pNetGame && pClump)
	{
		if (pNetGame->GetVehiclePool())
		{
			VEHICLEID vehId = pNetGame->GetVehiclePool()->FindIDFromRwObject((RwObject*)clump);
			CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt(vehId);
			if (pVehicle)
			{
				if (pVehicle->m_bReplacedTexture)
				{
					// RpClump* RpClumpForAllAtomics(RpClump* clump, RpAtomicCallBack callback, void* pData);
					((RpClump * (*)(RpClump*, uintptr_t, void*))(g_libGTASA + 0x001E0EA0 + 1))(pClump, (uintptr_t)CVehicle__SetupRenderCB, (void*)pVehicle); // RpClumpForAllAtomics
				}
			}
		}
	}

	CVehicleModelInfo__SetEditableMaterials(clump);
}

#include "..//cryptors/RESET_AFTER_RENDER_result.h"

void (*CVehicle__ResetAfterRender)(uintptr_t);
void CVehicle__ResetAfterRender_hook(uintptr_t thiz)
{
	PROTECT_CODE_RESET_AFTER_RENDER;

	for (auto& p : resetEntriesVehicle)
	{
		*p.first = p.second;
	}
	resetEntriesVehicle.clear();

	CVehicle__ResetAfterRender(thiz);
}

void (*CRopes__Update)();
void CRopes__Update_hook()
{
	
}

void* (*CCustomCarEnvMapPipeline__pluginEnvMatDestructorCB)(void* object, RwInt32 offset, RwInt32 size);
void* CCustomCarEnvMapPipeline__pluginEnvMatDestructorCB_hook(void* object, RwInt32 offset, RwInt32 size)
{
	if(pChatWindow) pChatWindow->AddDebugMessage("m_objects %x", *(uintptr_t * *)(g_libGTASA + 0x00669E48));
	return CCustomCarEnvMapPipeline__pluginEnvMatDestructorCB(object, offset, size);
}

#include "CRenderTarget.h"
#include "..//gui/CFontInstance.h"
#include "..//gui/CFontRenderer.h"
#include "CCustomPlateManager.h"
#include "../gtasa/ConfigReader.h"

static bool g_bFirstPersonOnFootEnabled = false;
extern bool bDisableTestGovno;
void (*CGame__Process)();
void CGame__Process_hook()
{
    CGame__Process();

    static bool once = false;
    if (!once)
    {
        CCustomPlateManager::Initialise();
        //CSnow::Initialise();
        once = true;
    }

    // дописать снег
    /*static uint32_t lastSnowProcessTime = 0;
    if (pNetGame && pNetGame->GetPlayerPool() && pNetGame->GetPlayerPool()->GetLocalPlayer())
    {
        int32_t currentTime = GetTickCount();
        if (currentTime - lastSnowProcessTime >= 2000)
        {
            CSnow::Process(pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed(), pGame->GetActiveInterior());
            lastSnowProcessTime = currentTime;
        }
    }*/

    if (pNetGame)
    {
        CTextDrawPool* pTextDrawPool = pNetGame->GetTextDrawPool();
        if (pTextDrawPool) {
            pTextDrawPool->SnapshotProcess();
        }
    }

    if (g_pWidgetManager)
    {
        PED_TYPE* pPed = GamePool_FindPlayerPed();
        if (g_pWidgetManager->GetSlotState(WIDGET_CAMERA_CYCLE) && pPed)
        {
            static uint32_t lastTick = GetTickCount();
            bool bPressed = false;
            if (g_pWidgetManager->IsTouched(WIDGET_CAMERA_CYCLE) && GetTickCount() - lastTick >= 250)
            {
                bPressed = true;
                lastTick = GetTickCount();
            }

            if (!CFirstPersonCamera::IsEnabled() && g_bFirstPersonOnFootEnabled)
            {
                CFirstPersonCamera::SetEnabled(true);
            }

            if (CFirstPersonCamera::IsEnabled() && !g_bFirstPersonOnFootEnabled)
            {
                CFirstPersonCamera::SetEnabled(false);
            }

            if (bPressed && !IN_VEHICLE(pPed))
            {
                CFirstPersonCamera::Toggle();
                g_bFirstPersonOnFootEnabled = CFirstPersonCamera::IsEnabled();
            }
        }
    }
    if (pAudioStream) pAudioStream->Process();

    CCustomPlateManager::Process();
}

#include "..//cryptors/AUTOMOBILE_COLLISION_result.h"

uint16_t g_usLastProcessedModelIndexAutomobile = 0;
int g_iLastProcessedModelIndexAutoEnt = 0;
void (*CAutomobile__ProcessEntityCollision)(VEHICLE_TYPE* a1, ENTITY_TYPE* a2, int a3);
void CAutomobile__ProcessEntityCollision_hook(VEHICLE_TYPE *a1, ENTITY_TYPE *a2, int a3) {
    PROTECT_CODE_AUTOMOBILE_COLLISION;

    g_usLastProcessedModelIndexAutomobile = a1->entity.nModelIndex;
    g_iLastProcessedModelIndexAutoEnt = a2->nModelIndex;

    bool bReplace = false;
    void *pOld = nullptr;
    uint8_t *pColData = nullptr;

    if (pNetGame) {
        if (pNetGame->GetVehiclePool()) {
            uint16_t vehId = pNetGame->GetVehiclePool()->FindIDFromGtaPtr(a1);
            CVehicle *pVeh = pNetGame->GetVehiclePool()->GetAt(vehId);
            if (pVeh) {
                if (pVeh->bHasSuspensionLines && pVeh->GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR) {
                    pColData = GetCollisionDataFromModel(a1->entity.nModelIndex);
                    if (pColData && pVeh->m_pSuspensionLines) {
                        if (*(void **) (pColData + 16)) {
                            pOld = *(void **) (pColData + 16);
                            *(void **) (pColData + 16) = pVeh->m_pSuspensionLines;
                            bReplace = true;
                        }
                    }
                }
            }
        }
    }
    CAutomobile__ProcessEntityCollision(a1, a2, a3);
    if (bReplace)
        *(void **) (pColData + 16) = pOld;
}

extern "C" JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_terminateSampLibrary(JNIEnv* env, jobject obj) {
    Log("Exiting game...");
    NOP(g_libGTASA + 0x00341FCC, 2); // nop PauseOpenSLES
    NOP(g_libGTASA + 0x0046389E, 2); // nop saving
    if (pNetGame)
    {
        if (pNetGame->GetRakClient())
        {
            pNetGame->GetRakClient()->Disconnect(500, 0);
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // правильно выходим с джавы чтобы потом не крашило
}

bool (*CGame__Shutdown)();
bool CGame__Shutdown_hook()
{
	Log("Exiting game...");
	NOP(g_libGTASA + 0x00341FCC, 2); // nop PauseOpenSLES
	NOP(g_libGTASA + 0x0046389E, 2); // nop saving
	if (pNetGame)
	{
		if (pNetGame->GetRakClient())
		{
			pNetGame->GetRakClient()->Disconnect(500, 0);
		}
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	return CGame__Shutdown();
}

// TODO: VEHICLE RESET SUSPENSION
void (*CShadows__StoreCarLightShadow)(VEHICLE_TYPE* vehicle, int id, RwTexture* texture, VECTOR* posn, float frontX, float frontY, float sideX, float sideY, unsigned char red, unsigned char green, unsigned char blue, float maxViewAngle);
void CShadows__StoreCarLightShadow_hook(VEHICLE_TYPE* vehicle, int id, RwTexture* texture, VECTOR* posn, float frontX, float frontY, float sideX, float sideY, unsigned char red, unsigned char green, unsigned char blue, float maxViewAngle)
{
	uint8_t r, g, b;
	r = red;
	g = green;
	b = blue;
	if (pNetGame)
	{
		if (pNetGame->GetVehiclePool())
		{
			uint16_t vehid = pNetGame->GetVehiclePool()->FindIDFromGtaPtr(vehicle);
			CVehicle* pVeh = pNetGame->GetVehiclePool()->GetAt(vehid);
			if (pVeh)
			{
				pVeh->ProcessHeadlightsColor(r, g, b);
			}
		}
	}

	return CShadows__StoreCarLightShadow(vehicle, id, texture, posn, frontX, frontY, sideX, sideY, r, g, b, maxViewAngle);
}

void CVehicle__DoHeadLightReflectionTwin(CVehicle* pVeh, MATRIX4X4* a2)
{
	VEHICLE_TYPE* v2; // r4
	int v3; // r2
	MATRIX4X4* v4; // r5
	float* v5; // r3
	float v6; // s12
	float v7; // s5
	float* v8; // r2
	float v9; // r0
	float v10; // r1
	float v11; // r2
	float v12; // s14
	float v13; // s11
	float v14; // s15
	float v15; // s13
	float v16; // s10
	float v17; // s12
	float v18; // s15
	float v19; // ST08_4

	uintptr_t* dwModelarray = (uintptr_t*)(g_libGTASA + 0x87BF48);

	v2 = pVeh->m_pVehicle;
	v3 = *((uintptr_t*)v2 + 5);
	v4 = a2;
	v5 = *(float**)(dwModelarray[v2->entity.nModelIndex] + 116);
	v6 = *v5;
	v7 = v5[1];
	if (v3)
		v8 = (float*)(v3 + 48);
	else
		v8 = (float*)((char*)v2 + 4);
	v9 = *v8;
	v10 = v8[1];
	v11 = v8[2];
	v12 = *((float*)v4 + 5);
	v13 = *((float*)v4 + 4);
	v14 = (float)(v12 * v12) + (float)(v13 * v13);
	if (v14 != 0.0)
		v14 = 1.0 / sqrtf(v14);
	v15 = v6 * 4.0;
	v16 = (float)(v15 + v15) + 1.0;
	v17 = v13 * v14;
	v18 = v12 * v14;

	v19 = v15 * v18;

	VECTOR pos;
	memcpy(&pos, &(v2->entity.mat->pos), sizeof(VECTOR));
	pos.Z += 2.0f;

	CShadows__StoreCarLightShadow(
		v2,
		(uintptr_t)v2 + 24,
		pVeh->m_Shadow.pTexture,
		&pos,
		(float)(v15 + v15) * v17 * pVeh->m_Shadow.fSizeX,
		(float)(v15 + v15) * v18 * pVeh->m_Shadow.fSizeX,
		v19 * pVeh->m_Shadow.fSizeY,
		-(float)(v15 * v17) * pVeh->m_Shadow.fSizeY,
		pVeh->m_Shadow.r, pVeh->m_Shadow.g, pVeh->m_Shadow.b,
		7.0f);
}

void (*CVehicle__DoHeadLightBeam)(VEHICLE_TYPE* vehicle, int arg0, MATRIX4X4& matrix, unsigned char arg2);
void CVehicle__DoHeadLightBeam_hook(VEHICLE_TYPE* vehicle, int arg0, MATRIX4X4& matrix, unsigned char arg2)
{
	uint8_t r, g, b;
	r = 0xFF;
	g = 0xFF;
	b = 0xFF;
	if (pNetGame)
	{
		if (pNetGame->GetVehiclePool())
		{
			uint16_t vehid = pNetGame->GetVehiclePool()->FindIDFromGtaPtr(vehicle);
			CVehicle* pVeh = pNetGame->GetVehiclePool()->GetAt(vehid);
			if (pVeh)
			{
				pVeh->ProcessHeadlightsColor(r, g, b);
			}
		}
	}

	*(uint8_t*)(g_libGTASA + 0x9BAA70) = r;
	*(uint8_t*)(g_libGTASA + 0x9BAA71) = g;
	*(uint8_t*)(g_libGTASA + 0x9BAA72) = b;

	*(uint8_t*)(g_libGTASA + 0x9BAA94) = r;
	*(uint8_t*)(g_libGTASA + 0x9BAA95) = g;
	*(uint8_t*)(g_libGTASA + 0x9BAA96) = b;

	*(uint8_t*)(g_libGTASA + 0x9BAB00) = r;
	*(uint8_t*)(g_libGTASA + 0x9BAB01) = g;
	*(uint8_t*)(g_libGTASA + 0x9BAB02) = b;

	CVehicle__DoHeadLightBeam(vehicle, arg0, matrix, arg2);

}

static CVehicle* g_pLastProcessedVehicleMatrix = nullptr;
static int g_iLastProcessedWheelVehicle = -1;

void (*CMatrix__Rotate)(void* thiz, float a1, float a2, float a3);
void CMatrix__Rotate_hook(void* thiz, float a1, float a2, float a3)
{
	uintptr_t dwRetAddr = 0;
	__asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));
	dwRetAddr -= g_libGTASA;

	if (dwRetAddr == 0x003A9D76 + 1)
	{
		CMatrix__Rotate(thiz, a1, a2, a3);
		return;
	}

	CMatrix__Rotate(thiz, a1, a2, a3);
	if (g_pLastProcessedVehicleMatrix && g_iLastProcessedWheelVehicle != -1)
	{
		if (g_pLastProcessedVehicleMatrix->m_bWheelAlignmentX || g_pLastProcessedVehicleMatrix->m_bWheelAlignmentY)
		{
			if (g_iLastProcessedWheelVehicle == 2)
			{
				((void(*)(void*, float))(g_libGTASA + 0x003E8BE4 + 1))(thiz, 0.0f - g_pLastProcessedVehicleMatrix->m_fWheelAlignmentX); // CMatrix::RotateY
			}
			if (g_iLastProcessedWheelVehicle == 4)
			{
				((void(*)(void*, float))(g_libGTASA + 0x003E8BE4 + 1))(thiz, 0.0f - g_pLastProcessedVehicleMatrix->m_fWheelAlignmentY); // CMatrix::RotateY
			}
			if (g_iLastProcessedWheelVehicle == 5)
			{
				((void(*)(void*, float))(g_libGTASA + 0x003E8BE4 + 1))(thiz, g_pLastProcessedVehicleMatrix->m_fWheelAlignmentX); // CMatrix::RotateY
			}
			if (g_iLastProcessedWheelVehicle == 7)
			{
				((void(*)(void*, float))(g_libGTASA + 0x003E8BE4 + 1))(thiz, g_pLastProcessedVehicleMatrix->m_fWheelAlignmentY); // CMatrix::RotateY
			}
		}
	}
}

void (*CMatrix__SetScale)(void* thiz, float x, float y, float z);
void CMatrix__SetScale_hook(void* thiz, float x, float y, float z)
{
	if (g_pLastProcessedVehicleMatrix && g_iLastProcessedWheelVehicle != -1)
	{
		if (g_iLastProcessedWheelVehicle >= 2 || g_iLastProcessedWheelVehicle <= 7)
		{
			// front wheel
			if (g_pLastProcessedVehicleMatrix->m_bWheelSize)
			{
				y *= g_pLastProcessedVehicleMatrix->m_fWheelSize * 1.3f; // ���� ��� �������� scale �� ������
				z *= g_pLastProcessedVehicleMatrix->m_fWheelSize * 1.3f;
			}
			if (g_pLastProcessedVehicleMatrix->m_bWheelWidth)
			{
				x = g_pLastProcessedVehicleMatrix->m_fWheelWidth;
			}
		}
	}

	CMatrix__SetScale(thiz, x, y, z);
}

void (*CAutomobile__UpdateWheelMatrix)(VEHICLE_TYPE* thiz, int, int);
void CAutomobile__UpdateWheelMatrix_hook(VEHICLE_TYPE* thiz, int nodeIndex, int flags)
{
	if (g_pLastProcessedVehicleMatrix)
	{
		g_iLastProcessedWheelVehicle = nodeIndex;
	}

	CAutomobile__UpdateWheelMatrix(thiz, nodeIndex, flags);
}

void (*CAutomobile__PreRender)(VEHICLE_TYPE* thiz);
void CAutomobile__PreRender_hook(VEHICLE_TYPE* thiz)
{
	uintptr_t* dwModelarray = (uintptr_t*)(g_libGTASA + 0x87BF48);
	uint8_t* pModelInfoStart = (uint8_t*)dwModelarray[thiz->entity.nModelIndex];

	float fOldFront = *(float*)(pModelInfoStart + 88);
	float fOldRear = *(float*)(pModelInfoStart + 92);
	CVehicle* pVeh = nullptr;
	if (pNetGame)
	{
		if (pNetGame->GetVehiclePool())
		{
			uint16_t vehid = pNetGame->GetVehiclePool()->FindIDFromGtaPtr(thiz);
			pVeh = pNetGame->GetVehiclePool()->GetAt(vehid);
			if (pVeh)
			{
				pVeh->ProcessWheelsOffset();
				g_pLastProcessedVehicleMatrix = pVeh;

				if (pVeh->m_bWheelSize)
				{
					*(float*)(pModelInfoStart + 92) = pVeh->m_fWheelSize;
					*(float*)(pModelInfoStart + 88) = pVeh->m_fWheelSize;
				}
				if (pVeh->m_bShadow && pVeh->m_Shadow.pTexture)
				{
					CVehicle__DoHeadLightReflectionTwin(pVeh, pVeh->m_pVehicle->entity.mat);
				}
			}
		}
	}

	CAutomobile__PreRender(thiz);

	*(float*)(pModelInfoStart + 88) = fOldFront;
	*(float*)(pModelInfoStart + 92) = fOldRear;

	g_pLastProcessedVehicleMatrix = nullptr;
	g_iLastProcessedWheelVehicle = -1;
}

#include "..//cryptors/INSTALLHOOKS_result.h"
#include "../gtasa/ConfigReader.h"
#include "../gtasa/GLES.h"

void (*CTaskSimpleUseGun__RemoveStanceAnims)(void* thiz, void* ped, float a3);
void CTaskSimpleUseGun__RemoveStanceAnims_hook(void* thiz, void* ped, float a3)
{
	uint32_t v5 = *((uint32_t*)thiz + 11);
	if (v5)
	{
		if (*(uint32_t*)(v5 + 20))
		{
			CTaskSimpleUseGun__RemoveStanceAnims(thiz, ped, a3);
			return;
		}
		else
		{
			return;
		}
	}
	else
	{
		CTaskSimpleUseGun__RemoveStanceAnims(thiz, ped, a3);
	}
}

float (*CRadar__LimitRadarPoint)(float* a1);
float CRadar__LimitRadarPoint_hook(float* a1)
{
	if (*(uint8_t*)(g_libGTASA + 0x0063E0B4))
	{
		return sqrtf((float)(a1[1] * a1[1]) + (float)(*a1 * *a1));
	}

	if (!CRadarRect::IsEnabled())
	{
		return CRadar__LimitRadarPoint(a1);
	}
	float value = CRadarRect::CRadar__LimitRadarPoint_hook(a1);

	if (pChatWindow)
	{
		//pChatWindow->AddDebugMessage("VALUE %f", value);
	}

	return value;
}

void (*CSprite2d__DrawBarChart)(float x, float y, unsigned short width, unsigned char height, float progress, signed char progressAdd,
	unsigned char drawPercentage, unsigned char drawBlackBorder, CRGBA* color, CRGBA* addColor);
void CSprite2d__DrawBarChart_hook(float x, float y, unsigned short width, unsigned char height, float progress, signed char progressAdd,
	unsigned char drawPercentage, unsigned char drawBlackBorder, CRGBA* color, CRGBA* addColor)
{
	uintptr_t dwRetAddr = 0;
	__asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));
	dwRetAddr -= g_libGTASA;

	float fX = x;
	float fY = y;

	unsigned short usWidth = width;
	unsigned char usHeight = height;

	if (dwRetAddr == 0x0027D524 + 1)
	{
		if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_HP).X != -1)
		{
			fX = pGUI->ScaleX(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_HP).X);
		}
		if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_HP).Y != -1)
		{
			fY = pGUI->ScaleY(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_HP).Y);
		}

		if (CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_HP).X != -1)
		{
			usWidth = pGUI->ScaleX(CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_HP).X);
		}
		if (CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_HP).Y != -1)
		{
			usHeight = pGUI->ScaleY(CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_HP).Y);
		}

		if (CAdjustableHudColors::IsUsingHudColor(E_HUD_ELEMENT::HUD_HP))
		{
			color->A = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_HP).A;
			color->R = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_HP).R;
			color->G = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_HP).G;
			color->B = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_HP).B;
		}
		color->A = 0;
		color->R = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_HP).R;
		color->G = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_HP).G;
		color->B = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_HP).B;
	}
	else if (dwRetAddr == 0x0027D83E + 1)
	{
		if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_ARMOR).X != -1)
		{
			fX = pGUI->ScaleX(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_ARMOR).X);
		}
		if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_ARMOR).Y != -1)
		{
			fY = pGUI->ScaleY(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_ARMOR).Y);
		}

		if (CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_ARMOR).X != -1)
		{
			usWidth = pGUI->ScaleX(CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_ARMOR).X);
		}
		if (CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_ARMOR).Y != -1)
		{
			usHeight = pGUI->ScaleY(CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_ARMOR).Y);
		}

		if (CAdjustableHudColors::IsUsingHudColor(E_HUD_ELEMENT::HUD_ARMOR))
		{
			color->A = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_ARMOR).A;
			color->R = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_ARMOR).R;
			color->G = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_ARMOR).G;
			color->B = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_ARMOR).B;
		}
		color->A = 0;
		color->R = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_ARMOR).R;
		color->G = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_ARMOR).G;
		color->B = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_ARMOR).B;
	}
	CSprite2d__DrawBarChart(fX, fY, usWidth, usHeight, progress, progressAdd, drawPercentage, drawBlackBorder, color, addColor);
}
static int g_iCurrentWanted = 0;
static float g_fInitialPos = 0.0f;
void (*CWidgetPlayerInfo__DrawWanted)(void*);
void CWidgetPlayerInfo__DrawWanted_hook(void* thiz)
{
	g_iCurrentWanted = 0;
	g_fInitialPos = *((float*)thiz + 10);
	CWidgetPlayerInfo__DrawWanted(thiz);
	g_iCurrentWanted = 0;
}

void (*CFont__PrintString)(float x, float y, uint16_t* text);
void CFont__PrintString_hook(float x, float y, uint16_t* text)
{
	uintptr_t dwRetAddr = 0;
	__asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));
	dwRetAddr -= g_libGTASA;

	float fX = x;
	float fY = y;

	if (dwRetAddr == 0x0027E15C + 1) // money
	{
		if (CAdjustableHudColors::IsUsingHudColor(E_HUD_ELEMENT::HUD_MONEY))
		{
			CRGBA col = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_MONEY);
			uint32_t m1 = col.ToInt();
			CFont::SetColor(&m1);
		}

		if (CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_MONEY).X != -1)
		{
			float value = (float)CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_MONEY).X / 100.0f;
			CFont::SetScale(value, 0.0f);
		}

		if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_MONEY).X != -1)
		{
			fX = pGUI->ScaleX(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_MONEY).X);
		}
		if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_MONEY).Y != -1)
		{
			fY = pGUI->ScaleY(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_MONEY).Y);
		}
	}
	else if (dwRetAddr == 0x0027D9E6 + 1) // wanted
	{
		if (CAdjustableHudColors::IsUsingHudColor(E_HUD_ELEMENT::HUD_WANTED))
		{
			CRGBA col = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_WANTED);
			uint32_t m1 = col.ToInt();
			CFont::SetColor(&m1);
		}

		if (CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_WANTED).X != -1)
		{
			float value = (float)CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_WANTED).X / 100.0f;
			CFont::SetScale(value, 0.0f);
		}
		
		if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_WANTED).X != -1)
		{
			fX -= g_fInitialPos;
			fX += pGUI->ScaleX(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_WANTED).X);
		}
		if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_WANTED).Y != -1)
		{
			fY = pGUI->ScaleY(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_WANTED).Y);
		}
		g_iCurrentWanted++;

	}
	else if (dwRetAddr == 0x0027D330 + 1) // ammo text
	{
		if (CAdjustableHudColors::IsUsingHudColor(E_HUD_ELEMENT::HUD_AMMO))
		{
			CRGBA col = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_AMMO);
			uint32_t m1 = col.ToInt();
			CFont::SetColor(&m1);
		}

		if (CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_AMMO).X != -1)
		{
			float value = (float)CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_AMMO).X / 100.0f;
			CFont::SetScale(value, 0.0f);
		}

		if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_AMMO).X != -1)
		{
			fX = pGUI->ScaleX(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_AMMO).X);
		}
		if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_AMMO).Y != -1)
		{
			fY = pGUI->ScaleY(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_AMMO).Y);
		}
	}
	CFont__PrintString(fX, fY, text);
}

/*void (*CSprite2d__Draw)(CSprite2d* a1, CRect* a2, CRGBA* a3);
void CSprite2d__Draw_hook(CSprite2d* a1, CRect* a2, CRGBA* a3)
{
	uintptr_t dwRetAddr = 0;
	__asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));
	dwRetAddr -= g_libGTASA;

	if (!pGUI)
	{
		return CSprite2d__Draw(a1, a2, a3);
	}

	if (dwRetAddr == 0x003D3796 + 1 || dwRetAddr == 0x003D376C + 1 || dwRetAddr == 0x003D373E + 1 || dwRetAddr == 0x003D3710 + 1)
	{
		if (CRadarRect::m_pDiscTexture == nullptr)
		{
			CRadarRect::m_pDiscTexture = a1->m_pRwTexture;
		}
		if (CRadarRect::IsEnabled() && CRadarRect::m_pRectTexture)
		{
			a1->m_pRwTexture = CRadarRect::m_pRectTexture;
			//pChatWindow->AddDebugMessage("Replacing for rect");
		}
		else
		{
			a1->m_pRwTexture = CRadarRect::m_pDiscTexture;
			//pChatWindow->AddDebugMessage("Replacing for disc");
		}

		if (CAdjustableHudColors::IsUsingHudColor(E_HUD_ELEMENT::HUD_RADAR))
		{
			CRGBA col = CAdjustableHudColors::GetHudColor(E_HUD_ELEMENT::HUD_RADAR);
			a3->A = col.A;
			a3->B = col.B;
			a3->G = col.G;
			a3->R = col.R;
		}
		float* thiz = (float*) * (uintptr_t*)(g_libGTASA + 0x6580C8);
		if (thiz)
		{
			if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_RADAR).X != -1)
			{
				thiz[3] = (float)CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_RADAR).X;
			}
			else
			{
				thiz[3] = 50.0f;
			}

			if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_RADAR).Y != -1)
			{
				thiz[4] = (float)CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_RADAR).Y;
			}
			else
			{
				thiz[4] = 70.0f;
			}

			if (CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_RADAR).X != -1)
			{
				thiz[5] = 45.0f * ((float)CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_RADAR).X / 100.0f);
			}
			else
			{
				thiz[5] = 45.0f;
			}

			if (CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_RADAR).X != -1)
			{
				thiz[6] = 45.0f * ((float)CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_RADAR).X / 100.0f);
			}
			else
			{
				thiz[6] = 45.0f;
			}
		}
		
	}

	return CSprite2d__Draw(a1, a2, a3);
}*/

void ShowRadarTexture() {
    if (!CHUD::radar) {
        CHUD::radar = new CSprite2d();
        CHUD::radar->m_pRwTexture = (RwTexture*)LoadTextureFromDB("samp", "hud_back");
    }
    CRGBA color(255, 255, 255, 255);
    CHUD::radar->Draw(CHUD::radarBgPos1.x1, CHUD::radarBgPos1.y1, CHUD::radarBgPos2.x1, CHUD::radarBgPos2.y1, color);
}

void ShowLogoTexture() {
    if (!CHUD::logo) {
        CHUD::logo = new CSprite2d();
        CHUD::logo->m_pRwTexture = (RwTexture*)LoadTextureFromDB("samp", "logo");
    }
    float textureWidth = 100.0f;
    float textureHeight = 50.0f;
    CRGBA color(255, 255, 255, 255);
    CHUD::logo->Draw(CHUD::logoPos.x1, CHUD::logoPos.y1, textureWidth, textureHeight, color);
}

void(*CHud__Draw)();
void CHud__Draw_hook() {
    if (CHUD::IsEnabled() && CHUD::m_bShow) {
        ShowRadarTexture();
        //ShowLogoTexture();
    }
    CHud__Draw();
}

/*void DrawRadarTexture(float x, float y, float size1, float size2)
{
    CSprite2d* radar = new CSprite2d();
    radar->m_pRwTexture = (RwTexture*)LoadTextureFromDB("samp", "hud_back");

    CRGBA color;
    color.A = 255;
    color.R = 255;
    color.G = 255;
    color.B = 255;

    radar->Draw(x, y, size1, size2, color);
}

void(*CHud__Draw)();
void CHud__Draw_hook()
{
    if(CHUD::IsEnabled()) DrawRadarTexture(CHUD::radarBgPos1.x1, CHUD::radarBgPos1.y1, CHUD::radarBgPos2.x1, CHUD::radarBgPos2.y1);
    CHud__Draw();
}*/

void (*CSprite2d__Draw)(CSprite2d* a1, CRect* a2, CRGBA* a3);
void CSprite2d__Draw_hook(CSprite2d* a1, CRect* a2, CRGBA* a3)
{
    uintptr_t dwRetAddr = 0;
    __asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));
    dwRetAddr -= g_libGTASA;

    if (!pGUI)
    {
        return CSprite2d__Draw(a1, a2, a3);
    }

    if (dwRetAddr == 0x003D3796 + 1 || dwRetAddr == 0x003D376C + 1 || dwRetAddr == 0x003D373E + 1 || dwRetAddr == 0x003D3710 + 1)
    {
        float* thiz = (float*) * (uintptr_t*)(g_libGTASA + 0x6580C8);
        if (thiz)
        {
            if(CHUD::IsEnabled())
            {
                CRect* rect = (CRect*)&thiz[3];
                CHUD::EditRadarBios(rect);
            }
        }

    }
    return CSprite2d__Draw(a1, a2, a3);
}

void (*CWidgetPlayerInfo__DrawWeaponIcon)(float* thiz, void* ped, CRect rect, float a4);
void CWidgetPlayerInfo__DrawWeaponIcon_hook(float* thiz, void* ped, CRect rect, float a4)
{
	float diffX = rect.right - rect.left;
	float diffY = rect.bottom - rect.top;
	if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_FIST).X != -1)
	{
		rect.left = pGUI->ScaleX(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_FIST).X);
		rect.right = pGUI->ScaleX(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_FIST).X) + diffX;

		thiz[38] = rect.left;
		thiz[40] = rect.right;
	}
	if (CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_FIST).Y != -1)
	{
		rect.top = pGUI->ScaleY(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_FIST).Y);
		rect.bottom = pGUI->ScaleY(CAdjustableHudPosition::GetElementPosition(E_HUD_ELEMENT::HUD_FIST).Y) + diffY;

		thiz[39] = rect.bottom;
		thiz[41] = rect.top;
	}

	if (CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_FIST).X != -1)
	{
		float coef = (float)CAdjustableHudScale::GetElementScale(E_HUD_ELEMENT::HUD_FIST).X / 100.0f;
		float diffX = rect.right - rect.left;
		float diffY = rect.bottom - rect.top;
		diffX *= coef;
		diffY *= coef;

		rect.right = rect.left + diffX;
		rect.bottom = rect.top + diffY;
	}

	return CWidgetPlayerInfo__DrawWeaponIcon(thiz, ped, rect, a4);
}

/*void(*CHud__Draw)();
void CHud__Draw_hook()
{
	float* thiz = (float*) * (uintptr_t*)(g_libGTASA + 0x6580C8);
	if (thiz)
	{
		thiz[5] = 45.0f;
		thiz[6] = 45.0f;
	}

	CHud__Draw();
}*/

void (*CCam__Process)(uintptr_t);
void CCam__Process_hook(uintptr_t thiz)
{

	if (pChatWindow)
	{
		//pChatWindow->AddDebugMessage("time %d %d %d", *(uint32_t*)(g_libGTASA + 0x008B0808 + 116), *(uint32_t*)(g_libGTASA + 0x008B0808 + 120), *(uint32_t*)(g_libGTASA + 0x008B0808 + 124));
		//pChatWindow->AddDebugMessage("camera %d %f %f", *(uint16_t*)(thiz + 14), *(float*)(thiz + 132), *(float*)(thiz + 148)); // 140 - fov, 132 - vertical, 148 - horizontal angle
	}

	VECTOR vecSpeed;
	CVehicle* pVeh = nullptr;
	float pOld = *(float*)(g_libGTASA + 0x00608558);
	if (pNetGame && (*(uint16_t*)(thiz + 14) == 18 || *(uint16_t*)(thiz + 14) == 16) && CFirstPersonCamera::IsEnabled())
	{
		if (pNetGame->GetPlayerPool())
		{
			if (pNetGame->GetPlayerPool()->GetLocalPlayer())
			{
				CPlayerPed* pPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
				pVeh = pNetGame->GetVehiclePool()->GetAt(pNetGame->GetPlayerPool()->GetLocalPlayer()->m_CurrentVehicle);
				if (pVeh)
				{
					pVeh->GetMoveSpeedVector(&vecSpeed);
					VECTOR vec;
					vec.X = vecSpeed.X * 6.0f;
					vec.Y = vecSpeed.Y * 6.0f;
					vec.Z = vecSpeed.Z * 6.0f;
					pVeh->SetMoveSpeedVector(vec);
					*(float*)(g_libGTASA + 0x00608558) = 200.0f;
				}
			}
		}
	}

	CCam__Process(thiz);
	if (pVeh)
	{
		pVeh->SetMoveSpeedVector(vecSpeed);
		*(float*)(g_libGTASA + 0x00608558) = pOld;
	}
	if (*(uint16_t*)(thiz + 14) == 4 || *(uint16_t*)(thiz + 14) == 53) // 53 is weapon
	{
		if (pNetGame)
		{
			if (pNetGame->GetPlayerPool())
			{
				if (pNetGame->GetPlayerPool()->GetLocalPlayer())
				{
					CPlayerPed* pPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
					if (pPed)
					{
						*(uint32_t*)(g_libGTASA + 0x008B0808 + 120) = 0xFFFFFFFF;
						*(uint32_t*)(g_libGTASA + 0x008B0808 + 124) = 0xFFFFFFFF;
						*(uint8_t*)(g_libGTASA + 0x008B0808 + 40) = 0;
						CFirstPersonCamera::ProcessCameraOnFoot(thiz, pPed);
					}
				}
			}
		}
	}
	if(* (uint16_t*)(thiz + 14) == 18 || *(uint16_t*)(thiz + 14) == 16) // in car
	{
		if (pNetGame)
		{
			if (pNetGame->GetPlayerPool())
			{
				if (pNetGame->GetPlayerPool()->GetLocalPlayer())
				{
					CPlayerPed* pPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
					if (pPed)
					{
						*(uint32_t*)(g_libGTASA + 0x008B0808 + 120) = 0xFFFFFFFF;
						*(uint32_t*)(g_libGTASA + 0x008B0808 + 124) = 0xFFFFFFFF;
						*(uint8_t*)(g_libGTASA + 0x008B0808 + 40) = 0;
						CFirstPersonCamera::ProcessCameraInVeh(thiz, pPed, pVeh);
					}
				}
			}
		}
	}
}
int g_iCounterVehicleCamera = 0;
int (*CPad__CycleCameraModeDownJustDown)(void*);
int CPad__CycleCameraModeDownJustDown_hook(void* thiz)
{
	/*if (!g_pWidgetManager)
	{
		return 0;
	}
	if (!g_pWidgetManager->GetSlotState(WIDGET_CAMERA_CYCLE))
	{
		return 0;
	}
	PED_TYPE* pPed = GamePool_FindPlayerPed();
	if (!pPed)
	{
		return 0;
	}

	static uint32_t lastTick = GetTickCount();
	bool bPressed = false;
	if (g_pWidgetManager->IsTouched(WIDGET_CAMERA_CYCLE) && GetTickCount() - lastTick >= 250)
	{
		bPressed = true;
		lastTick = GetTickCount();
	}

	if (IN_VEHICLE(pPed))
	{
		if (bPressed)
		{
			g_iCounterVehicleCamera++;
		}
		if (g_iCounterVehicleCamera == 6)
		{
			CFirstPersonCamera::SetEnabled(true);
			return 0;
		}
		else if (g_iCounterVehicleCamera >= 7)
		{
			g_iCounterVehicleCamera = 0;
			CFirstPersonCamera::SetEnabled(false);
			return 1;
			
		}
		else
		{
			CFirstPersonCamera::SetEnabled(false);
		}

		return bPressed;
	}*/
	return 0;
}

int (*CEntity__GetIsOnScreen)(ENTITY_TYPE*);
int CEntity__GetIsOnScreen_hook(ENTITY_TYPE* thiz)
{
	int retn = CEntity__GetIsOnScreen(thiz);
	
	/*if (Skybox::m_pSkyObjectDaily)
	{
		if (thiz == Skybox::m_pSkyObjectAfternoon->m_pEntity || thiz == Skybox::m_pSkyObjectDaily->m_pEntity ||
			thiz == Skybox::m_pSkyObjectEvening->m_pEntity || thiz == Skybox::m_pSkyObjectNight->m_pEntity)
		{
			return 1;
		}
	}*/

	return retn;
}

void (*FxEmitterBP_c__Render)(uintptr_t* a1, int a2, int a3, float a4, char a5);
void FxEmitterBP_c__Render_hook(uintptr_t* a1, int a2, int a3, float a4, char a5)
{
	uintptr_t* temp = *((uintptr_t**)a1 + 3);
	if (!temp)
	{
		return;
	}
	FxEmitterBP_c__Render(a1, a2, a3, a4, a5);
}

void(*CStreaming__RemoveModel)(int);
void CStreaming__RemoveModel_hook(int model)
{
	Log("Removing model %d", model);
	CStreaming__RemoveModel(model);
}

int g_iLastProcessedSkinCollision = 228;
int g_iLastProcessedEntityCollision = 228;

void (*CPed__ProcessEntityCollision)(PED_TYPE* thiz, ENTITY_TYPE* ent, void* colPoint);
void CPed__ProcessEntityCollision_hook(PED_TYPE* thiz, ENTITY_TYPE* ent, void* colPoint)
{
	g_iLastProcessedSkinCollision = thiz->entity.nModelIndex;
	g_iLastProcessedEntityCollision = ent->nModelIndex;

	CPed__ProcessEntityCollision(thiz, ent, colPoint);
}

void (*CPause__Pause)();
void CPause__Pause_hook()
{
    Log("Pause Event");
}

/*bool (*CAutomobile__BurstTyre)(uintptr_t thiz, int a2, int a3);
bool CAutomobile__BurstTyre_hook(uintptr_t thiz, int a2, int a3) {
    if (pNetGame) {
        // Did we get ped who fired?
        if (g_pCurrentFiredPed) {
            // Trying to get vehicle pointer
            CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
            if (pVehiclePool) {
                VEHICLEID vehId = pVehiclePool->FindIDFromGtaPtr((VEHICLE_TYPE *) thiz);
                if (vehId != INVALID_VEHICLE_ID) {
                    CVehicle *pVehicle = pVehiclePool->GetAt(vehId);
                    if (pVehicle)
                        pVehicle->ProcessDamage();
                }
            }
        }
    }
    return CAutomobile__BurstTyre(thiz, a2, a3);
}*/

int (*CVehicle__UsesSiren)(uintptr_t thiz);
int CVehicle__UsesSiren_hook(uintptr_t thiz) {
    uintptr_t dwRetAddr = 0;
    __asm__ volatile ("mov %0, lr" : "=r" (dwRetAddr));

    dwRetAddr -= g_libGTASA;
    if (dwRetAddr == 0x4E53E0 + 1 ||
        dwRetAddr == 0x35BFE4 + 1 ||
        dwRetAddr == 0x518920 + 1) {
        if (pNetGame) {
            VEHICLE_TYPE *pGtaVehicle = (VEHICLE_TYPE *) thiz;
            if (pGtaVehicle) {
                CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
                if (pVehiclePool) {
                    VEHICLEID vehId = pVehiclePool->FindIDFromGtaPtr(pGtaVehicle);
                    if (vehId != INVALID_VEHICLE_ID) {
                        if (pVehiclePool->GetSlotState(vehId)) {
                            CVehicle *pVehicle = pVehiclePool->GetAt(vehId);
                            if (pVehicle) {
                                if (pVehicle->IsSirenAdded())
                                    return 1;
                            }
                        }
                    }
                }
            }
        }
    }

    return CVehicle__UsesSiren(thiz);
}

int (*CAEVehicleAudioEntity__JustFinishedAccelerationLoop)(uintptr_t* thiz);
int CAEVehicleAudioEntity__JustFinishedAccelerationLoop_hook(uintptr_t* thiz){
    CVehicle *pVehicle = nullptr;
    CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
    CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
    VEHICLEID id = pVehiclePool->FindIDFromGtaPtr(pPlayerPed->GetGtaVehicle());
    pVehicle = pVehiclePool->GetAt(id);
    pVehicle->CreateExhaust(nullptr);
    return CAEVehicleAudioEntity__JustFinishedAccelerationLoop(thiz); // нада фиксить, краши из за выхлопа
}

int (*_RwTextureDestroy)(RwTexture *);
int _RwTextureDestroy_hook(RwTexture *texture) {
    if (!texture)
        return 1;

    return _RwTextureDestroy(texture);
}

uintptr_t (*GetMeshPriority)(RpMesh const *);
uintptr_t GetMeshPriority_hook(RpMesh const *rpMesh) {
    if (rpMesh) {
        RpMaterial *rpMeshMat = rpMesh->material;
        if (rpMeshMat) {
            if (rpMeshMat->texture)
                if (!rpMeshMat->texture->raster) return 0;
        }
    }

    return GetMeshPriority(rpMesh);
}

uint32_t
(*CCollision__ProcessVerticalLine)(float *colLine, float *transform, int colModel, int colPoint, int *maxTouchDistance,
                                   char seeThrough, int shootThrough, int storedCollPoly);

uint32_t CCollision__ProcessVerticalLine_hook(float *colLine, float *transform, int colModel, int colPoint,
                                              int *maxTouchDistance, char seeThrough, int shootThrough,
                                              int storedCollPoly) {
    if (colModel)
        return CCollision__ProcessVerticalLine(colLine, transform, colModel, colPoint, maxTouchDistance, seeThrough,
                                               shootThrough, storedCollPoly);

    return 0;
}

#include "../CDebugInfo.h"
float (*_Diag_GetFPS)(void);
float Diag_GetFPS_hook(void) {
    float fps = _Diag_GetFPS();
    CDebugInfo::FPS = fps; // кастом фпс
    return fps;
}

int(*mpg123_param)(void* mh, int key, long val, int ZERO, double fval);
int mpg123_param_hook(void* mh, int key, long val, int ZERO, double fval)
{
    // 0x2000 = MPG123_SKIP_ID3V2
    // 0x200  = MPG123_FUZZY
    // 0x100  = MPG123_SEEKBUFFER
    // 0x40   = MPG123_GAPLESS
    return mpg123_param(mh, key, val | (0x2000 | 0x200 | 0x100 | 0x40), ZERO, fval); // хук на флаги для mpg123
}

int(*CdStreamOpen)(void *thiz, const char *a2, void **thiz1);
int CdStreamOpen_hook(void *thiz, const char *a2, void **thiz1) {
    const char* lodsd = a2;
    Log("Open CD: %s", lodsd);
    return CdStreamOpen(thiz, a2, thiz1);
}

void InstallHooks()
{
	PROTECT_CODE_INSTALLHOOKS;

    CHook::InlineHook(g_libGTASA, 0x00281398, &CWidgetRegionLook__Update_hook, &CWidgetRegionLook__Update);
    CHook::InlineHook(g_libGTASA, 0x3D7CA8, &CLoadingScreen_DisplayPCScreen_hook, &CLoadingScreen_DisplayPCScreen);
    CHook::InlineHook(g_libGTASA, 0x39AEF4, &Render2dStuff_hook, &Render2dStuff);
    CHook::InlineHook(g_libGTASA, 0x39B098, &Render2dStuffAfterFade_hook, &Render2dStuffAfterFade);
    CHook::InlineHook(g_libGTASA, 0x239D5C, &TouchEvent_hook, &TouchEvent);
    //CHook::InlineHook(g_libGTASA, 0x28E83C, &CStreaming_InitImageList_hook, &CStreaming_InitImageList);
    CHook::InlineHook(g_libGTASA, 0x336690, &CModelInfo_AddPedModel_hook, &CModelInfo_AddPedModel); // hook is dangerous
    CHook::InlineHook(g_libGTASA, 0x3DBA88, &CRadar__GetRadarTraceColor_hook, &CRadar__GetRadarTraceColor); // dangerous
    CHook::InlineHook(g_libGTASA, 0x3DAF84, &CRadar__SetCoordBlip_hook, &CRadar__SetCoordBlip);
    CHook::InlineHook(g_libGTASA, 0x3DE9A8, &CRadar__DrawRadarGangOverlay_hook, &CRadar__DrawRadarGangOverlay);

    CHook::InlineHook(g_libGTASA, 0x1FEAB4, &mpg123_param_hook, &mpg123_param);

    // -- Crash fix
    CHook::InlineHook(g_libGTASA, 0x29947C, &CCollision__ProcessVerticalLine_hook, &CCollision__ProcessVerticalLine);

    CHook::InlineHook(g_libGTASA, 0x482E60, &CTaskComplexEnterCarAsDriver_hook, &CTaskComplexEnterCarAsDriver);
    CHook::InlineHook(g_libGTASA, 0x4833CC, &CTaskComplexLeaveCar_hook, &CTaskComplexLeaveCar);

    CodeInject(SA_ADDR(0x2D99F4), (uintptr_t) PickupPickUp_hook, 1); // fix

    CHook::InlineHook(g_libGTASA, 0x00327528, &ComputeDamageResponse_hooked, &ComputeDamageResponse);
    CHook::InlineHook(g_libGTASA, 0x00336268, &CModelInfo_AddAtomicModel_hook, &CModelInfo_AddAtomicModel);
    CHook::InlineHook(g_libGTASA, 0x00336618, &CModelInfo_AddVehicleModel_hook, &CModelInfo_AddVehicleModel); // dangerous
    CHook::InlineHook(g_libGTASA, 0x0033DA5C, &CAnimManager__UncompressAnimation_hook, &CAnimManager__UncompressAnimation);
    CHook::InlineHook(g_libGTASA, 0x00531118, &CCustomRoadsignMgr__RenderRoadsignAtomic_hook, &CCustomRoadsignMgr__RenderRoadsignAtomic);
    CHook::InlineHook(g_libGTASA, 0x001AECC0, &RwFrameAddChild_hook, &RwFrameAddChild);
    CHook::InlineHook(g_libGTASA, 0x002DFD30, &CUpsideDownCarCheck__IsCarUpsideDown_hook, &CUpsideDownCarCheck__IsCarUpsideDown);
    CHook::InlineHook(g_libGTASA, 0x0033AD78, &CAnimBlendNode__FindKeyFrame_hook, &CAnimBlendNode__FindKeyFrame);

    //CHook::InlineHook(g_libGTASA, 0x0035C7A4, &CAEVehicleAudioEntity__JustFinishedAccelerationLoop_hook, &CAEVehicleAudioEntity__JustFinishedAccelerationLoop); // shotcar
    //CHook::InlineHook(g_libGTASA, 0x510B08, &CVehicle__UsesSiren_hook, &CVehicle__UsesSiren); // siren

    //CHook::InlineHook(g_libGTASA, 0x5195C8, &CVehicle__DoTailLightEffect_hook, &CVehicle__DoTailLightEffect);

    // CHook::InlineHook(g_libGTASA, 0x23900C, &CdStreamOpen_hook, &CdStreamOpen);

    CHook::InlineHook(g_libGTASA, 0x39A054, &Diag_GetFPS_hook, &_Diag_GetFPS);

    // TextDraw render
    CHook::InlineHook(g_libGTASA, 0x003D5894, &CHud__DrawScriptText_hook, &CHud__DrawScriptText);

    // widgets
    CHook::InlineHook(g_libGTASA, 0x00276510, &CWidgetButtonEnterCar_Draw_hook, &CWidgetButtonEnterCar_Draw);

    // attached objects
    CHook::InlineHook(g_libGTASA, 0x003C1BF8, &CWorld_ProcessPedsAfterPreRender_hook, &CWorld_ProcessPedsAfterPreRender);

    // remove building
    CHook::InlineHook(g_libGTASA, 0x00395994, &CFileLoader__LoadObjectInstance_hook, &CFileLoader__LoadObjectInstance);

    // retexture
    CHook::InlineHook(g_libGTASA, 0x391E20, &CEntity__Render_hook, &CEntity__Render);
    CHook::InlineHook(g_libGTASA, 0x3EF518, &CObject__Render_hook, &CObject__Render);

    // gettexture fix crash
    CHook::InlineHook(g_libGTASA, 0x00258910, &GetTexture_hook, &GetTexture_orig);

    // steal objects fix
    CHook::InlineHook(g_libGTASA, 0x003AC114, &CPlayerInfo__FindObjectToSteal_hook, &CPlayerInfo__FindObjectToSteal);

    // GetFrameFromID fix
    CHook::InlineHook(g_libGTASA, 0x00335CC0, &CClumpModelInfo_GetFrameFromId_hook, &CClumpModelInfo_GetFrameFromId);

    // RLEDecompress fix
    CHook::InlineHook(g_libGTASA, 0x1BC314, &RLEDecompress_hook, &RLEDecompress);

    CHook::InlineHook(g_libGTASA, 0x01BDCC4, &LoadFullTexture_hook, &LoadFullTexture); // loadtext

    // random crash fix
    CHook::InlineHook(g_libGTASA, 0x1A8530, &RenderQueue__ProcessCommand_hook, &RenderQueue__ProcessCommand);

    // fix
    CHook::InlineHook(g_libGTASA, 0x1B9D74, &_rwFreeListFreeReal_hook, &_rwFreeListFreeReal1);

    CHook::InlineHook(g_libGTASA, 0x4052B8, &CVehicleModelInfo__SetupCommonData_hook, &CVehicleModelInfo__SetupCommonData);

    CHook::InlineHook(g_libGTASA, 0x35BE30, &CAEVehicleAudioEntity__GetVehicleAudioSettings_hook, &CAEVehicleAudioEntity__GetVehicleAudioSettings);

    CHook::InlineHook(g_libGTASA, 0x338CBC, &CVehicleModelInfo__SetEditableMaterials_hook, &CVehicleModelInfo__SetEditableMaterials);
    CHook::InlineHook(g_libGTASA, 0x50DEF4, &CVehicle__ResetAfterRender_hook, &CVehicle__ResetAfterRender);

    CHook::InlineHook(g_libGTASA, 0x3986CC, &CGame__Process_hook, &CGame__Process);

    // skybox
    CHook::InlineHook(g_libGTASA, 0x3B1778, &CRenderer__RenderEverythingBarRoads_hook, &CRenderer__RenderEverythingBarRoads);

    CHook::InlineHook(g_libGTASA, 0x4D4A6C, &CAutomobile__ProcessEntityCollision_hook, &CAutomobile__ProcessEntityCollision);

    CHook::InlineHook(g_libGTASA, 0x398334, &CGame__Shutdown_hook, &CGame__Shutdown);

    // headlights color, wheel size, wheel align
    CHook::InlineHook(g_libGTASA, 0x5466EC, &CShadows__StoreCarLightShadow_hook, &CShadows__StoreCarLightShadow);
    CHook::InlineHook(g_libGTASA, 0x518EC4, &CVehicle__DoHeadLightBeam_hook, &CVehicle__DoHeadLightBeam);

    CHook::InlineHook(g_libGTASA, 0x4E671C, &CAutomobile__PreRender_hook, &CAutomobile__PreRender);
    CHook::InlineHook(g_libGTASA, 0x4DC6E8, &CAutomobile__UpdateWheelMatrix_hook, &CAutomobile__UpdateWheelMatrix);
    CHook::InlineHook(g_libGTASA, 0x3E8D48, &CMatrix__Rotate_hook, &CMatrix__Rotate);
    CHook::InlineHook(g_libGTASA, 0x3E8884, &CMatrix__SetScale_hook, &CMatrix__SetScale);

    CHook::InlineHook(g_libGTASA, 0x46CEF4, &CTaskSimpleUseGun__RemoveStanceAnims_hook, &CTaskSimpleUseGun__RemoveStanceAnims);

    CHook::InlineHook(g_libGTASA, 0x3DA86C, &CRadar__LimitRadarPoint_hook, &CRadar__LimitRadarPoint); // TO FIX
    CHook::InlineHook(g_libGTASA, 0x5529FC, &CSprite2d__DrawBarChart_hook, &CSprite2d__DrawBarChart);
    CHook::InlineHook(g_libGTASA, 0x5353B4, &CFont__PrintString_hook, &CFont__PrintString);
    CHook::InlineHook(g_libGTASA, 0x55265C, &CSprite2d__Draw_hook, &CSprite2d__Draw);

    CHook::InlineHook(g_libGTASA, 0x27D8A8, &CWidgetPlayerInfo__DrawWanted_hook, &CWidgetPlayerInfo__DrawWanted);
    CHook::InlineHook(g_libGTASA, 0x27CE88, &CWidgetPlayerInfo__DrawWeaponIcon_hook, &CWidgetPlayerInfo__DrawWeaponIcon);
    CHook::InlineHook(g_libGTASA, 0x389D74, &CCam__Process_hook, &CCam__Process);

    CHook::InlineHook(g_libGTASA, 0x3D6E6C, &CHud__Draw_hook, &CHud__Draw);
    CHook::InlineHook(g_libGTASA, 0x39DC68, &CPad__CycleCameraModeDownJustDown_hook, &CPad__CycleCameraModeDownJustDown);

    CHook::InlineHook(g_libGTASA, 0x31B164, &FxEmitterBP_c__Render_hook, &FxEmitterBP_c__Render);
    CHook::InlineHook(g_libGTASA, 0x43A17C, &CPed__ProcessEntityCollision_hook, &CPed__ProcessEntityCollision);

    //CHook::InlineHook(g_libGTASA, 0x1B1808, &_RwTextureDestroy_hook, &_RwTextureDestroy);
    //CHook::InlineHook(g_libGTASA, 0x1E4AE4, &GetMeshPriority_hook, &GetMeshPriority);
    // CHook::InlineHook(g_libGTASA, 0x0026AA40, &CPause__Pause_hook, &CPause__Pause);

    HookCPad();
}