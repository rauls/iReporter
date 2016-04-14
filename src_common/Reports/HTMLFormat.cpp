
#include "FWA.h"

#include "HTMLFormat.h"

static char *HTMLFormat[] = { 
".html", // extension for file format
"<!-- Virtual List HEAD --><html><head><title>iReporter Virtual Domain report</title></head>\n\
<frameset cols=\"220,*\" frameborder=\"1\" framespacing=\"3\" border=\"3\">\n\
<frame src=\"frm_%s\" name=\"list\" marginwidth=\"0\" marginheight=\"0\" frameborder=\"0\" alt=\"Analysis Software (C) 1997-2002\">\n\
<frameset rows=\"*\">\n", // Virtual List head	(linkname)
"<!-- Virtual List TAIL -->    </frameset>\n\
</frameset>\n\
</html>\n", // Virtual List tail
"<!-- Virtual Host head --><html><head><title>Domains</title></head>\n\
<body bgcolor=\"#ffffff\">\n\
<a href=\"VHosts%s\" target=\"report\">\n<img src=\"VirtualBanner.gif\" border=\"0\"></a><hr>\n", // Virtual Host head	(linkname, imagename )
"<!-- Virtual Host tail --></table></center><br>\n", // Virtual Host tail
"<frame src=\"file:///%s/%s\" name=\"report\" resize>\n", // Virtual Frame Link1
"<frame src=\"%s/%s\" name=\"report\" resize>\n", // Virtual Frame Link2
"<frame src=\"%s\" name=\"report\" resize>\n", // Virtual Frame Link3
" %d. <a href=\"file:///%s/%s\" target=\"report\">%s</a><br>\n", // Virtual Link1
" %d. <a href=\"%s/%s\" target=\"report\">%s</a><br>\n", // Virtual Link2
"<head><meta http-equiv=\"refresh\" content=\"%ld; url=%s\"></head>", // refresh auto load file
"" // Empty string to denote last entry
};

char* GetHTMLFormat( long strId )
{
	return HTMLFormat[strId];
}

