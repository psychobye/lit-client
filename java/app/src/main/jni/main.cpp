#include <jni.h>
#include <android/log.h>
#include <ucontext.h>
#include <pthread.h>

#include "main.h"
#include "game/game.h"
#include "game/RW/RenderWare.h"
#include "net/netgame.h"
#include "gui/gui.h"
#include "chatwindow.h"
#include "playertags.h"
#include "dialog.h"
#include "keyboard.h"
#include "CSettings.h"
#include "CClientInfo.h"
#include "scoreboard.h"
#include "CAudioStream.h"

#include "util/armhook.h"
#include "CCheckFileHash.h"
#include "str_obfuscator_no_template.hpp"

#include "voice/CVoiceChatClient.h"
#include "crashlytics.h"

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "vendor/ini/config.h"

uintptr_t g_libGTASA = 0;
const char* g_pszStorage = nullptr;

#include "CServerManager.h"
#include "CLocalisation.h"

const cryptor::string_encryptor encLib = cryptor::create("libsamp.so", 11);
void CrashLog(const char* fmt, ...);
CGame *pGame = nullptr;
CNetGame *pNetGame = nullptr;
CChatWindow *pChatWindow = nullptr;
CPlayerTags *pPlayerTags = nullptr;
CDialogWindow *pDialogWindow = nullptr;
CVoiceChatClient* pVoice = nullptr;
CSnapShotHelper* pSnapShotHelper = nullptr;
CScoreBoard* pScoreBoard = nullptr;
CAudioStream	*pAudioStream 	= nullptr;

CGUI *pGUI = nullptr;
CKeyBoard *pKeyBoard = nullptr;
CSettings *pSettings = nullptr;

void InitHookStuff();
void InstallSpecialHooks();
void InitRenderWareFunctions();
void ApplyInGamePatches();
void ApplyPatches_level0();
void MainLoop();

void PrintBuildInfo()
{
	Log("SA:MP version: %d.%01d", SAMP_MAJOR_VERSION, SAMP_MINOR_VERSION);
	Log("Build times: %s %s", __TIME__, __DATE__);
}

#ifdef GAME_EDITION_CR
extern uint16_t g_usLastProcessedModelIndexAutomobile;
extern int g_iLastProcessedModelIndexAutoEnt;
#endif

extern int g_iLastProcessedSkinCollision;
extern int g_iLastProcessedEntityCollision;

extern char g_bufRenderQueueCommand[200];
extern uintptr_t g_dwRenderQueueOffset;

void PrintBuildCrashInfo()
{
	CrashLog("SA:MP version: %d.%01d", SAMP_MAJOR_VERSION, SAMP_MINOR_VERSION);
	CrashLog("Build time: psychobye :3");
	CrashLog("Last processed auto and entity: %d %d", g_usLastProcessedModelIndexAutomobile, g_iLastProcessedModelIndexAutoEnt);
	CrashLog("Last processed skin and entity: %d %d", g_iLastProcessedSkinCollision, g_iLastProcessedEntityCollision);
}

#include "gtasa/ConfigReader.h"
#include <sstream>
#include "CClientInfo.h"
#include "vendor/bass/bass.h"
#include "gui/CFontRenderer.h"
#include "util/CJavaWrapper.h"
#include "cryptors/INITSAMP_result.h"
#include "CDebugInfo.h"
#include "gtasa/GLES.h"
#include "gtasa/HUD.h"

void InitSAMP(JNIEnv* pEnv, jobject thiz);
extern "C"
{
	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_initSAMP(JNIEnv* pEnv, jobject thiz)
	{
		InitSAMP(pEnv, thiz);
	}
}

void InitBASSFuncs();
void InitSAMP(JNIEnv* pEnv, jobject thiz)
{
    PROTECT_CODE_INITSAMP;

    Log("Initializing SAMP..");

    InitBASSFuncs();
    if (!BASS_Init(-1, 44100, BASS_DEVICE_3D, 0, nullptr)) {
        Log("BASS initialization failed!");
        return;
    }

    g_pszStorage = "/storage/emulated/0/LEVEL/";

    if (!g_pszStorage) {
        Log("Error: storage path not found!");
        std::terminate();
    }

    PrintBuildInfo();
    Log("Storage: %s", g_pszStorage);

    pSettings = new CSettings();

    CRadarRect::SetEnabled(pSettings->GetReadOnly().iRadarRect);

    g_pJavaWrapper = new CJavaWrapper(pEnv, thiz);
    g_pJavaWrapper->SetUseFullScreen(pSettings->GetReadOnly().iCutout);

    CLocalisation::Initialise("ru.lc");
    firebase::crashlytics::SetUserId(pSettings->GetReadOnly().szNickName);

    CWeaponsOutFit::ParseDatFile();

    if (!CCheckFileHash::IsFilesValid()) {
        CClientInfo::bSAMPModified = false;
    }
}
extern CSnapShotHelper* pSnapShotHelper;
void ProcessCheckForKeyboard();

void InitInMenu()
{
	pGame = new CGame();
	pGame->InitInMenu();

	pGUI = new CGUI();
	pKeyBoard = new CKeyBoard();
	pChatWindow = new CChatWindow();
	pPlayerTags = new CPlayerTags();
	pDialogWindow = new CDialogWindow();
	pScoreBoard = new CScoreBoard();
	pSnapShotHelper = new CSnapShotHelper();
	pAudioStream = new CAudioStream();

	ProcessCheckForKeyboard();
	CFontRenderer::Initialise();
}
#include <unistd.h> // system api
#include <sys/mman.h>
#include <assert.h> // assert()
#include <dlfcn.h> // dlopen
bool unique_library_handler(const char* library)
{
	Dl_info info;
	if (dladdr(__builtin_return_address(0), &info) != 0)
	{
		void* current_library_addr = dlopen(info.dli_fname, RTLD_NOW);
		if (!current_library_addr)
			return false;

		//Log("%p | %p", current_library_addr, dlopen(library, RTLD_NOW));

		if (dlopen(library, RTLD_NOW) != current_library_addr)
			return false;
	}
	return true;
}

void ProcessCheckForKeyboard()
{
    pKeyBoard->EnableOldKeyboard();
}

void ObfuscatedForceExit3()
{
	// CEntity::PreRender

}
#ifdef GAME_EDITION_CR
int g_iServer = 0;
#else
int g_iServer = 1;
#endif
#include "net/CUDPSocket.h"
#include "gtasa/GLES.h"
#include "gtasa/Skybox.h"

void InitInGame()
{

	static bool bGameInited = false;
	static bool bNetworkInited = false;
	if (!bGameInited)
	{
		if (!unique_library_handler(encLib.decrypt()))
		{
			ObfuscatedForceExit3();
		}

		pGame->InitInGame();
		pGame->SetMaxStats();

                                    pNetGame = new CNetGame(
			g_sEncryptedAddresses[g_iServer].decrypt(),
			g_sEncryptedAddresses[g_iServer].getPort(),
			pSettings->GetReadOnly().szNickName,
			pSettings->GetReadOnly().szPassword);
                                    bGameInited = true;


        /*СonfigReader* readIni = new СonfigReader();
        bool isGo;

        const std::string graphicPath = std::string(g_pszStorage) + "multiplayer/graphics.ini";
        isGo = readIni->ReadIni(graphicPath.c_str(), "graphics", "opengl");

        if (isGo) {
            SetUpGLHooks();
            Log("OPENGL");
        } else {
            Log("NO OPENGL");
        }
        delete readIni;
        readIni = nullptr;*/

		return;
	}

    SetUpGLHooks();

	if (!bNetworkInited)
	{
		bNetworkInited = true;
		return;
	}
}

void (*CTimer__StartUserPause)();
void CTimer__StartUserPause_hook()
{
	// process pause event
	if (g_pJavaWrapper)
	{
		if (pKeyBoard)
		{
			if (pKeyBoard->IsNewKeyboard())
			{
				pKeyBoard->Close();
			}
		}
		g_pJavaWrapper->SetPauseState(true);
	}

	*(uint8_t*)(g_libGTASA + 0x008C9BA3) = 1;
}

void (*CTimer__EndUserPause)();
void CTimer__EndUserPause_hook()
{
	// process resume event
	if (g_pJavaWrapper)
	{
		g_pJavaWrapper->SetPauseState(false);
	}

	*(uint8_t*)(g_libGTASA + 0x008C9BA3) = 0;
}

#include "CDebugInfo.h"
#include "util/CStackTrace.h"

void MainLoop()
{
	InitInGame();

	if(pNetGame) pNetGame->Process();

	if (pNetGame)
	{
		if (pNetGame->GetPlayerPool())
		{
			if (pNetGame->GetPlayerPool()->GetLocalPlayer())
			{
				CVehicle* pVeh = pNetGame->GetVehiclePool()->GetAt(pNetGame->GetPlayerPool()->GetLocalPlayer()->m_CurrentVehicle);
				if (pVeh)
				{
					VECTOR vec;
					pVeh->GetMoveSpeedVector(&vec);
					CDebugInfo::ProcessSpeedMode(&vec);
				}
			}
		}
	}
}
extern int g_iLastRenderedObject;
char g_iLastBlock[512];

void PrintSymbols(void* pc, void* lr)
{
	Dl_info info_pc, info_lr;
	if (dladdr(pc, &info_pc) != 0)
	{
		CrashLog("PC: %s", info_pc.dli_sname);
	}
	if (dladdr(lr, &info_lr) != 0)
	{
		CrashLog("LR: %s", info_lr.dli_sname);
	}
}


struct sigaction act_old;
struct sigaction act1_old;
struct sigaction act2_old;
struct sigaction act3_old;

void handler3(int signum, siginfo_t *info, void *contextPtr) {
    ucontext *context = (ucontext_t *) contextPtr;

    if (act3_old.sa_sigaction) {
        act3_old.sa_sigaction(signum, info, contextPtr);
    }

    if (info->si_signo == SIGBUS) {
        PrintBuildCrashInfo();
        CrashLog("Last rendered object: %d", g_iLastRenderedObject);
        CrashLog("Last texture: %s", g_iLastBlock);
        CrashLog("SIGBUS | Fault address: 0x%X", info->si_addr);
        CrashLog("libGTASA base address: 0x%X", g_libGTASA);
        CrashLog("libsamp base address: 0x%X", FindLibrary("libsamp.so"));
        CrashLog("libc base address: 0x%X", FindLibrary("libc.so"));

        PRINT_CRASH_STATES(context);

        CrashLog("backtrace:");
        CrashLog("1: libGTASA.so + 0x%X", context->uc_mcontext.arm_pc - g_libGTASA);
        CrashLog("2: libGTASA.so + 0x%X", context->uc_mcontext.arm_lr - g_libGTASA);

        CrashLog("1: libsamp.so + 0x%X",
                 context->uc_mcontext.arm_pc - FindLibrary("libsamp.so"));
        CrashLog("2: libsamp.so + 0x%X",
                 context->uc_mcontext.arm_lr - FindLibrary("libsamp.so"));


        CrashLog("1: libc.so + 0x%X", context->uc_mcontext.arm_pc - FindLibrary("libc.so"));
        CrashLog("2: libc.so + 0x%X", context->uc_mcontext.arm_lr - FindLibrary("libc.so"));

        CStackTrace::printBacktrace(context);
        //DumpLibraries();

        //exit(0);
    }

    return;
}

void handler(int signum, siginfo_t *info, void *contextPtr) {
    ucontext *context = (ucontext_t *) contextPtr;

    if (act_old.sa_sigaction) {
        act_old.sa_sigaction(signum, info, contextPtr);
    }

    if (info->si_signo == SIGSEGV) {
        CrashLog(" ");
        PrintBuildCrashInfo();
        CrashLog("Last rendered object: %d", g_iLastRenderedObject);
        CrashLog("Last texture: %s", g_iLastBlock);
        CrashLog("SIGSEGV | Fault address: 0x%X", info->si_addr);
        CrashLog("libGTASA base address: 0x%X", g_libGTASA);
        CrashLog("libsamp base address: 0x%X", FindLibrary("libsamp.so"));
        CrashLog("libc base address: 0x%X", FindLibrary("libc.so"));

        PRINT_CRASH_STATES(context);

        CrashLog("backtrace:");
        CrashLog("1: libGTASA.so + 0x%X", context->uc_mcontext.arm_pc - g_libGTASA);
        CrashLog("2: libGTASA.so + 0x%X", context->uc_mcontext.arm_lr - g_libGTASA);

        CrashLog("1: libsamp.so + 0x%X",
                 context->uc_mcontext.arm_pc - FindLibrary("libsamp.so"));
        CrashLog("2: libsamp.so + 0x%X",
                 context->uc_mcontext.arm_lr - FindLibrary("libsamp.so"));

        CrashLog("1: libc.so + 0x%X", context->uc_mcontext.arm_pc - FindLibrary("libc.so"));
        CrashLog("2: libc.so + 0x%X", context->uc_mcontext.arm_lr - FindLibrary("libc.so"));

        CStackTrace::printBacktrace(context);
    }
}

void handler2(int signum, siginfo_t *info, void *contextPtr) {
    ucontext *context = (ucontext_t *) contextPtr;

    if (act2_old.sa_sigaction) {
        act2_old.sa_sigaction(signum, info, contextPtr);
    }

    if (info->si_signo == SIGFPE) {
        PrintBuildCrashInfo();
        CrashLog("Last rendered object: %d", g_iLastRenderedObject);
        CrashLog("Last texture: %s", g_iLastBlock);
        CrashLog("SIGFPE | Fault address: 0x%X", info->si_addr);
        CrashLog("libGTASA base address: 0x%X", g_libGTASA);
        CrashLog("libsamp base address: 0x%X", FindLibrary("libsamp.so"));
        CrashLog("libc base address: 0x%X", FindLibrary("libc.so"));

        PRINT_CRASH_STATES(context);

        CrashLog("backtrace:");
        CrashLog("1: libGTASA.so + 0x%X", context->uc_mcontext.arm_pc - g_libGTASA);
        CrashLog("2: libGTASA.so + 0x%X", context->uc_mcontext.arm_lr - g_libGTASA);

        CrashLog("1: libsamp.so + 0x%X",
                 context->uc_mcontext.arm_pc - FindLibrary("libsamp.so"));
        CrashLog("2: libsamp.so + 0x%X",
                 context->uc_mcontext.arm_lr - FindLibrary("libsamp.so"));


        CrashLog("1: libc.so + 0x%X", context->uc_mcontext.arm_pc - FindLibrary("libc.so"));
        CrashLog("2: libc.so + 0x%X", context->uc_mcontext.arm_lr - FindLibrary("libc.so"));

        CStackTrace::printBacktrace(context);
    }

    return;
}

void handler1(int signum, siginfo_t *info, void *contextPtr) {
    ucontext *context = (ucontext_t *) contextPtr;

    if (act1_old.sa_sigaction) {
        act1_old.sa_sigaction(signum, info, contextPtr);
    }

    if (info->si_signo == SIGABRT) {
        CrashLog(" ");
        PrintBuildCrashInfo();

        CrashLog("Last rendered object: %d", g_iLastRenderedObject);
        CrashLog("Last texture: %s", g_iLastBlock);
        CrashLog("SIGABRT | Fault address: 0x%X", info->si_addr);
        CrashLog("libGTASA base address: 0x%X", g_libGTASA);
        CrashLog("libsamp base address: 0x%X", FindLibrary("libsamp.so"));
        CrashLog("libc base address: 0x%X", FindLibrary("libc.so"));

        PRINT_CRASH_STATES(context);

        CrashLog("backtrace:");
        CrashLog("1: libGTASA.so + 0x%X", context->uc_mcontext.arm_pc - g_libGTASA);
        CrashLog("2: libGTASA.so + 0x%X", context->uc_mcontext.arm_lr - g_libGTASA);


        CrashLog("1: libsamp.so + 0x%X",
                 context->uc_mcontext.arm_pc - FindLibrary("libsamp.so"));
        CrashLog("2: libsamp.so + 0x%X",
                 context->uc_mcontext.arm_lr - FindLibrary("libsamp.so"));

        CrashLog("1: libc.so + 0x%X", context->uc_mcontext.arm_pc - FindLibrary("libc.so"));
        CrashLog("2: libc.so + 0x%X", context->uc_mcontext.arm_lr - FindLibrary("libc.so"));

        CStackTrace::printBacktrace(context);
    }

    return;
}

extern "C"
{
	JavaVM* javaVM = NULL;
	JavaVM* alcGetJavaVM(void) {
		return javaVM;
	}
}
extern "C" AL_API jint AL_APIENTRY JNI_OnLoad_alc(JavaVM* vm, void* reserved);

void (*RQ_Command_rqSetAlphaTest)(char**);
void RQ_Command_rqSetAlphaTest_hook(char** a1)
{
	return;
}

#include "CFPSFix.h"
CFPSFix g_fps;

void (*ANDRunThread)(void* a1);
void ANDRunThread_hook(void* a1)
{
	g_fps.PushThread(gettid());

	ANDRunThread(a1);
}

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
	JNI_OnLoad_alc(vm, reserved);
	javaVM = vm;

	Log("SAMP library loaded! Build time: " __DATE__ " " __TIME__);



	g_libGTASA = FindLibrary("libGTASA.so");
	if(g_libGTASA == 0)
	{
		Log("ERROR: libGTASA.so address not found!");
		return 0;
	}
	Log("libGTASA.so image base address: 0x%X", g_libGTASA);

	firebase::crashlytics::Initialize();

	uintptr_t libgtasa = FindLibrary("libGTASA.so");
	uintptr_t libsamp = FindLibrary("libsamp.so");
	uintptr_t libc = FindLibrary("libc.so");

	Log("libGTASA.so: 0x%x", libgtasa);
	Log("libsamp.so: 0x%x", libsamp);
	Log("libc.so: 0x%x", libc);

	char str[100];

	sprintf(str, "0x%x", libgtasa);
	firebase::crashlytics::SetCustomKey("libGTASA.so", str);

	sprintf(str, "0x%x", libsamp);
	firebase::crashlytics::SetCustomKey("libsamp.so", str);

	sprintf(str, "0x%x", libc);
	firebase::crashlytics::SetCustomKey("libc.so", str);

	srand(time(0));

	InitHookStuff();

	InitRenderWareFunctions();
	InstallSpecialHooks();
    //SetUpGLHooks();
	// increase render memory buffer
	SetUpHook(g_libGTASA + 0x003BF784, (uintptr_t)CTimer__StartUserPause_hook, (uintptr_t*)& CTimer__StartUserPause);
	SetUpHook(g_libGTASA + 0x003BF7A0, (uintptr_t)CTimer__EndUserPause_hook, (uintptr_t*)& CTimer__EndUserPause);

	// yes, just nop-out this fucking shit
	// this should prevent game from crashing when exiting(R*)
	NOP(g_libGTASA + 0x0039844E, 2);
	NOP(g_libGTASA + 0x0039845E, 2);
	NOP(g_libGTASA + 0x0039840A, 2);

	NOP(g_libGTASA + 0x002E1EDC, 2); // get the fuck up this uninitialised shit!
	NOP(g_libGTASA + 0x00398972, 2); // get out fucking roadblocks
	// maybe nop engine terminating ????
	// terminate all stuff when exiting
	// nop shit pause

	ApplyPatches_level0();

	struct sigaction act;
	act.sa_sigaction = handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &act, &act_old);

	struct sigaction act1;
	act1.sa_sigaction = handler1;
	sigemptyset(&act1.sa_mask);
	act1.sa_flags = SA_SIGINFO;
	sigaction(SIGABRT, &act1, &act1_old);

	struct sigaction act2;
	act2.sa_sigaction = handler2;
	sigemptyset(&act2.sa_mask);
	act2.sa_flags = SA_SIGINFO;
	sigaction(SIGFPE, &act2, &act2_old);

	struct sigaction act3;
	act3.sa_sigaction = handler3;
	sigemptyset(&act3.sa_mask);
	act3.sa_flags = SA_SIGINFO;
	sigaction(SIGBUS, &act3, &act3_old);

	return JNI_VERSION_1_4;
}

void Log(const char *fmt, ...)
{	
	char buffer[0xFF];
	static FILE* flLog = nullptr;

	if(flLog == nullptr && g_pszStorage != nullptr)
	{
		sprintf(buffer, "%smultiplayer/logs/log.txt", g_pszStorage);
		flLog = fopen(buffer, "a");
	}
	memset(buffer, 0, sizeof(buffer));

	va_list arg;
	va_start(arg, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, arg);
	va_end(arg);

	firebase::crashlytics::Log(buffer);

	//if(pDebug) pDebug->AddMessage(buffer);

	if(flLog == nullptr) return;
	fprintf(flLog, "%s\n", buffer);
	fflush(flLog);

	__android_log_write(ANDROID_LOG_INFO, "SAMP", buffer);

	return;
}

void CrashLog(const char* fmt, ...)
{
	char buffer[0xFF];
	static FILE* flLog = nullptr;

	if (flLog == nullptr && g_pszStorage != nullptr)
	{
		sprintf(buffer, "%smultiplayer/logs/crash.txt", g_pszStorage);
		flLog = fopen(buffer, "a");
	}

	memset(buffer, 0, sizeof(buffer));

	va_list arg;
	va_start(arg, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, arg);
	va_end(arg);

	__android_log_write(ANDROID_LOG_INFO, "SAMP", buffer);

	firebase::crashlytics::Log(buffer);

	if (flLog == nullptr) return;
	fprintf(flLog, "%s\n", buffer);
	fflush(flLog);

	return;
}

uint32_t GetTickCount()
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return (tv.tv_sec*1000+tv.tv_usec/1000);
}
