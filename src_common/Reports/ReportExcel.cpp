
#include "compiler.h"

#include <string>
#define NO_INLINE
#include <list>
#include <map>

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include "datetime.h"

#include "myansi.h"
#include "config_struct.h"

#include "gd.h"
#include "editpath.h"
#include "translate.h"
#include "ReportExcel.h"
#include "ReportFuncs.h"

#include "SerialReg.h"
#ifndef DEF_MAC
#include "postproc.h"
#endif

excelFile::excelFile() : baseFile()
{
	m_style = FORMAT_EXCEL;
	fileExtension = EXCEL_EXT;
	cellType = DATA;
	//file = 0;
	rowth=0;
	columth=0;
	sessionWriter = new CQExcelSessionWriter( fileExtension, *this );
}

excelFile::~excelFile()
{
}

int excelFile::Init( const char *fileName, FILE *filePtr )
{
/*	if ( filePtr == NULL )
		file = fopen( fileName, "wb" );
	else
		file = filePtr;*/

//	if ( file )
	if ( filePtr )
	{
		writeBOFExcel( filePtr );
		return 1;
	}
	return 0;
}

void excelFile::Stat_WriteBold( FILE *fp, const char *txt )
{
	columth=0;
	rowth++;
	writeTextGeneral( txt, fp );
}

void excelFile::Stat_WriteLine( FILE *fp, const char *txt )
{
	if (! strcmp(txt, " "))
	{
		initColumn(); 
		writeTextRightWithRLBorder( txt, fp );  // for search engine
	}
	else
	{
		columth=0;
		rowth++;
		writeTextGeneral( txt, fp ); //for log processimg time
		rowth=rowth+2;
	}
}

void excelFile::Stat_WriteRowStart( FILE *fp, long rgb, long col )
{
	tablewidth=columth;
	columth=1;
}

void excelFile::Stat_WriteTableEnd( FILE *fp )
{
	writeTableBottomLine( tablewidth+1, fp );
}




void excelFile::Stat_WriteRowEnd( FILE *fp )
{
	rowth++;
}

void excelFile::writeNextData()
{
	columth++;
}

void excelFile::Stat_WriteTitle( FILE *fp, long colspan, long list, const char *title )
{
	columth=1;	
	rowth=rowth+2;

	writeTextFont14( title, fp ); // eg, Hourly
	columth=0;  //to write table heading //eg request %
	rowth++; 
} 

void excelFile::writeTextFont14( const char* text, FILE *fp )
{
	RGBAttr[0]=00;
	RGBAttr[1]=0x80;
	RGBAttr[2]=00;
	writeToExcel( text, fp );

}

void excelFile::Stat_WriteNumberData( FILE *fp, long num )
{
	writeNumberRightWithRLBorder( num, fp );	
}

void excelFile::Stat_WriteFractionData( FILE *fp, long num, long num2 )
{
	writeNumberRightWithRLBorder( num, fp );
}

void excelFile::Stat_WriteColURLjump( FILE *fp, long cols, long rgb, const char *url, const char *name  )
{
	writeTextGeneral( name, fp );
	columth++;
}

void excelFile::Stat_WriteURL( FILE *fp, short cols, long rgb, const char *url, const char *name  )
{
	writeTextRightWithRLBorder( name, fp ); 
}

void excelFile::Stat_WriteURLRight( FILE *fp, short cols, long rgb, const char *url, const char *name  )
{
	writeTextRightWithRLBorder( name, fp );
}

void excelFile::WritePageTitle( VDinfoP VDptr, FILE *fh, const char *title )
{
//	columth=5;
//	rowth++;
//	writeTextCenter( title );
}


// **********************************************************************************
// Method:		excelFile::WriteFilterText
//
// Abstract:	This method breaks the string up at the newlines and adds them to 
//				the excel document.
//
// Declaration: void excelFile::WriteFilterText(FILE* fp, const char* szFilterText)
//
// Arguments:	
//
//
// Returns:		void
// **********************************************************************************
void excelFile::WriteFilterText(FILE* fp, const char* szFilterString)
{
	columth = 1;
	int	iStrLen;
	for (const char*	sz = szFilterString; iStrLen=mystrlen(sz); sz += (iStrLen+1))
	{
		writeToExcel(sz, fp);
		++rowth;
	}
	--rowth;	// Remove the excess newline.
}

void excelFile::WritePageFooter( VDinfoP VDptr, FILE *fp )
{
	writePageBreak( fp ); 		
}

void excelFile::WriteDemoBanner( VDinfoP VDptr, FILE *fp )
{
	if ( IsDemoReg() )
	{
		columth=5;
		rowth=rowth+2;
		writeTextCenter( "iReporter Demonstration", fp );
		rowth++;
		writeTextCenter( "Analysis Report", fp );
	}
}

void excelFile::SummaryTableEntryURL( FILE *fp, const char *text, const char *url, long normal )
{
	columth=0;
	rowth++;
	writeTextGeneral( text, fp ); 
}

void excelFile::SummaryTableEntry( FILE *fp, const char *text, const char *number, long rgb, long normal )
{
	columth=0;
	rowth++;
	writeTextGeneral( text, fp ); 
	if ( number ) 
	{
		columth=1;
		writeTextRight( number, fp );
	}
}

void excelFile::Stat_WriteText( FILE *fp, short cols, long rgb, const char *name )
{
	writeTextRightWithRLBorder( name, fp );
}

void excelFile::Stat_WriteRight( FILE *fp, short cols, long rgb, const char *name )
{
	writeTextRightWithRLBorder( name, fp );
	
}

void excelFile::Stat_WriteReqData( FILE *fp, long files, double perc )
{
	writeNumberRightWithRLBorder( files, fp );
	if ( perc >= 100.0)
		perc=100.0;
	writeNumberRightWithRLBorder( perc, fp );  //for chart display exclude percentage
}

void excelFile::Stat_WriteByteData( FILE *fp, __int64 bytes, double perc )
{
	writeNumberRightWithRLBorder( bytes, fp );
	if ( perc >= 100.0)
		perc=100.0;
	writeNumberRightWithRLBorder( perc, fp );  //for chart display
	
}

void excelFile::Stat_WriteHeader( FILE *fp, const char *txt, long space )  //eg time  rewuest % .... 12,bold
{  
	writeTextCenterWith4Border( txt, fp );
	columth++;
}

void excelFile::writeBOFExcel( FILE *fp )
{
	unsigned char BOF[]={0x09,00,0x04,00,0x02,00,0x10,00};  // BOF format

	fwrite( (const void *)BOF, sizeof(unsigned char), sizeof(BOF), fp );
	writeFooter( fp );

	unsigned char colWidth1[] = { 0x24, 00, 0x04, 00, 0x0, 0x02, 0xff, 22};
	fwrite( (const void *)colWidth1, sizeof(unsigned char), sizeof(colWidth1), fp ); // column width

	unsigned char colWidth[] = { 0x24, 00, 0x04, 00, 0x03, 0x0A, 0xff, 11 };
	fwrite( (const void *)colWidth, sizeof(unsigned char), sizeof(colWidth), fp ); // column width

	setUpFont( fp );
}

void excelFile::setUpFont( FILE *fp )
{
	unsigned char FONT10[] = { 0x31, 00, 0x14, 00, 0xc8, 0x00, 0x00, 0x00, 0x0F, 0x54, 0x69, 0x6D, 0x65, 0x73, 0x20,
		0x4E, 0x65, 0x77, 0x20, 0x52, 0x6f, 0x6d, 0x61, 0x6e }; //times new 10 normal
	fwrite( (const void *)FONT10, sizeof(unsigned char), sizeof(FONT10), fp );//00


	unsigned char FONT12[] = { 0x31, 00, 0x14, 00, 0xF0, 0x00, 0x01, 0x00, 0x0F, 0x54, 0x69, 0x6D, 0x65, 0x73, 0x20,
		0x4E, 0x65, 0x77, 0x20, 0x52, 0x6f, 0x6d, 0x61, 0x6e}; //times new 12 bold
	fwrite( (const void *)FONT12, sizeof(unsigned char), sizeof(FONT12), fp );//40

	unsigned char FONT14[] = { 0x31, 00, 0x14, 00, 0x18, 0x01, 0x01, 0x00, 0x0F, 0x54, 0x69, 0x6D, 0x65, 0x73, 0x20,
		0x4E, 0x65, 0x77, 0x20, 0x52, 0x6f, 0x6d, 0x61, 0x6e }; //times new 14 bold
	fwrite( (const void *)FONT14, sizeof(unsigned char), sizeof(FONT14), fp );//80

}

void excelFile::writeTextCenterWith4Border( const char *text, FILE *fp )
{
	RGBAttr[0]=00;
	RGBAttr[1]=0x40;
	RGBAttr[2]=0x7A;
	writeToExcel( text, fp );
}

void excelFile::writeTextGeneral( const char *text, FILE *fp )
{
	RGBAttr[0]=00;
	RGBAttr[1]=00;
	RGBAttr[2]=00;
	writeToExcel( text, fp );
}

void excelFile::writeTextRight( const char *text, FILE *fp )
{
	RGBAttr[0]=00;
	RGBAttr[1]=00;
	RGBAttr[2]=0x03;
	writeToExcel( text, fp );
}

void excelFile::writeTextRightWithRLBorder( const char *text, FILE *fp )
{
	RGBAttr[0]=00;
	RGBAttr[1]=00;
	RGBAttr[2]=0x1B;  
	writeToExcel( text, fp );
	columth++;  
}

void excelFile::writeTextGeneralWithRLBorder( const char *text, FILE *fp )
{
	RGBAttr[0]=00;
	RGBAttr[1]=00;
	RGBAttr[2]=0x19; //left R,L
	writeToExcel( text, fp );
}

void excelFile::writeTextCenter( const char *text, FILE *fp )
{
	RGBAttr[0]=00;
	RGBAttr[1]=0x40;//12 font bold
	RGBAttr[2]=0x02;
	writeToExcel( text, fp );
}

void excelFile::writeNumberRightWithRLBorder( double num, FILE *fp ) //write number
{
	RGBAttr[0]=00;
	RGBAttr[1]=0;
	RGBAttr[2]=0x1B;  

	short areaLastRow=areaFirstRow+rownum-1;
	unsigned char length=0x1B;

	formatToIEEE(num); // conver num to IEEE format
	
	if (cellType == SUM)
	{
		if (rownum)
		{
			unsigned char formula[]={0x06,00,length,00,(unsigned char)(rowth & 0x00FF), (unsigned char)((rowth & 0xFF00)>>8),
				(unsigned char)(columth & 0x00FF),(unsigned char)((columth & 0xFF00)>>8),RGBAttr[0],RGBAttr[1],RGBAttr[2]};
				unsigned char restFormular[]={0x01,0x0A,0x25,(unsigned char)(areaFirstRow & 0x00FF), (unsigned char)((areaFirstRow & 0xFF00)>>8),
				(unsigned char)(areaLastRow & 0x00FF), (unsigned char)((areaLastRow & 0xFF00)>>8), (unsigned char)(columth& 0x00FF),(unsigned char)(columth& 0x00FF), 0x19,0x10,00};

			fwrite((const void *) formula,sizeof(unsigned char),sizeof(formula), fp ); 
			for(int i=0; i<8;i++)
				fwrite((const void *) &IEEEData[7-i],sizeof(unsigned char),sizeof(unsigned char), fp );
			fwrite((const void *) restFormular,sizeof(unsigned char),sizeof(restFormular), fp );
		}
		else
			writeNoFormulaData( fp );  //no data items in th table, so need not to do formula 

	}
	else if (cellType == AVERAGE)
	{
		if (rownum)
		{
			unsigned char formula[]={0x06,00,length,00,(unsigned char)(rowth & 0x00FF), (unsigned char)((rowth & 0xFF00)>>8),
				(unsigned char)(columth & 0x00FF),(unsigned char)((columth & 0xFF00)>>8),RGBAttr[0],RGBAttr[1],RGBAttr[2]};
			unsigned char restFormular[]={0x01,0x0A,0x25,(unsigned char)(areaFirstRow & 0x00FF), (unsigned char)((areaFirstRow & 0xFF00)>>8),
				(unsigned char)(areaLastRow & 0x00FF), (unsigned char)((areaLastRow & 0xFF00)>>8), (unsigned char)(columth& 0x00FF),(unsigned char)(columth& 0x00FF),0x22,0x01,0x05};

			fwrite((const void *) formula,sizeof(unsigned char),sizeof(formula), fp ); 
		
			for(int i=0; i<8;i++)
				fwrite((const void *) &IEEEData[7-i],sizeof(unsigned char),sizeof(unsigned char), fp );
			fwrite((const void *) restFormular,sizeof(unsigned char),sizeof(restFormular), fp );
		}
		else
			writeNoFormulaData( fp );
	}
	else
		writeNoFormulaData( fp );
	columth++;  
}

void excelFile::writeNoFormulaData( FILE *fp )
{
	
	unsigned char numberFormat[]={0x03,00,0x0F,00,(unsigned char)(rowth & 0x00FF), (unsigned char)((rowth & 0xFF00)>>8),
		(unsigned char)(columth & 0x00FF),(unsigned char)((columth  & 0xFF00)>>8),RGBAttr[0],RGBAttr[1],RGBAttr[2]};  // number cell format
	fwrite((const void *) numberFormat,sizeof(unsigned char),sizeof(numberFormat), fp );
	for(int i=0; i<8;i++)
		fwrite((const void *) &IEEEData[7-i],sizeof(unsigned char),sizeof(unsigned char), fp );
}


void excelFile::writeToExcel( const char* text, FILE *fp ) //write string
{
	std::string transText;
	if (!text)
		transText = "";
	else
		transText = text;
	if ( MyPrefStruct.language )
		ConvertHTMLCharacterTokens( transText );

	unsigned char stringLength = transText.length();
	short bodyLength= stringLength +8;
	
	unsigned char LABEL[]={0x04,00, (unsigned char)(bodyLength & 0x00FF),(unsigned char)((bodyLength & 0xFF00)>>8),
		(unsigned char)(rowth & 0x00FF), (unsigned char)((rowth & 0xFF00)>>8),
		(unsigned char)(columth & 0x00FF),(unsigned char)((columth  & 0xFF00)>>8),
		RGBAttr[0],RGBAttr[1],RGBAttr[2],stringLength};  

	fwrite((const void *) LABEL,sizeof(unsigned char),sizeof(LABEL), fp );//format cell
	fwrite((const void *) transText.c_str(),sizeof(char),stringLength, fp );  //string
}

void excelFile::SummaryTableTitle( FILE *fp, const char *text, long rgb ) 
{
}

void excelFile::SummaryTableSubTitle( FILE *fp, const char *text, int x, int y )//eg session info, page info
{
	columth=0;
	rowth=rowth+3;

	RGBAttr[0]=00;
	RGBAttr[1]=0x40;
	RGBAttr[2]=0x20;

	writeToExcel( text, fp );
	++rowth;
}

void excelFile::writeFooter( FILE *fp )
{
	unsigned char footer[]={0x15,00,0x07,00,0x06,0x52,0x65,0x70,0x6F,0x72,0x74};  //footer 
	fwrite((const void *) footer,sizeof(unsigned char),sizeof(footer), fp );
}


void excelFile::writeTableBottomLine( short num, FILE *fp )
{
	unsigned char stringLength = 0;
	short bodyLength=stringLength+8;
	
	RGBAttr[0]=00;
	RGBAttr[1]=00;
	RGBAttr[2]=0x20;  //top line in cell

	for (short i=1; i<num-1;i++)
	{
		unsigned char LABEL[]={0x04,00,(unsigned char)(bodyLength & 0x00FF),(unsigned char)((bodyLength & 0xFF00)>>8),
			(unsigned char)(rowth & 0x00FF),(unsigned char)((rowth & 0xFF00)>>8),
			(unsigned char)(i & 0x00FF),(unsigned char)((i & 0xFF00)>>8),
			RGBAttr[0],RGBAttr[1],RGBAttr[2],stringLength};  

		fwrite( (const void *)LABEL, sizeof(unsigned char), sizeof(LABEL), fp );//format 
	}
	rowth=rowth+1;
}

void excelFile::writePageBreak( FILE *fp ) // to change to break page later
{
	rowth=rowth+1;
	unsigned char stringLength = 0;
	short bodyLength=stringLength+8;
	
	RGBAttr[0]=00;
	RGBAttr[1]=00;
	RGBAttr[2]=0x40;  //bottom line in cell
	
	for (short i=0; i<12;i++)
	{
		unsigned char LABEL[]={0x04,00,(unsigned char)(bodyLength & 0x00FF),(unsigned char)((bodyLength & 0xFF00)>>8),
			(unsigned char)(rowth & 0x00FF),(unsigned char)((rowth & 0xFF00)>>8),
			(unsigned char)(i & 0x00FF),(unsigned char)((i & 0xFF00)>>8),
			RGBAttr[0],RGBAttr[1],RGBAttr[2],stringLength};  
		fwrite( (const void *)LABEL, sizeof(unsigned char), sizeof(LABEL),fp );//format 
	}
	rowth=rowth+1;
}

void excelFile::writeMultiRow( long currentRow, const char * pathName, FILE *fp )
{
	char numStr[256];
	short tempColumth;

	FormatLongNum(currentRow,numStr);
	strcat(numStr,". ");
	strcat(numStr,pathName);

	writeTextGeneralWithRLBorder( numStr, fp );
	
	tempColumth = columth;
	columth=2;

	writeTextGeneralWithRLBorder( "", fp ); // only write R,L board 
	columth++;
	writeTextGeneralWithRLBorder( "", fp ); // only write R,L board 
	columth=tempColumth;
	
}

static const int bias=1024;
static const int minExp=-1022;  //exponent -1022~1023
static const int maxExp=1023;
static const short	expntBitPst=12; // significant bit beginning

void excelFile::formatToIEEE(double value)
{

	int i, mostSignPst;
	int count=0;
	double expntValue;

	// initialize array
	for(i=0; i<cnst;i++)
		binalValue[i] = 0;

	for(i=0; i<bit64; i++)
		result[i]=0;  

	for (i=0;i<8;i++)
		IEEEData[i]=0x00;

	if (value<0)
		result[0]=1;  //sign 

	convertIntDecPoint(value); // convert integer and decimal point
		
	//find most significatant bit of significand
	i=0;
	while((i<cnst) && (binalValue[i] !=1))
		i++;

	expntValue=bias-i; //(1024-i)

	if ((expntValue >= minExp) && (expntValue<=maxExp))
		mostSignPst=i+1;//after 1 position eg 1022+1  in 0  1(1022).0(1023)1

	else if (expntValue <minExp)
	{
		expntValue=minExp-1;//  -1023
		mostSignPst=bias-expntValue;

	}
		
	//	copy  (12~64 ) to result from mostSignPst beginning  (significant bits) 1.01 form 01 begining
	i=expntBitPst;  //12

	while((i<bit64) && (mostSignPst<cnst))
	{
		result[i]=binalValue[mostSignPst];
		mostSignPst++;
		i++;
	}
	//max expont
	if (expntValue>maxExp)
	{
		expntValue=maxExp+1;  //1023+1
		//zero the signifcand
		i=expntBitPst;  //12
		while(i<bit64)
		{
			result[i]=0;
			i++;
		}
	}

	convertExp(expntValue);	//convert exponent to binary
	convertToHex();//convert to hex

}

void excelFile::convertToHex()
{
	int count=0;
	for (int i=0; i<bit64;i=i+8)
	{
		unsigned char mask=0x80;
		for( int j=0; j<8; j++)
		{
			if (result[i+j])
				IEEEData[count]=IEEEData[count] | mask;
		
			mask=mask >>1;
		}
		count++;
	}
}

void excelFile::convertIntDecPoint(double value)
{
	int i;
	double intPart;
	double decPart;
	
	decPart= modf(value,&intPart);
	//convert integer part
	i=bias;
	while(((intPart/2) !=0) && (i>=0))
	{
		binalValue[i] = (unsigned char)intPart % 2;
		if((unsigned char)intPart % 2 == 0)
			intPart = intPart/2;
		else
			intPart = (intPart/2)-0.5;
			i--;
	}
	//convert decimal part
	i=bias+1;
	while((decPart>0) && (i<cnst))
	{
		decPart=decPart*2;
		if (decPart>=1)
		{
			binalValue[i]=1;
			decPart--;
		}
		else
			binalValue[i]=0;
		i++;
	}

}

void excelFile::convertExp(double expnt)
{
	int i;

	i=expntBitPst-1; //11

	expnt=expnt+maxExp;  //1023+2

	while((expnt/2) !=0)
	{
		result[i]=(unsigned char) expnt % 2;
		if (((unsigned char) expnt % 2) ==0)
			expnt=expnt/2;
		else
			expnt=expnt/2-0.5;
		i--;
	}
}

void excelFile::initAreaRow()
{
	areaFirstRow=rowth;  //formula first row
}

void excelFile::initColumn()
{
	columth=1;
}

void excelFile::FinishReport( VDinfoP VDptr, FILE *fp, const char *filename, long numberOfLogs )
{
	unsigned char EOFormat[]={0x0A,00,00,00};  // EOF format
	fwrite((const void *) EOFormat,sizeof(unsigned char),sizeof(EOFormat), fp );
	Fclose( fp );
}

int excelFile::ReportTurnedOn( long id )
{
	switch( id )
	{
		case HOURHIST_PAGE:
		case ERRORSHIST_PAGE:
		case ERRORURLHIST_PAGE:
		case PAGEHIST_PAGE:
		case PAGESFIRSTHIST_PAGE:
		case PAGESLASTHIST_PAGE:
		case TOPDIRSHIST_PAGE:
		case GROUPSHIST_PAGE:
		case DOWNLOADHIST_PAGE:
		case CLIENTHIST_PAGE:
		case USERHIST_PAGE:
		case REFERSITEHIST_PAGE:
		case ADVERTHIST_PAGE:
		case ADVERTCAMPHIST_PAGE:
			return 1;
		default:
			return baseFile::ReportTurnedOn( id );
	}
}

