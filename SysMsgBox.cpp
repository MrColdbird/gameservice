// ======================================================================================
// File         : SysMsgBoxManager.cpp
// Author       : Li Chen 
// Last Change  : 10/19/2010 | 16:11:06 PM | Tuesday,October
// Description  : 
// ======================================================================================

#include "stdafx.h"
#include "SysMsgBox.h"
#include "Task.h"
#include "Message.h"
#include "Master.h"
#include "InterfaceMgr.h"
#include "Interface.h"

namespace GameService
{

#if defined(_PS3)
#define CMD_PRIO			(1001)
#define CMD_STACKSIZE		(64*1024)

void thr_GameContentNoSpaceErrorDialog(GS_UINT64 arg)
{
    cellGameContentErrorDialog(CELL_GAME_ERRDIALOG_NOSPACE_EXIT, (int)(arg), NULL);
    sys_ppu_thread_exit(0);
}
#endif

SysMsgBoxManager::SysMsgBoxManager()
{
#if 0
    m_eMode = EMODE_IDLE;

#if defined(_PS3)
    int ret = cellSysmoduleLoadModule( CELL_SYSMODULE_SYSUTIL_GAME );   
	if ( ret != CELL_OK ) {
        Master::G()->Log( "ERROR : cellSysmoduleLoadModule( CELL_SYSMODULE_SYSUTIL_GAME ) = 0x%x", ret );
	}
#endif
#endif
}
SysMsgBoxManager::~SysMsgBoxManager()
{
#if 0
#if defined(_PS3)
	int ret = cellSysmoduleUnloadModule( CELL_SYSMODULE_SYSUTIL_GAME );
	if ( ret != CELL_OK ) {
        Master::G()->Log( "ERROR : cellSysmoduleUnloadModule( CELL_SYSMODULE_SYSUTIL_GAME ) = 0x%x", ret );
	}
#endif
#endif
}

void SysMsgBoxManager::Display(GS_INT mode, void* exdata1)
{
#if 0
    m_eMode = mode;

#if !defined(_WINDOWS)
    // localize according to System Language setting
    switch(m_eMode)
    {
		case EMODE_KeyFileCorrupted:
		{
	        switch(GetSystemLanguage())
            {
            default:
            case GS_ELang_English:
                strcpy(m_cSysMsgStr[m_eMode],"Your key file is corrupted, please choose 'Unlock Full Game' again to get the full version.");
                break;
            case GS_ELang_French:
                strcpy(m_cSysMsgStr[m_eMode],"Le fichier principal est corrompu, veuillez choisir 'D\xc3\xa9verrouiller le jeu complet' \xc3\xa0 nouveau pour r\xc3\xa9\1cup\xc3\xa9rer la version compl\xc3\xa8te.");
                break;
            case GS_ELang_German:
                strcpy(m_cSysMsgStr[m_eMode],"Ihre Schl\xc3\xbcsseldatei ist besch\xe4\1digt. Bitte w\xc3\xa4hlen Sie erneut 'Vollspiel freischalten', um die Komplettversion zu erhalten.");
                break;
            case GS_ELang_Spanish:
                strcpy(m_cSysMsgStr[m_eMode],"Tu archivo clave est\xc3\xa1 da\xc3\xb1\1ado. Elige de nuevo 'Desbloquear el juego completo' para conseguir la versi\xc3\xb3n completa.");
                break;
            case GS_ELang_Italian:
                strcpy(m_cSysMsgStr[m_eMode],"Il file chiave \xc3\xa8 corrotto, scegli di nuovo 'Sblocca gioco completo' per attivare la versione completa.");
                break;
            case GS_ELang_Dutch:
                strcpy(m_cSysMsgStr[m_eMode],"De code is beschadigd. Kies opnieuw Volledige game ontgrendelen om de volledige versie te bemachtigen.");
                break;
            }
		}
		break;
    case EMODE_SaveDataNoSpace:
        {
            GS_INT space = *(GS_INT*)exdata1;

#if defined(_PS3)
            Master::G()->SetForceQuit();

            // invoke GameContentError msgbox:
            sys_ppu_thread_t tid;
            sys_ppu_thread_create(&tid,
                thr_GameContentNoSpaceErrorDialog, space,
                CMD_PRIO, CMD_STACKSIZE,
                0, "thr_game_data");

#endif

            // switch(GS_GetSystemLanguage())
            // {
            // default:
            // case GS_ELang_English:
            //     sprintf(m_cSysMsgStr[m_eMode], "There is not enough available space in the HDD. To play the game, at least %d MB more space is required. Exit the game and obtain the necessary space", space/1024+2);
            //     break;
            // case GS_ELang_French:
            //     sprintf(m_cSysMsgStr[m_eMode], "Espace libre insuffisant sur le disque dur. Pour jouer, au moins %d MB d'espace suppl\xc3\xa9mentaire sont n\xc3\xa9\1cessaires. Quittez le jeu et lib\xc3\xa9rez l'espace n\xc3\xa9\1cessaire.",  space/1024+2);
            //     break;
            // case GS_ELang_German:
            //     sprintf(m_cSysMsgStr[m_eMode], "Auf der Festplatte ist nicht genug Speicherplatz vorhanden. Um das Spiel zu spielen werden mindestens %d MB mehr Platz ben\xc3\xb6tigt. Verlassen Sie das Spiel und schaffen Sie den ben\xc3\xb6tigten Platz.",  space/1024+2);
            //     break;
            // case GS_ELang_Spanish:
            //     sprintf(m_cSysMsgStr[m_eMode], "No hay suficiente espacio libre en el disco duro. Para jugar necesitas al menos %d MB de espacio adicional. Sal del juego y libera el espacio necesario.",  space/1024+2);
            //     break;
            // case GS_ELang_Italian:
            //     sprintf(m_cSysMsgStr[m_eMode], "Spazio libero insufficiente sul disco fisso. Per giocare sono necessari almeno altri %d MB di spazio. Abbandona il gioco e libera lo spazio necessario.",  space/1024+2);
            //     break;
            // case GS_ELang_Dutch:
            //     sprintf(m_cSysMsgStr[m_eMode], "Er is onvoldoende vrije ruimte op de harde schijf. Om de game te kunnen spelen, is ten minste %d MB aan ruimte vereist. Sluit de game af om de vereiste ruimte vrij te maken.",  space/1024+2);
            //     break;
            // }
        }
        break;
    case EMODE_TrophyNoSpace:
        {
            switch(GetSystemLanguage())
            {
            default:
            case GS_ELang_English:
                strcpy(m_cSysMsgStr[m_eMode],"Your HDD has insufficient space for installing Trophy package.");
                break;
            case GS_ELang_French:
                strcpy(m_cSysMsgStr[m_eMode],"Espace libre insuffisant sur le disque dur pour installer le package de troph\xc3\xa9\1es.");
                break;
            case GS_ELang_German:
                strcpy(m_cSysMsgStr[m_eMode],"Auf Ihrer Festplatte ist nicht genug Platz vorhanden, um das Troph\xc3\xa4\1enpaket zu installieren.");
                break;
            case GS_ELang_Spanish:
                strcpy(m_cSysMsgStr[m_eMode],"Tu disco duro no tiene espacio suficiente para instalar el paquete de trofeos.");
                break;
            case GS_ELang_Italian:
                strcpy(m_cSysMsgStr[m_eMode],"Spazio libero insufficiente sul disco fisso per l'installazione del pacchetto Trofei.");
                break;
            case GS_ELang_Dutch:
                strcpy(m_cSysMsgStr[m_eMode],"Er is onvoldoende vrije ruimte op de harde schijf om de trofeeën te installeren.");
                break;
            }
        }
        break;
    case EMODE_PlayOtherUserSaveData:
        {
            switch(GetSystemLanguage())
            {
            default:
            case GS_ELang_English:
                strcpy(m_cSysMsgStr[m_eMode],"You are loading the other user's save data.\n You can not unlock any trohpy.");
                break;
            case GS_ELang_French:
                strcpy(m_cSysMsgStr[m_eMode],"Vous chargez les sauvegardes de l'autre utilisateur.\n Vous ne pouvez pas d\xc3\xa9verrouiller de troph\xc3\xa9\1e ni mettre \xc3\xa0 jour votre classement.");
                break;
            case GS_ELang_German:
                strcpy(m_cSysMsgStr[m_eMode],"Sie laden die Speicherdaten des anderen Benutzers.\n Sie können keine Troph\xc3\xb6\1en freischalten oder die Punktzahl der Ranglisten aktualisieren.");
                break;
            case GS_ELang_Spanish:
                strcpy(m_cSysMsgStr[m_eMode],"Est\xc3\xa1s cargando la partida de otro usuario.\n No podr\xc3\xa1s desbloquear ning\xc3\xban trofeo o actualizar tu puntuaci\xc3\xb3n en los marcadores.");
                break;
            case GS_ELang_Italian:
                strcpy(m_cSysMsgStr[m_eMode],"Stai caricando i dati salvati di un altro utente.\n Non ti sar\xc3\xa0 possibile sbloccare alcun trofeo o aggiornare il tuo punteggio nelle classifiche.");
                break;
            case GS_ELang_Dutch:
                strcpy(m_cSysMsgStr[m_eMode],"Je laadt de opgeslagen gegevens van een andere gebruiker.\n Hierdoor kun je geen trofee\xc3\xabn vrijspelen of je score in het klassement verbeteren.");
                break;
            }
        }
	case EMODE_PlayerAgeForbidden:
		{
			switch(GetSystemLanguage())
			{
			default:
			case GS_ELang_English:
				strcpy(m_cSysMsgStr[m_eMode]," There is a PlayStation\xc2\xaeNetwork account restriction in place that prevents you from using this feature.");
				break;
			case GS_ELang_French:
				strcpy(m_cSysMsgStr[m_eMode],"Une restriction du compte PlayStation\xc2\xaeNetwork vous emp\xc3\xaa\x63he d'utiliser cette fonctionnalit\xc3\xa9.");
				break;
			case GS_ELang_German:
				strcpy(m_cSysMsgStr[m_eMode],"Wegen Einschr\xc3\xa4nkung des PlayStation\xc2\xaeNetwork Kontos steht diese Funktion nicht zur Verf\xc3\xbcgung.");
				break;
			case GS_ELang_Spanish:
				strcpy(m_cSysMsgStr[m_eMode],"Una restircti\xc3\xb3n de la cuenta de PlayStation\xc2\xaeNetwork te impide usar esta funci\xc3\xb3n.");
				break;
			case GS_ELang_Italian:
				strcpy(m_cSysMsgStr[m_eMode],"Una restrizione all'account di PlayStation\xc2\xaeNetwork ti impedisce di utilizzare questa funzione.");
				break;
			case GS_ELang_Dutch:
				strcpy(m_cSysMsgStr[m_eMode],"Vanwege een PlayStation\xc2\xaeNetwork account-beperking kun je niet gebruikmaken van dit onderdeel.");
				break;
			}
		}
        break;
    }
#endif
#endif
}

#if defined(_PS3)
void CB_Dialog_Global( int button_type, void *userdata )
{
#if 0
	SysMsgBoxManager* mgr = (SysMsgBoxManager*)userdata;
	mgr->CB_Dialog(button_type);
#endif
}
#endif

void SysMsgBoxManager::Update()
{
#if 0//defined(_PS3)
	int ret = -1;
    switch(m_eMode)
    {
	case EMODE_KeyFileCorrupted:
		{
			unsigned int type =   CELL_MSGDIALOG_TYPE_SE_TYPE_ERROR			
								| CELL_MSGDIALOG_TYPE_BG_VISIBLE			
								| CELL_MSGDIALOG_TYPE_BUTTON_TYPE_OK	
								| CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_ON	
                                | CELL_MSGDIALOG_TYPE_DEFAULT_CURSOR_OK;	

			ret = cellMsgDialogOpen2( type, m_cSysMsgStr[EMODE_KeyFileCorrupted], CB_Dialog_Global, (void*)this, NULL );
			if ( ret != CELL_OK ) {
				if ( ret == (int)CELL_SYSUTIL_ERROR_BUSY ) {
                    Master::G()->Log( "[GameService] - WARN  : cellMsgDialogOpen2() = 0x%x (CELL_SYSUTIL_ERROR_BUSY) ... Retry.", ret );
					return;
				}
				Master::G()->Log( "ERROR : cellMsgDialogOpen2() = 0x%x", ret );
				return;
			}
		}
		break;
	case EMODE_SaveDataNoSpace:
		{
            // unsigned int type =   CELL_MSGDIALOG_TYPE_SE_TYPE_ERROR			
            //                     | CELL_MSGDIALOG_TYPE_BG_VISIBLE			
            //                     | CELL_MSGDIALOG_TYPE_BUTTON_TYPE_OK	
            //                     | CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_ON	
            //                     | CELL_MSGDIALOG_TYPE_DEFAULT_CURSOR_OK;	

            // ret = cellMsgDialogOpen2( type, m_cSysMsgStr[EMODE_SaveDataNoSpace], CB_Dialog_Global, (void*)this, NULL );
            // if ( ret != CELL_OK ) {
            //     if ( ret == (int)CELL_SYSUTIL_ERROR_BUSY ) {
            //         Master::G()->Log( "[GameService] - WARN  : cellMsgDialogOpen2() = 0x%x (CELL_SYSUTIL_ERROR_BUSY) ... Retry.", ret );
            //         return;
            //     }
            //     Master::G()->Log( "ERROR : cellMsgDialogOpen2() = 0x%x", ret );
            //     return;
            // }
		}
		break;
	case EMODE_TrophyNoSpace:
		{
			unsigned int type =   CELL_MSGDIALOG_TYPE_SE_TYPE_ERROR			
								| CELL_MSGDIALOG_TYPE_BG_VISIBLE			
								| CELL_MSGDIALOG_TYPE_BUTTON_TYPE_OK	
								| CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_OFF	
                                | CELL_MSGDIALOG_TYPE_DEFAULT_CURSOR_OK;	

			ret = cellMsgDialogOpen2( type, m_cSysMsgStr[EMODE_TrophyNoSpace], CB_Dialog_Global, (void*)this, NULL );
			if ( ret != CELL_OK ) {
				if ( ret == (int)CELL_SYSUTIL_ERROR_BUSY ) {
					Master::G()->Log( "[GameService] - WARN  : cellMsgDialogOpen2() = 0x%x (CELL_SYSUTIL_ERROR_BUSY) ... Retry.", ret );
					return;
				}
				Master::G()->Log( "ERROR : cellMsgDialogOpen2() = 0x%x", ret );
				return;
			}
            m_eMode = EMODE_IDLE;
		}
		break;
 	case EMODE_PlayOtherUserSaveData:
		{
			unsigned int type =   CELL_MSGDIALOG_TYPE_SE_TYPE_NORMAL			
								| CELL_MSGDIALOG_TYPE_BG_VISIBLE			
								| CELL_MSGDIALOG_TYPE_BUTTON_TYPE_OK	
								| CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_OFF	
                                | CELL_MSGDIALOG_TYPE_DEFAULT_CURSOR_OK;	

			ret = cellMsgDialogOpen2( type, m_cSysMsgStr[EMODE_PlayOtherUserSaveData], CB_Dialog_Global, (void*)this, NULL );
			if ( ret != CELL_OK ) {
				if ( ret == (int)CELL_SYSUTIL_ERROR_BUSY ) {
					Master::G()->Log( "[GameService] - WARN  : cellMsgDialogOpen2() = 0x%x (CELL_SYSUTIL_ERROR_BUSY) ... Retry.", ret );
					return;
				}
				Master::G()->Log( "ERROR : cellMsgDialogOpen2() = 0x%x", ret );
				return;
			}
            m_eMode = EMODE_IDLE;
		}
		break;
	case EMODE_PlayerAgeForbidden:
		{
			unsigned int type =   CELL_MSGDIALOG_TYPE_SE_TYPE_NORMAL			
				| CELL_MSGDIALOG_TYPE_BG_VISIBLE			
				| CELL_MSGDIALOG_TYPE_BUTTON_TYPE_OK	
				| CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_OFF	
				| CELL_MSGDIALOG_TYPE_DEFAULT_CURSOR_OK;	

			ret = cellMsgDialogOpen2( type, m_cSysMsgStr[EMODE_PlayerAgeForbidden], CB_Dialog_Global, (void*)this, NULL );
			if ( ret != CELL_OK ) {
				if ( ret == (int)CELL_SYSUTIL_ERROR_BUSY ) {
					Master::G()->Log( "[GameService] - WARN  : cellMsgDialogOpen2() = 0x%x (CELL_SYSUTIL_ERROR_BUSY) ... Retry.", ret );
					return;
				}
				Master::G()->Log( "ERROR : cellMsgDialogOpen2() = 0x%x", ret );
				return;
			}
			m_eMode = EMODE_IDLE;
		}
		break;
    // case EMODE_YesNo:
    //     {
    //         unsigned int type =   CELL_MSGDIALOG_TYPE_SE_TYPE_NORMAL			
    //                             | CELL_MSGDIALOG_TYPE_BG_VISIBLE			
    //                             | CELL_MSGDIALOG_TYPE_BUTTON_TYPE_YESNO	
    //                             | CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_ON	
    //                             | CELL_MSGDIALOG_TYPE_DEFAULT_CURSOR_NO;	

    //         ret = cellMsgDialogOpen2( type, sampleMsgDialogString1, cb_mode_string_dialog_test_1, (void*)m_eMode, NULL );
    //         if ( ret != CELL_OK ) {
    //             if ( ret == (int)CELL_SYSUTIL_ERROR_BUSY ) {
    //                 Master::G()->Log( "[GameService] - WARN  : cellMsgDialogOpen2() = 0x%x (CELL_SYSUTIL_ERROR_BUSY) ... Retry.", ret );
    //                 return;
    //             }
    //             Master::G()->Log( "ERROR : cellMsgDialogOpen2() = 0x%x", ret );
    //             return;
    //         }
    //     }
    //     break;
    }
#endif
}

#if defined(_PS3)
void SysMsgBoxManager::CB_Dialog( int button_type )
{
	GS_INT msgtype = 0;
    switch(button_type)
    {
	case CELL_MSGDIALOG_BUTTON_YES:
	case CELL_MSGDIALOG_BUTTON_NO:
	case CELL_MSGDIALOG_BUTTON_ESCAPE:
	case CELL_MSGDIALOG_BUTTON_NONE:
        break;
    }
	switch(m_eMode)
	{
	case EMODE_SaveDataNoSpace:
		msgtype = EGSTaskType_SysMsgBox_SaveDataNoFreeSpace;
		break;
	case EMODE_TrophyNoSpace:
		msgtype = EGSTaskType_SysMsgBox_TrophyNoFreeSpace;
		break;
    case EMODE_PlayOtherUserSaveData:
        msgtype = EGSTaskType_SysMsgBox_PlayOtherUserSaveData;
        break;
	}

    m_eMode = EMODE_IDLE;

	// Message to Interface
	Message* msg = Message::Create(EMessage_CallBackInterface);
	if (msg)
	{
		GS_BOOL result = (CELL_MSGDIALOG_BUTTON_YES == button_type) ? TRUE : FALSE;

		msg->AddPayload(msgtype);

		msg->AddPayload(result);

		msg->AddTarget(Master::G()->GetInterfaceMgr());

		Master::G()->GetMessageMgr()->Send(msg);
	}
}


#endif

} // namespace
