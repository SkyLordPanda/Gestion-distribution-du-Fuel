  /*
  Note :
  Prog GSM pour GSM1 et 2.
  Module GSM sure socket1 de la carte mikroPic.
  Le J5 de la carte mikroPic doit être sur 3.3V.
  Il faut mettre un afficheur 2X16 sur le port B de la carte mikroPic.
  Un niveau bas sur RA4 déclenche l'envoi d'un message.
  Ne pas mettre de pull up 5V sur RC6 et RC7.
  */
  
  // Commande AT pour controler le module GSM **********************************
  #define AT "AT"                             //--------------------------------Début d'une commande AT
  #define AT_ECHO_OFF "ATE0"                  //--------------------------------Dévalide la mode echo
  #define AT_MODE_TEXTE  "AT+CMGF=1"          //--------------------------------Message en mode texte
  #define AT_ENVOI_MESSAGE "AT+CMGS=\""       //--------------------------------Envoi d'un message à un numéro de téléphone
  #define AT_LECTURE_MESSAGE "AT+CMGR=1"      //--------------------------------Commande de lecture des messages en position 1 de la inbox
  #define AT_STATUS_MESSAGE "AT+CMGL=\"ALL\"" //--------------------------------Vérifier le status des messages entrants
  #define AT_EFFACEMENT_MESSAGE "AT+CMGD=1,4" //--------------------------------Effacement des messages
  
  //réponse aux commandes AT****************************************************
  const GSM_OK=0;
  const GSM_PRET_A_RECEVOIR_MESSAGE=1;        //--------------------------------Utilisé dans le programme d'envoi de sms
  const GSM_ERROR=2;
  const GSM_UNREAD=3;
  //buffer des commandes AT*****************************************************
  char AT_cmd[33];
  //si RTS utilisé**************************************************************
  sbit RTS at LATE0_bit;
  sbit RTS_Direction at TRISE0_bit;
  // choix gsm *****************************************************************
  
  //GSM1
  //sbit GSM_ON_OFF at LATA2_bit;
  //sbit GSM_ON_OFF_Direction at TRISA2_bit;
  
  //GSM2
  sbit GSM_ON_OFF at LATE1_bit;
  sbit GSM_ON_OFF_Direction at TRISE1_bit;
  //Grille de commutaion du port série******************************************
  sbit A_4052 at LATC0_bit;
  sbit B_4052 at LATC1_bit;
  sbit A_4052_direction at TRISC0_bit;
  sbit B_4052_direction at TRISC1_bit;
  // Variable message***********************************************************
  char phone_nb_entrant[20] = "";
  char sms_rcpt_message[50] = ""; //--------------------------------------------Buffer de reception des messgaes
  // Variables de controle******************************************************
  char Etat_gsm=0; //-----------------------------------------------------------Prédiction attente caractere a suivre
  char Signal_Reponse_gsm=0; //-------------------------------------------------Signale une réponse valide du gsm
  char Reponse_gsm=-1; //-------------------------------------------------------Type de reponse du gsm OK ou UNREAD....
  short ReponseID=-1;  //-------------------------------------------------------Sauvegarder Reponse_gsm avant traitement
  char Etat_gsm_nb=0;  //-------------------------------------------------------Prédiction attente chiffre ou fin du numéro de telephone  a suivre
  unsigned char SMS_pos=0; //---------------------------------------------------Controle de la position dans la chaine de caractere du message
  char Message_complet=0; //----------------------------------------------------Signaleun message complet dans le buffer de reception
  unsigned char Phone_nb_pos=0; //----------------------------------------------Controle de la position dans la chaine de caractere du numero de telephone
  char Msg_non_lu=0; //---------------------------------------------------------Signale un message non lu dans le buffer de reception
  char Arobas=0; //-------------------------------------------------------------Le sms entrant doit etre compris entre deux arobas
  char Envoi_En_Cours=0; //-----------------------------------------------------Signalement que le message a été envoyé
  //Module LCD sur port B données sur 4 bits ***********************************
  sbit LCD_RS at RB4_bit;
  sbit LCD_EN at RB5_bit;
  sbit LCD_D4 at RB0_bit;
  sbit LCD_D5 at RB1_bit;
  sbit LCD_D6 at RB2_bit;
  sbit LCD_D7 at RB3_bit;
  //
  sbit LCD_RS_Direction at TRISB4_bit;
  sbit LCD_EN_Direction at TRISB5_bit;
  sbit LCD_D4_Direction at TRISB0_bit;
  sbit LCD_D5_Direction at TRISB1_bit;
  sbit LCD_D6_Direction at TRISB2_bit;
  sbit LCD_D7_Direction at TRISB3_bit;
  //
  char txtL1_lcd[15]; //--------------------------------------------------------Buffer affichage sur lcd de la ligne1
  char txtL2_lcd[15]; //--------------------------------------------------------Buffer affichage sur lcd de la ligne2
  //
  unsigned int k=0; //----------------------------------------------------------Compteur  dans boucle principale


  //****************************************************************************
  //**************            SOUS PROGRAMME                      **************
  //****************************************************************************

  //INTERRUPTION UART
  void interrupt()
  {
  char tmp;
  if (RCIF_bit==1)//------------------------------------------------------------Est une demande d'interruption de réception sur UART
     {
     tmp=UART_Rd_Ptr(); //------------------------------------------------------Lecture de la donnée en réception
     if (ReponseID==GSM_UNREAD)
        {
        if (Arobas==1) //-------------------------------------------------------On reçois déjà l'arobase marquant le début texte et on enregistre le texte
           {
           sms_rcpt_message[SMS_pos]=tmp;
           SMS_pos++;
           }
        if (SMS_pos==50)  SMS_pos=49; //----------------------------------------On a atteint la longueur max du buffer de reception
        SMS_rcpt_message[SMS_pos]=0; //-----------------------------------------Ajout d'un fin de chaine
        }
     if (tmp=='@') //-----------------------------------------------------------Marqueur de début ou fin de texte
        {
        if (Arobas==0) //-------------------------------------------------------Si premier arobase
           {
           Arobas=1; //---------------------------------------------------------Alors : marquer que premier arobas reçu
           SMS_pos=0; //--------------------------------------------------------positionner l'index du texte au début
           SMS_rcpt_message[SMS_pos]=0; //--------------------------------------mettre le caractère fin de texte
           }
        else //-----------------------------------------------------------------Si deuxième arobas
           {
           SMS_pos--; //--------------------------------------------------------Alors : on l'enlève du buffer de réception
           SMS_rcpt_message[SMS_pos]=0; //--------------------------------------et on le remplace par un fin de chaine
           Arobas=0; //---------------------------------------------------------interdira d'enregistrer dans le buffer de reception
           //-------------------------------------------------------------------c'est le OK Cr Lf qui marquera la reception du message complet
           }
        }
     
     //Analyse des réponses du module GSM  on recherche OK Cr Lf ou UNREAD******
     switch (Etat_gsm)
            {
            case 0:
                 {
                 Reponse_GSM=-1;
                 if (tmp=='O') Etat_gsm=1; //-----------------------------------Suivant peut etre 'K'
                 if (tmp=='>') Etat_gsm=10; //----------------------------------Suivant peut etre " " suite a l'envoi d'une commande d'envoi de message
                 if (tmp=='U') Etat_gsm=120; //---------------------------------Suivant peut etre 'N' pour UNREAD
                 break;
                 }
            case 1:
                 {
                 if (tmp=='K') //-----------------------------------------------Si K reçu
                    {
                    Reponse_GSM=GSM_OK; //--------------------------------------Alors : Reponse du module gsm  on a reçu OK
                    Etat_gsm=20; //---------------------------------------------suivant peut être Cr et puis Lf
                    }
                 else Etat_GSM=0; //--------------------------------------------Sinon remise à zéro K pas arrivé
                 break;
                 }
            case 10:
                 {
                 if (tmp==' ')
                    {
                    Signal_Reponse_gsm=1; //------------------------------------On a reçu "> " on va pouvoir ecrire le message au gsm
                    Reponse_GSM= GSM_PRET_A_RECEVOIR_MESSAGE;
                    ReponseID=Reponse_GSM; //-----------------------------------Sauvegarder réponse GSM pret a recevoir message avant traitement
                    Etat_gsm=0; //----------------------------------------------Remise à zero etat gsm
                    break;
                    }
                 }
            case 20:
                 {
                 if (tmp==13)  Etat_gsm=21; //----------------------------------Si Cr arrivé alors attente du Lf
                 else Etat_gsm=0; //--------------------------------------------Sinon pas un Cr qui arrive après OK donc remise à zero
                 break;
                 }
            case 21:
                 {
                 if (tmp==10) //------------------------------------------------Si Lf arrivé alors une réponse OK_Cr_Lf complète
                    {
                    Signal_Reponse_gsm=1; //------------------------------------Alors : signaler réponse valide du gsm
                    if (ReponseID==GSM_UNREAD) //-------------------------------Si un message non lu
                       {
                       Message_complet=1; //------------------------------------Alors :signalé message complet dans le buffer de reception
                       }
                    ReponseID=Reponse_GSM; //-----------------------------------Puis sauvegarder réponse OK avant traitement
                    }
                 Etat_gsm=0; //-------------------------------------------------Fin réception OK Cr Lf
                 }
            case 120:
                 {
                 if (tmp=='N')  Etat_gsm=121; //--------------------------------Si N de unread prochain devrait etre R
                 else Etat_gsm=0; //--------------------------------------------Sinon pas N qui arrive après U donc remise à zero
                 break;
                 }
            case 121:
                 {
                 if (tmp=='R')  Etat_gsm=122; //--------------------------------Si R de unread prochain devrait etre E
                 else Etat_gsm=0; //--------------------------------------------Sinon pas R qui arrive après  N donc remise à zero
                 break;
                 }
            case 122:
                 {
                 if (tmp=='E')  Etat_gsm=123; //--------------------------------Si E de unread prochain devrait etre A
                 else Etat_gsm=0; //--------------------------------------------Sinon pas E qui arrive après R donc remise à zero
                 break;
                 }
            case 123:
                 {
                 if (tmp=='A')  Etat_gsm=124; //--------------------------------Si A de unread prochain devrait etre D
                 else Etat_gsm=0; //--------------------------------------------Sinon pas A qui arrive après E donc remise à zero
                 break;
                 }
            case 124:
                 {
                 if (tmp=='D') //-----------------------------------------------Un message unread de trouvé
                    {
                    Signal_Reponse_gsm=1; //------------------------------------Signaler réponse valide du gsm
                    Reponse_GSM=GSM_UNREAD; //----------------------------------Signaler message non lu trouvé
                    ReponseID=Reponse_GSM; //-----------------------------------Sauvegarder réponse avant traitement
                    SMS_pos=0; //-----------------------------------------------Buffer reception initialisé au début
                    SMS_rcpt_message[0]=0; //-----------------------------------....
                    Message_complet=0; //---------------------------------------Signalé message incomplet
                    Arobas=0; //------------------------------------------------Init attente début message signalé par une @
                    }
                 Etat_gsm=0; //-------------------------------------------------Remise à zero etat gsm
                 break;
                 }
            default:
                 {
                  Etat_gsm=0;
                 }
            }
     
     //Recherche du numéro de téléphone entrant débutant par "+"****************
     switch (Etat_gsm_nb)
            {
            case 0:
                 {
                 if (tmp=='"') //-----------------------------------------------Si " peut signaler un numéro entrant
                    {
                     Etat_gsm_nb=1; //------------------------------------------Alors : caractere suivant sera peut etre un +
                     Phone_nb_pos=0;
                    }
                 break;
                 }
            case 1:
                 {
                 if (tmp=='+') //-----------------------------------------------Si "+" on a un numéro de téléphone
                    {
                    phone_nb_entrant[Phone_nb_pos]=tmp; //----------------------Alors : mémorise le +
                    Phone_nb_pos++; //------------------------------------------positionné sur le caractére suivant
                    phone_nb_entrant[Phone_nb_pos]=0; //------------------------mettre un fin de chaine
                    Etat_gsm_nb=2;
                    }
                 else //--------------------------------------------------------Sinon reinit
                    {
                    Etat_gsm_nb=0;
                    Phone_nb_pos=0;
                    }
                 break;
                 }
            case 2:
                 {
                 if (tmp>='0' && tmp<='9') //-----------------------------------Si les caractères suivants sont des chiffres
                    {
                    phone_nb_entrant[Phone_nb_pos]=tmp; //----------------------Alors : enregistrer les chiffres du numéro
                    Phone_nb_pos++;
                    phone_nb_entrant[Phone_nb_pos]=0; //------------------------mettre un fin de chaine
                    }
                 else if (tmp=='"')//-------------------------------------------Sinon Si " alors fin du numéro
                    {
                    Phone_nb_pos=0;
                    Etat_gsm_nb=0;
                    }
                 else //--------------------------------------------------------Sinon numéro de tel invalide et reinitialiser
                    {
                    phone_nb_entrant[0]=0;
                    Phone_nb_pos=0;
                    Etat_gsm_nb=0;
                    }
                 break;
                 }
            default: //---------------------------------------------------------Défaut caractère non prévu arrivé alors reinitialiser
                 {
                 phone_nb_entrant[0]=0;
                 Phone_nb_pos=0;
                 Etat_gsm_nb=0;
                 }
            }
     }
  }
  //fin interruption************************************************************

  
  //SP d'envoi de commande ou chaine de données  au module gsm******************
  void GSM_EnvoiCmd_AT (char *s)
  {
  while (*s)
        {
        UART_Wr_Ptr (*s++); //--------------------------------------------------Envoi la chaine sur uart1
        }
        UART_Wr_Ptr(0x0D); //---------------------------------------------------Envoi terminé par un Cr
  }
  
  //SP de réception des réponses du module gsm**********************************
  short GSM_Reponse()
  {
  if (Signal_Reponse_gsm)
     {
      Signal_Reponse_gsm=0;
      return ReponseID;
     }
  else return -1;
  }
  
  //SP d'attente de reponse du module GSM***************************************
  void GSM_Attente_reponse (char type_reponse)
  {
  char attente=1;
  char reponseGSM;
  while (attente)
     {
     reponseGSM=GSM_Reponse();
     if ((reponseGSM==type_reponse) || (reponseGSM==GSM_ERROR)) attente=0; //---Attente de la reponse approprié ou reponse erreur
     }
  }
  
  //SP affichage****************************************************************
  void Afficher()
  {
  Lcd_Cmd(_LCD_CLEAR);
  Lcd_Out(1,1,txtL1_lcd); //----------------------------------------------------Afficher sur ligne 1 le contenu buffer ligne1
  Lcd_Out(2,1,txtL2_lcd); //----------------------------------------------------Afficher sur ligne 2 le contenu buffer ligne2
  }
  
  //SP délai 3s*****************************************************************
  void Pause_3S()
  {
   Delay_ms(3000);
  }

  //****************************************************************************
  //**************             FIN DU SOUS PROGRAMME              **************
  //****************************************************************************



  //****************************************************************************
  //*************          PROGRAMME PRINCIPAL                     *************
  //****************************************************************************

  void main()
  {
  
  // tous les ports paramétrés en numérique*************************************
  ANSELA=0;
  ANSELB=0;
  ANSELC=0;
  ANSELD=0;
  ANSELE=0;

  SLRCON=0; //------------------------------------------------------------------Output slew rate sur tous les ports en standard
  
  
  // Grille de commutation sur UART controlé par RC0 et RC1*********************
  A_4052_direction=0; //--------------------------------------------------------Port en sortie
  B_4052_direction=0;
  
  // Sélection de M1 pour le module GSM*****************************************
  A_4052=0;
  B_4052=0;

  //LED de test*****************************************************************
  led_Direction=0;
  led=1;
  
  
  //initialisation de l'UART****************************************************
  UART1_Init(9600);
  Delay_ms(300);
  
  
  //valider les interruptions avant l'envoi des commandes d'init au module gsm**
  RCIE_bit = 1; //--------------------------------------------------------------Interruption réception sur UART seront valides si GIE=1
  PEIE_bit = 1; //--------------------------------------------------------------Interruption périphérique valide seront valide si GIE=1
  GIE_bit = 1; //---------------------------------------------------------------Interruption GIE valide

  GSM_ON_OFF_Direction = 0;
  GSM_ON_OFF =0 ;

  RTS_Direction = 0; //---------------------------------------------------------RTS bloqué
  RTS = 0;
  
  
  //initialisation du lcd, écran effacé, sans curseur***************************
  Lcd_Init();
  Lcd_Cmd(_LCD_CLEAR);
  Lcd_Cmd(_LCD_CURSOR_OFF);
  
  strcpy(txtL1_lcd, "Init GSM");
  strcpy(txtL2_lcd, "*");
  Afficher();
  
  GSM_ON_OFF =1 ; //------------------------------------------------------------Forcer un reset sur le module GSM
  Delay_ms(2500); //------------------------------------------------------------Attente
  GSM_ON_OFF =0 ; //------------------------------------------------------------GSM on
  Pause_3s(); //----------------------------------------------------------------Attente 3s que le module gsm trouve le réseau
  led=0;
  
  
  //autoconfiguration du module gsm à la vitesse de 9600************************
  strcpy (AT_cmd,AT);
  while(GSM_Reponse() != GSM_OK) //---------------------------------------------Attente d'une réponse OK Cr Lf
     {
     OERR1_bit=0;
     FERR1_bit=0;
     GSM_EnvoiCmd_AT (AT_cmd); //-----------------------------------------------à l'envoi de AT Cr
     Delay_ms(100); //----------------------------------------------------------attente de 100ms pour la réponse
     }

  strcpy (AT_cmd,AT_ECHO_OFF);
  GSM_EnvoiCmd_AT (AT_cmd); //--------------------------------------------------Pas d'écho aux commandes envoyés au gsm
  GSM_Attente_reponse(GSM_OK); //-----------------------------------------------Attente d'un "OKCrLf"

  strcpy (AT_cmd,AT_MODE_TEXTE);
  GSM_EnvoiCmd_AT (AT_cmd); //--------------------------------------------------GSM en mode texte
  GSM_Attente_reponse(GSM_OK); //-----------------------------------------------Attente d'un "OKCrLf"
  
  
  //effacement des messages anciens*********************************************
  strcpy (AT_cmd,AT_EFFACEMENT_MESSAGE);
  
  while(GSM_Reponse() != GSM_OK) //---------------------------------------------Attente d'une réponse OK Cr Lf
     {
     GSM_EnvoiCmd_AT (AT_cmd); //-----------------------------------------------à la demande d'effacement des messages
     Delay_ms(500); //----------------------------------------------------------attente de 500ms pour la réponse
     }
     
  strcpy(txtL1_lcd, "Initialisation");
  strcpy(txtL2_lcd, "GSM termine");
  Afficher();
  
  //BOUCLE INFINI pour envoi et réception des messages**************************
  while(1)
     {
     if (k==500) //-------------------------------------------------------------On verifie un message toutes les 5,2s
        {
        //lecture des messages arrivés
        /* ex d'un message en récetion suite à AT+CMGL="ALL":
        
        +CMGL: 2,"REC UNREAD","+85291234567",,"07/02/18,00:07:22+32"
        @lat:xxxxxxx.xxxx,lon:xxxxxxxxx.xxxx,vit=xxxxx.xx@
        
        OK
        */
          strcpy (AT_cmd,AT_STATUS_MESSAGE);
          GSM_EnvoiCmd_AT (AT_cmd); //------------------------------------------Permet de chercher les messages non lu UNREAD
          Delay_ms(200); //-----------------------------------------------------Attente lecture message s'il y en a
          k=0;
        } //--------------------------------------------------------------------500x10ms +200ms=5,2s
     
     k++;
     Delay_ms(10); //-----------------------------------------------------------10ms de la boucle 500 on neglige le temps instruction
      
     if(Message_complet) //-----------------------------------------------------Si message complet entre @ arrivé
          {
            Message_complet=0; //-----------------------------------------------Alors : effacé le signalement
            
            //affiché le message************************************************
            strcpy(txtL1_lcd, phone_nb_entrant); //-----------------------------Affiché le numéro de telephone du sms arrivé
            strcpy(txtL2_lcd, sms_rcpt_message); //-----------------------------Affiché le sms entre deux @
            Afficher();
            Delay_ms(20); //----------------------------------------------------Attendre 20ms avant effacer message
            
            //effacé le message*************************************************
            strcpy (AT_cmd,AT_EFFACEMENT_MESSAGE);
            while(GSM_Reponse() != GSM_OK) //-----------------------------------Attente d'une réponse OK Cr Lf
               {
                GSM_EnvoiCmd_AT (AT_cmd); //------------------------------------à la demande d'effacement des messages
                Delay_ms(500); //-----------------------------------------------attente de 500ms pour la réponse
               }
          }
     }
  }