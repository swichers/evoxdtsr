#pragma once

#define HELP_CAP		"Eh, no docs yet."
#define RUNDOWN_TEXT	"I haven't gotten around to really doing this yet.\n\n" \
						"If you need assistance please pop on to EFNet and into #mphacking. " \
						"I would really appreciate any suggestions, or bug reports you may have. " \
						"EvoX DTSR can only get better via user feedback."

#define PORT			23
#define DEF_IP			0xC0A80198 //192.168.1.152
#define DEF_LOG			"logfile.txt"
#define DEF_CAPACITY	1000

#define ANSI_CLR_TO_ENDL	"\x1B[K"
#define CRLF				"\r\n"

#define CBBPEXE		"Execute"
#define CBBPBYTEW	"BYTE: Write"
#define CBBPBYTER	"BYTE: Read"
#define CBBPBYTERW	"BYTE: RW"
#define CBBPWORDW	"WORD: Write"
#define CBBPWORDR	"WORD: Read"
#define CBBPWORDRW	"WORD: RW"
#define CBBPDWORDW	"DWORD: Write"
#define CBBPDWORDR	"DWORD: Read"
#define CBBPDWORDRW	"DWORD: RW"

#define ZERO		"0"
#define ONE			"1"
#define TWO			"2"
#define THREE		"3"

#define STAT_START	"EvoXDTSR Started."

#define ERR_LOADCC		"Couldn't load the required common controls."
#define	ERR_LOADSET		"Failed to load the settings window. Perhaps your common controls (comctl32.dll) file is too old?"
#define ERR_CAP			"Error!"
#define DEF_CAP			"EvoXDTSR"

#define BPWIN_ERR_NOT03			"Possible problem with breakpoint window. Exiting."
#define BPWIN_ERR_NOLOAD		"Could not create breakpoint window."

#define SOCK_DISCONNECTED		"EvoXDTSR disconnected."
#define SOCK_ERR_NOTCON			"Not connected."
#define SOCK_ALREADYCON			"Already connected."
#define SOCK_CONNECTED			"EvoXDTSR connected."
#define SOCK_ERRNOCONNECT		"Couldn't connect."

#define PKT_ERR_MISSADD			"An offset was not specified."

#define MEMPKT_ERR_MISSPARAM	"A required parameter was missing."

#define BPPKT_ERR_UNSUPBPSET	"Unsupported breakpoint type."
#define BPPKT_ERR_NORW			"Couldn't determine what flag to set (R or RW)."

#define MNU_CLEARSCRNLOG	"Do you wish to erase all of the on screen log?"
#define MNU_CLEARFILELOG	"This will erase all of the contents of the log file. Do you wish to continue?"