#line 1 "C:/Users/Nicolas PAGEOT/Documents/Electronique/Projet/MikroC/GSM_LPJJ.c"
#line 20 "C:/Users/Nicolas PAGEOT/Documents/Electronique/Projet/MikroC/GSM_LPJJ.c"
 const GSM_OK=0;
 const GSM_PRET_A_RECEVOIR_MESSAGE=1;
 const GSM_ERROR=2;
 const GSM_UNREAD=3;

 char AT_cmd[33];


 sbit RTS at LATE0_bit;
 sbit RTS_Direction at TRISE0_bit;







 sbit GSM_ON_OFF at LATE1_bit;
 sbit GSM_ON_OFF_Direction at TRISE1_bit;


 sbit A_4052 at LATC0_bit;
 sbit B_4052 at LATC1_bit;
 sbit A_4052_direction at TRISC0_bit;
 sbit B_4052_direction at TRISC1_bit;


 sbit BP1 at RA4_bit;
 sbit BP1_direction at TRISA4_bit;


 sbit led at LATD0_bit;
 sbit led_Direction at TRISD0_bit;


 char phone_nb_sortant[20] = "+33753739385";
 char phone_nb_entrant[20] = "";
 char sms_type_message[30] = "";
 char sms_rcpt_message[50] = "";


 char Etat_gsm=0;
 char Signal_Reponse_gsm=0;
 char Reponse_gsm=-1;
 short ReponseID=-1;
 char Etat_gsm_nb=0;
 unsigned char SMS_pos=0;
 char Message_complet=0;
 unsigned char Phone_nb_pos=0;
 char Msg_non_lu=0;
 char Arobas=0;
 char Envoi_En_Cours=0;


 sbit LCD_RS at RB4_bit;
 sbit LCD_EN at RB5_bit;
 sbit LCD_D4 at RB0_bit;
 sbit LCD_D5 at RB1_bit;
 sbit LCD_D6 at RB2_bit;
 sbit LCD_D7 at RB3_bit;

 sbit LCD_RS_Direction at TRISB4_bit;
 sbit LCD_EN_Direction at TRISB5_bit;
 sbit LCD_D4_Direction at TRISB0_bit;
 sbit LCD_D5_Direction at TRISB1_bit;
 sbit LCD_D6_Direction at TRISB2_bit;
 sbit LCD_D7_Direction at TRISB3_bit;

 char txtL1_lcd[15];
 char txtL2_lcd[15];

 unsigned int k=0;







 void interrupt()
 {
 char tmp;
 if (RCIF_bit==1)
 {
 tmp=UART_Rd_Ptr();
 if (ReponseID==GSM_UNREAD)
 {
 if (Arobas==1)
 {
 sms_rcpt_message[SMS_pos]=tmp;
 SMS_pos++;
 }
 if (SMS_pos==50) SMS_pos=49;
 SMS_rcpt_message[SMS_pos]=0;
 }
 if (tmp=='@')
 {
 if (Arobas==0)
 {
 Arobas=1;
 SMS_pos=0;
 SMS_rcpt_message[SMS_pos]=0;
 }
 else
 {
 SMS_pos--;
 SMS_rcpt_message[SMS_pos]=0;
 Arobas=0;

 }
 }


 switch (Etat_gsm)
 {
 case 0:
 {
 Reponse_GSM=-1;
 if (tmp=='O') Etat_gsm=1;
 if (tmp=='>') Etat_gsm=10;
 if (tmp=='U') Etat_gsm=120;
 break;
 }
 case 1:
 {
 if (tmp=='K')
 {
 Reponse_GSM=GSM_OK;
 Etat_gsm=20;
 }
 else Etat_GSM=0;
 break;
 }
 case 10:
 {
 if (tmp==' ')
 {
 Signal_Reponse_gsm=1;
 Reponse_GSM= GSM_PRET_A_RECEVOIR_MESSAGE;
 ReponseID=Reponse_GSM;
 Etat_gsm=0;
 break;
 }
 }
 case 20:
 {
 if (tmp==13) Etat_gsm=21;
 else Etat_gsm=0;
 break;
 }
 case 21:
 {
 if (tmp==10)
 {
 Signal_Reponse_gsm=1;
 if (ReponseID==GSM_UNREAD)
 {
 Message_complet=1;
 }
 ReponseID=Reponse_GSM;
 }
 Etat_gsm=0;
 }
 case 120:
 {
 if (tmp=='N') Etat_gsm=121;
 else Etat_gsm=0;
 break;
 }
 case 121:
 {
 if (tmp=='R') Etat_gsm=122;
 else Etat_gsm=0;
 break;
 }
 case 122:
 {
 if (tmp=='E') Etat_gsm=123;
 else Etat_gsm=0;
 break;
 }
 case 123:
 {
 if (tmp=='A') Etat_gsm=124;
 else Etat_gsm=0;
 break;
 }
 case 124:
 {
 if (tmp=='D')
 {
 Signal_Reponse_gsm=1;
 Reponse_GSM=GSM_UNREAD;
 ReponseID=Reponse_GSM;
 SMS_pos=0;
 SMS_rcpt_message[0]=0;
 Message_complet=0;
 Arobas=0;
 }
 Etat_gsm=0;
 break;
 }
 default:
 {
 Etat_gsm=0;
 }
 }


 switch (Etat_gsm_nb)
 {
 case 0:
 {
 if (tmp=='"')
 {
 Etat_gsm_nb=1;
 Phone_nb_pos=0;
 }
 break;
 }
 case 1:
 {
 if (tmp=='+')
 {
 phone_nb_entrant[Phone_nb_pos]=tmp;
 Phone_nb_pos++;
 phone_nb_entrant[Phone_nb_pos]=0;
 Etat_gsm_nb=2;
 }
 else
 {
 Etat_gsm_nb=0;
 Phone_nb_pos=0;
 }
 break;
 }
 case 2:
 {
 if (tmp>='0' && tmp<='9')
 {
 phone_nb_entrant[Phone_nb_pos]=tmp;
 Phone_nb_pos++;
 phone_nb_entrant[Phone_nb_pos]=0;
 }
 else if (tmp=='"')
 {
 Phone_nb_pos=0;
 Etat_gsm_nb=0;
 }
 else
 {
 phone_nb_entrant[0]=0;
 Phone_nb_pos=0;
 Etat_gsm_nb=0;
 }
 break;
 }
 default:
 {
 phone_nb_entrant[0]=0;
 Phone_nb_pos=0;
 Etat_gsm_nb=0;
 }
 }
 }
 }




 void GSM_EnvoiCmd_AT (char *s)
 {
 while (*s)
 {
 UART_Wr_Ptr (*s++);
 }
 UART_Wr_Ptr(0x0D);
 }


 short GSM_Reponse()
 {
 if (Signal_Reponse_gsm)
 {
 Signal_Reponse_gsm=0;
 return ReponseID;
 }
 else return -1;
 }


 void GSM_Attente_reponse (char type_reponse)
 {
 char attente=1;
 char reponseGSM;
 while (attente)
 {
 reponseGSM=GSM_Reponse();
 if ((reponseGSM==type_reponse) || (reponseGSM==GSM_ERROR)) attente=0;
 }
 }


 void Afficher()
 {
 Lcd_Cmd(_LCD_CLEAR);
 Lcd_Out(1,1,txtL1_lcd);
 Lcd_Out(2,1,txtL2_lcd);
 }


 void Envoi_Message(char Message_Nb)
 {
 char Type_Message[33];

 strcpy(txtL1_lcd, "routine message");
 strcpy(txtL2_lcd, "*");
 Afficher();

 strcpy (AT_cmd, "AT+CMGS=\"" );
 strcat (AT_cmd,phone_nb_sortant);
 strcat (AT_cmd,"\"");
 GSM_EnvoiCmd_AT (AT_cmd);
 GSM_Attente_reponse(GSM_PRET_A_RECEVOIR_MESSAGE);


 switch (Message_Nb)
 {
 case 1:
 strcpy(Type_Message,"Chute");
 break;
 case 2:
 strcpy(Type_Message,"Medoc non pris");
 break;
 case 3:
 strcpy(Type_Message,"Pb cardiaque");
 break;
 case 4:
 strcpy(Type_Message,"Pillulier vide");
 break;
 default:
 strcpy(Type_Message,"Anomalie systeme");
 break;
 }

 GSM_EnvoiCmd_AT (Type_Message);
 UART_Wr_Ptr (0x1A);
 UART_Wr_Ptr (0x0D);
 GSM_Attente_reponse(GSM_OK);
 Envoi_En_Cours=0;

 strcpy(txtL1_lcd, "message envoye:");
 strcpy(txtL2_lcd, Type_Message);
 Afficher();
 }



 void Pause_3S()
 {
 Delay_ms(3000);
 }
#line 393 "C:/Users/Nicolas PAGEOT/Documents/Electronique/Projet/MikroC/GSM_LPJJ.c"
 void main()
 {


 ANSELA=0;
 ANSELB=0;
 ANSELC=0;
 ANSELD=0;
 ANSELE=0;

 SLRCON=0;



 A_4052_direction=0;
 B_4052_direction=0;



 A_4052=0;
 B_4052=0;



 BP1_direction=1;



 led_Direction=0;
 led=1;



 UART1_Init(9600);
 Delay_ms(300);



 RCIE_bit = 1;
 PEIE_bit = 1;
 GIE_bit = 1;

 GSM_ON_OFF_Direction = 0;
 GSM_ON_OFF =0 ;

 RTS_Direction = 0;
 RTS = 0;



 Lcd_Init();
 Lcd_Cmd(_LCD_CLEAR);
 Lcd_Cmd(_LCD_CURSOR_OFF);

 strcpy(txtL1_lcd, "Init GSM");
 strcpy(txtL2_lcd, "*");
 Afficher();

 GSM_ON_OFF =1 ;
 Delay_ms(2500);
 GSM_ON_OFF =0 ;
 Pause_3s();
 led=0;



 strcpy (AT_cmd, "AT" );
 while(GSM_Reponse() != GSM_OK)
 {
 OERR1_bit=0;
 FERR1_bit=0;
 GSM_EnvoiCmd_AT (AT_cmd);
 Delay_ms(100);
 }

 strcpy (AT_cmd, "ATE0" );
 GSM_EnvoiCmd_AT (AT_cmd);
 GSM_Attente_reponse(GSM_OK);

 strcpy (AT_cmd, "AT+CMGF=1" );
 GSM_EnvoiCmd_AT (AT_cmd);
 GSM_Attente_reponse(GSM_OK);



 strcpy (AT_cmd, "AT+CMGD=1,4" );
 while(GSM_Reponse() != GSM_OK)
 {
 GSM_EnvoiCmd_AT (AT_cmd);
 Delay_ms(500);
 }

 strcpy(txtL1_lcd, "Initialisation");
 strcpy(txtL2_lcd, "GSM termine");

 Afficher();



 while(1)
 {
 if (k==500)
 {
#line 504 "C:/Users/Nicolas PAGEOT/Documents/Electronique/Projet/MikroC/GSM_LPJJ.c"
 strcpy (AT_cmd, "AT+CMGL=\"ALL\"" );
 GSM_EnvoiCmd_AT (AT_cmd);
 Delay_ms(200);
 k=0;
 }

 k++;
 Delay_ms(10);

 if(Message_complet)
 {
 Message_complet=0;


 strcpy(txtL1_lcd, phone_nb_entrant);
 strcpy(txtL2_lcd, sms_rcpt_message);
 Afficher();
 Delay_ms(20);


 strcpy (AT_cmd, "AT+CMGD=1,4" );
 while(GSM_Reponse() != GSM_OK)
 {
 GSM_EnvoiCmd_AT (AT_cmd);
 Delay_ms(500);
 }
 }

 if ((BP1==0) && (Envoi_En_Cours==0))
 {
 Envoi_En_Cours=1;
 Envoi_Message(2);
 }
 }
 }
