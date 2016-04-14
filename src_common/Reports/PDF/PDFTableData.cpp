
#include "FWA.h"

#include "PDFTableData.h"

void PDFTableData::WrapColumnHeadings( int& diff )
{
	// If we are wrapping text then try to wrap the headings too...
	if ( textWrap && diff > 0 )
	{
		int j = 0;
		for( TableColWidthIter aIter = FirstColumnPtr(); aIter != BeyondLastColumnPtr(); NextColumnPtr( aIter ), j++ )
		{
			int thisColWidth = (*aIter).ColWidth();
			int changeHeadWidthTo = SeperateHeadingOverMultipleLinesAtSpaceChars( j, thisColWidth, diff );
			(*aIter).ColWidth( thisColWidth );
	/*		if ( j == columnWidths.colHeadingsExtraWidthBiggestCol )
			{
				if ( diff <= columnWidths.colHeadingsExtraWidthBiggestSize )
					ReduceHeadingLength( timesNRFont, j, (*aIter), diff, diff );
				else
					ReduceHeadingLength( timesNRFont, j, (*aIter), diff, columnWidths.colHeadingsExtraWidthBiggestSize );
				break;
			}*/
		}
	}
}

void PDFTableData::CalculateColumnWidths( void *font, int tableDrawWidth )
{
	PDFCharScaleFont *timesNRFont = (PDFCharScaleFont *)font;

	// Check the column heading lengths and the data
	// in the column to determine how wide the column should be
	SetColumnsLengths( timesNRFont );

	// Find out what the difference between the "draw" column widths and the "page draw" width
	int diff = ColumnsWidth()/*data.totalWidth*/ - tableDrawWidth;

	WrapColumnHeadings( diff );

	if ( diff > 0 ) // Table is too wide
	{
		FindBiggestDataColumn();
		if ( !textWrap )
			TableTooWide( timesNRFont, diff );
		else
			SeperateDataOverMultipleLines( diff );
	}
	else if ( diff < 0 ) // Table is too narrow
		TableTooNarrow( diff );

	ResetCellHeadingText();

	diff = ColumnsWidth() - tableDrawWidth;
}

void PDFTableData::SetColumnsLengths( void *font )
{
	PDFCharScaleFont *timesNRFont = (PDFCharScaleFont *)font;
	std::string cellText;
	//data.totalWidth = 0;
	for( int i = 0; i != data.Columns(); i++ )
	{
		int headingLenSpace = 0;
		int headingLenSpaceTemp = 0;
		PDFCellTextStr *textStr = GetNextCellHeadingText();

		/*if ( textStr )
		{
			if ( textStr->Lines() > 1 )
			{
				int i = 0;
				while ( i < textStr->Lines() )
				{
					cellText = textStr->Str( i );
					headingLenSpaceTemp = timesNRFont->GetStringLenWithSpacing( textStr->Str(), heads.fontSize, heads.fontStyle );
					if ( headingLenSpaceTemp > headingLenSpace )
						headingLenSpace = headingLenSpaceTemp;
					i++;
				}
			}
			else*/
				headingLenSpace = timesNRFont->GetStringLenWithSpacing( textStr->Str(), heads.fontSize, heads.fontStyle );
		//}

		// Get the current column data length
		IndexedInts::iterator iter = columnWidths.colMaxDataWidths.find(i);
		int curLen = 0;
		std::string debugTemp;
		if( iter != columnWidths.colMaxDataWidths.end() )
		{
			curLen = (*iter).second.width;
			debugTemp = (*iter).second.str;
			columnWidths.Insert( i, headingLenSpace, curLen );
			if ( curLen > headingLenSpace )
			{
				headingLenSpace = curLen;
				columnWidths.colHeadingsExtraWidth.push_back( 0 );
			}
			else
			{
				int diffLen = headingLenSpace - curLen;
				columnWidths.colHeadingsExtraWidth.push_back( diffLen );
				columnWidths.totalColHeadingsExtraWidth += diffLen;
				if ( diffLen > columnWidths.colHeadingsExtraWidthBiggestSize )
				{
					columnWidths.colHeadingsExtraWidthBiggestSize = diffLen;
					columnWidths.colHeadingsExtraWidthBiggestCol = i;
				}
			}
		}

		AddColumn( headingLenSpace, curLen );
		//data.totalWidth += headingLenSpace;
	}
}

bool PDFTableData::ReduceHeadingLength( void *font, int headPos, int& origLen, int& updatedDiff, int shortenHeadByP )
{
	// Set the font
	PDFCharScaleFont *timesNRFont = (PDFCharScaleFont *)font;

	int shortenHeadBy = shortenHeadByP;

	// Get the 
	ColWidthMap::iterator iter = columnWidths.colWidths.find( headPos );
	if ( iter == columnWidths.colWidths.end() )
		return 0;

	// Work out if we 
	ColWidth * tempColWidth = &(*iter).second;
	if ( shortenHeadBy > tempColWidth->diff )
		shortenHeadBy = tempColWidth->diff;

	int newLen = heads.StrLen( headPos ) - shortenHeadBy;
	std::string currHead = heads.Str( headPos );
	std::string newHead = timesNRFont->SetStringLen( currHead, newLen, heads.fontSize, PDF_RIGHT_NOELLIPSE, heads.fontStyle );
	int diffNewAndOrig = origLen - newLen;

	// Make sure the heading is "minimumTableColumnHeadingChars" characters or more in length or at
	// least the size of the original heading if it was less than "minimumTableColumnHeadingChars" chars
	if ( currHead.length() >= minLen && newHead.length() < minLen || currHead.length() < minLen && newHead.length() != currHead.length() )
	{
		if ( currHead.length() > minLen )
			currHead.resize( minLen );
		newHead = currHead;
		newLen = timesNRFont->GetStringLenWithSpacing( newHead, heads.fontSize, heads.fontStyle );
		diffNewAndOrig = origLen - newLen;
	}
	if ( heads.Amend( headPos, newHead, newLen ) )
	{
		if ( diffNewAndOrig < 0)
			int error = 0;
		origLen -= diffNewAndOrig;
		updatedDiff -= diffNewAndOrig;
		tempColWidth->AmendHead( newLen );
		return true;
	}
	return false;
}


int PDFTableData::SeperateHeadingOverMultipleLinesAtSpaceChars( int colPos, int &colWidth, int &diff )
{
	int i;
	i = 0;
	//float length = lengthP - 2*textCellXIndent;

	//int rows = data.Rows();
	//int len;
	//std::string str;

	PDFCellTextStr* text;
	text = heads.CellText( colPos );
	std::string splitStrOrig = text->Str();
	int spacePos = splitStrOrig.find( ' ' );
	if ( spacePos != std::string::npos && spacePos != splitStrOrig.length()-1 )
	{
		text->CellStrList()->clear();
		int newHeadLen = 0;
		int newHeadLenMax = 0;
		std::string splitStr;
		int done = 0;
		int setDoneNextTime = 0;
		while( !done )
		{
			splitStr = splitStrOrig.substr( 0, spacePos );
			splitStrOrig.erase( 0, spacePos+1 );
			int pos = splitStrOrig.find_first_not_of( ' ' );
			while( pos != 0 && pos != std::string::npos ) // Eat up spaces
			{
				splitStrOrig.erase( 0, 1 );
				pos = splitStrOrig.find_first_not_of( ' ' );
			}
			newHeadLen = font->GetStringLenWithSpacing( splitStr, heads.fontSize, heads.fontStyle );
			if ( newHeadLen > newHeadLenMax )
				newHeadLenMax = newHeadLen;
			PDFMultiLineCellTextStr temp( splitStr, newHeadLen/*font->GetStringLen( splitStr, heads.fontSize, heads.fontStyle )*/ );
			text->CellStrList()->insert( text->CellStrList()->end(), temp );
			spacePos = splitStrOrig.find( ' ' );
			if ( setDoneNextTime )
				done = 1;

			setDoneNextTime = 1;
		}

		if ( diff - colWidth + newHeadLenMax < 0 )
			newHeadLenMax = colWidth - diff; 

		diff -= colWidth;
		diff += newHeadLenMax;
		columnWidths.AmendHead( colPos, newHeadLenMax );
		colWidth = newHeadLenMax;

		return newHeadLenMax;
	}
	return 0;

/*	while ( i < rows )
	{
		PDFCellTextStr* text;
		text = head.CellText( colPos );
		PDFMultiLineCellTextStrList::iterator iter;
		for( iter = text->CellStrList()->begin(); iter != text->CellStrList()->end(); iter++ )
		{
			//len = (*iter).StrLen();
			str = (*iter).Str();

			//len = font->GetStringLen( str, data.fontSize, data.fontStyle );
			if ( len > length )
			{
				std::string splitStrOrig = (*iter).Str();
				std::string splitStr;
				int charLen;
				while ( len > length )
				{
					splitStr = splitStrOrig;
					splitStr = font->SetStringLen( splitStr, length, data.fontSize, PDF_RIGHT, data.fontStyle );
					charLen = splitStr.length();
					splitStrOrig.erase( 0, charLen );
					len = font->GetStringLen( splitStr, data.fontSize, data.fontStyle );
					PDFMultiLineCellTextStr temp( splitStr, len );
					text->CellStrList()->insert( iter, temp );
					len = font->GetStringLen( splitStrOrig, data.fontSize, data.fontStyle );
				}
				(*iter).SetStr( splitStrOrig );
				(*iter).SetStrLen( len );
			}
		}
		i++;
	}*/
}

void PDFTableData::FindBiggestDataColumn()
{

	data.BiggestColNum( -1 );
	data.biggestColSize = 0;

	// Find the biggest data column
	TableColWidthIter colWidthsIter;
	int i = 0;
	for( colWidthsIter = FirstColumnPtr(); colWidthsIter != BeyondLastColumnPtr(); NextColumnPtr( colWidthsIter ) )
	{
		int width = (*colWidthsIter).ColWidth();
		if ( width > data.biggestColSize )
		{
			data.BiggestColNum( i );
			data.biggestColSize = width;
		}
		i++;
	}
}

void PDFTableData::TableTooWide( void *font, int& diff )
{
	PDFCharScaleFont *timesNRFont = (PDFCharScaleFont *)font;

	ReduceLargeDataColumnByPercent( diff );

	if ( diff )
		ReduceHeadingLengths( timesNRFont, diff );

	if ( diff )
		ReduceLargeDataColumnByDiff( diff );
}

void PDFTableData::ReduceLargeDataColumnByPercent( int &diff )
{
	if ( textWrap )
		return;

	if ( reduceDataBy == (float)0 )
		return;

	int i = 0;
	TableColWidthIter colWidthsIter;
	for( colWidthsIter = FirstColumnPtr(); colWidthsIter != BeyondLastColumnPtr(); NextColumnPtr( colWidthsIter ), i++ )
	{
		// Try to reduce the size of the biggest column first, 
		if ( i == data.BiggestColNum() )
		{
			int currColWidth = (*colWidthsIter).ColWidth();
			if ( diff/(float)currColWidth < reduceDataBy ) // Got to leave a percentage of the column here... say "reduceDataBy" min
			{
				(*colWidthsIter).ColWidth( (*colWidthsIter).ColWidth() - diff );
				diff = 0;
				reduceStrCol = i;
				return;
			}
			else // Just reduce the column by reduceDataBy
			{
				diff -= (*colWidthsIter).ColWidth() * reduceDataBy;
				(*colWidthsIter).ColWidth( (*colWidthsIter).ColWidth() - (*colWidthsIter).ColWidth() * reduceDataBy );
				//(*colWidthsIter).ColWidth() -= (*colWidthsIter).ColWidth() * reduceDataBy;
				data.biggestColSize = (*colWidthsIter).ColWidth();
				reduceStrCol = i;
			}
		}
	}
}

void PDFTableData::ReduceHeadingLengths( void *font, int &diff )
{
	if ( textWrap )
		return;

	PDFCharScaleFont *timesNRFont = (PDFCharScaleFont *)font;

	// OK, now that we haven't reduced the column widths yet (in particular the largest data column),
	// lets try reducing the headings' lengths... but there must remain a minimum number of characters in the heading...
	int j = 0;
	TableColWidthIter colWidthsIter;
	for( colWidthsIter = FirstColumnPtr(); colWidthsIter != BeyondLastColumnPtr(); NextColumnPtr( colWidthsIter ), j++ )
	{
		if ( j == columnWidths.colHeadingsExtraWidthBiggestCol )
		{
			int thisColWidth = (*colWidthsIter).ColWidth();
			if ( diff <= columnWidths.colHeadingsExtraWidthBiggestSize )
				ReduceHeadingLength( timesNRFont, j, thisColWidth, diff, diff );
			else
				ReduceHeadingLength( timesNRFont, j, thisColWidth, diff, columnWidths.colHeadingsExtraWidthBiggestSize );
			(*colWidthsIter).ColWidth( thisColWidth );
			break;
		}
	}
	int sizeLeft = diff;
	j = 0;
	for( intList::iterator bigHeadIter = columnWidths.colHeadingsExtraWidth.begin();
		bigHeadIter != columnWidths.colHeadingsExtraWidth.end() && sizeLeft > 0; bigHeadIter++, j++ )
	{
		int& bigHeadCol = (*bigHeadIter);
		if ( bigHeadCol != 0 && bigHeadCol != columnWidths.colHeadingsExtraWidthBiggestCol )
		{
			int k = 0;
			for( colWidthsIter = FirstColumnPtr(); colWidthsIter != BeyondLastColumnPtr(); NextColumnPtr( colWidthsIter ), k++ )
			{
				if ( k == j )
				{
					int thisColWidth = (*colWidthsIter).ColWidth();
					if ( sizeLeft <= bigHeadCol )
						ReduceHeadingLength( timesNRFont, j, thisColWidth, sizeLeft, sizeLeft );
					else
						ReduceHeadingLength( timesNRFont, j, thisColWidth, sizeLeft, bigHeadCol );
					(*colWidthsIter).ColWidth( thisColWidth );
					break;
				}
			}
		}
	}
	diff = sizeLeft;
}

void PDFTableData::ReduceLargeDataColumnByDiff( int &diff )
{
	// If we have reduced the main data column by "reduceDataBy" and all the other columns to the width of there headings... and the
	// table is still too big... then reduce the main data column again to less than the "reduceDataBy" percentage than before...
	int i = 0;
	if ( diff > 0 )
	{
		TableColWidthIter colWidthsIter;
		for( colWidthsIter = FirstColumnPtr(), i=0; colWidthsIter != BeyondLastColumnPtr() && diff > 0; NextColumnPtr( colWidthsIter ), i++ )
		{
			if ( i == data.BiggestColNum() )
			{
				int currColWidth = (*colWidthsIter).ColWidth();
				(*colWidthsIter).ColWidth( (*colWidthsIter).ColWidth() - diff );
				data.biggestColSize = (*colWidthsIter).ColWidth();
				diff = 0;
				reduceStrCol = i;
				return;
			}
		}
	}
}

void PDFTableData::TableTooNarrow( int& diff )
{
	int i = 0;
	int columnsTooSmallSize = -diff;

	int notNumberRowSize = 40;
	int adjustColumns;
	int firstRowSize = FirstColumnWidth();
	if ( firstRowSize > notNumberRowSize )
		adjustColumns = Columns();
	else
		adjustColumns = Columns()-1;
	if (adjustColumns == 0)
		adjustColumns = 1;
	int adjustSize = columnsTooSmallSize / adjustColumns;
	int leftOver = columnsTooSmallSize % adjustColumns; 

	for( TableColWidthIter colWidthsIter = FirstColumnPtr(); colWidthsIter != BeyondLastColumnPtr(); NextColumnPtr( colWidthsIter ) )
	{
		// We don't make the first column which is just a "row number" any bigger,
		// but if it isn't just a row number, lets say it is bigger than 40 pixels wide, then make it larger too
		if ( i > 0 || (i == 0 && (*colWidthsIter).ColWidth() > notNumberRowSize) ) 
		{
			(*colWidthsIter).ColWidth( (*colWidthsIter).ColWidth() + adjustSize );
			if ( i == columnWidths.biggestHeadDiffCol )
				(*colWidthsIter).ColWidth( (*colWidthsIter).ColWidth() + leftOver );
		}
		i++;
	}
}

bool PDFTableData::SeperateDataOverMultipleLines( int &diff )
{
	//int origDiff = diff;
	//int colPos = data.biggestColNum; 

	int len;
	std::string str;

	ReduceLargeDataColumnByDiff( diff );
	float length = data.biggestColSize - 2*textCellXIndent - 1;

	int i = 0;
	int rows = data.Rows();
	while ( i < rows )
	{
		PDFCellTextStr* text;
		text = data.CellText( data.BiggestColNum(), i );
		PDFMultiLineCellTextStrList::iterator iter;
		for( iter = text->CellStrList()->begin(); iter != text->CellStrList()->end(); iter++ )
		{
			//len = (*iter).StrLen();
			str = (*iter).Str();
			len = font->GetStringLen( str, data.fontSize, data.fontStyle );
			if ( len > length )
			{
				std::string splitStrOrig = (*iter).Str();
				std::string splitStr;
				int charLen;
				while ( len > length )
				{
					splitStr = splitStrOrig;
					splitStr = font->SetStringLen( splitStr, length, data.fontSize, PDF_RIGHT|PDF_BREAK_AT_SPACE, data.fontStyle );
					charLen = splitStr.length();
					if ( splitStrOrig[charLen] == ' ' )
						splitStrOrig.erase( 0, charLen+1 );
					else
						splitStrOrig.erase( 0, charLen );
					len = font->GetStringLen( splitStr, data.fontSize, data.fontStyle );
					PDFMultiLineCellTextStr temp( splitStr, len );
					text->CellStrList()->insert( iter, temp );
					len = font->GetStringLen( splitStrOrig, data.fontSize, data.fontStyle );
				}
				(*iter).SetStr( splitStrOrig );
				(*iter).SetStrLen( len );
			}
		}
		i++;
	}
	return true;
}

