/*
 * html.c
 *
 * HTML parsing functions for the Quoteriser. These support only a subset
 * of full HTML; refer to the documentation for a full description of the
 * tags available.
 *
 *      Created: 4th March, 1997
 * Version 1.00: 9th March, 1997
 * Version 2.00: 8th December, 1997
 * Version 2.10: 8th September, 1998
 *
 * (C) 1997-1998 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "html.h"
#include "general.h"


int HTMLGetNextChunk(char *pszInput, char *pszChunk, char **ppszNext)
/*
 * Get the next portion of the input HTML that can be acted upon as a
 * whole. Chunks can be a whole word, part of a word (i.e. a word
 * interrupted by an HTML tag) or an HTML tag.
 *
 * We're fairly tolerant on broken HTML here; the block of code immediately
 * following the case shows what we accept as a terminator to a tag, macro,
 * etc., in the comment on the right "oops!" occurs for broken HTML.
 *
 * char *pszInput	- the input HTML
 * char *pszChunk	- the next chunk (output)
 * char *pszNext	- the continuation point (output)
 *
 * Return: HTML_WORD_END  - pszChunk points to a whole word
 *         HTML_WORD_MID  - pszChunk points to part of a word
 *         HTML_TAG_END   - pszChunk points to an HTML tag at the end of a word
 *         HTML_TAG_MID   - pszChunk points to an HTML in the middle of a word
 *         HTML_MACRO_END - pszChunk points to an HTML macro at the end of a word
 *         HTML_MACRO_MID - pszChunk points to an HTML macro in the middle of a word
 *         HTML_END       - we have reached the end of the input
 */
{
	char *pszStartChunk;
	int iLength, iRet;

	/* ignore white space */
	pszStartChunk = pszInput;
	while ((pszStartChunk[0] != '\0') && isspace(pszStartChunk[0]))
		pszStartChunk++;

	switch (pszStartChunk[0]) {
		case '<':
			/* we at the start of an HTML tag */
			(*ppszNext) = strchr(pszStartChunk, '>');		/* closing bracket */
			(*ppszNext) = strfchr(pszStartChunk, *ppszNext, '\0');	/* end of input - oops! */
			
			/* copy the tag into pszChunk and append closing angle bracket */
			iLength = (*ppszNext) - pszStartChunk;
			strncpy(pszChunk, pszStartChunk, iLength);
			pszChunk[iLength] = '\0';
			strcat(pszChunk, ">");

			/* skip over the angle bracket if we did not insert it */
			if ((*ppszNext)[0] == '>')
				(*ppszNext) += 1;

			/* determine return value */
			if (isspace((*ppszNext)[0]))
				iRet = HTML_TAG_END;
			else
				iRet = HTML_TAG_MID;
			break;

		case '&':
			/* we are at the start of an HTML macro */
			(*ppszNext) = strchr(pszStartChunk, ';');		/* semi-colon */
			(*ppszNext) = strfchr(pszStartChunk, *ppszNext, '\n');	/* line feed - oops! */
			(*ppszNext) = strfchr(pszStartChunk, *ppszNext, '\r');	/* carriage return - oops! */
			(*ppszNext) = strfchr(pszStartChunk, *ppszNext, '\t');	/* tab - oops! */
			(*ppszNext) = strfchr(pszStartChunk, *ppszNext, ' ');	/* space - oops! */
			(*ppszNext) = strfchr(pszStartChunk, *ppszNext, '\0');	/* end of input - oops! */

			/* copy the macro into pszChunk and append the closing semi-colon */
			iLength = (*ppszNext) - pszStartChunk;
			strncpy(pszChunk, pszStartChunk, iLength);
			pszChunk[iLength] = '\0';
			strcat(pszChunk, ";");

			/* skip over the semi-colon if we did not insert it */
			if (*(*ppszNext) == ';')
				(*ppszNext) += 1;

			/* determine return value */
			if (isspace((*ppszNext)[0]))
				iRet = HTML_MACRO_END;
			else
				iRet = HTML_MACRO_MID;
			break;

		case '\0':
			/* we have reached the end of the input */
			iRet = HTML_END;
			break;

		default:
 	   		/* we are at the start of a normal word */
			(*ppszNext) = strchr(pszStartChunk, ' ');		/* space */
			(*ppszNext) = strfchr(pszStartChunk, *ppszNext, '\t');	/* tab */
			(*ppszNext) = strfchr(pszStartChunk, *ppszNext, '\n');	/* line feed */
			(*ppszNext) = strfchr(pszStartChunk, *ppszNext, '\r');	/* carriage return */
			(*ppszNext) = strfchr(pszStartChunk, *ppszNext, '<');	/* start of a tag */
			(*ppszNext) = strfchr(pszStartChunk, *ppszNext, '&');	/* start of a macro */
			(*ppszNext) = strfchr(pszStartChunk, *ppszNext, '\0');	/* end of input */

			/* copy the chunk into pszChunk */
			iLength = (*ppszNext) - pszStartChunk;
			strncpy(pszChunk, pszStartChunk, iLength);
			pszChunk[iLength] = '\0';

			/* determine return value */
			if (isspace((*ppszNext)[0]))
				iRet = HTML_WORD_END;
			else
				iRet = HTML_WORD_MID;
			break;
	}

	return (iRet);
}


int HTMLParseTag(char *pszTag)
/*
 * Decode an HTML tag.
 *
 * char *pszTag	- the tag to be decoded (including enclosing brackets).
 *
 * Return: HTML_UNKNOWN       - unrecognised tag
 *         HTML_PARAGRAPH     - new paragraph tag <P>
 *         HTML_ITALICS_START - opening italices tag <I>
 *         HTML_ITALICS_END   - closing italices tag </I>
 *         HTML_BOLD_START    - opening "bold" tag <B>, <EM> or <STRONG>
 *         HTML_BOLD_END      - closing "bold" tag </B>, </EM> or </STRONG>
 *         HTML_LINEBREAK     - line break tag <BR>
 *         HTML_TABLE_ROW     - table row start <TR>
 *         HTML_TABLE_DATA    - table data (i.e. cell) start <TD>
 *         HTML_INVALID       - this is an incorrect tag (i.e. missing brackets)
 */
{
	char *pch, szElement[HTML_MAX_ELEM + 1];
	int i, iRet;

	/* perform some sanity checking */
	if ((pszTag[0] != '<') || (strchr(pszTag, '>') == NULL))
		return (HTML_INVALID);

	/* extract the element from within the tag */
	pch = pszTag + 1;
	while (isspace(*pch) && ((*pch) != '>'))
		pch++;
	i = 0;
	while (!isspace(*pch) && ((*pch) != '>') && (i < HTML_MAX_ELEM)) {
		szElement[i++] = *pch;
		pch++;
	}
	szElement[i] = '\0';

	/* find out what the element is and determine return value */
	if (strcmpci(szElement, "P") == 0)
		iRet = HTML_PARAGRAPH;
	else if (strcmpci(szElement, "I") == 0)
		iRet = HTML_ITALICS_START;
	else if (strcmpci(szElement, "/I") == 0)
		iRet = HTML_ITALICS_END;
	else if (strcmpci(szElement, "B") == 0)
		iRet = HTML_BOLD_START;
	else if (strcmpci(szElement, "/B") == 0)
		iRet = HTML_BOLD_END;
	else if (strcmpci(szElement, "EM") == 0)
		iRet = HTML_BOLD_START;
	else if (strcmpci(szElement, "/EM") == 0)
		iRet = HTML_BOLD_END;
	else if (strcmpci(szElement, "STRONG") == 0)
		iRet = HTML_BOLD_START;
	else if (strcmpci(szElement, "/STRONG") == 0)
		iRet = HTML_BOLD_END;
	else if (strcmpci(szElement, "BR") == 0)
		iRet = HTML_LINEBREAK;
	else if (strcmpci(szElement, "TR") == 0)
		iRet = HTML_TABLE_ROW;
	else if (strcmpci(szElement, "TD") == 0)
		iRet = HTML_TABLE_DATA;
	else
		iRet = HTML_UNKNOWN;

	return (iRet);
}


int HTMLParseMacro(char *pszMacro)
/*
 * Decode an HTML macro.
 *
 * char *pszMacro	- the macro to be decoded (including enclosing ampersand/semi-colon)
 *
 * Return: the ISO Latin-1 code corresponding to the macro
 *         HTML_INVALID if this macro is not recognised
 *
 */
{
	char szStripped[HTML_MAX_MACRO + 1];
	char *pchSemiColon;
	int iLength, iRet;

	/* perform some sanity checking */
	if ((pszMacro[0] != '&') || ((pchSemiColon = strchr(pszMacro, ';')) == NULL))
		return (HTML_INVALID);

	/* strip the ampersand and semi-colon */
	if ((iLength = pchSemiColon - pszMacro - 1) > HTML_MAX_MACRO)
		iLength = HTML_MAX_MACRO;
	strncpy(szStripped, pszMacro + 1, iLength);
	szStripped[iLength] = '\0';

	iRet = HTML_INVALID;
	if (szStripped[0] == '#') {
		/* the macro gives the ISO Latin-1 code */
		iRet = atoi(szStripped + 1);
		if ((iRet == 0) || (iRet > 255))
			iRet = HTML_INVALID;
	} else if (strcmp(szStripped, "quot") == 0)
		iRet = 34;
	else if (strcmp(szStripped, "amp") == 0)
		iRet = 38;
	else if (strcmp(szStripped, "lt") == 0)
		iRet = 60;
	else if (strcmp(szStripped, "gt") == 0)
		iRet = 62;
       else if (strcmp(szStripped, "nbsp") == 0)
		iRet = 160;
	else if (strcmp(szStripped, "iexcl") == 0)
		iRet = 161;
	else if (strcmp(szStripped, "cent") == 0)
		iRet = 162;
	else if (strcmp(szStripped, "pound") == 0)
		iRet = 163;
	else if (strcmp(szStripped, "curren") == 0)
		iRet = 164;
	else if (strcmp(szStripped, "yen") == 0)
		iRet = 165;
	else if (strcmp(szStripped, "brvbar") == 0)
		iRet = 166;
	else if (strcmp(szStripped, "sect") == 0)
		iRet = 167;
	else if (strcmp(szStripped, "uml") == 0)
		iRet = 168;
	else if (strcmp(szStripped, "copy") == 0)
		iRet = 169;
	else if (strcmp(szStripped, "ordf") == 0)
		iRet = 170;
	else if (strcmp(szStripped, "laquo") == 0)
		iRet = 171;
	else if (strcmp(szStripped, "not") == 0)
		iRet = 172;
	else if (strcmp(szStripped, "shy") == 0)
		iRet = 173;
	else if (strcmp(szStripped, "reg") == 0)
		iRet = 174;
	else if (strcmp(szStripped, "macr") == 0)
		iRet = 175;
	else if (strcmp(szStripped, "deg") == 0)
		iRet = 176;
	else if (strcmp(szStripped, "plusmn") == 0)
		iRet = 177;
	else if (strcmp(szStripped, "sup2") == 0)
		iRet = 178;
	else if (strcmp(szStripped, "sup3") == 0)
		iRet = 179;
	else if (strcmp(szStripped, "acute") == 0)
		iRet = 180;
	else if (strcmp(szStripped, "micro") == 0)
		iRet = 181;
	else if (strcmp(szStripped, "para") == 0)
		iRet = 182;
	else if (strcmp(szStripped, "middot") == 0)
		iRet = 183;
	else if (strcmp(szStripped, "cedil") == 0)
		iRet = 184;
	else if (strcmp(szStripped, "sup1") == 0)
		iRet = 185;
	else if (strcmp(szStripped, "ordm") == 0)
		iRet = 186;
	else if (strcmp(szStripped, "raquo") == 0)
		iRet = 187;
	else if (strcmp(szStripped, "frac14") == 0)
		iRet = 188;
	else if (strcmp(szStripped, "frac12") == 0)
		iRet = 189;
	else if (strcmp(szStripped, "frac34") == 0)
		iRet = 190;
	else if (strcmp(szStripped, "iquest") == 0)
		iRet = 191;
	else if (strcmp(szStripped, "Agrave") == 0)
		iRet = 192;
	else if (strcmp(szStripped, "Aacute") == 0)
		iRet = 193;
	else if (strcmp(szStripped, "Acirc") == 0)
		iRet = 194;
	else if (strcmp(szStripped, "Atilde") == 0)
		iRet = 195;
	else if (strcmp(szStripped, "Auml") == 0)
		iRet = 196;
	else if (strcmp(szStripped, "Aring") == 0)
		iRet = 197;
	else if (strcmp(szStripped, "AElig") == 0)
		iRet = 198;
	else if (strcmp(szStripped, "Ccedil") == 0)
		iRet = 199;
	else if (strcmp(szStripped, "Egrave") == 0)
		iRet = 200;
	else if (strcmp(szStripped, "Eacute") == 0)
		iRet = 201;
	else if (strcmp(szStripped, "Ecirc") == 0)
		iRet = 202;
	else if (strcmp(szStripped, "Euml") == 0)
		iRet = 203;
	else if (strcmp(szStripped, "Igrave") == 0)
		iRet = 204;
	else if (strcmp(szStripped, "Iacute") == 0)
		iRet = 205;
	else if (strcmp(szStripped, "Ihat") == 0)
		iRet = 206;
	else if (strcmp(szStripped, "Iuml") == 0)
		iRet = 207;
	else if (strcmp(szStripped, "ETH") == 0)
		iRet = 208;
	else if (strcmp(szStripped, "Ntilde") == 0)
		iRet = 209;
	else if (strcmp(szStripped, "Ograve") == 0)
		iRet = 210;
	else if (strcmp(szStripped, "Oacute") == 0)
		iRet = 211;
	else if (strcmp(szStripped, "Ocirc") == 0)
		iRet = 212;
	else if (strcmp(szStripped, "Otilde") == 0)
		iRet = 213;
	else if (strcmp(szStripped, "Ouml") == 0)
		iRet = 214;
	else if (strcmp(szStripped, "times") == 0)
		iRet = 215;
	else if (strcmp(szStripped, "Oslash") == 0)
		iRet = 216;
	else if (strcmp(szStripped, "Ugrave") == 0)
		iRet = 217;
	else if (strcmp(szStripped, "Uacute") == 0)
		iRet = 218;
	else if (strcmp(szStripped, "Uhat") == 0)
		iRet = 219;
	else if (strcmp(szStripped, "Uuml") == 0)
		iRet = 220;
	else if (strcmp(szStripped, "Yacute") == 0)
		iRet = 221;
	else if (strcmp(szStripped, "THORN") == 0)
		iRet = 222;
	else if (strcmp(szStripped, "szlig") == 0)
		iRet = 223;
	else if (strcmp(szStripped, "agrave") == 0)
		iRet = 224;
	else if (strcmp(szStripped, "aacute") == 0)
		iRet = 225;
	else if (strcmp(szStripped, "acirc") == 0)
		iRet = 226;
	else if (strcmp(szStripped, "atilde") == 0)
		iRet = 227;
	else if (strcmp(szStripped, "auml") == 0)
		iRet = 228;
	else if (strcmp(szStripped, "aring") == 0)
		iRet = 229;
	else if (strcmp(szStripped, "aelig") == 0)
		iRet = 230;
	else if (strcmp(szStripped, "ccedil") == 0)
		iRet = 231;
	else if (strcmp(szStripped, "egrave") == 0)
		iRet = 232;
	else if (strcmp(szStripped, "eacute") == 0)
		iRet = 233;
	else if (strcmp(szStripped, "ehat") == 0)
		iRet = 234;
	else if (strcmp(szStripped, "euml") == 0)
		iRet = 235;
	else if (strcmp(szStripped, "igrave") == 0)
		iRet = 236;
	else if (strcmp(szStripped, "iacute") == 0)
		iRet = 237;
	else if (strcmp(szStripped, "ihat") == 0)
		iRet = 238;
	else if (strcmp(szStripped, "iuml") == 0)
		iRet = 239;
	else if (strcmp(szStripped, "eth") == 0)
		iRet = 240;
	else if (strcmp(szStripped, "ntilde") == 0)
		iRet = 241;
	else if (strcmp(szStripped, "ograve") == 0)
		iRet = 242;
	else if (strcmp(szStripped, "oacute") == 0)
		iRet = 243;
	else if (strcmp(szStripped, "ohat") == 0)
		iRet = 244;
	else if (strcmp(szStripped, "otilde") == 0)
		iRet = 245;
	else if (strcmp(szStripped, "ouml") == 0)
		iRet = 246;
	else if (strcmp(szStripped, "ugrave") == 0)
		iRet = 249;
	else if (strcmp(szStripped, "uacute") == 0)
		iRet = 250;
	else if (strcmp(szStripped, "uhat") == 0)
		iRet = 251;
	else if (strcmp(szStripped, "uuml") == 0)
		iRet = 252;
	else if (strcmp(szStripped, "yacute") == 0)
		iRet = 253;
	else if (strcmp(szStripped, "thorn") == 0)
		iRet = 254;
	else if (strcmp(szStripped, "yuml") == 0)
		iRet = 255;
	else
		iRet = HTML_INVALID;

	return (iRet);
}


char *HTMLMakePlain(char *pszString)
/*
 * Make an HTML string into plain text, i.e.
 *   (i)   remove all HTML tags,
 *   (ii)  compress all white space into a single space character, and
 *   (iii) perform macro substitutions.
 *
 * char *pszString	- the string to be made plain
 *
 * Return: pszString
 */
{
	char *pch1, *pch2, *pch3;
	char szMacro[HTML_MAX_MACRO + 1];

	/* start at the beginning of the string */
	pch1 = pch2 = pszString;

	/* skip leading white space */
	while (isspace(*pch2) && ((*pch2) != '\0'))
		pch2++;

	while ((*pch2) != '\0') {
		if (isspace(*pch2)) {
			/* white space; add a space to pszString and skip the rest */
			(*pch1) = ' ';
			pch1++;
			while (isspace(*pch2) && ((*pch2) != '\0'))
				pch2++;
		} else if ((*pch2) == '<') {
			/* tag; skip until end */
			do
				pch2++;
			while (((*pch2) != '>') && ((*pch2) != '\0'));
			if ((*pch2) == '>')
				pch2++;
		} else if ((*pch2) == '&') {
			/* macro; substitute */
			pch3 = strchr(pch2, ';');		/* semi-colon */
			pch3 = strfchr(pch2, pch3, '\n');	/* line feed - oops! */
			pch3 = strfchr(pch2, pch3, '\r');	/* carriage return - oops! */
			pch3 = strfchr(pch2, pch3, '\t');	/* tab - oops! */
			pch3 = strfchr(pch2, pch3, ' ');	/* space - oops! */
			pch3 = strfchr(pch2, pch3, '\0');	/* end of input - oops! */

			/* copy the macro into szMacro and append the closing semi-colon */
			strncpy(szMacro, pch2, pch3 - pch2);
			szMacro[pch3 - pch2] = '\0';
			strcat(szMacro, ";");

			/* substitute */
			if (((*pch1) = HTMLParseMacro(szMacro)) == HTML_INVALID)
				(*pch1) = 255;
			pch1++;
			pch2 = pch3;
		} else {
			/* normal character; append to pszString and continue */
			(*pch1) = (*pch2);
			pch1++;
			pch2++;
		}
	}
	(*pch1) = '\0';

	return (pszString);
}
