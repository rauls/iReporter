
#ifndef PDFTABLEDATA_H
#define PDFTABLEDATA_H

#include <map>
#include "PDFBase.h"
#include "PDFFont.h"

#define PDF_LEFTJUSTIFY				0x0001
#define PDF_CENTERED				0x0002
#define PDF_LEFTCENTEREDJUSTIFY		PDF_LEFTJUSTIFY | PDF_CENTERED
#define PDF_RIGHTJUSTIFY			0x0004
#define PDF_RIGHTCENTEREDJUSTIFY	PDF_RIGHTJUSTIFY | PDF_CENTERED
#define PDF_HEADING_STYLE			0x0008
#define PDF_URI						0x0010
#define PDF_INTERNAL_LINK			0x0020

class PDFCellTextStr
{
public:
	PDFCellTextStr( std::string strP, double lenP, int justifyP )
	{
		biggestLineLen = lenP;
		justify = justifyP;
		haveReducedTextSize = 0;
		PDFMultiLineCellTextStr cellTextListItem( strP, biggestLineLen );
		cellStrList.push_back( cellTextListItem );
	}
	PDFCellTextStr( PDFMultiLineCellTextStrList cellStrListP, int justifyP )
	{
		biggestLineLen = 0;
		justify = justifyP;
		haveReducedTextSize = 0;
		cellStrList = cellStrListP;

		// Set the Length
		PDFMultiLineCellTextStrList::iterator iter;
		for ( iter = cellStrList.begin(); iter != cellStrList.end(); iter++ )
		{
			PDFMultiLineCellTextStr multiLineStr = (*iter);
			int aStrLen = multiLineStr.StrLen();
			if ( aStrLen > biggestLineLen )
				biggestLineLen = aStrLen;
		}

	}
	PDFCellTextStr& operator=( PDFCellTextStr orig )
	{
		biggestLineLen = orig.biggestLineLen;
		justify = orig.justify;
		cellStrList = orig.cellStrList;
		return *this;
	}
	void SetStr( std::string strP )
	{
		PDFMultiLineCellTextStr& tempCellText = cellStrList.front();
		tempCellText.SetStr( strP );
	}
	std::string Str() { return Str( 0 ); }
	double StrLen() { return biggestLineLen; }
	void SetStrLen( double lenP ) { biggestLineLen = lenP; }
	std::string Str( int pos /*= 0*/ )
	{
		PDFMultiLineCellTextStrList::iterator iter;
		int i = 0;
		for( iter = cellStrList.begin(); iter != cellStrList.end(); iter++, i++ )
		{
			if ( pos == i )
				return (*iter).Str();
		}
		return "";
	}
	double StrLen( int pos /*= 0*/ )
	{
		PDFMultiLineCellTextStrList::iterator iter;
		int i = 0;
		for( iter = cellStrList.begin(); iter != cellStrList.end(); iter++, i++ )
		{
			if ( pos == i )
				return (*iter).StrLen();
		}
		return 0;
	}
	int Justify() { return justify; }
	int Lines() { return cellStrList.size(); }
	void HaveReducedTextSize( short haveReducedTextSizeP ) { haveReducedTextSize = haveReducedTextSizeP; }
	short HaveReducedTextSize() { return haveReducedTextSize; }
	PDFMultiLineCellTextStrList *CellStrList() { return &cellStrList; }

private:
	double biggestLineLen;
	int justify;
	short haveReducedTextSize;
	PDFMultiLineCellTextStrList cellStrList;
};

class WidestStr
{
public:
	WidestStr( int widthP, std::string strP )
	{
		width = widthP;
		str = strP;
	}
	int width;
	std::string str;
};
typedef std::map< int, WidestStr > IndexedInts;
typedef std::map<int, PDFCellTextStr> PDFCellTextStrMap;
typedef std::pair< PDFCellTextStrMap::iterator, bool > PDFStrIntMapPair;

class PDFColStrs
{
public:
	int currCol;
	int fontSize;
	int fontStyle;
	int fontColor;
	PDFCellTextStr Error; 
private:
	PDFCellTextStrMap columnStrs;
public:
	PDFCellTextStrMap& ColumnStrs() { return columnStrs; }
public:
	PDFColStrs()
		: Error( "PDFError", 20, 0 )
	{
		Clear();
	}
	PDFColStrs( int fontSizeP, int fontStyleP, int fontColorP )
		: Error( "PDFError", 20, 0 )
	{
		fontSize = fontSizeP;
		fontStyle = fontStyleP;
		fontColor = fontColorP;
		Clear();
	}
	void SetFonts( int size, int style, int color )
	{
		fontSize = size;
		fontStyle = style;
		fontColor = color;
	}
	int Columns() { return ColumnStrs().size(); }
	PDFColStrs& operator=( PDFColStrs orig )
	{
		columnStrs	= orig.ColumnStrs();
		fontSize	= orig.fontSize;
		fontStyle	= orig.fontStyle;
		fontColor	= orig.fontColor;
		return *this;
	}
	PDFCellTextStr* CellText( int pos )
	{
		PDFCellTextStrMap::iterator iter;
		iter = ColumnStrs().find( pos );
		if ( iter != ColumnStrs().end() )
			return &(*iter).second;
		else
			return &Error;
	}
	std::string Str( int pos )
	{
		PDFCellTextStrMap::iterator iter;
		iter = ColumnStrs().find( pos );
		if ( iter != ColumnStrs().end() )
			return (*iter).second.Str();
		else
			return "";
	}
	double StrLen( int pos )
	{
		PDFCellTextStrMap::iterator iter;
		iter = ColumnStrs().find( pos );
		if ( iter != ColumnStrs().end() )
			return (*iter).second.StrLen();
		else
			return 0;
	}
	int Justify( int pos )
	{
		PDFCellTextStrMap::iterator iter;
		iter = ColumnStrs().find( pos );
		if ( iter != ColumnStrs().end() )
			return (*iter).second.Justify();
		else
			return 0;
	}
	void Clear()
	{
		ColumnStrs().clear();
		fontSize = 0;
		currCol = 0;
	}
	bool Insert( int pos, std::string str, double len, int justify )
	{
		PDFStrIntMapPair result = ColumnStrs().insert( PDFCellTextStrMap::value_type(pos, PDFCellTextStr( str, len, justify ) ) );
		return result.second;
	}

	bool Insert( int pos, PDFMultiLineCellTextStrList cellStrList, int justify )
	{
//Trev
		PDFStrIntMapPair result = ColumnStrs().insert( PDFCellTextStrMap::value_type(pos, PDFCellTextStr( cellStrList, justify ) ) );
		return result.second;
	}

	bool Amend( int pos, std::string str, double len )
	{
		PDFCellTextStrMap::iterator iter;
		iter = ColumnStrs().find( pos );
		if ( iter == ColumnStrs().end() )
			return false;
		(*iter).second.SetStr( str );
		(*iter).second.SetStrLen( len );
		return true;
	}
	int RowCellLines()
	{
		int lines = 0;
		PDFCellTextStrMap::iterator iter;
		for ( iter = ColumnStrs().begin(); iter != ColumnStrs().end(); iter++ )
		{
			PDFCellTextStr text = (*iter).second;
			if ( text.Lines() > lines )
				lines = text.Lines();
		}
		return lines;
	}
};

typedef std::map<int, PDFColStrs> PDFColStrsMap;
typedef std::pair< PDFColStrsMap::iterator, bool > PDFColStrsMapPair;


class ColWidth
{
public:
	ColWidth( int headP, int dataP )
	{
		data = dataP;
		AmendHead( headP );
	}
	void AmendHead( int headP )
	{
		head = headP;
		length = head;
		if ( data > length )
			length = data;
		diff = head - data;
	}
//private:
	int head;
	int data;
	int length;
	int diff;
};

typedef std::map< int, ColWidth > ColWidthMap;
typedef std::pair< ColWidthMap::iterator, bool > ColWidthMapPair;

class TableWidths
{
public:
	TableWidths()
	{
		Clear();
	}
	void Clear()
	{
		colMaxDataWidths.clear();
		colWidths.clear();
		headTotal = 0;
		dataTotal = 0;
		lengthTotal = 0;
		headDiffTotal = 0;
		biggestDataCol = -1;
		biggestHeadDiffCol = 0;
		biggestHeadDiffSize = 0;
		colHeadingsExtraWidth.clear();
		totalColHeadingsExtraWidth = 0;
		colHeadingsExtraWidthBiggestCol = -1;
		colHeadingsExtraWidthBiggestSize = 0;
	}
	bool Insert( int pos, int head, int data )
	{
		ColWidthMapPair result = colWidths.insert( ColWidthMap::value_type(pos, ColWidth( head, data ) ) );
		if ( result.second )
		{
			headTotal += head;
			dataTotal += data;
			if ( head > data )
			{
				int diff = head - data;
				if ( diff > biggestHeadDiffCol )
				{
					biggestHeadDiffCol = pos;
					biggestHeadDiffSize = diff;
				}
				lengthTotal += head;
				headDiffTotal += diff;
			}
			else
				lengthTotal += data;
		}
		return result.second;
	}
/*	bool Amend( int pos, int head, int data )
	{
		ColWidthMap::iterator iter;
		iter = colWidths.find( pos );
		if ( iter == colWidths.end() )
			return false;
		headTotal += head - (*iter).second.head;
		dataTotal += data - (*iter).second.data;
		if ( head > data )
			headDiffTotal += (head - data) - (*iter).second.diff;
		(*iter).second.head = head;
		(*iter).second.data = data;
		(*iter).second.diff = head - data;
		(*iter).second.head = head;
		return true;
	}*/
	bool AmendHead( int pos, int head )
	{
		ColWidthMap::iterator iter;
		iter = colWidths.find( pos );
		if ( iter == colWidths.end() )
			return false;
		headTotal += head - (*iter).second.head;
		if ( head > (*iter).second.data )
			headDiffTotal += ((head - (*iter).second.data) - (*iter).second.diff);
		(*iter).second.head = head;
		(*iter).second.diff = head - (*iter).second.data;
		return true;
	}
	IndexedInts colMaxDataWidths;
	ColWidthMap colWidths;
	int headTotal;
	int dataTotal;
	int lengthTotal;
	int headDiffTotal;
	int biggestDataCol;
	int biggestHeadDiffCol;
	int biggestHeadDiffSize;
	intList colHeadingsExtraWidth;
	int totalColHeadingsExtraWidth;
	int colHeadingsExtraWidthBiggestCol;
	int colHeadingsExtraWidthBiggestSize;
};

class PDFTableStrs
{
public:
	int currCol;
	int currRow;
	int fontSize;
	int fontStyle;
	int fontColor;
	int biggestColSize;
	//int totalWidth;
	PDFCellTextStr Error;
	int BiggestColNum() { return biggestColNum; }
	void BiggestColNum( int biggestColNumP ) { biggestColNum = biggestColNumP; }
private:
	int biggestColNum;
	PDFColStrsMap strs;
public:
	PDFColStrsMap& StringTable() { return strs; }
public:
	PDFTableStrs()
		: Error( "PDFError", 20, 0 )
	{
		Clear();
	}
	PDFTableStrs( int fontSizeP )
		: Error( "PDFError", 20, 0 )
	{
		fontSize = fontSizeP;
		currCol = 0;
		currRow = 0;
	}
	void FontSize( int fontSizeP ) { fontSize = fontSizeP; }

	void SetFonts( int size, int style, int color )
	{
		fontSize = size;
		fontStyle = style;
		fontColor = color;
	}
	void Clear()
	{
		PDFColStrsMap::iterator rowIter;
		for( rowIter = StringTable().begin(); rowIter != StringTable().end(); rowIter++ )
		{
			(*rowIter).second.ColumnStrs().clear();
		}
		StringTable().clear();
		//colMaxDataWidths.clear();
		biggestColNum = -1;
		biggestColSize = 0;
		//totalWidth = 0;
		currCol = 0;
		currRow = 0;
	}
	int Columns() 
	{
		if ( StringTable().size() )
		{
			PDFColStrsMap::iterator iter = StringTable().begin();
			return (*iter).second.ColumnStrs().size();
		}
		return 0;
	}
	int Rows() { return StringTable().size(); }
	//PDFCellTextStrMap
		PDFColStrs* Row( int rowPos )
	{
		PDFColStrsMap::iterator rowIter;
		rowIter = StringTable().find( rowPos );
		if ( rowIter == StringTable().end() )
			return NULL;//&((*rowIter).second);//.ColumnStrs());
		return &((*rowIter).second);//.ColumnStrs());
	}
	PDFCellTextStr* CellText( int colPos, int rowPos )
	{
		PDFColStrsMap::iterator rowIter;
		rowIter = StringTable().find( rowPos );
		if ( rowIter == StringTable().end() )
			return &Error;
		PDFCellTextStrMap::iterator colIter;
		colIter = (*rowIter).second.ColumnStrs().find( colPos );
		if ( colIter != (*rowIter).second.ColumnStrs().end() )
			return &(*colIter).second;
		else
			return &Error;
	}
	std::string Str( int colPos, int rowPos )
	{
		PDFColStrsMap::iterator rowIter;
		rowIter = StringTable().find( rowPos );
		if ( rowIter == StringTable().end() )
			return "";
		PDFCellTextStrMap::iterator colIter;
		colIter = (*rowIter).second.ColumnStrs().find( colPos );
		if ( colIter != (*rowIter).second.ColumnStrs().end() )
			return (*colIter).second.Str();
		else
			return "";
	}
	double StrLen( int colPos, int rowPos )
	{
		PDFColStrsMap::iterator rowIter;
		rowIter = StringTable().find( rowPos );
		if ( rowIter == StringTable().end() )
			return 0;
		PDFCellTextStrMap::iterator colIter;
		colIter = (*rowIter).second.ColumnStrs().find( colPos );
		if ( colIter != (*rowIter).second.ColumnStrs().end() )
			return (*colIter).second.StrLen();
		else
			return 0;
	}
	int Justify( int colPos, int rowPos )
	{
		PDFColStrsMap::iterator rowIter;
		rowIter = StringTable().find( rowPos );
		if ( rowIter == StringTable().end() )
			return 0;
		PDFCellTextStrMap::iterator colIter;
		colIter = (*rowIter).second.ColumnStrs().find( colPos );
		if ( colIter != (*rowIter).second.ColumnStrs().end() )
			return (*colIter).second.Justify();
		else
			return 0;
	}
	bool Insert( int colPos, int rowPos, std::string str, double len, int justify )
	{
		int retResult;
		PDFColStrsMap::iterator rowIter;
		rowIter = StringTable().find( rowPos );
		if ( rowIter == StringTable().end() )
		{
			PDFColStrs temp( fontSize, fontStyle, fontColor );
			if ( !temp.Insert( colPos, str, len, justify ) )
				return false;
			PDFColStrsMapPair result = StringTable().insert( PDFColStrsMap::value_type(rowPos, temp ) );
				retResult = result.second;
		}
		else
		{
			PDFStrIntMapPair result = (*rowIter).second.ColumnStrs().insert( PDFCellTextStrMap::value_type(colPos, PDFCellTextStr( str, len, justify ) ) );
			retResult = result.second;
		}
		return retResult!=0;
		
	}
	bool Insert( int colPos, int rowPos, PDFMultiLineCellTextStrList cellStrList, int justify )
	{
// Trev
		int retResult;
		PDFColStrsMap::iterator rowIter;
		rowIter = StringTable().find( rowPos );
		if ( rowIter == StringTable().end() )
		{
			PDFColStrs temp( fontSize, fontStyle, fontColor );
			if ( !temp.Insert( colPos, cellStrList, justify ) )
				return false;
			PDFColStrsMapPair result = StringTable().insert( PDFColStrsMap::value_type(rowPos, temp ) );
				retResult = result.second;
		}
		else
		{
			PDFStrIntMapPair result = (*rowIter).second.ColumnStrs().insert( PDFCellTextStrMap::value_type(colPos, PDFCellTextStr( cellStrList, justify ) ) );
			retResult = result.second;
		}
		return retResult!=0;
	}
	bool Amend( int colPos, int rowPos, std::string str, double len )
	{
		PDFColStrsMap::iterator rowIter;
		rowIter = StringTable().find( rowPos );
		if ( rowIter == StringTable().end() )
			return false;

		PDFColStrs temp = (*rowIter).second;
		PDFCellTextStrMap temp2 = temp.ColumnStrs();
		PDFCellTextStrMap::iterator colIter;
		colIter = temp2.find( colPos );
		if ( colIter == temp2.end() )
			return false;
		(*colIter).second.SetStr( str );
		(*colIter).second.SetStrLen( len );
		return true;
	}
	int RowCellLines( int row )
	{
		PDFColStrs* rowMap = Row( row );
		if ( rowMap == NULL )
			return 0;
// Trev do test for rowMap 
		return rowMap->RowCellLines();
/*		int lines = 0;
		int colPos = 0;
		PDFCellTextStr* rowData = CellText( colPos, row );
		while( rowData != &Error )
		{
			if ( rowData->Lines() > lines )
				lines = rowData->Lines();
			colPos++;
			rowData = CellText( colPos, row );
		}*/
/*		PDFColStrsMap::iterator temp = theTableData->data.strs.begin();
		for ( ; temp != theTableData->data.strs.end(); temp++ )
		{
			PDFColStrs temp2 =  (*temp).second;
			int calRowHeight = temp2.RowCellHeight();
			totalCalRowsHeight += calRowHeight;
		}*/
		//return lines;
	}
	int TableLines()
	{
		int lines = 0;
		for( int i = 0; i < Rows(); i++ )
			lines += RowCellLines( i );
		return lines;
	}
};

class TableColWidth
{
public:
	TableColWidth( int colWidthP, int maxDataWidthP )
		: colWidth( colWidthP ), maxDataWidth( maxDataWidthP ){}
	float AjustmentOffset() // This function is used to center text in a column
	{
		if ( colWidth < maxDataWidth )
			return 0;
		return ((float)(colWidth - maxDataWidth)) / 2;
	}
	void ColWidth( int colWidthP )
	{
		colWidth = colWidthP;
		if ( colWidth < 0 )
			colWidth = 20;
	}
	int ColWidth() { return colWidth; }
	void MaxDataWidth( int maxDataWidthP ) { maxDataWidth = maxDataWidthP; }
	int MaxDataWidth() { return maxDataWidth; }
private:
	int colWidth;
	int maxDataWidth;
};

typedef std::list<TableColWidth> TableColWidths;
typedef TableColWidths::iterator TableColWidthIter;

class PDFTableData
{
public:
	PDFTableData( int minLenP, float reduceDataByP, int textWrapP, int textCellXIndentP )
	{
		minLen = minLenP;
		reduceDataBy = reduceDataByP;
		textWrap = textWrapP;
		textCellXIndent = textCellXIndentP;
	}
	PDFCellTextStr* GetNextCellDataText()
	{
		// Get the cell text using the column and row positions
		cellText = data.CellText( data.currCol, data.currRow );

		// Update the table row and column numbers
		int cols = data.Columns();
		data.currCol++;
		if ( data.currCol == cols )
		{
			data.currCol = 0;
			data.currRow++;
		}
		int rows = data.Rows();
		if ( data.currRow == rows )
		{
			data.currRow = 0;
		}
		return cellText;
	}
	PDFCellTextStr* GetNextCellHeadingText()
	{
		cellText = heads.CellText( heads.currCol );
		heads.currCol++;
		if ( heads.currCol == heads.Columns() )
			heads.currCol = 0;
		return cellText;
	}
	void ResetCellHeadingText() { heads.currCol = 0; }
	void Clear()
	{
		linkTransTitle = "";
		heads.Clear();
		data.Clear();
		columnWidths.Clear();
		cellColorList.clear();
		urlCellBkgrndColorList.clear();
		urlCellBkgrndColorCol = -1;
		oldcolWidths.clear();
		reduceStrCol = -1;
	}

	bool InsertData( int colPos, int rowPos, std::string str, double len, int justify )
	{
		if (!data.Insert( colPos, rowPos, str, len, justify ))
			return false;

		IndexedInts::iterator iter = columnWidths.colMaxDataWidths.find( colPos );
		if( iter != columnWidths.colMaxDataWidths.end() )
		{
			// Check if the current string data is wider than the current column width
			int curLen = (*iter).second.width;
			//std::string curStr = (*iter).second.str;
			if ( len > curLen )
			{
				// Data was wider, so remove old and replace with the current
				columnWidths.colMaxDataWidths.erase( iter );
				columnWidths.colMaxDataWidths.insert( IndexedInts::value_type( colPos, WidestStr( len, str ) ) ); // Put in iter's position in the map
			}
		}
		else
			columnWidths.colMaxDataWidths.insert( IndexedInts::value_type( colPos, WidestStr( len, str ) ) ); // Put in last position in the map*/
		return true;
	}

	bool InsertData( int colPos, int rowPos, PDFMultiLineCellTextStrList cellStrList, int justify )
	{
		if (!data.Insert( colPos, rowPos, cellStrList, justify ))
			return false;

// Trev
		int len = data.StrLen( colPos, rowPos );
		IndexedInts::iterator iter = columnWidths.colMaxDataWidths.find( colPos );
		if( iter != columnWidths.colMaxDataWidths.end() )
		{
			// Check if the current string data is wider than the current column width
			int curLen = (*iter).second.width; 
			if ( len > curLen )
			{
				// Data was wider, so remove old and replace with the current
				columnWidths.colMaxDataWidths.erase( iter );
				columnWidths.colMaxDataWidths.insert( IndexedInts::value_type( colPos, WidestStr( len, "Multiline String" ) ) ); // Put in iter's position in the map
			}
		}
		else
			columnWidths.colMaxDataWidths.insert( IndexedInts::value_type( colPos, WidestStr( len, "Multiline String" ) ) ); // Put in last position in the map
		return true;
	}

	int SeperateHeadingOverMultipleLinesAtSpaceChars( int colPos, int &colWidth, int &diff );
	bool SeperateDataOverMultipleLines( int &diff );

	void SetFonts( int titleSize, int titleStyle, int titleColor,
		int headSize, int headStyle, int headColor, int dataSize, int dataStyle, int dataColor )
	{
		titleFontSize = titleSize;
		titleFontStyle = titleStyle;
		titleFontColor = titleColor;
		heads.SetFonts( headSize, headStyle, headColor );
		data.SetFonts( dataSize, dataStyle, dataColor );
	}
	void SetPDFFont( PDFCharScaleFont *fontP )
	{
		font = fontP;
	}

	void WrapColumnHeadings( int& diff );
	void CalculateColumnWidths( void *font, int tableDrawWidth );
	void SetColumnsLengths( void *font );
	bool ReduceHeadingLength( void *font, int headPos, int& origLen, int& updatedDiff, int currTableWidthDiff );
	void FindBiggestDataColumn();
	void TableTooWide( void *font, int& diff );
	void TableTooNarrow( int& diff );
	void ReduceLargeDataColumnByPercent( int &diff );
	void ReduceHeadingLengths( void *font, int &diff );
	void ReduceLargeDataColumnByDiff( int &diff );

	int FirstColumnWidth() { return oldcolWidths.front().ColWidth(); }
	TableColWidthIter FirstColumnPtr() { return oldcolWidths.begin(); }
	TableColWidthIter BeyondLastColumnPtr() { return oldcolWidths.end(); }
	void NextColumnPtr( TableColWidthIter &iter ) { iter++; }
	int Columns() { return oldcolWidths.size(); }
	void AddColumn( int width, int maxData )
	{
		TableColWidth tableColWidth( width, maxData );
		oldcolWidths.push_back( tableColWidth );
	}
	int ColumnsWidth()
	{
		int tableWidth = 0;
		for( TableColWidthIter iter = FirstColumnPtr(); iter != BeyondLastColumnPtr(); iter++ )
		{
			tableWidth += (*iter).ColWidth();
		}
		return tableWidth;
	}


public:
	std::string linkTransTitle;
	int titleFontSize;
	int titleFontStyle;
	int titleFontColor;
	PDFColStrs heads;
	PDFTableStrs data;
	TableWidths columnWidths;
	intList cellColorList;
	intList urlCellBkgrndColorList;
	int urlCellBkgrndColorCol;
	PDFCellTextStr* cellText;
	int reduceStrCol;
	int minLen;
	float reduceDataBy;
	int textWrap;
	PDFCharScaleFont *font;
	int textCellXIndent;
protected:
	TableColWidths oldcolWidths;
};




#endif PDFTABLEDATA_H
