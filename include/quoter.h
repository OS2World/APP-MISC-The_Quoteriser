/*
 * quoter.h
 *
 * Header file containing definitions for the Quoteriser's main
 * program.
 *
 *      Created: 18th January, 1997
 * Version 1.00: 9th April, 1997
 * Version 2.00: 21st December, 1997
 * Version 2.10: 18th October, 1998
 *
 * (C) 1997 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */


#ifndef _QUOTERISER_QUOTER_H
#define _QUOTERISER_QUOTER_H

#include "types.h"

/* icon */
#define IDI_ICON		1

/* menu */
#define IDM_MAIN		2
#define IDM_FILE		20
#define IDM_OPEN_QUOTES		200
#define IDM_OPEN_AUTHORS	201
#define IDM_REORG_QUOTES	202
#define IDM_REORG_AUTHORS	203
#define IDM_CLOSE_QUOTES	204
#define IDM_CLOSE_AUTHORS	205
#define IDM_SETTINGS		206
#define IDM_EXIT		207
#define IDM_USEDDB1		208
#define IDM_EDIT		21
#define IDM_COPY_TEXT		210
#define IDM_COPY_METAFILE	211
#define IDM_AUTHORS		22
#define IDM_ADD_AUTHOR		220
#define IDM_GET_AUTHOR		221
#define IDM_GET_AUTHOR_ALL	2210
#define IDM_GET_AUTHOR_DESC	2211
#define IDM_GET_AUTHOR_NAME	2212
#define IDM_DEL_AUTHOR		222
#define IDM_DEL_AUTHOR_CURRENT	2220
#define IDM_DEL_AUTHOR_ALL	2221
#define IDM_EDIT_AUTHOR		223
#define IDM_EDIT_AUTHOR_CURRENT	2230
#define IDM_EDIT_AUTHOR_ALL	2231
#define IDM_IMPORT_AUTHORS	224
#define IDM_QUOTES		23
#define IDM_ADD_QUOTE		230
#define IDM_GET_QUOTE		231
#define IDM_GET_QUOTE_ALL	2310
#define IDM_GET_QUOTE_KEY	2311
#define IDM_GET_QUOTE_TEXT	2312
#define IDM_GET_QUOTE_NAME	2313
#define IDM_DEL_QUOTE		232
#define IDM_DEL_QUOTE_CURRENT	2320
#define IDM_DEL_QUOTE_ALL	2321
#define IDM_EDIT_QUOTE		233
#define IDM_EDIT_QUOTE_CURRENT	2330
#define IDM_EDIT_QUOTE_ALL	2331
#define IDM_IMPORT_QUOTES	234
#define IDM_QOTD		24
#define IDM_HELP		29
#define IDM_GENHELP		290
#define IDM_LICENSE		298
#define IDM_ABOUT		299

/* general strings */
#define IDS_APPNAME		3000
#define IDS_SEARCHING		3001
#define IDS_SAMPLE		3002

/* error messages */
#define IDS_OPENFAILED		3100
#define IDS_SPAWNFAILED		3101
#define IDS_NOMEM		3102
#define IDS_THREADFAILED	3103
#define IDS_NOCODE		3104
#define IDS_NOTEXT		3105
#define IDS_BADRX		3106

/* window titles */
#define IDS_TITLE_QS_KEY	3200
#define IDS_TITLE_QS_TEXT	3201
#define IDS_TITLE_AS_DESC	3202
#define IDS_TITLE_AS_NAME	3203
#define IDS_TITLE_QCODE		3204
#define IDS_TITLE_ACODE		3205
#define IDS_TITLE_FONT		3206
#define IDS_TITLE_OPEN		3207
#define IDS_TITLE_BROWSE	3208
#define IDS_TITLE_ADDA		3209
#define IDS_TITLE_EDITA		3210
#define IDS_TITLE_ADDQ		3211
#define IDS_TITLE_EDITQ		3212

/* questions for the user */
#define IDS_MAKENEWDB		3300
#define IDS_REPLACECODE		3301
#define IDS_DEFAULT		3302

/* information strings */
#define IDS_NOQUOTES		3400
#define IDS_NOAUTHORS		3401

/* message box titles */
#define IDS_ERROR		3500
#define IDS_CONFIRM		3501
#define IDS_RETRY		3502

/* intro dialogue box */
#define IDD_INTRO		40
#define IDL_INTRO_NAME		4001
#define IDL_INTRO_COPYRIGHT	4002
#define IDL_INTRO_DECLARATION	4003
#define IDL_INTRO_LICENSE	4004
#define IDC_INTRO_OKAY		4005

/* about dialogue box */
#define IDD_ABOUT		41
#define IDL_ABOUT_NAME		4100
#define IDL_ABOUT_VERSION	4101
#define IDL_ABOUT_COPYRIGHT	4102
#define IDC_ABOUT_OKAY		4103

/* settings dialogue box */
#define IDD_SETTINGS		42
#define IDL_SET_EDITOR		4200
#define IDC_SET_EDITOR		4201
#define IDL_SET_EDPARAMS	4202
#define IDC_SET_EDPARAMS	4203
#define IDL_SET_DATAPATH	4204
#define IDC_SET_DATAPATH	4205
#define IDL_SET_DOCPATH		4206
#define IDC_SET_DOCPATH		4207
#define IDL_SET_BROWSER		4208
#define IDC_SET_BROWSER		4209
#define IDL_SET_BROWSERPARAMS	4210
#define IDC_SET_BROWSERPARAMS	4211
#define IDC_SET_FONT		4212
#define IDC_SET_OKAY		4213
#define IDC_SET_DEFAULT		4214
#define IDC_SET_CANCEL		4215

/* edit quote dialogue box */
#define IDD_EDITQUOTE		43
#define IDL_EDITQ_CODE		4300
#define IDC_EDITQ_CODE		4301
#define IDL_EDITQ_SOURCE	4302
#define IDC_EDITQ_SOURCE	4303
#define IDL_EDITQ_ACODE		4304
#define IDC_EDITQ_ACODE		4305
#define IDC_EDITQ_ABROWSE	4306
#define IDL_EDITQ_KEYWORDS	4307
#define IDC_EDITQ_KEYWORDS	4308
#define IDC_EDITQ_TEXT		4309
#define IDC_EDITQ_OKAY		4310
#define IDC_EDITQ_CANCEL	4311

/* edit author dialogue box */
#define IDD_EDITAUTHOR		44
#define IDL_EDITA_CODE		4400
#define IDC_EDITA_CODE		4401
#define IDL_EDITA_SURNAME	4402
#define IDC_EDITA_SURNAME	4403
#define IDL_EDITA_GNAME		4404
#define IDC_EDITA_GNAME		4405
#define IDL_EDITA_BORN		4406
#define IDC_EDITA_BORN		4407
#define IDL_EDITA_DIED		4408
#define IDC_EDITA_DIED		4409
#define IDC_EDITA_DESC		4410
#define IDC_EDITA_OKAY		4411
#define IDC_EDITA_CANCEL	4412

/* choose code dialogue box */
#define IDD_CHOOSECODE		45
#define IDC_CODE_CODE		4501
#define IDC_CODE_OKAY		4502
#define IDC_CODE_CANCEL		4503
#define IDL_CODE_COUNT		4504
#define IDC_CODE_COUNT		4505

/* get text dialogue box */
#define IDD_GETTEXT		46
#define IDL_GETTEXT_TEXT	4600
#define IDC_GETTEXT_TEXT	4601
#define IDC_GETTEXT_OKAY	4602
#define IDC_GETTEXT_CANCEL	4603

/* quote-of-the-day dialogue box */
#define IDD_QOTD		47
#define IDL_QOTD_QDB		4700
#define IDC_QOTD_QDB		4701
#define IDC_QOTD_QDB_BROWSE	4702
#define IDL_QOTD_ADB		4703
#define IDC_QOTD_ADB		4704
#define IDC_QOTD_ADB_BROWSE	4705
#define IDC_QOTD_OKAY		4706
#define IDC_QOTD_CANCEL		4707

/* import database dialogue box */
#define IDD_IMPORT		48
#define IDL_IMPORT_SOURCE	4800
#define IDC_IMPORT_SOURCE	4801
#define IDC_IMPORT_BROWSE	4802
#define IDC_IMPORT_REPLACE	4803
#define IDC_IMPORT_REPLACE_ALL	4804
#define IDC_IMPORT_REPLACE_NO	4805
#define IDC_IMPORT_REPLACE_ASK	4806
#define IDC_IMPORT_OKAY		4807
#define IDC_IMPORT_CANCEL	4808

/* timers */
#define IDT_INTRO		60

/* messages to pass to EditQuoteBox and EditAuthorBox */
#define Q_ADD		0
#define Q_EDIT		1

/* some standard window control styles */
#define TEXT_CENTRE	SS_TEXT | DT_CENTER | DT_VCENTER | WS_VISIBLE
#define TEXT_RIGHT	SS_TEXT | DT_RIGHT | DT_VCENTER | WS_VISIBLE
#define TEXT_LEFT	SS_TEXT | DT_LEFT | DT_VCENTER | WS_VISIBLE
#define EF_STD		ES_LEFT | ES_AUTOSCROLL | ES_MARGIN | WS_VISIBLE | WS_TABSTOP
#define GB_STD		SS_GROUPBOX | DT_LEFT | DT_TOP | WS_VISIBLE | WS_GROUP
#define RADIO_STD	BS_AUTORADIOBUTTON | WS_VISIBLE | WS_TABSTOP

/* variables in quoter.c */
extern HAB hab;
extern HWND hwndMain;
extern HWND hwndVertScroll;
extern PROGSTATE ps;

/* variables in settings.c */
extern PROFILE prf;

/* functions in quoter.c */
MRESULT EXPENTRY MainWinProc(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY IntroBoxProc(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY AboutBoxProc(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY GetTextBox(HWND, ULONG, MPARAM, MPARAM);

/* functions in qdlg.c */
MRESULT EXPENTRY ChooseQuoteBox(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY EditQuoteBox(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY ImportQuotesBox(HWND, ULONG, MPARAM, MPARAM);
BOOL MenuOpenQuotes(HWND, char *);
BOOL MenuAddQuote(HWND);
BOOL MenuEditQuote(HWND, int);
void MenuDelQuote(HWND, int);
BOOL MenuGetQuoteAll(HWND);
BOOL MenuGetQuoteKeyword(HWND);
BOOL MenuGetQuoteText(HWND);
BOOL MenuGetQuoteName(HWND);

/* functions in adlg.c */
MRESULT EXPENTRY ChooseAuthorBox(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY EditAuthorBox(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY ImportAuthorsBox(HWND, ULONG, MPARAM, MPARAM);
BOOL MenuOpenAuthors(HWND, char *);
BOOL MenuAddAuthor(HWND);
BOOL MenuEditAuthor(HWND, int);
void MenuDelAuthor(HWND, int);
BOOL MenuGetAuthorAll(HWND);
BOOL MenuGetAuthorDesc(HWND);
BOOL MenuGetAuthorName(HWND);

/* functions in settings.c */
MRESULT EXPENTRY SettingsBox(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY QuoteOfTheDayBox(HWND, ULONG, MPARAM, MPARAM);
void GetINISettings(void);
void SetINISettings(void);
void SetUsedDB(char *, char *);
void GetWindowPosition(HWND);
void SaveWindowPosition(HWND);

#endif
