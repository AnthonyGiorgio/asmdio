#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "s99.h"
#include "wrappers.h"

static void* __ptr32 MALLOC31(size_t bytes) 
{
	void* __ptr32 p = __malloc31(bytes);
#ifdef DEBUG
	memset(p, 0xFE, bytes);
#endif
	return p;
}

static size_t text_unit_size(struct s99_text_unit* inunit) 
{
	struct s99_basic_text_unit* tunit = (struct s99_basic_text_unit*) inunit;
	size_t tunitsize;
	size_t i;
	switch (tunit->s99tukey) {
		case DALBRTKN:
			tunitsize = sizeof(struct s99_browse_token_text_unit); 
			break;
		default:
			tunitsize = sizeof(struct s99_basic_text_unit);
			for (i=0; i<tunit->s99tunum; ++i) {
				tunitsize += (sizeof(unsigned short) + tunit->entry[i].s99tulng);
			}
			break;
	}
	return tunitsize;
}

static struct s99_text_unit* __ptr32 calloc_text_unit(struct s99_text_unit* inunit) 
{
	struct s99_text_unit* __ptr32 outunit;
	int i;

	size_t tunitsize;
	tunitsize = text_unit_size(inunit);
	outunit = MALLOC31(tunitsize);
	if (outunit) {
		memcpy(outunit, inunit, tunitsize);
	}
	return outunit;
}

static void dumpstg(FILE* stream, void* p, size_t len) 
{
	char* buff = p;
	size_t i;
	for (i=0; i<len; ++i) { 
		if (i % 4 == 0) {
			fprintf(stream, " ");
		}
		fprintf(stream, "%2.2X", buff[i]);
	}
}

void s99_fmt_dmp(FILE* stream, struct s99rb* __ptr32 parms) 
{
	size_t tunitsize;
	unsigned int* __ptr32 p;
	unsigned int* __ptr32 pp;
	struct s99_text_unit* wtu;
	struct s99_text_unit* __ptr32 * __ptr32 textunit = parms->s99txtpp;
	struct s99_rbx* __ptr32 rbx = parms->s99s99x;
	int i=0;
	char* s99verb = (char*)&parms->s99verb;
	unsigned short* s99flag1 = (unsigned short*)&parms->s99flag1;
	unsigned short* s99error = (unsigned short*)&parms->s99error;
	unsigned short* s99info = (unsigned short*)&parms->s99info;
	unsigned int* s99flag2 = (unsigned int*)&parms->s99flag2;

	fprintf(stream, "SVC99 Formatted Dump\n");
	fprintf(stream, "  RBLN:%d VERB:%d FLAG1:%4.4X ERROR:%4.4X INFO:%4.4X FLAG2:%8.8X\n", 
		parms->s99rbln, *s99verb, *s99flag1, *s99error, *s99info, *s99flag2);         

	fprintf(stream, "  S99X: %8.8X", rbx);
	if (rbx) {
		char* s99eopts = (char*) &rbx->s99eopts;
		char* s99emgsv = (char*) &rbx->s99emgsv;
		fprintf(stream, "    EID:%6.6s EVER: %2.2X EOPTS: %2.2X SUBP: %2.2x EKEY: %2.2X EMGSV: %2.2X ECPPL: %8.8X EMSGP: %8.8X\n", 
			rbx->s99eid, rbx->s99ever, *s99eopts, rbx->s99esubp, rbx->s99ekey, *s99emgsv, rbx->s99ecppl, rbx->s99emsgp); 
	} else {
		fprintf(stream, "\n");
	}
	do {
		pp = (unsigned int* __ptr32) &textunit[i];
		wtu = (struct s99_text_unit*) textunit[i];
		tunitsize = text_unit_size(wtu);
		fprintf(stream, "  textunit[%d] %X %3zu ", i, *pp, tunitsize);
		dumpstg(stream, textunit[i], tunitsize);
		fprintf(stream, "\n");
		++i;
	} while (((*pp) & 0x80000000) == 0);
	return;
}

struct s99rb* __ptr32 s99_init(SVC99Verb_T verb, SVC99Flag1_T flag1, SVC99Flag2_T flag2, struct s99_rbx* rbxin, size_t numtextunits, ...) i
{
	va_list arg_ptr;
	size_t i;
	struct s99rb* __ptr32 parms;
	struct s99_rbx* __ptr32 rbxp;
	struct s99_text_unit* __ptr32 * __ptr32 textunit;
	unsigned int* __ptr32 pp;

	textunit = MALLOC31(numtextunits * (sizeof(struct s99_text_unit* __ptr32)));
	if (!textunit) {
		return 0;
	}
	rbxp = MALLOC31(sizeof(struct s99_rbx));
	if (!rbxp) {
		return 0;
	} 
	parms = MALLOC31(sizeof(struct s99rb));
	if (!parms) {
		return 0;
	} 

	va_start(arg_ptr, numtextunits);
	for (i=0; i<numtextunits; ++i) {
		struct s99_text_unit* inunit = (struct s99_text_unit*) va_arg(arg_ptr, void*);
		textunit[i] = calloc_text_unit(inunit);
	}
	pp = (unsigned int* __ptr32) (&textunit[numtextunits-1]);	
	*pp |= 0x80000000;

	va_end(arg_ptr);

	*rbxp = *rbxin;

	parms->s99rbln = sizeof(struct s99rb);
	parms->s99verb = verb;
	parms->s99flag1 = flag1;
	parms->s99txtpp = textunit;
	parms->s99s99x = rbxp;
	parms->s99flag2 = flag2;

	return parms;
}

void s99_free(struct s99rb* __ptr32 parms) 
{
	int i=0;
	unsigned int txtunit;
	do {
		unsigned int* __ptr32 txtunitp = (unsigned int* __ptr32) (&parms->s99txtpp[i]);
		txtunit = *txtunitp;
		free(parms->s99txtpp[i]); 
		++i;
	} while ((txtunit & 0x80000000) == 0);
	free(parms->s99txtpp);
	free(parms->s99s99x);
	free(parms);
}

int s99(struct s99rb* __ptr32 parms) 
{
	return S99A(parms);
}

void s99_em_fmt_dmp(FILE* stream, struct s99_em* __ptr32 parms) {
	char* funct = (char* ) parms;
	fprintf(stream, "SVC99 EM Parms Dump\n");
	fprintf(stream, "  EMParms %8.8X FUNCT:%2.2X IDNUM:%2.2X NMSGBAK:%d S99RBP:%8.8X RETCOD:%8.8X CPPLP:%8.8X BUFP:%8.8X WTPCDP:%8.8X\n", 
		funct, *funct, parms->emidnum, parms->emnmsgbk, parms->ems99rbp, parms->emretcod, parms->emcpplp, parms->embufp, parms->emwtpcdp);
}

int s99_prt_msg(FILE* stream, struct s99rb* __ptr32 svc99parms, int svc99rc) 
{
	struct s99_em* __ptr32 msgparms; 
	int rc;

	msgparms = MALLOC31(sizeof(struct s99_em));
	if (!msgparms) {
		return 16;
	}
	memset(msgparms, 0, sizeof(struct s99_em));
	msgparms->emreturn = 1;
	msgparms->emidnum = (svc99parms->s99verb == S99VRBUN) ? EMFREE : EMSVC99;
	msgparms->emnmsgbk = 2;
	msgparms->emretcod = svc99rc;
	msgparms->ems99rbp = svc99parms;
	msgparms->emwtpcdp = &msgparms->emwtdert;
	msgparms->embufp = &msgparms->embuf;

	fprintf(stream, "SVC99 rc:0x%x\n", svc99rc);
	fprintf(stream, "SVC99 failed with error:%d (0x%x) info: %d (0x%x)\n", 
		svc99parms->s99error, svc99parms->s99error, svc99parms->s99info, svc99parms->s99info);
	rc = S99MSGA(msgparms);
	if (rc) {
		fprintf(stream, "SVC99MSG rc:0x%x\n", rc);
		fprintf(stream, "IEFDB476 failed with rc:0x%x\n", rc);
		SVC99emfmtdmp(stderr, msgparms);
	} else {
		fprintf(stream, "%.*s\n", msgparms->embuf.embufl1, &msgparms->embuf.embuft1[msgparms->embuf.embufo1]);
		fprintf(stream, "%.*s\n", msgparms->embuf.embufl2, &msgparms->embuf.embuft2[msgparms->embuf.embufo2]);
	}

	free(msgparms);

	return rc;
}
